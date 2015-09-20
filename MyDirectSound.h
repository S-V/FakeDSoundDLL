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
// Структура, описывающая заголовок WAV файла.
struct WAVHEADER
{
    // WAV-формат начинается с RIFF-заголовка:

    // Содержит символы "RIFF" в ASCII кодировке
    // (0x52494646 в big-endian представлении)
    char chunkId[4];

    // 36 + subchunk2Size, или более точно:
    // 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
    // Это оставшийся размер цепочки, начиная с этой позиции.
    // Иначе говоря, это размер файла - 8, то есть,
    // исключены поля chunkId и chunkSize.
    unsigned long chunkSize;

    // Содержит символы "WAVE"
    // (0x57415645 в big-endian представлении)
    char format[4];

    // Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
    // Подцепочка "fmt " описывает формат звуковых данных:
    
    // Содержит символы "fmt "
    // (0x666d7420 в big-endian представлении)
    char subchunk1Id[4];

    // 16 для формата PCM.
    // Это оставшийся размер подцепочки, начиная с этой позиции.
    unsigned long subchunk1Size;

    // Аудио формат, полный список можно получить здесь http://audiocoding.ru/wav_formats.txt
    // Для PCM = 1 (то есть, Линейное квантование).
    // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    unsigned short audioFormat;

    // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    unsigned short numChannels;

    // Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
    unsigned long sampleRate;

    // sampleRate * numChannels * bitsPerSample/8
    unsigned long byteRate;

    // numChannels * bitsPerSample/8
    // Количество байт для одного сэмпла, включая все каналы.
    unsigned short blockAlign;

    // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
    unsigned short bitsPerSample;

    // Подцепочка "data" содержит аудио-данные и их размер.

    // Содержит символы "data"
    // (0x64617461 в big-endian представлении)
    char subchunk2Id[4];

    // numSamples * numChannels * bitsPerSample/8
    // Количество байт в области данных.
    unsigned long subchunk2Size;

    // Далее следуют непосредственно Wav данные.
};

bool SaveWavToFile( MyDirectSoundBuffer* o, const void* data, DWORD size, const char* filepath );
