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

// Volume control
void SetGameMusicVolume(float volume);
void SetGameSFXVolume(float volume);
float GetGameMusicVolume(void);
float GetGameSFXVolume(void);

// Game music (plays during gameplay)
void MusicStart(void);
void MusicStop(void);
void MusicPause(void);
void MusicResume(void);
void MusicUpdate(void);
bool IsMusicPlaying(void);

// Intro music (plays on menu screen)
void IntroMusicStart(void);
void IntroMusicStop(void);
void IntroMusicUpdate(void);
bool IsIntroMusicPlaying(void);

// Smooth transition from intro to game music
void TransitionToGameMusic(void);
void UpdateMusicTransition(float dt);

#endif
