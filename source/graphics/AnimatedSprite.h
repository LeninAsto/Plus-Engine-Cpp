/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * AnimatedSprite - Sparrow Atlas Sprite with Frame Animations
 * 
 * Mirrors FlxSprite.animation:
 *   AddByPrefix  - add animation from frames whose name starts with a prefix
 *   AddByIndices - add animation from specific numbered frames
 *   Play         - play a named animation
 * 
 * Sparrow XML (Adobe Animate): each SubTexture has a source rect in the atlas
 * plus optional trim data (frameX/Y/Width/Height).
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace FNF {

// ---------------------------------------------------------------------------
// A single frame entry from a Sparrow atlas XML
// ---------------------------------------------------------------------------
struct SparrowFrame {
    std::string name;

    // Source rect within the atlas texture
    int x = 0, y = 0, w = 0, h = 0;

    // Trim offset: actual pixel content starts at (−frameX, −frameY)
    // within a full frame of (frameW x frameH)
    int frameX = 0, frameY = 0;
    int frameW = 0, frameH = 0; // original (un-trimmed) frame size
};

// ---------------------------------------------------------------------------
// A named animation: sequence of frame indices + playback settings
// ---------------------------------------------------------------------------
struct FrameAnimation {
    std::vector<int> frameIndices; // into m_Frames
    float fps       = 24.0f;
    bool  loop      = true;
    bool  finished  = false;
};

// ---------------------------------------------------------------------------
// AnimatedSprite
// ---------------------------------------------------------------------------
class AnimatedSprite {
public:
    float x      = 0.0f;
    float y      = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float alpha  = 1.0f;
    float angle  = 0.0f;
    bool  visible = true;

    Uint8 colorR = 255, colorG = 255, colorB = 255;

    /**
     * Load atlas: PNG texture + Sparrow XML.
     * The XML path may be empty if no animation is needed (static sprite).
     */
    bool Load(SDL_Renderer* renderer,
              const std::string& imagePath,
              const std::string& xmlPath = "");

    /**
     * Add animation using all frames whose name STARTS WITH the given prefix,
     * taken in the order they appear in the XML.
     */
    void AddByPrefix(const std::string& animName,
                     const std::string& prefix,
                     int fps = 24, bool loop = true);

    /**
     * Add animation using specific frame numbers (e.g. [15,16,17]).
     * The actual frame looked up is: prefix + zero-padded-index (4 digits).
     * Frames not found in the atlas are silently skipped.
     */
    void AddByIndices(const std::string& animName,
                      const std::string& prefix,
                      const std::vector<int>& indices,
                      int fps = 24, bool loop = false);

    /**
     * Start playing a named animation.
     * @param forceRestart  true = restart even if already playing this anim
     */
    void Play(const std::string& animName, bool forceRestart = false);

    /** Update animation playback — call every frame */
    void Update(float dt);

    /** Draw to renderer */
    void Draw(SDL_Renderer* renderer) const;

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------
    bool IsLoaded()   const { return m_Texture != nullptr; }
    bool IsFinished() const;
    bool IsPlaying(const std::string& animName) const { return m_CurAnim == animName; }

    /** Effective (un-trimmed) width of the current frame × scaleX */
    float GetWidth()  const;
    /** Effective (un-trimmed) height of the current frame × scaleY */
    float GetHeight() const;

    /** Center on screen (default 1280×720) */
    void ScreenCenter(int screenW = 1280, int screenH = 720);

    void SetScale(float s) { scaleX = scaleY = s; }

private:
    SDL_Texture*                                 m_Texture    = nullptr;
    std::string                                  m_ImagePath;
    std::vector<SparrowFrame>                    m_Frames;
    std::unordered_map<std::string, FrameAnimation> m_Animations;

    std::string  m_CurAnim;
    int          m_CurFrameIdx = 0;
    float        m_FrameTimer  = 0.0f;

    // Returns the SparrowFrame for the current animation frame (or null)
    const SparrowFrame* CurrentFrame() const;
};

} // namespace FNF
