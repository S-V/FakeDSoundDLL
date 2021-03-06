#include "stdafx.h"
// std::remove
#include <algorithm>
#include "MyDirectSound.h"
#include "MyDirectSoundBuffer.h"
#include "SDKwavefile.h"

template< typename T >
struct delete_pointer {
	void operator()( T* ptr ) const {
		delete ptr;
	}
};

MyDirectSound::MyDirectSound( IDirectSound* pDSound )
	: m_pDSound( pDSound )
{
	M_TRACE_FUNCTION;
	m_nextSoundBufferId = 0;
}

MyDirectSound::~MyDirectSound()
{
	M_TRACE_FUNCTION;

	if( !m_createdBuffers.empty() )
	{
		M_TRACE("Deleting %d sound buffers\n", (int)m_createdBuffers.size());
		std::for_each( m_createdBuffers.begin(), m_createdBuffers.end(), delete_pointer<MyDirectSoundBuffer>() );
		m_createdBuffers.clear();
	}
	m_nextSoundBufferId = 0;
}

void MyDirectSound::AddBuffer( MyDirectSoundBuffer* buffer )
{
	m_createdBuffers.push_back(buffer);
}
void MyDirectSound::RemoveBuffer( MyDirectSoundBuffer* buffer )
{
	m_createdBuffers.erase(std::remove(m_createdBuffers.begin(), m_createdBuffers.end(), buffer), m_createdBuffers.end());
}

void MyDirectSound::SaveSoundBlock( MyDirectSoundBuffer* o, const void* data, DWORD size )
{
	M_ASSERT(data != NULL);
	M_ASSERT(size > 0);

	const UINT64 hash = MurmurHash64( data, size );

	SoundChunksByHashT::iterator it = m_soundChunks.find( hash );

	if( it == m_soundChunks.end() )
	{

#ifdef OUTPUT_FOLDER

		// generate unique file name
		char buffer[32];
		sprintf(buffer, "%016llX", hash);

		char filepath[MAX_PATH];
		strcpy_s(filepath, OUTPUT_FOLDER);
		strcat_s(filepath, buffer);
		strcat_s(filepath, ".wav");

		SaveWavToFile( o, data, size, filepath );

#endif // if defined (OUTPUT_FOLDER)

		SoundChunk newChunk;

		//// redundant memory allocations and a copy, but who cares...
		//newChunk.data.resize(size);
		//memcpy(&newChunk.data[0], data, size);

		m_soundChunks.insert( std::make_pair( hash, newChunk ) );
	}
}

HRESULT __stdcall MyDirectSound::QueryInterface(REFIID riid, void** ppvObj)
{
	M_TRACE_FUNCTION;
	*ppvObj = NULL;
	HRESULT hr = m_pDSound->QueryInterface(riid, ppvObj); 
	if (hr == NOERROR)
	{
		*ppvObj = this;
	}
	return hr;
}

ULONG __stdcall MyDirectSound::AddRef(void)
{
	M_TRACE_FUNCTION;
	return m_pDSound->AddRef();
}

ULONG __stdcall MyDirectSound::Release(void)
{  
	M_TRACE_FUNCTION;
	ULONG count = m_pDSound->Release();
	if( !count ) {
		// NOTE: all objects dependent on IDirectSound* must be released.
		delete this;
	}
	return count;
}
/// The CreateSoundBuffer method creates a sound buffer object to manage audio samples. 
HRESULT __stdcall MyDirectSound::CreateSoundBuffer(THIS_ __in LPCDSBUFFERDESC pcDSBufferDesc, __deref_out LPDIRECTSOUNDBUFFER *ppDSBuffer, __null LPUNKNOWN pUnkOuter)
{
	M_ASSERT(pUnkOuter == NULL);
	// dwBufferBytes - Size of the new buffer, in bytes.
	// This value must be 0 when creating a buffer with the DSBCAPS_PRIMARYBUFFER flag.
	// For secondary buffers, the minimum and maximum sizes allowed are specified by DSBSIZE_MIN and DSBSIZE_MAX, defined in Dsound.h.
	const int soundBufferId = m_nextSoundBufferId++;
	M_TRACE("CreateSoundBuffer(): id = %d, dwBufferBytes = %d\n", soundBufferId, pcDSBufferDesc->dwBufferBytes);

#if USE_SOUND_BUFFER_PROXY

	IDirectSoundBuffer* pDSBuffer = NULL;
	V_RET(m_pDSound->CreateSoundBuffer(pcDSBufferDesc, &pDSBuffer, pUnkOuter));

	*ppDSBuffer = new MyDirectSoundBuffer( pcDSBufferDesc, pDSBuffer, this, soundBufferId );
	return DS_OK;

#else

	return m_pDSound->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);

#endif
}

HRESULT __stdcall MyDirectSound::GetCaps(THIS_ __out LPDSCAPS pDSCaps)
{
	M_TRACE_FUNCTION;
	return m_pDSound->GetCaps(pDSCaps);
}
/// The DuplicateSoundBuffer method creates a new secondary buffer that shares the original buffer's memory.
/// pDSBufferOriginal - Address of the IDirectSoundBuffer or IDirectSoundBuffer8 interface of the buffer to duplicate.
/// ppDSBufferDuplicate - Address of a variable that receives the IDirectSoundBuffer interface pointer for the new buffer.
HRESULT __stdcall MyDirectSound::DuplicateSoundBuffer(THIS_ __in LPDIRECTSOUNDBUFFER pDSBufferOriginal, __deref_out LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate)
{
#if 0//USE_SOUND_BUFFER_PROXY
	IDirectSoundBuffer* pDSBufferDuplicate = NULL;
	V_RET(m_pDSound->DuplicateSoundBuffer(pDSBufferOriginal, pDSBufferDuplicate));

	const int soundBufferId = m_nextSoundBufferId++;
	MyDirectSoundBuffer* pOriginal = static_cast< MyDirectSoundBuffer* >( pDSBufferOriginal );
	MyDirectSoundBuffer* pDuplicate = new MyDirectSoundBuffer( pcDSBufferDesc, pDSBuffer, this, soundBufferId );
	M_TRACE("DuplicateSoundBuffer(): %d -> %d (%d bytes)\n", soundBufferId, pOriginal->dwBufferBytes);
	return DS_OK;
#else
	M_TRACE_FREQUENT_FUNCTION;
	return m_pDSound->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);
#endif
}

HRESULT __stdcall MyDirectSound::SetCooperativeLevel(THIS_ HWND hwnd, DWORD dwLevel)
{
	M_TRACE_FUNCTION;
	return m_pDSound->SetCooperativeLevel(hwnd,dwLevel);
}

HRESULT __stdcall MyDirectSound::Compact(THIS)
{
	M_TRACE_FUNCTION;
	return m_pDSound->Compact();
}

HRESULT __stdcall MyDirectSound::GetSpeakerConfig(THIS_ __out LPDWORD pdwSpeakerConfig)
{
	M_TRACE_FUNCTION;
	return m_pDSound->GetSpeakerConfig(pdwSpeakerConfig);
}

HRESULT __stdcall MyDirectSound::SetSpeakerConfig(THIS_ DWORD dwSpeakerConfig)
{
	M_TRACE_FUNCTION;
	return m_pDSound->SetSpeakerConfig(dwSpeakerConfig);
}

HRESULT __stdcall MyDirectSound::Initialize(THIS_ __in_opt LPCGUID pcGuidDevice)
{
	M_TRACE_FUNCTION;
	return m_pDSound->Initialize(pcGuidDevice);
}

HRESULT SaveWavToFile( MyDirectSoundBuffer* o, const void* data, DWORD size, const char* filepath )
{
	// The WAVEFORMATEX structure can have a variable length that depends on the details of the format.
	// Before retrieving the format description, the application should query the DirectSoundBuffer object
	// for the size of the format by calling this method and specifying NULL for the lpwfxFormat parameter.
	// The size of the structure will be returned in the lpdwSizeWritten parameter. 
	DWORD bytesToAllocate = 0;
	V_RET(o->m_pDSoundBuffer->GetFormat(NULL, 0, &bytesToAllocate));

	WAVEFORMATEX* lpwfxFormat = (WAVEFORMATEX*) alloca( bytesToAllocate );
	V_RET(o->m_pDSoundBuffer->GetFormat(lpwfxFormat, bytesToAllocate, NULL));


#ifdef FOLDER_TO_SAVE_WAVEFORMATEX
	{
		const UINT64 format_hash = MurmurHash64( lpwfxFormat, bytesToAllocate );

		// generate unique file name
		char buffer[32];
		sprintf(buffer, "%016llX", format_hash);

		char filepath[MAX_PATH];
		strcpy_s(filepath, FOLDER_TO_SAVE_WAVEFORMATEX);
		strcat_s(filepath, buffer);
		strcat_s(filepath, ".wav_header");

		FILE* pFile = ::fopen( filepath, "w" );
		if( pFile )
		{
			::fwrite( lpwfxFormat, sizeof(char), bytesToAllocate, pFile );
			::fclose( pFile );
			pFile = NULL;
		}
	}
#endif

	CWaveFile  waveFile;
	V_RET(waveFile.Open( (char*)filepath, lpwfxFormat, WAVEFILE_WRITE ));

	UINT bytesWritten;
	V_RET(waveFile.Write( size, (BYTE*)data, &bytesWritten ));

	V_RET(waveFile.Close());

	return S_OK;
}
