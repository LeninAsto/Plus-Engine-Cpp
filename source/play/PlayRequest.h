/**
 * Friday Night Funkin' Plus Engine - C++ Rewrite
 * Shared request data for LoadingState -> PlayState.
 */

#pragma once

#include <string>

namespace FNF {

struct PlayRequest {
    std::string songName;
    std::string difficultyName;
    std::string chartPath;
    std::string instPath;
    std::string fallbackStage = "stage";
};

} // namespace FNF