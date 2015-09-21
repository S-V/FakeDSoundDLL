#pragma once

#include <vector>

#include "DSound_Proxy.h"

class MyDirectSound;

enum { SOUND_BUFFER_ALIGNMENT = 16 };

struct MyDirectSoundBuffer : public IDirectSoundBuffer
{
	IDirectSoundBuffer *	m_pDSoundBuffer;
	MyDirectSound *			m_pDSoundDevice;
	DWORD					m_bufferSize;	// 0 if primary buffer
	const int				m_uniqueId;		// for debugging
	LONG					m_refCount;

	void *	m_alignedBuffer;	// buffer for capturing WAV files
	DWORD	m_allocated;
	DWORD	m_bytes;	// non-zero between Lock()-Unlock()
	DWORD	m_flags;	// lock flags

public:
	MyDirectSoundBuffer(
		LPCDSBUFFERDESC pcDSBufferDesc,
		IDirectSoundBuffer* pDSoundBuffer,
		MyDirectSound* pDSoundDevice,
		int nUniqueID
	);
	~MyDirectSoundBuffer();

    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ __in REFIID, __deref_out LPVOID*) override;
    STDMETHOD_(ULONG,AddRef)        (THIS) override;
    STDMETHOD_(ULONG,Release)       (THIS) override;

    // IDirectSoundBuffer methods
    STDMETHOD(GetCaps)              (THIS_ __out LPDSBCAPS pDSBufferCaps) override;
    STDMETHOD(GetCurrentPosition)   (THIS_ __out_opt LPDWORD pdwCurrentPlayCursor, __out_opt LPDWORD pdwCurrentWriteCursor) override;
    STDMETHOD(GetFormat)            (THIS_ __out_bcount_opt(dwSizeAllocated) LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, __out_opt LPDWORD pdwSizeWritten) override;
    STDMETHOD(GetVolume)            (THIS_ __out LPLONG plVolume) override;
    STDMETHOD(GetPan)               (THIS_ __out LPLONG plPan) override;
    STDMETHOD(GetFrequency)         (THIS_ __out LPDWORD pdwFrequency) override;
    STDMETHOD(GetStatus)            (THIS_ __out LPDWORD pdwStatus) override;
    STDMETHOD(Initialize)           (THIS_ __in LPDIRECTSOUND pDirectSound, __in LPCDSBUFFERDESC pcDSBufferDesc) override;
    STDMETHOD(Play)                 (THIS_ DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) override;
    STDMETHOD(SetCurrentPosition)   (THIS_ DWORD dwNewPosition) override;
    STDMETHOD(SetFormat)            (THIS_ __in LPCWAVEFORMATEX pcfxFormat) override;
    STDMETHOD(SetVolume)            (THIS_ LONG lVolume) override;
    STDMETHOD(SetPan)               (THIS_ LONG lPan) override;
    STDMETHOD(SetFrequency)         (THIS_ DWORD dwFrequency) override;
    STDMETHOD(Stop)                 (THIS) override;
    STDMETHOD(Lock)                 (THIS_ DWORD dwOffset, DWORD dwBytes,
									   __deref_out_bcount(*pdwAudioBytes1) LPVOID *ppvAudioPtr1, __out LPDWORD pdwAudioBytes1,
									   __deref_opt_out_bcount(*pdwAudioBytes2) LPVOID *ppvAudioPtr2, __out_opt LPDWORD pdwAudioBytes2, DWORD dwFlags) override;
	STDMETHOD(Unlock)               (THIS_ __in_bcount(dwAudioBytes1) LPVOID pvAudioPtr1, DWORD dwAudioBytes1,
                                           __in_bcount_opt(dwAudioBytes2) LPVOID pvAudioPtr2, DWORD dwAudioBytes2) override;
    STDMETHOD(Restore)              (THIS) override;
};
