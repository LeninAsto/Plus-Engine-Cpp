/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Stage implementation.
 */

#include "Stage.h"
#include "../backend/Logger.h"
#include "../backend/JsonLoader.h"
#include "../backend/Paths.h"
#include "../objects/Texture.h"

#include <cctype>

namespace {

std::string StripImagesPrefix(const std::string& key) {
    if (key.rfind("images/", 0) == 0) {
        return key.substr(7);
    }
    return key;
}

void PushFallbackObject(std::vector<FNF::StageData::ObjectData>& objects,
                        const std::string& image,
                        float x, float y,
                        float scaleX, float scaleY,
                        bool foreground,
                        float alpha = 1.0f,
                        bool flipX = false,
                        bool flipY = false,
                        float angle = 0.0f) {
    FNF::StageData::ObjectData object;
    object.image = image;
    object.x = x;
    object.y = y;
    object.scaleX = scaleX;
    object.scaleY = scaleY;
    object.alpha = alpha;
    object.flipX = flipX;
    object.flipY = flipY;
    object.angle = angle;
    object.foreground = foreground;
    objects.push_back(object);
}

std::string NormalizeSongKey(const std::string& value) {
    std::string out;
    out.reserve(value.size());

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

FNF::StageData MakeHardcodedStageData(const std::string& stageName) {
    FNF::StageData out;
    out.stageName = stageName;
    out.directory = "week1";
    out.stageUI = "normal";
    out.defaultZoom = 0.9f;
    out.boyfriendX = 770.0f;
    out.boyfriendY = 100.0f;
    out.girlfriendX = 400.0f;
    out.girlfriendY = 130.0f;
    out.opponentX = 100.0f;
    out.opponentY = 100.0f;
    out.cameraSpeed = 1.0f;
    out.hideGirlfriend = false;

    if (stageName == "stage") {
        PushFallbackObject(out.objects, "stageback", -600.0f, -200.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "stagefront", -650.0f, 600.0f, 1.1f, 1.1f, false);
        PushFallbackObject(out.objects, "stage_light", -125.0f, -100.0f, 1.1f, 1.1f, false);
        PushFallbackObject(out.objects, "stage_light", 1225.0f, -100.0f, 1.1f, 1.1f, false, 1.0f, true);
        PushFallbackObject(out.objects, "stagecurtains", -500.0f, -300.0f, 0.9f, 0.9f, true);
    }

    if (stageName == "spooky") {
        out.directory = "week2";
        out.defaultZoom = 1.05f;
        PushFallbackObject(out.objects, "halloween_bg", -200.0f, -100.0f, 1.0f, 1.0f, false);
    } else if (stageName == "spookyMansionErect") {
        out.directory = "week2";
        out.defaultZoom = 1.0f;
        out.boyfriendX = 1030.0f;
        out.boyfriendY = 70.0f;
        out.girlfriendX = 440.0f;
        out.cameraBoyfriendX = -50.0f;
        out.cameraBoyfriendY = 10.0f;
    } else if (stageName == "philly") {
        out.defaultZoom = 1.05f;
    } else if (stageName == "phillyTrainErect") {
        out.defaultZoom = 1.05f;
        out.cameraOpponentX = 30.0f;
    } else if (stageName == "limo") {
        out.boyfriendX = 1030.0f;
        out.boyfriendY = -120.0f;
        out.cameraBoyfriendX = -200.0f;
    } else if (stageName == "limoRideErect") {
        out.boyfriendX = 1050.0f;
        out.boyfriendY = -160.0f;
        out.girlfriendY = 150.0f;
        out.cameraBoyfriendX = -200.0f;
    } else if (stageName == "mall") {
        out.defaultZoom = 0.8f;
        out.boyfriendX = 970.0f;
        out.cameraBoyfriendY = -100.0f;
    } else if (stageName == "mallEvil") {
        out.defaultZoom = 1.05f;
        out.boyfriendX = 1090.0f;
        out.opponentY = 20.0f;
    } else if (stageName == "mallXmasErect") {
        out.defaultZoom = 0.8f;
        out.boyfriendX = 970.0f;
        out.cameraBoyfriendY = -100.0f;
    } else if (stageName == "school") {
        out.defaultZoom = 1.05f;
        out.stageUI = "pixel";
        out.boyfriendX = 970.0f;
        out.boyfriendY = 320.0f;
        out.girlfriendX = 580.0f;
        out.girlfriendY = 430.0f;
        out.cameraBoyfriendX = -100.0f;
        out.cameraBoyfriendY = -100.0f;
    } else if (stageName == "schoolEvil") {
        out.defaultZoom = 1.05f;
        out.stageUI = "pixel";
        out.boyfriendX = 970.0f;
        out.boyfriendY = 320.0f;
        out.girlfriendX = 580.0f;
        out.girlfriendY = 430.0f;
        out.cameraBoyfriendX = -100.0f;
        out.cameraBoyfriendY = -100.0f;
    } else if (stageName == "schoolErect") {
        out.defaultZoom = 1.05f;
        out.stageUI = "pixel";
        out.boyfriendX = 1216.0f;
        out.boyfriendY = 292.0f;
        out.girlfriendX = 680.0f;
        out.girlfriendY = 370.0f;
        out.cameraBoyfriendX = -400.0f;
        out.cameraBoyfriendY = -130.0f;
    } else if (stageName == "schoolEvilErect") {
        out.defaultZoom = 1.05f;
        out.stageUI = "pixel";
        out.boyfriendX = 970.0f;
        out.boyfriendY = 320.0f;
        out.girlfriendX = 580.0f;
        out.girlfriendY = 430.0f;
        out.cameraBoyfriendX = -200.0f;
        out.cameraBoyfriendY = -94.0f;
        out.cameraGirlfriendX = 50.0f;
        out.cameraGirlfriendY = 50.0f;
    } else if (stageName == "tank") {
        out.boyfriendX = 810.0f;
        out.girlfriendX = 200.0f;
        out.girlfriendY = 65.0f;
        out.opponentX = 20.0f;
    } else if (stageName == "tankmanBattlefieldErect") {
        out.defaultZoom = 0.7f;
        out.boyfriendX = 895.0f;
        out.boyfriendY = -30.0f;
        out.girlfriendX = 280.0f;
        out.girlfriendY = 0.0f;
        out.opponentX = -215.0f;
        out.opponentY = -70.0f;
        out.cameraBoyfriendX = -185.0f;
        out.cameraBoyfriendY = -80.0f;
        out.cameraOpponentX = 140.0f;
        out.cameraOpponentY = -40.0f;
    } else if (stageName == "phillyStreets") {
        out.defaultZoom = 0.77f;
        out.directory = "weekend1";
        out.boyfriendX = 1930.0f;
        out.boyfriendY = 450.0f;
        out.girlfriendX = 1175.0f;
        out.girlfriendY = 200.0f;
        out.opponentX = 620.0f;
        out.opponentY = 445.0f;
        out.cameraBoyfriendX = -220.0f;
        out.cameraBoyfriendY = -15.0f;
        out.cameraOpponentX = 450.0f;
        out.cameraOpponentY = -110.0f;
        PushFallbackObject(out.objects, "phillyStreets/phillySkybox", -650.0f, -375.0f, 0.65f, 0.65f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillySkyline", -545.0f, -273.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillyForegroundCity", 625.0f, 94.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillyConstruction", 1800.0f, 364.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillyHighwayLights", 284.0f, 305.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillyHighway", 139.0f, 209.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "phillyStreets/phillyForeground", 88.0f, 317.0f, 1.0f, 1.0f, true);
    } else if (stageName == "phillyStreetsErect") {
        out.defaultZoom = 0.77f;
        out.boyfriendX = 1930.0f;
        out.boyfriendY = 450.0f;
        out.girlfriendX = 1175.0f;
        out.girlfriendY = 200.0f;
        out.opponentX = 620.0f;
        out.opponentY = 445.0f;
        out.cameraBoyfriendX = -220.0f;
        out.cameraBoyfriendY = -15.0f;
        out.cameraOpponentX = 450.0f;
        out.cameraOpponentY = -110.0f;
    } else if (stageName == "phillyBlazin") {
        out.defaultZoom = 0.75f;
        out.boyfriendX = 900.0f;
        out.boyfriendY = 800.0f;
        out.girlfriendX = 470.0f;
        out.girlfriendY = 595.0f;
        out.opponentX = 300.0f;
        out.opponentY = 800.0f;
        out.cameraOpponentX = 50.0f;
        out.cameraOpponentY = -455.0f;
    } else if (stageName == "mainStageErect") {
        out.defaultZoom = 1.0f;
        out.boyfriendX = 747.5f;
        out.boyfriendY = 80.0f;
        out.girlfriendX = 280.0f;
        out.girlfriendY = 150.0f;
        out.opponentX = -250.0f;
        out.opponentY = 90.0f;
        out.cameraBoyfriendX = -150.0f;
        out.cameraOpponentX = 150.0f;
    } else if (stageName == "sserafim") {
        out.defaultZoom = 0.5f;
        out.directory = "sserafim";
        out.boyfriendX = 1250.0f;
        out.boyfriendY = 242.0f;
        out.girlfriendX = 700.0f;
        out.girlfriendY = -250.0f;
        out.opponentX = -830.0f;
        out.opponentY = -225.0f;
    }

    return out;
}

} // namespace

namespace FNF {

std::string StageScene::VanillaSongStage(const std::string& songName) {
    const std::string normalized = NormalizeSongKey(songName);

    if (normalized == "spookeez" || normalized == "south" || normalized == "monster") return "spooky";
    if (normalized == "spookeez-erect") return "spookyMansionErect";
    if (normalized == "pico" || normalized == "blammed" || normalized == "philly" || normalized == "philly-nice") return "philly";
    if (normalized == "pico-erect" || normalized == "blammed-erect" || normalized == "philly-nice-erect") return "phillyTrainErect";
    if (normalized == "milf" || normalized == "satin-panties" || normalized == "high") return "limo";
    if (normalized == "satin-panties-erect" || normalized == "high-erect") return "limoRideErect";
    if (normalized == "cocoa" || normalized == "eggnog") return "mall";
    if (normalized == "cocoa-erect" || normalized == "eggnog-erect") return "mallXmasErect";
    if (normalized == "winter-horrorland") return "mallEvil";
    if (normalized == "senpai" || normalized == "roses") return "school";
    if (normalized == "senpai-erect" || normalized == "roses-erect") return "schoolErect";
    if (normalized == "thorns") return "schoolEvil";
    if (normalized == "thorns-erect") return "schoolEvilErect";
    if (normalized == "ugh" || normalized == "guns" || normalized == "stress") return "tank";
    if (normalized == "ugh-erect" || normalized == "guns-erect" || normalized == "stress-erect") return "tankmanBattlefieldErect";
    if (normalized == "darnell" || normalized == "lit-up" || normalized == "2hot" || normalized == "blazin") return "phillyStreets";
    if (normalized == "darnell-erect" || normalized == "lit-up-erect" || normalized == "2hot-erect" || normalized == "blazin-erect") return "phillyStreetsErect";
    if (normalized == "dadbattle-erect" || normalized == "dad-battle-erect") return "mainStageErect";
    return "stage";
}

std::optional<StageData> StageScene::LoadData(const std::string& stageName) {
    StageData out = MakeHardcodedStageData(stageName);
    const std::string stagePath = Paths::StageData(stageName);
    if (stagePath.empty()) {
        Logger::Warn("[StageScene] Stage JSON not found: " + stageName);
        return out;
    }

    auto json = JsonLoader::LoadFile(stagePath);
    if (!json.has_value()) {
        return out;
    }

    out.stageName = stageName;
    out.directory = JsonLoader::Get(*json, "directory", out.directory);
    out.stageUI = JsonLoader::Get(*json, "stageUI", out.stageUI);
    out.defaultZoom = JsonLoader::Get(*json, "defaultZoom", out.defaultZoom);
    out.hideGirlfriend = JsonLoader::Get(*json, "hide_girlfriend", out.hideGirlfriend);

    if (json->contains("boyfriend") && (*json)["boyfriend"].is_array() && (*json)["boyfriend"].size() >= 2) {
        out.boyfriendX = (*json)["boyfriend"][0].get<float>();
        out.boyfriendY = (*json)["boyfriend"][1].get<float>();
    }
    if (json->contains("opponent") && (*json)["opponent"].is_array() && (*json)["opponent"].size() >= 2) {
        out.opponentX = (*json)["opponent"][0].get<float>();
        out.opponentY = (*json)["opponent"][1].get<float>();
    }
    if (json->contains("girlfriend") && (*json)["girlfriend"].is_array() && (*json)["girlfriend"].size() >= 2) {
        out.girlfriendX = (*json)["girlfriend"][0].get<float>();
        out.girlfriendY = (*json)["girlfriend"][1].get<float>();
    }
    if (json->contains("camera_boyfriend") && (*json)["camera_boyfriend"].is_array() && (*json)["camera_boyfriend"].size() >= 2) {
        out.cameraBoyfriendX = (*json)["camera_boyfriend"][0].get<float>();
        out.cameraBoyfriendY = (*json)["camera_boyfriend"][1].get<float>();
    }
    if (json->contains("camera_opponent") && (*json)["camera_opponent"].is_array() && (*json)["camera_opponent"].size() >= 2) {
        out.cameraOpponentX = (*json)["camera_opponent"][0].get<float>();
        out.cameraOpponentY = (*json)["camera_opponent"][1].get<float>();
    }
    if (json->contains("camera_girlfriend") && (*json)["camera_girlfriend"].is_array() && (*json)["camera_girlfriend"].size() >= 2) {
        out.cameraGirlfriendX = (*json)["camera_girlfriend"][0].get<float>();
        out.cameraGirlfriendY = (*json)["camera_girlfriend"][1].get<float>();
    }
    out.cameraSpeed = JsonLoader::Get(*json, "camera_speed", out.cameraSpeed);

    if (json->contains("preload") && (*json)["preload"].is_object()) {
        for (auto it = (*json)["preload"].begin(); it != (*json)["preload"].end(); ++it) {
            out.preloadImages.push_back(StripImagesPrefix(it.key()));
        }
    }

    if (json->contains("objects") && (*json)["objects"].is_array()) {
        for (const auto& objectJson : (*json)["objects"]) {
            if (!objectJson.is_object()) {
                continue;
            }

            StageData::ObjectData object;
            object.image = StripImagesPrefix(JsonLoader::Get(objectJson, "image", std::string()));
            object.x = JsonLoader::Get(objectJson, "x", 0.0f);
            object.y = JsonLoader::Get(objectJson, "y", 0.0f);
            object.scaleX = JsonLoader::GetNested(objectJson, "scale.x", 1.0f);
            object.scaleY = JsonLoader::GetNested(objectJson, "scale.y", 1.0f);
            object.alpha = JsonLoader::Get(objectJson, "alpha", 1.0f);
            object.angle = JsonLoader::Get(objectJson, "angle", 0.0f);
            object.flipX = JsonLoader::Get(objectJson, "flipX", JsonLoader::Get(objectJson, "flip_x", false));
            object.flipY = JsonLoader::Get(objectJson, "flipY", JsonLoader::Get(objectJson, "flip_y", false));
            object.foreground = JsonLoader::Get(objectJson, "foreground", JsonLoader::Get(objectJson, "onFront", false));
            object.antialiasing = !JsonLoader::Get(objectJson, "no_antialiasing", false);
            if (!object.image.empty()) {
                out.objects.push_back(std::move(object));
            }
        }
    }

    if (out.objects.empty()) {
        PushFallbackObject(out.objects, "stageback", -600.0f, -200.0f, 1.0f, 1.0f, false);
        PushFallbackObject(out.objects, "stagefront", -650.0f, 600.0f, 1.1f, 1.1f, false);
        PushFallbackObject(out.objects, "stagecurtains", -500.0f, -300.0f, 0.9f, 0.9f, true);
    }

    return out;
}

bool StageScene::Precache(SDL_Renderer* renderer, const StageData& data) {
    for (const auto& key : data.preloadImages) {
        std::string imagePath = Paths::Image(key, data.directory);
        if (imagePath.empty()) {
            imagePath = Paths::Image(key);
        }
        if (!imagePath.empty()) {
            TextureCache::Load(renderer, imagePath);
        }
    }
    return true;
}

bool StageScene::PrecacheGL(const StageData& data) {
    for (const auto& key : data.preloadImages) {
        std::string imagePath = Paths::Image(key, data.directory);
        if (imagePath.empty()) {
            imagePath = Paths::Image(key);
        }
        if (!imagePath.empty()) {
            TextureCache::LoadGL(imagePath);
        }
    }
    return true;
}

bool StageScene::Load(SDL_Renderer* renderer, const StageData& data) {
    m_Sprites.clear();

    for (const StageData::ObjectData& object : data.objects) {
        std::string imagePath = Paths::Image(object.image, data.directory);
        if (imagePath.empty()) {
            imagePath = Paths::Image(object.image);
        }
        if (imagePath.empty()) {
            continue;
        }

        StageSprite stageSprite;
        if (!stageSprite.sprite.Load(renderer, imagePath)) {
            continue;
        }

        stageSprite.sprite.x = object.x;
        stageSprite.sprite.y = object.y;
        stageSprite.sprite.scaleX = object.scaleX;
        stageSprite.sprite.scaleY = object.scaleY;
        stageSprite.sprite.alpha = object.alpha;
        stageSprite.sprite.angle = object.angle;
        stageSprite.sprite.flipX = object.flipX;
        stageSprite.sprite.flipY = object.flipY;
        stageSprite.foreground = object.foreground;
        m_Sprites.push_back(std::move(stageSprite));
    }

    return !m_Sprites.empty();
}

bool StageScene::LoadGL(const StageData& data) {
    m_Sprites.clear();

    for (const StageData::ObjectData& object : data.objects) {
        std::string imagePath = Paths::Image(object.image, data.directory);
        if (imagePath.empty()) {
            imagePath = Paths::Image(object.image);
        }
        if (imagePath.empty()) {
            continue;
        }

        StageSprite stageSprite;
        if (!stageSprite.sprite.LoadGL(imagePath)) {
            continue;
        }

        stageSprite.sprite.x = object.x;
        stageSprite.sprite.y = object.y;
        stageSprite.sprite.scaleX = object.scaleX;
        stageSprite.sprite.scaleY = object.scaleY;
        stageSprite.sprite.alpha = object.alpha;
        stageSprite.sprite.angle = object.angle;
        stageSprite.sprite.flipX = object.flipX;
        stageSprite.sprite.flipY = object.flipY;
        stageSprite.foreground = object.foreground;
        m_Sprites.push_back(std::move(stageSprite));
    }

    return !m_Sprites.empty();
}

void StageScene::Draw(SDL_Renderer* renderer) const {
    Draw(renderer, 0.0f, 0.0f, 1.0f);
}

void StageScene::Draw(SDL_Renderer* renderer, float cameraX, float cameraY, float zoom) const {
    for (const StageSprite& sprite : m_Sprites) {
        if (!sprite.foreground) {
            sprite.sprite.Draw(renderer, cameraX, cameraY, zoom);
        }
    }
    for (const StageSprite& sprite : m_Sprites) {
        if (sprite.foreground) {
            sprite.sprite.Draw(renderer, cameraX, cameraY, zoom);
        }
    }
}

void StageScene::DrawGL(OpenGLESBackend& backend) const {
    DrawGL(backend, 0.0f, 0.0f, 1.0f);
}

void StageScene::DrawGL(OpenGLESBackend& backend, float cameraX, float cameraY, float zoom) const {
    for (const StageSprite& sprite : m_Sprites) {
        if (!sprite.foreground) {
            sprite.sprite.DrawGL(backend, cameraX, cameraY, zoom);
        }
    }
    for (const StageSprite& sprite : m_Sprites) {
        if (sprite.foreground) {
            sprite.sprite.DrawGL(backend, cameraX, cameraY, zoom);
        }
    }
}

} // namespace FNF
