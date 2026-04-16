/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * JSON Loader Utility
 *
 * Thin wrapper around nlohmann/json that adds safe helpers
 * and logs errors instead of throwing exceptions everywhere.
 *
 * Author: LeninAsto
 * Date: March 2026
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <optional>
#include "../core/Logger.h"

namespace FNF {

using Json = nlohmann::json;

class JsonLoader {
public:
    /**
     * Load and parse a JSON file from disk.
     * Returns std::nullopt and logs an error on failure.
     */
    static std::optional<Json> LoadFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            Logger::Error("JsonLoader: cannot open file: " + path);
            return std::nullopt;
        }

        try {
            Json data = Json::parse(file, nullptr, /*exceptions=*/true, /*ignore_comments=*/true);
            return data;
        } catch (const Json::parse_error& e) {
            Logger::Error("JsonLoader: parse error in " + path + " -> " + e.what());
            return std::nullopt;
        }
    }

    /**
     * Safely get a value from a JSON object, returning a default if missing.
     */
    template<typename T>
    static T Get(const Json& obj, const std::string& key, const T& defaultValue) {
        if (obj.contains(key) && !obj[key].is_null()) {
            try {
                return obj[key].get<T>();
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    /**
     * Safely get a nested value using "parent.child" dot notation.
     */
    template<typename T>
    static T GetNested(const Json& obj, const std::string& dotPath, const T& defaultValue) {
        const Json* current = &obj;
        std::istringstream ss(dotPath);
        std::string token;

        while (std::getline(ss, token, '.')) {
            if (!current->is_object() || !current->contains(token)) {
                return defaultValue;
            }
            current = &(*current)[token];
        }

        try {
            return current->get<T>();
        } catch (...) {
            return defaultValue;
        }
    }
};

} // namespace FNF
