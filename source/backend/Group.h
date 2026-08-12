/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Group - Small FlxGroup-style owner for Basic objects
 *
 * Groups let states, notes, splashes, menus, and effects update/draw through
 * the same lifecycle rules instead of each system hand-rolling loops.
 */

#pragma once

#include "Basic.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace FNF {

class Group : public Basic {
public:
    using Ptr = std::unique_ptr<Basic>;

    Basic* Add(Ptr object) {
        if (!object) return nullptr;
        Basic* raw = object.get();
        m_Members.push_back(std::move(object));
        return raw;
    }

    void Remove(Basic* object, bool destroy = false) {
        auto it = std::find_if(m_Members.begin(), m_Members.end(),
            [object](const Ptr& member) { return member.get() == object; });

        if (it == m_Members.end()) return;
        if (destroy && *it) (*it)->Destroy();
        m_Members.erase(it);
    }

    void Clear(bool destroy = false) {
        if (destroy) {
            for (auto& member : m_Members) {
                if (member) member->Destroy();
            }
        }
        m_Members.clear();
    }

    void Update(float dt) override {
        if (!ShouldUpdate()) return;

        for (auto& member : m_Members) {
            if (member && member->ShouldUpdate()) {
                member->Update(dt);
            }
        }
    }

    void Draw(SDL_Renderer* renderer) const override {
        if (!ShouldDraw()) return;

        for (const auto& member : m_Members) {
            if (member && member->ShouldDraw()) {
                member->Draw(renderer);
            }
        }
    }

    void DrawGL(OpenGLESBackend& backend) const override {
        if (!ShouldDraw()) return;

        for (const auto& member : m_Members) {
            if (member && member->ShouldDraw()) {
                member->DrawGL(backend);
            }
        }
    }

    std::size_t Size() const { return m_Members.size(); }
    bool Empty() const { return m_Members.empty(); }

protected:
    std::vector<Ptr> m_Members;
};

} // namespace FNF
