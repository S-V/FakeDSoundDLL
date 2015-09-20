#pragma once

// for WAVEFORMATEX, etc
#include <MMSystem.h>

#include "include/dsound.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	CONFIGURATION
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//NOTE: no error checking is performed!

// comment out to disable logging
//#define LOG_FILE_NAME	"R:/dsound_log.txt"

// folder where .wav files will be saved
#define OUTPUT_FOLDER	"R:/temp/"

#define USE_SOUND_BUFFER_PROXY	(1)

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	EXPORTED FUNCTIONS
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

HRESULT WINAPI DirectSoundCreate(
								 __in_opt LPCGUID pcGuidDevice,
								 __deref_out LPDIRECTSOUND *ppDS,
								 __null LPUNKNOWN pUnkOuter
								 );

HRESULT WINAPI DirectSoundCreate8(
								  __in_opt LPCGUID pcGuidDevice,
								  __deref_out LPDIRECTSOUND8 *ppDS8,
								  __null LPUNKNOWN pUnkOuter
								  );

HRESULT WINAPI GetDeviceID(
						   __in_opt LPCGUID pGuidSrc,
						   __out LPGUID pGuidDest
						   );

typedef HRESULT WINAPI F_DirectSoundCreate(
	__in_opt LPCGUID pcGuidDevice,
	__deref_out LPDIRECTSOUND *ppDS,
	__null LPUNKNOWN pUnkOuter
	);

typedef HRESULT WINAPI F_DirectSoundCreate8(
	__in_opt LPCGUID pcGuidDevice,
	__deref_out LPDIRECTSOUND8 *ppDS8,
	__null LPUNKNOWN pUnkOuter
	);

typedef HRESULT WINAPI F_GetDeviceID(
									 __in_opt LPCGUID pGuidSrc,
									 __out LPGUID pGuidDest
									 );


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	LIFETIME AND RESOURCE MANAGEMENT
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

HRESULT InitInstance(HANDLE hModule);
void ExitInstance(void);

HINSTANCE LazyLoadOriginalDLL(void);

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	LOGGING
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool OpenLog();
void Logf( const char* fmt, ... );
void CloseLog();
bool IsLogging();

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	MACROS
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Convert the expanded result of a macro to a char string.
// Expands the expression before stringifying it. See:
// http://c-faq.com/ansi/stringize.html
#define TO_STR2(X)		#X

// Creates a string from the given expression ('Stringize','Stringify').
// stringizes even macros
//
#define TO_STR(X)		TO_STR2(X)


// failed to load DLL
#define ERR_FAILED_TO_LOAD_DLL	0x8007007E

#define V_RET(X)\
	{\
		const HRESULT hr = (X);\
		if( hr != S_OK ) {\
			M_TRACE("%s failed with error 0x%lx\n", #X, hr);\
			return hr;\
		}\
	}

#define V_RET_IF_NIL( X, HR )\
	if( !X )\
	{\
		M_TRACE("%s failed with error 0x%lx\n", #X, HR);\
		return HR;\
	}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	DEBUGGING
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#if 1
	#define M_TRACE( format,...)\
		Logf( format, ##__VA_ARGS__ )
#else
	#define M_TRACE( format,...)\
		Logf( __FILE__ "(" TO_STR(__LINE__) "): " format, ##__VA_ARGS__ )
#endif

#define M_TRACE_FUNCTION	M_TRACE( __FUNCTION__ "\n" )

// dont' spam the log
#define M_TRACE_FREQUENT_FUNCTION	__noop


#define M_ASSERT( X )\
	{if( !(X) )\
		Logf( __FILE__ "(" TO_STR(__LINE__) "): ASSERTION FAILED: %s\n", #X );\
	}

#define M_ASSERT2( condition, format,... )\
	{if( !(condition) )\
		Logf( __FILE__ "(" TO_STR(__LINE__) "): ASSERTION FAILED: " ##TO_STR(condition) ", " format, ##__VA_ARGS__ );\
	}

// http://stackoverflow.com/questions/173126/attaching-a-dll-to-debug
#ifdef _DEBUG
	#define ASK_TO_ATTACH_DEBUGGER\
		{\
			static bool s_showMessageBox = true;\
			if( s_showMessageBox && MB_OK == ::MessageBox( NULL, "Attach debugger now", "ATTACH", MB_OK ) )\
			{\
				s_showMessageBox = false;\
				DebugBreak();\
			}\
		}
#else
	#define ASK_TO_ATTACH_DEBUGGER	__noop
#endif


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//	UTILITY FUNCTIONS
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

/// MurmurHash is a non-cryptographic hash function suitable for general hash-based lookup.
UINT64 MurmurHash64( const void* buffer, UINT32 sizeBytes, UINT64 seed = 0 );
