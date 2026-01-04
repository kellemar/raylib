#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

typedef enum SoundType {
    SOUND_SHOOT,
    SOUND_EXPLOSION,
    SOUND_PICKUP,
    SOUND_LEVELUP,
    SOUND_HIT,
    SOUND_COUNT
} SoundType;

void AudioInit(void);
void AudioCleanup(void);
void PlayGameSound(SoundType type);

void MusicStart(void);
void MusicStop(void);
void MusicPause(void);
void MusicResume(void);
void MusicUpdate(void);
bool IsMusicPlaying(void);

#endif
