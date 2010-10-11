#ifndef NABOO_WAVE_H_
#define NABOO_WAVE_H_


#include <QString>
#include "sound/sound.h"

struct WaveHeader
{
    char chunkId[4];
    unsigned int chunkSize;
    char format[4];
    char subChunkId[4];
    unsigned int subChunkSize;
    unsigned short audioFormat;
    unsigned short numChannels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAlign;
    unsigned short bitsPerSample;
    char subChunkId2[4];
    unsigned int subChunkSize2;
};

bool playWaveFile(const QString & path, Sound & sound);

#endif
