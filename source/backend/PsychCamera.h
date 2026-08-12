/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * PsychCamera - Small camera layer matching Psych's camGame/camHUD/camOther idea
 */

#pragma once

namespace FNF {

struct PsychCamera {
    float scrollX = 0.0f;
    float scrollY = 0.0f;
    float zoom = 1.0f;
    float alpha = 1.0f;
    bool visible = true;

    float ScreenX(float worldX) const {
        return (worldX - scrollX) * zoom;
    }

    float ScreenY(float worldY) const {
        return (worldY - scrollY) * zoom;
    }
};

} // namespace FNF
