/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Asset Path Manager
 *
 * Handles resolving paths to game assets across all asset folders.
 * Mirrors the behavior of Paths.hx from the Haxe version.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace FNF {

class Paths {
public:
    /**
     * Initialize Paths system with the root assets directory.
     * Should be called once at startup.
     * @param assetsRoot  Absolute or relative path to the "assets/" folder
     */
    static void Init(const std::string& assetsRoot = "assets");

    // =====================================================================
    // Audio
    // =====================================================================

    /** Instrumental track for a song (prefers .ogg, falls back to .mp3) */
    static std::string Inst(const std::string& songName);

    /** Voices track for a song */
    static std::string Voices(const std::string& songName, const std::string& suffix = "");

    /** A generic sound from assets/shared/sounds/ */
    static std::string Sound(const std::string& key);

    /** Background music from assets/shared/music/ */
    static std::string Music(const std::string& key);

    // =====================================================================
    // Images / Sprites
    // =====================================================================

    /** A sprite sheet image (.png) */
    static std::string Image(const std::string& key, const std::string& library = "shared");

    /** XML sprite sheet data */
    static std::string Xml(const std::string& key, const std::string& library = "shared");

    /** JSON sprite sheet data (Animate-style) */
    static std::string ImageJson(const std::string& key, const std::string& library = "shared");

    // =====================================================================
    // Data / Charts
    // =====================================================================

    /** Chart JSON for a song (e.g. tutorial.json) */
    static std::string SongData(const std::string& songName, const std::string& difficulty = "");

    /** Chart metadata JSON */
    static std::string SongMeta(const std::string& songName);

    /** Character definition JSON from assets/shared/characters/ */
    static std::string CharacterData(const std::string& charName);

    /** Stage definition JSON */
    static std::string StageData(const std::string& stageName);

    /** Week definition JSON */
    static std::string WeekData(const std::string& weekName);

    /** Specific chart file stem inside a song folder */
    static std::string SongVariantData(const std::string& songFolder, const std::string& fileStem);

    // =====================================================================
    // Fonts
    // =====================================================================

    static std::string Font(const std::string& key);

    // =====================================================================
    // Helpers
    // =====================================================================

    /** Returns true if the file exists on disk */
    static bool Exists(const std::string& path);

    /** Get the assets root directory */
    static const std::string& GetRoot() { return s_AssetsRoot; }

    /** Resolve the first relative path that exists */
    static std::string ResolveFirstExisting(const std::vector<std::string>& relatives);

private:
    static std::string s_AssetsRoot;

    /**
     * Build a full path from parts, trying .ogg then .mp3 for audio files.
     * Returns empty string if nothing is found.
     */
    static std::string ResolvePath(const std::string& relative);
    static std::string ResolveAudio(const std::string& relative);
};

} // namespace FNF
