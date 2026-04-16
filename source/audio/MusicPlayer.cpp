/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Music Player Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "MusicPlayer.h"
#include "../core/Logger.h"

namespace FNF {

Mix_Music* MusicPlayer::s_Music       = nullptr;
float      MusicPlayer::s_Volume      = 0.7f;
bool       MusicPlayer::s_Initialized = false;

bool MusicPlayer::Init(int frequency, int channels, int chunkSize) {
    if (s_Initialized) return true;

    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, channels, chunkSize) < 0) {
        Logger::Error("MusicPlayer: Mix_OpenAudio failed: " + std::string(Mix_GetError()));
        return false;
    }

    // Enable OGG and MP3 support
    int flags = MIX_INIT_OGG | MIX_INIT_MP3;
    int inited = Mix_Init(flags);
    if ((inited & flags) != flags) {
        Logger::Warn("MusicPlayer: Some audio formats unavailable: " + std::string(Mix_GetError()));
    }

    s_Initialized = true;
    Logger::Info("[OK] MusicPlayer initialized (OGG/MP3 support)");
    return true;
}

void MusicPlayer::Shutdown() {
    if (!s_Initialized) return;
    Stop();
    Mix_CloseAudio();
    Mix_Quit();
    s_Initialized = false;
}

bool MusicPlayer::Play(const std::string& path, int loops, float volume) {
    if (!s_Initialized) return false;

    // Free previous music
    if (s_Music) {
        Mix_HaltMusic();
        Mix_FreeMusic(s_Music);
        s_Music = nullptr;
    }

    s_Music = Mix_LoadMUS(path.c_str());
    if (!s_Music) {
        Logger::Error("MusicPlayer: failed to load '" + path + "': " + Mix_GetError());
        return false;
    }

    s_Volume = volume;
    Mix_VolumeMusic(static_cast<int>(volume * MIX_MAX_VOLUME));

    if (Mix_PlayMusic(s_Music, loops) < 0) {
        Logger::Error("MusicPlayer: Mix_PlayMusic failed: " + std::string(Mix_GetError()));
        return false;
    }

    Logger::Info("[OK] Playing music: " + path);
    return true;
}

void MusicPlayer::Stop() {
    Mix_HaltMusic();
    if (s_Music) {
        Mix_FreeMusic(s_Music);
        s_Music = nullptr;
    }
}

void MusicPlayer::Pause()  { Mix_PauseMusic(); }
void MusicPlayer::Resume() { Mix_ResumeMusic(); }

void MusicPlayer::FadeIn(int ms, float targetVolume) {
    if (!s_Music || !s_Initialized) return;
    s_Volume = targetVolume;
    Mix_VolumeMusic(static_cast<int>(targetVolume * MIX_MAX_VOLUME));
    Mix_FadeInMusic(s_Music, -1, ms);
}

void MusicPlayer::SetVolume(float volume) {
    s_Volume = volume;
    Mix_VolumeMusic(static_cast<int>(volume * MIX_MAX_VOLUME));
}

float MusicPlayer::GetVolume() { return s_Volume; }

double MusicPlayer::GetPositionMs() {
    if (!s_Initialized || !s_Music || !Mix_PlayingMusic()) return 0.0;
    double pos = Mix_GetMusicPosition(s_Music);
    return (pos >= 0.0) ? pos * 1000.0 : 0.0;
}

bool MusicPlayer::IsPlaying() {
    return s_Initialized && Mix_PlayingMusic() && !Mix_PausedMusic();
}

bool MusicPlayer::IsPaused() {
    return s_Initialized && Mix_PausedMusic();
}

} // namespace FNF
