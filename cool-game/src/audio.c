#include "audio.h"
#include "raylib.h"
#include <math.h>

#define SAMPLE_RATE 44100
#define MAX_SAMPLES 44100

static Sound gameSounds[SOUND_COUNT] = { 0 };
static Music gameMusic = { 0 };
static bool musicLoaded = false;
static bool audioInitialized = false;

static Wave GenerateSquareWave(float frequency, float duration, float volume)
{
    int sampleCount = (int)(SAMPLE_RATE * duration);
    if (sampleCount > MAX_SAMPLES) sampleCount = MAX_SAMPLES;

    Wave wave = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(sampleCount * sizeof(short));

    short *samples = (short *)wave.data;
    for (int i = 0; i < sampleCount; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float envelope = 1.0f - (t / duration);
        float val = sinf(2.0f * PI * frequency * t) > 0.0f ? 1.0f : -1.0f;
        samples[i] = (short)(val * 32767.0f * volume * 0.3f * envelope);
    }

    return wave;
}

static Wave GenerateNoise(float duration, float volume)
{
    int sampleCount = (int)(SAMPLE_RATE * duration);
    if (sampleCount > MAX_SAMPLES) sampleCount = MAX_SAMPLES;

    Wave wave = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(sampleCount * sizeof(short));

    short *samples = (short *)wave.data;
    for (int i = 0; i < sampleCount; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float envelope = 1.0f - (t / duration);
        float noise = ((float)(GetRandomValue(0, 32767) - 16383) / 16383.0f);
        samples[i] = (short)(noise * 32767.0f * volume * 0.2f * envelope);
    }

    return wave;
}

static Wave GenerateSweep(float startFreq, float endFreq, float duration, float volume)
{
    int sampleCount = (int)(SAMPLE_RATE * duration);
    if (sampleCount > MAX_SAMPLES) sampleCount = MAX_SAMPLES;

    Wave wave = { 0 };
    wave.frameCount = sampleCount;
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = MemAlloc(sampleCount * sizeof(short));

    short *samples = (short *)wave.data;
    float phase = 0.0f;
    for (int i = 0; i < sampleCount; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float progress = t / duration;
        float freq = startFreq + (endFreq - startFreq) * progress;
        float envelope = 1.0f - progress;
        phase += 2.0f * PI * freq / SAMPLE_RATE;
        samples[i] = (short)(sinf(phase) * 32767.0f * volume * envelope);
    }

    return wave;
}

void AudioInit(void)
{
    if (audioInitialized) return;

    Wave shootWave = GenerateSquareWave(880.0f, 0.08f, 0.3f);
    gameSounds[SOUND_SHOOT] = LoadSoundFromWave(shootWave);
    UnloadWave(shootWave);

    Wave explosionWave = GenerateNoise(0.3f, 0.5f);
    gameSounds[SOUND_EXPLOSION] = LoadSoundFromWave(explosionWave);
    UnloadWave(explosionWave);

    Wave pickupWave = GenerateSweep(400.0f, 800.0f, 0.15f, 0.4f);
    gameSounds[SOUND_PICKUP] = LoadSoundFromWave(pickupWave);
    UnloadWave(pickupWave);

    Wave levelupWave = GenerateSweep(300.0f, 1200.0f, 0.5f, 0.4f);
    gameSounds[SOUND_LEVELUP] = LoadSoundFromWave(levelupWave);
    UnloadWave(levelupWave);

    Wave hitWave = GenerateNoise(0.1f, 0.4f);
    gameSounds[SOUND_HIT] = LoadSoundFromWave(hitWave);
    UnloadWave(hitWave);

    if (FileExists("resources/music/background.ogg"))
    {
        gameMusic = LoadMusicStream("resources/music/background.ogg");
        musicLoaded = true;
    }
    else if (FileExists("resources/music/background.mp3"))
    {
        gameMusic = LoadMusicStream("resources/music/background.mp3");
        musicLoaded = true;
    }
    else if (FileExists("resources/music/background.wav"))
    {
        gameMusic = LoadMusicStream("resources/music/background.wav");
        musicLoaded = true;
    }

    // Set music volume to 90% (10% quieter than default)
    if (musicLoaded)
    {
        SetMusicVolume(gameMusic, 0.9f);
    }

    audioInitialized = true;
}

void AudioCleanup(void)
{
    if (!audioInitialized) return;

    for (int i = 0; i < SOUND_COUNT; i++)
    {
        if (gameSounds[i].frameCount > 0)
        {
            UnloadSound(gameSounds[i]);
        }
    }

    if (musicLoaded)
    {
        UnloadMusicStream(gameMusic);
        musicLoaded = false;
    }

    audioInitialized = false;
}

void PlayGameSound(SoundType type)
{
    if (!audioInitialized) return;
    if (type < 0 || type >= SOUND_COUNT) return;
    if (gameSounds[type].frameCount > 0)
    {
        PlaySound(gameSounds[type]);
    }
}

void MusicStart(void)
{
    if (!audioInitialized || !musicLoaded) return;
    PlayMusicStream(gameMusic);
}

void MusicStop(void)
{
    if (!audioInitialized || !musicLoaded) return;
    StopMusicStream(gameMusic);
}

void MusicPause(void)
{
    if (!audioInitialized || !musicLoaded) return;
    PauseMusicStream(gameMusic);
}

void MusicResume(void)
{
    if (!audioInitialized || !musicLoaded) return;
    ResumeMusicStream(gameMusic);
}

void MusicUpdate(void)
{
    if (!audioInitialized || !musicLoaded) return;
    UpdateMusicStream(gameMusic);
}

bool IsMusicPlaying(void)
{
    if (!audioInitialized || !musicLoaded) return false;
    return IsMusicStreamPlaying(gameMusic);
}
