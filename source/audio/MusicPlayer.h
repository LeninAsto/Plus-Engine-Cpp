/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Music Player - SDL2_mixer wrapper
 * 
 * Handles music streaming and playback with volume/fade control.
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL_mixer.h>
#include <string>

namespace FNF {

class MusicPlayer {
public:
    /**
     * Initialize SDL2_mixer audio subsystem.
     * Called once at application startup.
     */
    static bool Init(int frequency = 44100, int channels = 2, int chunkSize = 2048);
    static void Shutdown();

    /**
     * Load and play a music file (OGG/MP3).
     * @param path   Full path to the music file
     * @param loops  -1 = loop forever, 0 = play once
     * @param volume 0.0 to 1.0
     */
    static bool Play(const std::string& path, int loops = -1, float volume = 0.7f);

    static void Stop();
    static void Pause();
    static void Resume();

    /** Fade in from silence over `ms` milliseconds */
    static void FadeIn(int ms, float targetVolume = 0.7f);

    /** 0.0 to 1.0 */
    static void SetVolume(float volume);
    static float GetVolume();

    /**
     * Current playback position in MILLISECONDS.
     * Returns 0 if no music is playing.
     */
    static double GetPositionMs();

    static bool IsPlaying();
    static bool IsPaused();

private:
    static Mix_Music* s_Music;
    static float      s_Volume;
    static bool       s_Initialized;
};

} // namespace FNF
