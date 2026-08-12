/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * FreeplayState - Classic Psych-style Freeplay menu
 */

#pragma once

#include "../backend/MusicBeatState.h"
#include "../objects/Alphabet.h"
#include "../objects/Sprite.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

namespace FNF {

class OpenGLESBackend;

class FreeplayState : public MusicBeatState {
public:
    void Enter() override;
    void Exit() override;
    void HandleEvent(const SDL_Event& e) override;
    void Update(float dt) override;
    void Render(OpenGLESBackend& renderer) override;

private:
    struct SongEntry {
        std::string name;
        std::string iconId;
        std::string stageName = "stage";
        std::string weekName;
        std::vector<std::string> difficulties;
        std::string lastDifficulty = "Normal";
        uint8_t colorR = 146;
        uint8_t colorG = 113;
        uint8_t colorB = 253;
    };

    struct ResolvedSong {
        std::string folderName;
        std::string chartPath;
        std::string instPath;
        std::string stageName = "stage";
        float bpm = 102.0f;
        bool isValid() const { return !chartPath.empty() && !instPath.empty(); }
    };

    static constexpr int SCR_W = 1280;
    static constexpr int SCR_H = 720;
    static constexpr float CENTER_Y = 320.0f;
    static constexpr float ROW_H = 96.0f;
    static constexpr float LERP_SPD = 10.0f;
    static constexpr float COLOR_SPD = 2.5f;

    std::vector<SongEntry> m_Songs;
    std::vector<Alphabet> m_Labels;
    std::vector<Sprite> m_Icons;

    Sprite m_Background;
    TTF_Font* m_Font = nullptr;
    TTF_Font* m_FontBold = nullptr;

    bool m_AssetsLoaded = false;
    bool m_PreviewPlaying = false;
    bool m_PreviewPaused = false;
    int m_PreviewSongIndex = -1;
    int m_CurSelected = 0;
    int m_CurDifficulty = 0;
    float m_LerpSelected = 0.0f;

    float m_BgR = 146.0f;
    float m_BgG = 113.0f;
    float m_BgB = 253.0f;
    float m_TgtR = 146.0f;
    float m_TgtG = 113.0f;
    float m_TgtB = 253.0f;

    std::string m_StatusMessage;

    void BuildSongList();
    void LoadAssets(OpenGLESBackend& renderer);
    void LoadFonts();
    void CloseFonts();
    void ChangeSelection(int delta, bool playSound = true);
    void ChangeDifficulty(int delta);
    void SyncDifficultyForSelection();
    void StartPreview();
    void TogglePreviewPause();
    void StopPreview(bool restoreMenuMusic);
    void ConfirmSelection();
    ResolvedSong ResolveSelectedSong() const;
    ResolvedSong ResolveSong(const SongEntry& song, const std::string& difficulty) const;

    void DrawText(OpenGLESBackend& renderer, TTF_Font* font, const std::string& text,
                  int x, int y, SDL_Color color, bool centered = false) const;
    SDL_Point MeasureText(TTF_Font* font, const std::string& text) const;
};

} // namespace FNF
