/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Transform2D - Shared sprite transform data
 *
 * Keeps the public field names the current engine already uses while giving
 * Sprite and AnimatedSprite one common transform model.
 */

#pragma once

namespace FNF {

struct Transform2D {
    float x      = 0.0f;
    float y      = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float alpha  = 1.0f;
    float angle  = 0.0f; // degrees
    bool  flipX  = false;
    bool  flipY  = false;

    bool transformDirty = true;

    void SetPosition(float nextX, float nextY) {
        if (x == nextX && y == nextY) return;
        x = nextX;
        y = nextY;
        transformDirty = true;
    }

    void SetScale(float s) {
        SetScale(s, s);
    }

    void SetScale(float nextScaleX, float nextScaleY) {
        if (scaleX == nextScaleX && scaleY == nextScaleY) return;
        scaleX = nextScaleX;
        scaleY = nextScaleY;
        transformDirty = true;
    }

    void SetAngle(float nextAngle) {
        if (angle == nextAngle) return;
        angle = nextAngle;
        transformDirty = true;
    }
};

} // namespace FNF
