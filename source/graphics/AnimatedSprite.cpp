    /**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * AnimatedSprite Implementation
 * 
 * Author: LeninAsto
 * Date: March 2026
 */

#include "AnimatedSprite.h"
#include "Texture.h"
#include "../core/Logger.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace FNF {

// ---------------------------------------------------------------------------
// Sparrow XML helpers (minimal attribute extractor — no external parser needed)
// ---------------------------------------------------------------------------

static std::string XmlAttrStr(const std::string& line, const std::string& attr) {
    auto key = attr + "=\"";
    auto pos = line.find(key);
    if (pos == std::string::npos) return {};
    pos += key.size();
    auto end = line.find('"', pos);
    if (end == std::string::npos) return {};
    return line.substr(pos, end - pos);
}

static int XmlAttrInt(const std::string& line, const std::string& attr, int def = 0) {
    auto s = XmlAttrStr(line, attr);
    if (s.empty()) return def;
    try { return std::stoi(s); } catch (...) { return def; }
}

static bool XmlAttrBool(const std::string& line, const std::string& attr, bool def = false) {
    auto s = XmlAttrStr(line, attr);
    if (s.empty()) return def;

    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s == "true" || s == "1";
}

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------

bool AnimatedSprite::Load(SDL_Renderer* renderer,
                           const std::string& imagePath,
                           const std::string& xmlPath) {
    m_ImagePath = imagePath;
    m_Texture   = TextureCache::Load(renderer, imagePath);
    if (!m_Texture) return false;

    m_Frames.clear();
    m_Animations.clear();
    m_CurAnim     = {};
    m_CurFrameIdx = 0;
    m_FrameTimer  = 0.0f;

    if (xmlPath.empty()) return true;

    std::ifstream file(xmlPath);
    if (!file.is_open()) {
        Logger::Warn("AnimatedSprite: XML not found: " + xmlPath);
        return true; // Still OK as static sprite
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("<SubTexture") == std::string::npos) continue;

        SparrowFrame f;
        f.name   = XmlAttrStr(line, "name");
        f.x      = XmlAttrInt(line, "x");
        f.y      = XmlAttrInt(line, "y");
        f.w      = XmlAttrInt(line, "width");
        f.h      = XmlAttrInt(line, "height");
        f.rotated = XmlAttrBool(line, "rotated", false);
        f.frameX = XmlAttrInt(line, "frameX",      0);
        f.frameY = XmlAttrInt(line, "frameY",      0);
        f.frameW = XmlAttrInt(line, "frameWidth",  f.w);
        f.frameH = XmlAttrInt(line, "frameHeight", f.h);

        if (!f.name.empty() && f.w > 0 && f.h > 0) {
            m_Frames.push_back(std::move(f));
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// AddByPrefix
// ---------------------------------------------------------------------------

void AnimatedSprite::AddByPrefix(const std::string& animName,
                                  const std::string& prefix,
                                  int fps, bool loop) {
    FrameAnimation anim;
    anim.fps      = static_cast<float>(fps);
    anim.loop     = loop;
    anim.finished = false;

    for (int i = 0; i < static_cast<int>(m_Frames.size()); i++) {
        const auto& name = m_Frames[i].name;
        if (name.size() >= prefix.size() &&
            name.compare(0, prefix.size(), prefix) == 0) {
            anim.frameIndices.push_back(i);
        }
    }

    if (anim.frameIndices.empty()) {
        Logger::Warn("AnimatedSprite: no frames found for prefix '" + prefix + "'");
        return;
    }

    m_Animations[animName] = std::move(anim);
}

// ---------------------------------------------------------------------------
// AddByIndices
// ---------------------------------------------------------------------------

void AnimatedSprite::AddByIndices(const std::string& animName,
                                   const std::string& prefix,
                                   const std::vector<int>& indices,
                                   int fps, bool loop) {
    FrameAnimation anim;
    anim.fps      = static_cast<float>(fps);
    anim.loop     = loop;
    anim.finished = false;

    for (int idx : indices) {
        // Frame name = prefix + 4-digit zero-padded number (e.g. "gfDance0015")
        char pad[8];
        std::snprintf(pad, sizeof(pad), "%04d", idx);
        std::string target = prefix + pad;

        for (int i = 0; i < static_cast<int>(m_Frames.size()); i++) {
            if (m_Frames[i].name == target) {
                anim.frameIndices.push_back(i);
                break;
            }
        }
        // Missing frames are silently skipped (matches Flixel behaviour)
    }

    if (anim.frameIndices.empty()) {
        Logger::Warn("AnimatedSprite: no frames found for indices anim '" + animName + "'");
        return;
    }

    m_Animations[animName] = std::move(anim);
}

// ---------------------------------------------------------------------------
// Play
// ---------------------------------------------------------------------------

void AnimatedSprite::Play(const std::string& animName, bool forceRestart) {
    if (m_CurAnim == animName && !forceRestart) return;

    auto it = m_Animations.find(animName);
    if (it == m_Animations.end()) {
        Logger::Warn("AnimatedSprite: animation not found: '" + animName + "'");
        return;
    }

    m_CurAnim              = animName;
    m_CurFrameIdx          = 0;
    m_FrameTimer           = 0.0f;
    it->second.finished    = false;
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void AnimatedSprite::Update(float dt) {
    if (m_CurAnim.empty()) return;

    auto it = m_Animations.find(m_CurAnim);
    if (it == m_Animations.end()) return;

    FrameAnimation& anim = it->second;
    if (anim.frameIndices.empty()) return;
    if (anim.finished) return;

    if (anim.fps > 0.0f) {
        m_FrameTimer += dt;
        float frameTime = 1.0f / anim.fps;

        while (m_FrameTimer >= frameTime) {
            m_FrameTimer -= frameTime;
            m_CurFrameIdx++;

            if (m_CurFrameIdx >= static_cast<int>(anim.frameIndices.size())) {
                if (anim.loop) {
                    m_CurFrameIdx = 0;
                } else {
                    m_CurFrameIdx = static_cast<int>(anim.frameIndices.size()) - 1;
                    anim.finished = true;
                    return;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void AnimatedSprite::Draw(SDL_Renderer* renderer) const {
    Draw(renderer, nullptr);
}

void AnimatedSprite::Draw(SDL_Renderer* renderer, const SDL_Rect* clipRect) const {
    if (!visible || !m_Texture) return;

    SDL_Rect previousClip = {};
    const bool hadClipRect = SDL_RenderIsClipEnabled(renderer) == SDL_TRUE;
    if (hadClipRect) {
        SDL_RenderGetClipRect(renderer, &previousClip);
    }
    if (clipRect) {
        SDL_RenderSetClipRect(renderer, clipRect);
    }

    const SparrowFrame* sf = CurrentFrame();

    SDL_Rect src;
    int drawOffX = 0, drawOffY = 0; // offset from trim (positive values)

    // Flixel Sparrow convention:
    //   (x, y, width, height) = trimmed region inside the atlas texture
    //   frameX, frameY = negative offset of the trimmed region within the
    //                    original (un-trimmed) frame bounding box
    //   frameWidth, frameHeight = original un-trimmed frame size
    //
    // To reproduce a sprite at its correct position we must:
    //   - draw the trimmed region at  (sprite.x - frameX*scale, sprite.y - frameY*scale)
    //   - the dst rect sized to       (trimmedW * scale, trimmedH * scale)
    // GetWidth()/GetHeight() already return frameW*scale so centering is correct.

    if (sf) {
        src      = { sf->x, sf->y, sf->w, sf->h };
        // frameX / frameY are stored as negative values in the XML (e.g. -4, -10)
        // Subtracting them places the trimmed image at the right logical offset
        drawOffX = -(sf->frameX); // positive: shift right  (frameX is negative)
        drawOffY = -(sf->frameY); // positive: shift down   (frameY is negative)
    } else {
        // No atlas / no current frame — draw whole texture
        int tw, th;
        SDL_QueryTexture(m_Texture, nullptr, nullptr, &tw, &th);
        src = { 0, 0, tw, th };
    }

    // Clamp source rect to actual texture bounds.
    // Some Sparrow atlases are exported at lower resolution than the XML
    // coordinate space expects (e.g. titleEnter.png is ~69% of the XML size).
    // Without clamping, SDL stretches the available edge pixels to fill the
    // oversized destination, making sprites appear too wide / distorted.
    int texW = 0, texH = 0;
    SDL_QueryTexture(m_Texture, nullptr, nullptr, &texW, &texH);

    int clampedW = (src.x < texW) ? std::min(src.w, texW - src.x) : 0;
    int clampedH = (src.y < texH) ? std::min(src.h, texH - src.y) : 0;
    float ratioW = (src.w > 0 && clampedW < src.w)
                       ? static_cast<float>(clampedW) / static_cast<float>(src.w)
                       : 1.0f;
    float ratioH = (src.h > 0 && clampedH < src.h)
                       ? static_cast<float>(clampedH) / static_cast<float>(src.h)
                       : 1.0f;

    SDL_Rect clampedSrc = { src.x, src.y, clampedW, clampedH };

    SDL_SetTextureAlphaMod(m_Texture, static_cast<Uint8>(alpha * 255.0f));
    SDL_SetTextureColorMod(m_Texture, colorR, colorG, colorB);
    SDL_SetTextureBlendMode(m_Texture, SDL_BLENDMODE_BLEND);

    // dst: position accounts for trim offset; size scales by texture ratio
    // so the display width matches the actual available pixel content.
    SDL_Rect dst = {
        static_cast<int>(x + static_cast<float>(drawOffX) * scaleX),
        static_cast<int>(y + static_cast<float>(drawOffY) * scaleY),
        static_cast<int>(static_cast<float>(src.w) * scaleX * ratioW),
        static_cast<int>(static_cast<float>(src.h) * scaleY * ratioH)
    };

    double finalAngle = static_cast<double>(angle);
    if (sf && sf->rotated) {
        const int rotatedWidth = dst.h;
        const int rotatedHeight = dst.w;

        dst.x += (rotatedWidth - dst.w) / 2;
        dst.y += (rotatedHeight - dst.h) / 2;
        finalAngle -= 90.0;
    }

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (flipX) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_HORIZONTAL);
    if (flipY) flip = static_cast<SDL_RendererFlip>(flip | SDL_FLIP_VERTICAL);

    if (finalAngle != 0.0 || flip != SDL_FLIP_NONE) {
        SDL_RenderCopyEx(renderer, m_Texture, &clampedSrc, &dst,
                         finalAngle, nullptr, flip);
    } else {
        SDL_RenderCopy(renderer, m_Texture, &clampedSrc, &dst);
    }

    SDL_SetTextureAlphaMod(m_Texture, 255);
    SDL_SetTextureColorMod(m_Texture, 255, 255, 255);

    if (clipRect) {
        if (hadClipRect) {
            SDL_RenderSetClipRect(renderer, &previousClip);
        } else {
            SDL_RenderSetClipRect(renderer, nullptr);
        }
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

const SparrowFrame* AnimatedSprite::CurrentFrame() const {
    if (m_Frames.empty()) return nullptr;

    if (!m_CurAnim.empty()) {
        auto it = m_Animations.find(m_CurAnim);
        if (it != m_Animations.end()) {
            const FrameAnimation& anim = it->second;
            if (!anim.frameIndices.empty()) {
                int idx = anim.frameIndices[m_CurFrameIdx];
                if (idx >= 0 && idx < static_cast<int>(m_Frames.size())) {
                    return &m_Frames[idx];
                }
            }
        }
    }

    return &m_Frames[0]; // fallback to first frame
}

bool AnimatedSprite::IsFinished() const {
    if (m_CurAnim.empty()) return true;
    auto it = m_Animations.find(m_CurAnim);
    return (it == m_Animations.end()) || it->second.finished;
}

float AnimatedSprite::GetWidth() const {
    const SparrowFrame* sf = CurrentFrame();
    if (sf) return static_cast<float>(sf->frameW) * scaleX;
    int tw = 0;
    SDL_QueryTexture(m_Texture, nullptr, nullptr, &tw, nullptr);
    return static_cast<float>(tw) * scaleX;
}

float AnimatedSprite::GetHeight() const {
    const SparrowFrame* sf = CurrentFrame();
    if (sf) return static_cast<float>(sf->frameH) * scaleY;
    int th = 0;
    SDL_QueryTexture(m_Texture, nullptr, nullptr, nullptr, &th);
    return static_cast<float>(th) * scaleY;
}

void AnimatedSprite::ScreenCenter(int screenW, int screenH) {
    x = (screenW  - GetWidth())  * 0.5f;
    y = (screenH - GetHeight()) * 0.5f;
}

} // namespace FNF
