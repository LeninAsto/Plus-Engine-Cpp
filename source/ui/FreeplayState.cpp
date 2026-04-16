/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * FreeplayState Implementation
 */

#include "FreeplayState.h"
#include "LoadingState.h"
#include "MainMenuState.h"
#include "../audio/Conductor.h"
#include "../audio/MusicPlayer.h"
#include "../audio/SoundPlayer.h"
#include "../core/Logger.h"
#include "../core/StateManager.h"
#include "../data/JsonLoader.h"
#include "../data/Paths.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

std::string Trim(const std::string& value) {
    size_t start = 0;
    size_t end = value.size();

    while (start < end && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

std::vector<std::string> SplitCsv(const std::string& value) {
    std::vector<std::string> out;
    std::stringstream ss(value);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = Trim(item);
        if (!item.empty()) {
            out.push_back(item);
        }
    }
    if (out.empty()) {
        out.push_back("Normal");
    }
    return out;
}

std::string ToSongPath(const std::string& value) {
    std::string out;
    bool lastWasDash = false;

    for (unsigned char ch : value) {
        if (std::isalnum(ch)) {
            out.push_back(static_cast<char>(std::tolower(ch)));
            lastWasDash = false;
        } else if (ch == '(' || ch == ')') {
            out.push_back(static_cast<char>(ch));
            lastWasDash = false;
        } else if (!lastWasDash) {
            out.push_back('-');
            lastWasDash = true;
        }
    }

    while (!out.empty() && out.front() == '-') out.erase(out.begin());
    while (!out.empty() && out.back() == '-') out.pop_back();
    return out;
}

float ReadChartBpm(const std::string& chartPath) {
    auto json = FNF::JsonLoader::LoadFile(chartPath);
    if (!json.has_value()) {
        return 102.0f;
    }

    if (json->contains("song") && (*json)["song"].is_object()) {
        return FNF::JsonLoader::Get((*json)["song"], "bpm", 102.0f);
    }

    return FNF::JsonLoader::Get(*json, "bpm", 102.0f);
}

std::string ResolveIconPath(const std::string& iconId) {
    std::string key = "icons/icon-" + iconId;

    std::string path = FNF::Paths::Image(key);
    if (!path.empty()) return path;

    path = FNF::Paths::Image(key, "base_game");
    if (!path.empty()) return path;

    path = FNF::Paths::Image("icons/icon-face");
    if (!path.empty()) return path;

    return FNF::Paths::Image("icons/icon-dad");
}

} // namespace

namespace FNF {

void FreeplayState::Enter() {
    Logger::Info("[FreeplayState] Enter");
    m_AssetsLoaded = false;
    m_PreviewPlaying = false;
    m_PreviewPaused = false;
    m_PreviewSongIndex = -1;
    m_CurSelected = 0;
    m_CurDifficulty = 0;
    m_LerpSelected = 0.0f;
    m_StatusMessage.clear();
    BuildSongList();

    if (!MusicPlayer::IsPlaying()) {
        const std::string musicPath = Paths::Music("freakyMenu");
        if (!musicPath.empty()) {
            Conductor::ClearBPMChanges();
            Conductor::SetBPM(102.0f);
            MusicPlayer::Play(musicPath, -1, 0.7f);
        }
    }
}

void FreeplayState::Exit() {
    if (m_PreviewPlaying) {
        StopPreview(false);
    }
    CloseFonts();
    Logger::Info("[FreeplayState] Exit");
}

void FreeplayState::BuildSongList() {
    m_Songs.clear();

    std::ifstream file(Paths::GetRoot() + "/shared/weeks/weekList.txt");
    if (!file.is_open()) {
        Logger::Warn("[FreeplayState] weekList.txt not found");
        return;
    }

    std::string weekId;
    while (std::getline(file, weekId)) {
        weekId = Trim(weekId);
        if (weekId.empty()) {
            continue;
        }

        const std::string weekPath = Paths::WeekData(weekId);
        if (weekPath.empty()) {
            continue;
        }

        auto weekJson = JsonLoader::LoadFile(weekPath);
        if (!weekJson.has_value()) {
            continue;
        }

        if (JsonLoader::Get(*weekJson, "hideFreeplay", false)) {
            continue;
        }

        const std::vector<std::string> difficulties = SplitCsv(JsonLoader::Get(*weekJson, "difficulties", std::string("Normal")));
        const std::string weekName = JsonLoader::Get(*weekJson, "weekName", weekId);

        uint8_t weekR = 146;
        uint8_t weekG = 113;
        uint8_t weekB = 253;
        if (weekJson->contains("freeplayColor") && (*weekJson)["freeplayColor"].is_array() && (*weekJson)["freeplayColor"].size() >= 3) {
            weekR = (*weekJson)["freeplayColor"][0].get<uint8_t>();
            weekG = (*weekJson)["freeplayColor"][1].get<uint8_t>();
            weekB = (*weekJson)["freeplayColor"][2].get<uint8_t>();
        }

        if (!weekJson->contains("songs") || !(*weekJson)["songs"].is_array()) {
            continue;
        }

        for (const auto& songJson : (*weekJson)["songs"]) {
            if (!songJson.is_array() || songJson.empty()) {
                continue;
            }

            SongEntry song;
            song.name = songJson[0].get<std::string>();
            song.iconId = songJson.size() > 1 && songJson[1].is_string() ? songJson[1].get<std::string>() : "face";
            song.stageName = JsonLoader::Get(*weekJson, "weekBackground", std::string("stage"));
            song.weekName = weekName;
            song.difficulties = difficulties;

            if (songJson.size() > 2 && songJson[2].is_array() && songJson[2].size() >= 3) {
                song.colorR = songJson[2][0].get<uint8_t>();
                song.colorG = songJson[2][1].get<uint8_t>();
                song.colorB = songJson[2][2].get<uint8_t>();
            } else {
                song.colorR = weekR;
                song.colorG = weekG;
                song.colorB = weekB;
            }

            std::vector<std::string> available;
            for (const auto& difficulty : song.difficulties) {
                if (!ResolveSong(song, difficulty).chartPath.empty()) {
                    available.push_back(difficulty);
                }
            }

            if (!available.empty()) {
                song.difficulties = available;
                if (std::find(song.difficulties.begin(), song.difficulties.end(), "Normal") == song.difficulties.end()) {
                    song.lastDifficulty = song.difficulties.front();
                }
                m_Songs.push_back(std::move(song));
            }
        }
    }

    if (!m_Songs.empty()) {
        m_CurSelected = 0;
        m_LerpSelected = 0.0f;
        m_BgR = m_TgtR = static_cast<float>(m_Songs[0].colorR);
        m_BgG = m_TgtG = static_cast<float>(m_Songs[0].colorG);
        m_BgB = m_TgtB = static_cast<float>(m_Songs[0].colorB);
        SyncDifficultyForSelection();
    }
}

void FreeplayState::LoadFonts() {
    if (!TTF_WasInit() && TTF_Init() < 0) {
        Logger::Error("[FreeplayState] TTF_Init failed: " + std::string(TTF_GetError()));
        return;
    }

    if (!m_Font) {
        const std::string fontPath = Paths::Font("inter.otf");
        if (!fontPath.empty()) {
            m_Font = TTF_OpenFont(fontPath.c_str(), 24);
        }
    }

    if (!m_FontBold) {
        std::string boldPath = Paths::Font("inter-bold.otf");
        if (boldPath.empty()) {
            boldPath = Paths::Font("inter.otf");
        }
        if (!boldPath.empty()) {
            m_FontBold = TTF_OpenFont(boldPath.c_str(), 30);
        }
    }
}

void FreeplayState::CloseFonts() {
    if (m_FontBold) {
        TTF_CloseFont(m_FontBold);
        m_FontBold = nullptr;
    }
    if (m_Font) {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }
}

void FreeplayState::LoadAssets(SDL_Renderer* renderer) {
    Alphabet::LoadAtlas(renderer);
    LoadFonts();

    m_Background.Load(renderer, Paths::Image("menuDesat"));
    if (m_Background.texWidth > 0) {
        float scl = static_cast<float>(SCR_W) / m_Background.texWidth * 1.175f;
        m_Background.SetScale(scl);
        m_Background.x = (SCR_W - m_Background.GetWidth()) * 0.5f;
        m_Background.y = (SCR_H - m_Background.GetHeight()) * 0.5f;
    }

    m_Labels.clear();
    m_Icons.clear();
    m_Labels.resize(m_Songs.size());
    m_Icons.resize(m_Songs.size());

    for (size_t i = 0; i < m_Songs.size(); ++i) {
        m_Labels[i].SetText(m_Songs[i].name, true);
        Logger::Info("[FreeplayState] Alphabet label '" + m_Songs[i].name
            + "' width=" + std::to_string(static_cast<int>(m_Labels[i].GetWidth()))
            + " height=" + std::to_string(static_cast<int>(m_Labels[i].GetHeight()))
            + " visible=" + std::string(m_Labels[i].GetWidth() > 0.0f ? "yes" : "no"));
        std::string iconPath = ResolveIconPath(m_Songs[i].iconId);
        if (!iconPath.empty()) {
            m_Icons[i].Load(renderer, iconPath);
            m_Icons[i].SetScale(0.5f);
        }
    }

    m_AssetsLoaded = true;
    ChangeSelection(0, false);
}

void FreeplayState::SyncDifficultyForSelection() {
    if (m_Songs.empty()) {
        m_CurDifficulty = 0;
        return;
    }

    auto& song = m_Songs[m_CurSelected];
    auto it = std::find(song.difficulties.begin(), song.difficulties.end(), song.lastDifficulty);
    if (it != song.difficulties.end()) {
        m_CurDifficulty = static_cast<int>(std::distance(song.difficulties.begin(), it));
        return;
    }

    auto normalIt = std::find(song.difficulties.begin(), song.difficulties.end(), "Normal");
    if (normalIt != song.difficulties.end()) {
        m_CurDifficulty = static_cast<int>(std::distance(song.difficulties.begin(), normalIt));
        song.lastDifficulty = "Normal";
        return;
    }

    m_CurDifficulty = 0;
    if (!song.difficulties.empty()) {
        song.lastDifficulty = song.difficulties.front();
    }
}

void FreeplayState::ChangeSelection(int delta, bool playSound) {
    if (m_Songs.empty()) {
        return;
    }

    m_CurSelected = (m_CurSelected + delta + static_cast<int>(m_Songs.size())) % static_cast<int>(m_Songs.size());
    SyncDifficultyForSelection();

    m_TgtR = static_cast<float>(m_Songs[m_CurSelected].colorR);
    m_TgtG = static_cast<float>(m_Songs[m_CurSelected].colorG);
    m_TgtB = static_cast<float>(m_Songs[m_CurSelected].colorB);

    if (playSound) {
        const std::string sfx = Paths::Sound("scrollMenu");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 0.4f);
        }
    }
}

void FreeplayState::ChangeDifficulty(int delta) {
    if (m_Songs.empty()) {
        return;
    }

    auto& diffs = m_Songs[m_CurSelected].difficulties;
    if (diffs.empty()) {
        m_CurDifficulty = 0;
        return;
    }

    m_CurDifficulty = (m_CurDifficulty + delta + static_cast<int>(diffs.size())) % static_cast<int>(diffs.size());
    m_Songs[m_CurSelected].lastDifficulty = diffs[m_CurDifficulty];

    const std::string sfx = Paths::Sound("scrollMenu");
    if (!sfx.empty()) {
        SoundPlayer::Play(sfx, 0.4f);
    }
}

FreeplayState::ResolvedSong FreeplayState::ResolveSong(const SongEntry& song, const std::string& difficulty) const {
    const std::string baseName = ToSongPath(song.name);
    const std::string diffName = ToSongPath(difficulty);

    std::vector<std::string> folders = { baseName };
    if (diffName == "erect" || diffName == "nightmare") {
        folders.insert(folders.begin(), baseName + "-erect");
    }

    for (const auto& folder : folders) {
        std::vector<std::string> fileStems;
        if (diffName.empty() || diffName == "normal") {
            fileStems = { folder, baseName };
        } else {
            fileStems.push_back(folder + "-" + diffName);
            if (folder != baseName) fileStems.push_back(baseName + "-" + diffName);
            fileStems.push_back(folder);
            if (folder != baseName) fileStems.push_back(baseName);
        }

        for (const auto& stem : fileStems) {
            const std::string chartPath = Paths::SongVariantData(folder, stem);
            if (chartPath.empty()) {
                continue;
            }

            ResolvedSong resolved;
            resolved.folderName = folder;
            resolved.chartPath = chartPath;
            resolved.instPath = Paths::Inst(folder);
            resolved.stageName = song.stageName;
            resolved.bpm = ReadChartBpm(chartPath);

            if (!resolved.instPath.empty()) {
                return resolved;
            }
        }
    }

    return {};
}

FreeplayState::ResolvedSong FreeplayState::ResolveSelectedSong() const {
    if (m_Songs.empty()) {
        return {};
    }
    const auto& song = m_Songs[m_CurSelected];
    const std::string difficulty = song.difficulties.empty() ? "Normal" : song.difficulties[m_CurDifficulty];
    return ResolveSong(song, difficulty);
}

void FreeplayState::StartPreview() {
    if (m_Songs.empty()) {
        return;
    }

    const ResolvedSong resolved = ResolveSelectedSong();
    if (!resolved.isValid()) {
        m_StatusMessage = "Missing chart or Inst for " + m_Songs[m_CurSelected].name;
        const std::string sfx = Paths::Sound("cancelMenu");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 1.0f);
        }
        return;
    }

    if (!MusicPlayer::Play(resolved.instPath, -1, 0.8f)) {
        m_StatusMessage = "Could not start preview for " + m_Songs[m_CurSelected].name;
        return;
    }

    Conductor::ClearBPMChanges();
    Conductor::SetBPM(resolved.bpm);
    Conductor::songPosition = 0.0f;

    m_PreviewPlaying = true;
    m_PreviewPaused = false;
    m_PreviewSongIndex = m_CurSelected;
    m_StatusMessage = "Previewing " + m_Songs[m_CurSelected].name + " - press SPACE to pause, ESC to stop";
}

void FreeplayState::TogglePreviewPause() {
    if (!m_PreviewPlaying) {
        StartPreview();
        return;
    }

    if (m_PreviewPaused) {
        MusicPlayer::Resume();
        m_PreviewPaused = false;
        m_StatusMessage = "Preview resumed";
    } else {
        MusicPlayer::Pause();
        m_PreviewPaused = true;
        m_StatusMessage = "Preview paused";
    }
}

void FreeplayState::StopPreview(bool restoreMenuMusic) {
    MusicPlayer::Stop();
    m_PreviewPlaying = false;
    m_PreviewPaused = false;
    m_PreviewSongIndex = -1;

    if (restoreMenuMusic) {
        const std::string menuMusic = Paths::Music("freakyMenu");
        if (!menuMusic.empty()) {
            Conductor::ClearBPMChanges();
            Conductor::SetBPM(102.0f);
            MusicPlayer::Play(menuMusic, -1, 0.0f);
            MusicPlayer::FadeIn(600, 0.7f);
        }
        m_StatusMessage = "Preview stopped";
    }
}

void FreeplayState::ConfirmSelection() {
    if (m_Songs.empty()) {
        return;
    }

    const ResolvedSong resolved = ResolveSelectedSong();
    if (!resolved.isValid()) {
        m_StatusMessage = "Missing chart for " + m_Songs[m_CurSelected].name;
        const std::string sfx = Paths::Sound("cancelMenu");
        if (!sfx.empty()) {
            SoundPlayer::Play(sfx, 1.0f);
        }
        return;
    }

    const std::string sfx = Paths::Sound("confirmMenu");
    if (!sfx.empty()) {
        SoundPlayer::Play(sfx, 0.8f);
    }

    Logger::Info("[FreeplayState] Selected chart: " + resolved.chartPath);

    PlayRequest request;
    request.songName = m_Songs[m_CurSelected].name;
    request.difficultyName = m_Songs[m_CurSelected].difficulties.empty()
        ? std::string("Normal")
        : m_Songs[m_CurSelected].difficulties[m_CurDifficulty];
    request.chartPath = resolved.chartPath;
    request.instPath = resolved.instPath;
    request.playerVoicesPath = Paths::Voices(ToSongPath(m_Songs[m_CurSelected].name), "Player");
    request.opponentVoicesPath = Paths::Voices(ToSongPath(m_Songs[m_CurSelected].name), "Opponent");
    request.fallbackStage = resolved.stageName;

    StateManager::Get().SwitchWithFade(std::make_unique<LoadingState>(request), 0.7f);
}

void FreeplayState::HandleEvent(const SDL_Event& e) {
    if (e.type != SDL_KEYDOWN) {
        return;
    }

    if (m_PreviewPlaying) {
        switch (e.key.keysym.sym) {
            case SDLK_SPACE:
                TogglePreviewPause();
                break;
            case SDLK_ESCAPE:
                {
                    const std::string sfx = Paths::Sound("cancelMenu");
                    if (!sfx.empty()) {
                        SoundPlayer::Play(sfx, 1.0f);
                    }
                    StopPreview(true);
                }
                break;
            default:
                break;
        }
        return;
    }

    switch (e.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            ChangeSelection(-1);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            ChangeSelection(+1);
            break;
        case SDLK_LEFT:
        case SDLK_a:
            ChangeDifficulty(-1);
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            ChangeDifficulty(+1);
            break;
        case SDLK_SPACE:
            StartPreview();
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            ConfirmSelection();
            break;
        case SDLK_ESCAPE:
            {
                const std::string sfx = Paths::Sound("cancelMenu");
                if (!sfx.empty()) {
                    SoundPlayer::Play(sfx, 1.0f);
                }
                StateManager::Get().SwitchWithFade(std::make_unique<MainMenuState>(), 0.7f);
            }
            break;
        default:
            break;
    }
}

void FreeplayState::Update(float dt) {
    if (!m_AssetsLoaded) {
        return;
    }

    m_LerpSelected += (static_cast<float>(m_CurSelected) - m_LerpSelected) * (std::min)(1.0f, dt * LERP_SPD);

    m_BgR += (m_TgtR - m_BgR) * (std::min)(1.0f, dt * COLOR_SPD);
    m_BgG += (m_TgtG - m_BgG) * (std::min)(1.0f, dt * COLOR_SPD);
    m_BgB += (m_TgtB - m_BgB) * (std::min)(1.0f, dt * COLOR_SPD);

    m_Background.colorR = static_cast<Uint8>(m_BgR);
    m_Background.colorG = static_cast<Uint8>(m_BgG);
    m_Background.colorB = static_cast<Uint8>(m_BgB);

    if (!m_PreviewPlaying && MusicPlayer::IsPlaying() && MusicPlayer::GetVolume() < 0.7f) {
        MusicPlayer::SetVolume((std::min)(0.7f, MusicPlayer::GetVolume() + dt * 0.5f));
    }

    for (size_t i = 0; i < m_Labels.size(); ++i) {
        float rel = static_cast<float>(i) - m_LerpSelected;
        float absRel = std::abs(rel);
        m_Labels[i].visible = absRel < 5.5f;
        m_Labels[i].y = CENTER_Y + rel * ROW_H;
        m_Labels[i].x = (absRel < 0.1f) ? 110.0f : 160.0f + absRel * 35.0f;
        m_Labels[i].alpha = (absRel < 0.1f) ? 1.0f : (std::max)(0.25f, 0.7f - absRel * 0.12f);

        if (i < m_Icons.size() && m_Icons[i].IsLoaded()) {
            m_Icons[i].visible = m_Labels[i].visible;
            m_Icons[i].alpha = m_Labels[i].alpha;
            m_Icons[i].x = m_Labels[i].x + m_Labels[i].GetWidth() + 22.0f;
            m_Icons[i].y = m_Labels[i].y - 14.0f;
        }
    }

    MusicBeatState::Update(dt);
}

SDL_Point FreeplayState::MeasureText(TTF_Font* font, const std::string& text) const {
    SDL_Point size = {0, 0};
    if (!font || text.empty()) {
        return size;
    }
    TTF_SizeUTF8(font, text.c_str(), &size.x, &size.y);
    return size;
}

void FreeplayState::DrawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text,
                             int x, int y, SDL_Color color, bool centered) const {
    if (!font || text.empty()) {
        return;
    }

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        SDL_Rect dst = { x, y, surface->w, surface->h };
        if (centered) {
            dst.x -= surface->w / 2;
        }
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
    }

    SDL_FreeSurface(surface);
}

void FreeplayState::Render(SDL_Renderer* renderer) {
    if (!m_AssetsLoaded) {
        LoadAssets(renderer);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (m_Background.IsLoaded()) {
        m_Background.Draw(renderer);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
    SDL_Rect overlay = { 0, 0, SCR_W, SCR_H };
    SDL_RenderFillRect(renderer, &overlay);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect leftPanel = { 40, 70, 760, 570 };
    SDL_RenderFillRect(renderer, &leftPanel);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_Rect rightPanel = { 835, 70, 405, 290 };
    SDL_RenderFillRect(renderer, &rightPanel);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 190);
    SDL_Rect bottomBar = { 0, SCR_H - 78, SCR_W, 78 };
    SDL_RenderFillRect(renderer, &bottomBar);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    for (auto& label : m_Labels) {
        if (label.visible) {
            label.Draw(renderer);
        }
    }
    for (auto& icon : m_Icons) {
        if (icon.visible) {
            icon.Draw(renderer);
        }
    }

    const SDL_Color white = { 255, 255, 255, 255 };
    const SDL_Color soft = { 210, 210, 210, 255 };
    const SDL_Color accent = {
        static_cast<Uint8>(m_TgtR),
        static_cast<Uint8>(m_TgtG),
        static_cast<Uint8>(m_TgtB),
        255
    };

    DrawText(renderer, m_FontBold, "Freeplay", 64, 28, white, false);

    if (!m_Songs.empty()) {
        const auto& song = m_Songs[m_CurSelected];
        const std::string diffText = song.difficulties.empty() ? "NORMAL" : song.difficulties[m_CurDifficulty];
        const ResolvedSong resolved = ResolveSelectedSong();

        DrawText(renderer, m_FontBold, song.name, 865, 98, white, false);
        DrawText(renderer, m_Font, "Week: " + song.weekName, 865, 150, soft, false);
        DrawText(renderer, m_Font, "Difficulty: " + diffText, 865, 186, accent, false);
        DrawText(renderer, m_Font, "Chart folder: " + (resolved.folderName.empty() ? std::string("missing") : resolved.folderName), 865, 222, soft, false);
        DrawText(renderer, m_Font, "BPM: " + std::to_string(static_cast<int>(resolved.bpm)), 865, 258, soft, false);
    }

    const std::string hint = m_PreviewPlaying
        ? "SPACE pause/resume preview   ESC stop preview"
        : "UP/DOWN select   LEFT/RIGHT difficulty   SPACE preview   ENTER accept   ESC back";
    DrawText(renderer, m_Font, hint, SCR_W / 2, SCR_H - 58, white, true);

    if (!m_StatusMessage.empty()) {
        DrawText(renderer, m_Font, m_StatusMessage, SCR_W / 2, SCR_H - 30, accent, true);
    }
}

} // namespace FNF