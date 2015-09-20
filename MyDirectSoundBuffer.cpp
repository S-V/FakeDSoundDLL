#include "stdafx.h"
#include "MyDirectSound.h"
#include "MyDirectSoundBuffer.h"

MyDirectSoundBuffer::MyDirectSoundBuffer(
	LPCDSBUFFERDESC pcDSBufferDesc,
	IDirectSoundBuffer* pDSoundBuffer,
	MyDirectSound* pDSoundDevice,
	int nUniqueID
	)
	: m_pDSoundBuffer( pDSoundBuffer ), m_pDSoundDevice( pDSoundDevice ), m_uniqueId( nUniqueID )
{
	//M_TRACE_FUNCTION;
	m_bufferSize = pcDSBufferDesc->dwBufferBytes;

	m_alignedBuffer = NULL;
	m_allocated = 0;
	m_bytes = 0;
	m_flags = 0;

	if( m_bufferSize != 0 )
	{
		m_alignedBuffer = _aligned_malloc(m_bufferSize, SOUND_BUFFER_ALIGNMENT);
		m_allocated = m_bufferSize;
	}

	m_pDSoundDevice->AddBuffer(this);
}

MyDirectSoundBuffer::~MyDirectSoundBuffer()
{
	//M_TRACE_FUNCTION;
	if( m_alignedBuffer ) {
		M_TRACE("%d: Free 0x%p (%d bytes)\n", m_uniqueId, (void*)m_alignedBuffer, m_allocated);
		_aligned_free(m_alignedBuffer);
		m_alignedBuffer = 0;
	}
	m_allocated = 0;
	m_bytes = 0;
}

HRESULT __stdcall MyDirectSoundBuffer::QueryInterface(REFIID riid, void** ppvObj)
{  
	M_TRACE_FUNCTION;
	*ppvObj = NULL;
	HRESULT hr = m_pDSoundBuffer->QueryInterface(riid, ppvObj); 
	if (hr == NOERROR)
	{
		*ppvObj = this;
	}
	return hr;
}

ULONG __stdcall MyDirectSoundBuffer::AddRef(void)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->AddRef();
}

ULONG __stdcall MyDirectSoundBuffer::Release(void)
{  
	M_TRACE("%d: Release()\n", m_uniqueId);
	m_pDSoundDevice->RemoveBuffer(this);
	// NOTE: all objects dependent on IDirectSoundBuffer* must be released.
	ULONG count = m_pDSoundBuffer->Release();
	delete this;
	return count;
}

HRESULT __stdcall MyDirectSoundBuffer::GetCaps(THIS_ __out LPDSBCAPS pDSBufferCaps)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetCaps(pDSBufferCaps);
}

HRESULT __stdcall MyDirectSoundBuffer::GetCurrentPosition(THIS_ __out_opt LPDWORD pdwCurrentPlayCursor, __out_opt LPDWORD pdwCurrentWriteCursor)
{
	HRESULT hr = m_pDSoundBuffer->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);
	//DWORD dwCurrentPlayCursor = pdwCurrentPlayCursor ? *pdwCurrentPlayCursor : -1;
	//DWORD dwCurrentWriteCursor = pdwCurrentWriteCursor ? *pdwCurrentWriteCursor : -1;
	//M_TRACE("GetCurrentPosition(%d): PlayCursor = %d, WriteCursor = %d\n", m_uniqueId, dwCurrentPlayCursor, dwCurrentWriteCursor );
	return hr;
}

HRESULT __stdcall MyDirectSoundBuffer::GetFormat(THIS_ __out_bcount_opt(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, __out_opt LPDWORD pdwSizeWritten)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT __stdcall MyDirectSoundBuffer::GetVolume(THIS_ __out LPLONG plVolume)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetVolume(plVolume);
}

HRESULT __stdcall MyDirectSoundBuffer::GetPan(THIS_ __out LPLONG plPan)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetPan(plPan);
}

HRESULT __stdcall MyDirectSoundBuffer::GetFrequency(THIS_ __out LPDWORD pdwFrequency)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetFrequency(pdwFrequency);
}

HRESULT __stdcall MyDirectSoundBuffer::GetStatus(THIS_ __out LPDWORD pdwStatus)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->GetStatus(pdwStatus);
}

HRESULT __stdcall MyDirectSoundBuffer::Initialize(THIS_ __in LPDIRECTSOUND pDirectSound, __in LPCDSBUFFERDESC pcDSBufferDesc)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->Initialize(pDirectSound, pcDSBufferDesc);
}

HRESULT __stdcall MyDirectSoundBuffer::Play(THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	M_TRACE("%d: Play(): dwPriority=%d, dwFlags=%d\n", m_uniqueId, dwPriority, dwFlags);
	return m_pDSoundBuffer->Play(dwReserved1, dwPriority, dwFlags);
}

HRESULT __stdcall MyDirectSoundBuffer::SetCurrentPosition(THIS_ DWORD dwNewPosition)
{
	M_TRACE("%d: SetCurrentPosition(): dwNewPosition=%d\n", m_uniqueId, dwNewPosition);
	return m_pDSoundBuffer->SetCurrentPosition(dwNewPosition);
}

HRESULT __stdcall MyDirectSoundBuffer::SetFormat(THIS_ __in LPCWAVEFORMATEX pcfxFormat)
{
	M_TRACE("%d: SetFormat(): wFormatTag=%d, nChannels=%d, wBitsPerSample=%d\n",
		m_uniqueId, pcfxFormat->wFormatTag, pcfxFormat->nChannels, pcfxFormat->wBitsPerSample);
	return m_pDSoundBuffer->SetFormat(pcfxFormat);
}

HRESULT __stdcall MyDirectSoundBuffer::SetVolume(THIS_ LONG lVolume)
{
	M_TRACE_FREQUENT_FUNCTION;
	return m_pDSoundBuffer->SetVolume(lVolume);
}

HRESULT __stdcall MyDirectSoundBuffer::SetPan(THIS_ LONG lPan)
{
	M_TRACE_FREQUENT_FUNCTION;
	return m_pDSoundBuffer->SetPan(lPan);
}

/// The SetFrequency method sets the frequency, in hertz (Hz), at which the audio samples are played.
/// A value of DSBFREQUENCY_ORIGINAL resets the frequency to the default value of the buffer format.
HRESULT __stdcall MyDirectSoundBuffer::SetFrequency(THIS_ DWORD dwFrequency)
{
	M_TRACE_FREQUENT_FUNCTION;
	return m_pDSoundBuffer->SetFrequency(dwFrequency);
}

HRESULT __stdcall MyDirectSoundBuffer::Stop(THIS)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->Stop();
}

// DWORD dwWriteCursor,    // Offset of lock start 
// DWORD dwWriteBytes,    // Size of lock, if zero it is ignored 
// LPVOID lplpvAudioPtr1,    // Address of lock start 
// LPDWORD lpdwAudioBytes1,    // Number of bytes to lock or NULL 
// LPVOID lplpvAudioPtr2,    // Address of the wrap-around start or 0 
// LPDWORD lpdwAudioBytes2,    // Number of bytes in wrap-around or 0 
// DWORD dwFlags);    // DSBLOCK_FROMWRITECURSOR or DSBLOCK_ENTIREBUFFER
HRESULT __stdcall MyDirectSoundBuffer::Lock(THIS_ DWORD dwOffset, DWORD dwBytes,
											__deref_out_bcount(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, __out LPDWORD pdwAudioBytes1,
											__deref_opt_out_bcount(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, __out_opt LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	M_TRACE("%d: Lock(): dwOffset=%d, dwBytes=%d, flags=%d\n",
		m_uniqueId, dwOffset, dwBytes, dwFlags);

	if( m_bufferSize ) {
		M_ASSERT2(dwBytes <= m_bufferSize, "dwBytes=%d, m_bufferSize=%d\n", dwBytes, m_bufferSize);
	}

	M_ASSERT2(0==(dwFlags & DSBLOCK_FROMWRITECURSOR), "DSBLOCK_FROMWRITECURSOR is not supported");

	bool isPrimarySoundBuffer = (m_bufferSize == 0);

	bool record = !isPrimarySoundBuffer
		// support for DSBLOCK_ENTIREBUFFER not implemented yet
		&& ((dwFlags & DSBLOCK_ENTIREBUFFER) == 0)
		;

	// non-zero offsets are not supported yet
	if( dwOffset == 0 && ppvAudioPtr2 == NULL ) {
		record = false;
	}

	if( record )
	{
		M_ASSERT2(m_bytes == 0, "Already recording");

		if( dwFlags & DSBLOCK_ENTIREBUFFER )
		{
			// Lock the entire buffer. The dwBytes parameter is ignored.
			M_ASSERT(m_bufferSize > 0);
			m_bytes = m_bufferSize;
		}
		else
		{
			// resize memory buffer, if needed
			if( dwBytes > m_allocated )
			{
				M_TRACE("%d: Lock(): realloc %d -> %d\n", m_uniqueId, m_allocated, dwBytes);
				_aligned_free(m_alignedBuffer);
				m_alignedBuffer = _aligned_malloc(dwBytes, SOUND_BUFFER_ALIGNMENT);
				m_allocated = dwBytes;
			}
			m_bytes = dwBytes;
		}
		M_TRACE("%d: Lock(): START RECORDING\n", m_uniqueId);

		m_flags = dwFlags;

		// force the client to copy sound data to our buffer
		*ppvAudioPtr1 = m_alignedBuffer;
		*pdwAudioBytes1 = dwBytes;
		*ppvAudioPtr2 = NULL;
		*pdwAudioBytes2 = 0;

		return DS_OK;
	}
	else
	{
		// just in case
		m_bytes = 0;
		m_flags = 0;

		//M_TRACE("Lock(%d): calling m_pDSoundBuffer->Lock()\n", m_uniqueId);

		HRESULT hr = m_pDSoundBuffer->Lock(
			dwOffset, dwBytes,
			ppvAudioPtr1, pdwAudioBytes1,
			ppvAudioPtr2, pdwAudioBytes2, dwFlags
		);

		return hr;
	}
}
// LPVOID lpvAudioPtr1  // Address of lock start 
// DWORD dwAudioBytes1  // Number of bytes locked or NULL 
// LPVOID lpvAudioPtr2  // Address of the wrap-around start or 0 
// DWORD dwAudioBytes2  // Number of bytes in wrap-around or 0
HRESULT __stdcall MyDirectSoundBuffer::Unlock(THIS_ __in_bcount(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
											  __in_bcount_opt(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	M_TRACE("%d: Unlock(): dwAudioBytes1=%d, dwAudioBytes2=%d\n", m_uniqueId, dwAudioBytes1, dwAudioBytes2);
	bool recording = (m_bytes != 0);
	if( recording )
	{
		M_TRACE("%d: Unlock(): FINISH RECORDING\n", m_uniqueId);

		// http://blog.nektra.com/main/2009/02/24/directsound-capture-using-deviare/#2.1.Direct%20Sound%20Capturing%7Coutline
		// At this point, the user is telling DirectSound that he has finished writing his wave output
		// and the locks may be released. So, if we step in between, we can safely read the buffer.
		// The user is no longer writing to it, and DirectSound has not yet taken control of it.

		m_pDSoundDevice->SaveSoundBlock( this, m_alignedBuffer, m_bytes );

		LPVOID 	lock_pvAudioPtr1 = NULL;
		DWORD	lock_dwAudioBytes1 = 0;
		LPVOID 	lock_pvAudioPtr2 = NULL;
		DWORD	lock_dwAudioBytes2 = 0;

		V_RET(m_pDSoundBuffer->Lock(
			0, m_bytes,
			&lock_pvAudioPtr1, &lock_dwAudioBytes1,
			&lock_pvAudioPtr2, &lock_dwAudioBytes2, m_flags
		));

		M_ASSERT(lock_pvAudioPtr1 != NULL);
		M_ASSERT(lock_dwAudioBytes1 > 0);

		//M_TRACE("Unlock(%d): *pdwAudioBytes1=%d, *pdwAudioBytes2=%d\n", m_uniqueId, lock_dwAudioBytes1, lock_dwAudioBytes2);

		memcpy(lock_pvAudioPtr1, m_alignedBuffer, m_bytes);

		m_bytes = 0;
		m_flags = 0;

		return m_pDSoundBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	}
	else
	{
		return m_pDSoundBuffer->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	}
}

HRESULT __stdcall MyDirectSoundBuffer::Restore(THIS)
{
	M_TRACE_FUNCTION;
	return m_pDSoundBuffer->Restore();
}
