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

	void SaveSoundBlock( MyDirectSoundBuffer* o, const void* data, DWORD size );

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

// Taken from https://gist.github.com/alexey-detr/1664067
// ���������, ����������� ��������� WAV �����.
struct WAVHEADER
{
    // WAV-������ ���������� � RIFF-���������:

    // �������� ������� "RIFF" � ASCII ���������
    // (0x52494646 � big-endian �������������)
    char chunkId[4];

    // 36 + subchunk2Size, ��� ����� �����:
    // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
    // ��� ���������� ������ �������, ������� � ���� �������.
    // ����� ������, ��� ������ ����� - 8, �� ����,
    // ��������� ���� chunkId � chunkSize.
    unsigned long chunkSize;

    // �������� ������� "WAVE"
    // (0x57415645 � big-endian �������������)
    char format[4];

    // ������ "WAVE" ������� �� ���� ����������: "fmt " � "data":
    // ���������� "fmt " ��������� ������ �������� ������:
    
    // �������� ������� "fmt "
    // (0x666d7420 � big-endian �������������)
    char subchunk1Id[4];

    // 16 ��� ������� PCM.
    // ��� ���������� ������ ����������, ������� � ���� �������.
    unsigned long subchunk1Size;

    // ����� ������, ������ ������ ����� �������� ����� http://audiocoding.ru/wav_formats.txt
    // ��� PCM = 1 (�� ����, �������� �����������).
    // ��������, ������������ �� 1, ���������� ��������� ������ ������.
    unsigned short audioFormat;

    // ���������� �������. ���� = 1, ������ = 2 � �.�.
    unsigned short numChannels;

    // ������� �������������. 8000 ��, 44100 �� � �.�.
    unsigned long sampleRate;

    // sampleRate * numChannels * bitsPerSample/8
    unsigned long byteRate;

    // numChannels * bitsPerSample/8
    // ���������� ���� ��� ������ ������, ������� ��� ������.
    unsigned short blockAlign;

    // ��� ���������� "��������" ��� �������� ��������. 8 ���, 16 ��� � �.�.
    unsigned short bitsPerSample;

    // ���������� "data" �������� �����-������ � �� ������.

    // �������� ������� "data"
    // (0x64617461 � big-endian �������������)
    char subchunk2Id[4];

    // numSamples * numChannels * bitsPerSample/8
    // ���������� ���� � ������� ������.
    unsigned long subchunk2Size;

    // ����� ������� ��������������� Wav ������.
};

bool SaveWavToFile( MyDirectSoundBuffer* o, const void* data, DWORD size, const char* filepath );
