#include "Note.h"

#include "../data/Paths.h"
#include "../graphics/RGBPalette.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace FNF {

bool Note::s_TemplatesLoaded = false;
std::array<AnimatedSprite, 4> Note::s_HeadTemplates = {};
std::array<AnimatedSprite, 4> Note::s_BodyTemplates = {};
std::array<AnimatedSprite, 4> Note::s_TailTemplates = {};

namespace {

const std::array<std::string, 4> kHeadPrefixes = {
    "purple",
    "blue",
    "green",
    "red"
};

const std::array<std::string, 4> kBodyPrefixes = {
    "purple hold piece",
    "blue hold piece",
    "green hold piece",
    "red hold piece"
};

const std::array<std::string, 4> kTailPrefixes = {
    "pruple end hold",
    "blue hold end",
    "green hold end",
    "red hold end"
};

}

bool Note::EnsureTemplates(SDL_Renderer* renderer) {
    if (s_TemplatesLoaded) {
        return true;
    }

    std::string imagePath = Paths::Image("noteSkins/NOTE_assets");
    std::string xmlPath = Paths::Xml("noteSkins/NOTE_assets");
    if (imagePath.empty() || xmlPath.empty()) {
        imagePath = Paths::Image("NOTE_assets");
        xmlPath = Paths::Xml("NOTE_assets");
    }

    for (int lane = 0; lane < 4; ++lane) {
        if (!s_HeadTemplates[lane].Load(renderer, imagePath, xmlPath)) {
            return false;
        }
        if (!s_BodyTemplates[lane].Load(renderer, imagePath, xmlPath)) {
            return false;
        }
        if (!s_TailTemplates[lane].Load(renderer, imagePath, xmlPath)) {
            return false;
        }

        s_HeadTemplates[lane].scaleX = kScale;
        s_HeadTemplates[lane].scaleY = kScale;
        s_BodyTemplates[lane].scaleX = kScale;
        s_BodyTemplates[lane].scaleY = kScale;
        s_TailTemplates[lane].scaleX = kScale;
        s_TailTemplates[lane].scaleY = kScale;

        s_HeadTemplates[lane].AddByIndices("idle", kHeadPrefixes[lane], { 0 }, 24, false);
        s_BodyTemplates[lane].AddByIndices("idle", kBodyPrefixes[lane], { 0 }, 24, false);
        s_TailTemplates[lane].AddByIndices("idle", kTailPrefixes[lane], { 0 }, 24, false);

        RGBPalette::ApplyLaneTint(s_HeadTemplates[lane], lane, 1.0f);
        RGBPalette::ApplyLaneTint(s_BodyTemplates[lane], lane, 0.92f);
        RGBPalette::ApplyLaneTint(s_TailTemplates[lane], lane, 1.08f);

        s_HeadTemplates[lane].Play("idle", true);
        s_BodyTemplates[lane].Play("idle", true);
        s_TailTemplates[lane].Play("idle", true);
    }

    s_TemplatesLoaded = true;
    return true;
}

bool Note::Load(SDL_Renderer* renderer, int lane, bool mustHit, float strumTime, float sustainLength, bool upScroll) {
    if (!EnsureTemplates(renderer)) {
        return false;
    }

    m_Lane = lane;
    m_MustHit = mustHit;
    m_StrumTime = strumTime;
    m_SustainLength = std::max(0.0f, sustainLength);
    m_Alive = true;
    m_SustainPixels = 0.0f;
    m_UpScroll = upScroll;

    m_Head = s_HeadTemplates[lane];
    m_Body = s_BodyTemplates[lane];
    m_Tail = s_TailTemplates[lane];
    return true;
}

void Note::SetStrumAnchor(const StrumNote& strum) {
    m_StrumX = strum.GetX();
    m_StrumY = strum.GetY();
    m_StrumWidth = strum.GetWidth();
}

void Note::Refresh(float songPositionMs, float pixelsPerMs) {
    const float deltaMs = m_StrumTime - songPositionMs;
    const float direction = m_UpScroll ? 1.0f : -1.0f;
    const float headY = m_StrumY + deltaMs * pixelsPerMs * direction;

    m_SustainPixels = m_SustainLength * pixelsPerMs;
    m_Head.x = m_StrumX + (m_StrumWidth - m_Head.GetWidth()) * 0.5f;
    m_Head.y = headY;

    m_Body.x = m_StrumX + (m_StrumWidth - m_Body.GetWidth()) * 0.5f;
    m_Tail.x = m_StrumX + (m_StrumWidth - m_Tail.GetWidth()) * 0.5f;
}

void Note::Draw(SDL_Renderer* renderer) const {
    if (!m_Alive) {
        return;
    }

    if (m_Head.y < -1024.0f || m_Head.y > 2048.0f) {
        return;
    }

    if (HasSustain() && m_SustainPixels > 1.0f) {
        const float bodyHeight = std::max(1.0f, m_SustainPixels);
        const float baseBodyHeight = std::max(1.0f, m_Body.GetHeight());

        AnimatedSprite body = m_Body;
        body.scaleY = m_Body.scaleY * (bodyHeight / baseBodyHeight);

        const float bodyY = m_UpScroll
            ? (m_Head.y + m_Head.GetHeight() * 0.42f)
            : (m_Head.y - bodyHeight + m_Head.GetHeight() * 0.1f);
        body.y = bodyY;

        SDL_Rect clipRect = {
            static_cast<int>(std::floor(body.x)),
            static_cast<int>(std::floor(bodyY)),
            static_cast<int>(std::ceil(body.GetWidth())),
            static_cast<int>(std::ceil(bodyHeight))
        };
        body.Draw(renderer, &clipRect);

        AnimatedSprite tail = m_Tail;
        tail.y = m_UpScroll
            ? (bodyY + bodyHeight - tail.GetHeight() * 0.28f)
            : (bodyY - tail.GetHeight() * 0.72f);
        tail.Draw(renderer);
    }

    m_Head.Draw(renderer);
}

bool Note::CanBeHit(float songPositionMs, float hitWindowMs) const {
    return m_Alive && std::abs(songPositionMs - m_StrumTime) <= hitWindowMs;
}

bool Note::IsLate(float songPositionMs, float lateWindowMs) const {
    return m_Alive && songPositionMs > (m_StrumTime + lateWindowMs);
}

void Note::MarkHit() {
    m_Alive = false;
}

void Note::MarkMissed() {
    m_Alive = false;
}

void Note::InvalidateSharedResources() {
    s_TemplatesLoaded = false;
    s_HeadTemplates = {};
    s_BodyTemplates = {};
    s_TailTemplates = {};
}

} // namespace FNF