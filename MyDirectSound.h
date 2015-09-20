#pragma once

#include <vector>
#include <map>

#include "DSound_Proxy.h"

class MyDirectSoundBuffer;

struct SoundChunk
{
	//std::vector< char >	data;
};

typedef std::map< UINT64, SoundChunk > SoundChunksByHashT;

class MyDirectSound : public IDirectSound
{
	IDirectSound *	m_pDSound;
	int	m_nextSoundBufferId;	// for generating unique IDs

	std::vector< MyDirectSoundBuffer* >	m_createdBuffers;	// to prevent memory leaks

	SoundChunksByHashT	m_soundChunks;

public:
	MyDirectSound( IDirectSound* pDSound );
	~MyDirectSound();

	// called by MyDirectSoundBuffer:
	void AddBuffer( MyDirectSoundBuffer* buffer );
	void RemoveBuffer( MyDirectSoundBuffer* buffer );

	void SaveSoundBlock( const void* data, DWORD size );

    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ __in REFIID, __deref_out LPVOID*) override;
    STDMETHOD_(ULONG,AddRef)        (THIS) override;
    STDMETHOD_(ULONG,Release)       (THIS) override;

    // IDirectSound methods
    STDMETHOD(CreateSoundBuffer)    (THIS_ __in LPCDSBUFFERDESC pcDSBufferDesc, __deref_out LPDIRECTSOUNDBUFFER *ppDSBuffer, __null LPUNKNOWN pUnkOuter) override;
    STDMETHOD(GetCaps)              (THIS_ __out LPDSCAPS pDSCaps) override;
    STDMETHOD(DuplicateSoundBuffer) (THIS_ __in LPDIRECTSOUNDBUFFER pDSBufferOriginal, __deref_out LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) override;
    STDMETHOD(SetCooperativeLevel)  (THIS_ HWND hwnd, DWORD dwLevel) override;
    STDMETHOD(Compact)              (THIS) override;
    STDMETHOD(GetSpeakerConfig)     (THIS_ __out LPDWORD pdwSpeakerConfig) override;
    STDMETHOD(SetSpeakerConfig)     (THIS_ DWORD dwSpeakerConfig) override;
    STDMETHOD(Initialize)           (THIS_ __in_opt LPCGUID pcGuidDevice) override;
};
