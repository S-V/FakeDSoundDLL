#include "stdafx.h"
#include "DSound_Proxy.h"
#include "MyDirectSound.h"

// headers needed for working with minidumps

// "Dbghelp.lib"
#pragma pack( push, 8 )
	#include <DbgHelp.h>
#pragma pack( pop )


// Global variables
#pragma data_seg (".dsound_shared")
FILE *			g_LogFile;
HINSTANCE		g_hModule;
HINSTANCE		g_hOriginalDLL;
MyDirectSound *	g_pMyDirectSound;
#pragma data_seg ()

static LONG WINAPI ExceptionCallback( EXCEPTION_POINTERS* exceptionInfo );

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	// to avoid compiler level 4 warnings 
    UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
	{
	    case DLL_PROCESS_ATTACH: InitInstance(hModule); break;
	    case DLL_PROCESS_DETACH: ExitInstance(); break;

        case DLL_THREAD_ATTACH:  break;
	    case DLL_THREAD_DETACH:  break;
	}
    return TRUE;
}

/// Exported function
HRESULT WINAPI DirectSoundCreate(
								 __in_opt LPCGUID pcGuidDevice,
								 __deref_out LPDIRECTSOUND *ppDS,
								 __null LPUNKNOWN pUnkOuter
								 )
{
	M_TRACE_FUNCTION;

	HINSTANCE hDSOUND = LazyLoadOriginalDLL();
	V_RET_IF_NIL(hDSOUND, ERR_FAILED_TO_LOAD_DLL);

	F_DirectSoundCreate* pDirectSoundCreate = (F_DirectSoundCreate*)
		::GetProcAddress( hDSOUND, "DirectSoundCreate" );

	V_RET_IF_NIL(pDirectSoundCreate, E_NOINTERFACE);

	// Controlling unknown of the aggregate. Its value must be NULL.
	M_ASSERT(pUnkOuter == NULL);

	IDirectSound* pDSound = NULL;
	V_RET((*pDirectSoundCreate)( pcGuidDevice, &pDSound, pUnkOuter ));

	g_pMyDirectSound = new MyDirectSound( pDSound );
	*ppDS = g_pMyDirectSound;
	return S_OK;
}
/// Exported function
HRESULT WINAPI DirectSoundCreate8(
								 __in_opt LPCGUID pcGuidDevice,
								 __deref_out LPDIRECTSOUND8 *ppDS8,
								 __null LPUNKNOWN pUnkOuter
								 )
{
	M_TRACE_FUNCTION;

	HINSTANCE hDSOUND = LazyLoadOriginalDLL();
	V_RET_IF_NIL(hDSOUND, ERR_FAILED_TO_LOAD_DLL);

	F_DirectSoundCreate8* pDirectSoundCreate8 = (F_DirectSoundCreate8*)
		::GetProcAddress( hDSOUND, "DirectSoundCreate8" );

	V_RET_IF_NIL(pDirectSoundCreate8, TYPE_E_DLLFUNCTIONNOTFOUND);

	return (*pDirectSoundCreate8)( pcGuidDevice, ppDS8, pUnkOuter );
}
/// Exported function
HRESULT WINAPI GetDeviceID(
						   __in_opt LPCGUID pGuidSrc,
						   __out LPGUID pGuidDest
						   )
{
	M_TRACE_FUNCTION;
	HINSTANCE hDSOUND = LazyLoadOriginalDLL();
	V_RET_IF_NIL(hDSOUND, ERR_FAILED_TO_LOAD_DLL);

	F_GetDeviceID* pGetDeviceID = (F_GetDeviceID*) ::GetProcAddress( hDSOUND, "GetDeviceID" );

	V_RET_IF_NIL(pGetDeviceID, TYPE_E_DLLFUNCTIONNOTFOUND);

	return (*pGetDeviceID)( pGuidSrc, pGuidDest );
}

HRESULT InitInstance(HANDLE hModule) 
{
	g_LogFile = NULL;
	g_hModule = (HINSTANCE) hModule;
	g_hOriginalDLL = NULL;
	g_pMyDirectSound = NULL;

	OutputDebugString("InitInstance() called.\r\n");

	OpenLog();

	M_TRACE_FUNCTION;

	return S_OK;
}

// NOTE: lazy because you should never call LoadLibrary*() from within DllMain().
HINSTANCE LazyLoadOriginalDLL(void)
{
	if( g_hOriginalDLL ) {
		return g_hOriginalDLL;
	}

    char buffer[MAX_PATH];
	::GetSystemDirectory(buffer,MAX_PATH);
	strcat_s(buffer,"\\dsound.dll");

	g_hOriginalDLL = ::LoadLibrary(buffer);
	if( !g_hOriginalDLL ) {
		Logf("Failed to load DSOUND.DLL");
	}

	// Register our own exception handler with Windows.
	//::SetUnhandledExceptionFilter( &ExceptionCallback );
	//NOTE: it doesn't work, the application sets its own exception filter, see
	//https://social.msdn.microsoft.com/forums/windowsdesktop/en-us/0ac370e2-14f0-45d3-84c1-1d001abaf978/setunhandledexceptionfilter-problem

	return g_hOriginalDLL;
}

void ExitInstance() 
{
	M_TRACE_FUNCTION;

    OutputDebugString("PROXYDLL: ExitInstance called.\r\n");

	if( g_pMyDirectSound )
	{
		//NOTE: this should have deleted itself in Release()
		//delete g_pMyDirectSound;
		g_pMyDirectSound = NULL;
	}

	if (g_hOriginalDLL)
	{
		::FreeLibrary(g_hOriginalDLL);
	    g_hOriginalDLL = NULL;  
	}

	CloseLog();
}

bool OpenLog()
{
#ifdef LOG_FILE_NAME
	{
		g_LogFile = ::fopen( LOG_FILE_NAME, "w" );
		if( NULL == g_LogFile ) {
			::MessageBoxA( NULL, "Unable to create a log file", "Error", MB_OK );
			return false;
		}
	}
#endif // if defined (LOG_FILE_NAME)
	return g_LogFile != NULL;
}

void Logf( const char* fmt, ... )
{
	if( g_LogFile != NULL )
	{
		va_list args;
		va_start( args, fmt );
		vfprintf( g_LogFile, fmt, args );
		va_end( args );

		fflush( g_LogFile );
	}
}

void CloseLog()
{
	if( g_LogFile != NULL )
	{
		::fclose( g_LogFile );
		g_LogFile = NULL;
	}
}

bool IsLogging()
{
	return NULL != g_LogFile;
}

/// build a filename for the dump file
static bool BuildMiniDumpFilename( char *outFileName, int numChars )
{
	// get the fully qualified path for the file that contains the specified module
	char filePath[MAX_PATH] = {0};
	DWORD numBytes = ::GetModuleFileNameA( NULL, filePath, _countof(filePath) );
	if( numBytes > 0 )
	{
		char driveName[32] = {0};
		char folderName[256] = {0};
		char pureFileName[256] = {0};
		char fileExtension[32] = {0};

		int ret = _splitpath_s(
			filePath,
			driveName,
			folderName,
			pureFileName,
			fileExtension
		);
		if( ret != 0 ) {
			M_TRACE( "_splitpath_s() failed with error code '%d'.\n", ret );
		}

		SYSTEMTIME t;
		::GetLocalTime( &t );

		char timeStr[256];
		sprintf_s( timeStr, "%04d-%02d-%02d_%02d-%02d-%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond );

		sprintf_s( outFileName, numChars, "%s%s_%s.dmp", folderName, pureFileName, timeStr );

		return true;
	}
	return false;
}

/**
    Exception handler function called back by Windows when something
    unexpected happens.
*/
static LONG WINAPI ExceptionCallback( EXCEPTION_POINTERS* exceptionInfo )
{
	char filename[512] = {0};
    if( BuildMiniDumpFilename( filename, _countof(filename) ) )
    {
		char message[1024] = {0};
		sprintf_s( message,
			"An unhandled error occurred.\nDo you wish to dump debug info into file '%s'?",
			filename
		);
		const int ret = ::MessageBoxA( NULL, message, "Fatal error", MB_OKCANCEL );
		if( ret == MB_OK )
		{
			HANDLE hFile = 
				::CreateFileA(
					filename,                 // lpFileName
					GENERIC_WRITE,            // dwDesiredAccess
					FILE_SHARE_READ,          // dwShareMode
					0,                        // lpSecurityAttributes
					CREATE_ALWAYS,            // dwCreationDisposition,
					FILE_ATTRIBUTE_NORMAL,    // dwFlagsAndAttributes
					NULL                      // hTemplateFile
			);
			if( NULL != hFile )
			{
				M_TRACE("Dumping debug info to '%s'\n", filename);
				HANDLE hProc = ::GetCurrentProcess();
				DWORD procId = ::GetCurrentProcessId();
				BOOL res = FALSE;
				if( NULL != exceptionInfo )
				{
					// extended exception info is available
					MINIDUMP_EXCEPTION_INFORMATION extInfo = { 0 };
					extInfo.ThreadId = ::GetCurrentThreadId();
					extInfo.ExceptionPointers = exceptionInfo;
					extInfo.ClientPointers = TRUE;
					res = ::MiniDumpWriteDump( hProc, procId, hFile, MiniDumpNormal, &extInfo, NULL, NULL );
				}
				else
				{
					// extended exception info is not available
					res = ::MiniDumpWriteDump( hProc, procId, hFile, MiniDumpNormal, NULL, NULL, NULL );
				}
				::CloseHandle( hFile );
			}
		}
    }
	return EXCEPTION_CONTINUE_SEARCH;
}

#ifdef _M_X64

UINT64 MurmurHash64( const void* buffer, UINT32 sizeBytes, UINT64 seed )
{
	const UINT64 m = 0xc6a4a7935bd1e995;
	const INT r = 47;

	UINT64 h = seed ^ (sizeBytes * m);

	const UINT64 * data = (const UINT64 *)buffer;
	const UINT64 * end = data + (sizeBytes/8);

	while(data != end)
	{
		UINT64 k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(sizeBytes & 7)
	{
	case 7: h ^= UINT64(data2[6]) << 48;
	case 6: h ^= UINT64(data2[5]) << 40;
	case 5: h ^= UINT64(data2[4]) << 32;
	case 4: h ^= UINT64(data2[3]) << 24;
	case 3: h ^= UINT64(data2[2]) << 16;
	case 2: h ^= UINT64(data2[1]) << 8;
	case 1: h ^= UINT64(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

#elif _M_IX86

UINT64 MurmurHash64( const void* buffer, UINT32 sizeBytes, UINT64 seed )
{
	const UINT64 m = 0xc6a4a7935bd1e995;
	const INT r = 47;

	UINT64 h = seed ^ (sizeBytes * m);

	const UINT64 * data = (const UINT64 *)buffer;
	const UINT64 * end = data + (sizeBytes/8);

	while(data != end)
	{
		UINT64 k = *data++;

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h ^= k;
		h *= m; 
	}

	const unsigned char * data2 = (const unsigned char*)data;

	switch(sizeBytes & 7)
	{
	case 7: h ^= UINT64(data2[6]) << 48;
	case 6: h ^= UINT64(data2[5]) << 40;
	case 5: h ^= UINT64(data2[4]) << 32;
	case 4: h ^= UINT64(data2[3]) << 24;
	case 3: h ^= UINT64(data2[2]) << 16;
	case 2: h ^= UINT64(data2[1]) << 8;
	case 1: h ^= UINT64(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

#endif
