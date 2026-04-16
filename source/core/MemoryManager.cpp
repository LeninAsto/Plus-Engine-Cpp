#include "MemoryManager.h"

#include "../audio/MusicPlayer.h"
#include "../audio/SoundPlayer.h"
#include "../audio/VocalsPlayer.h"
#include "../graphics/Texture.h"
#include "../play/HoldSplash.h"
#include "../play/Note.h"
#include "../play/NoteSplash.h"
#include "Logger.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#endif

namespace FNF {

void MemoryManager::Collect(bool aggressive) {
    Note::InvalidateSharedResources();
    NoteSplash::InvalidateSharedResources();
    HoldSplash::InvalidateSharedResources();

    TextureCache::ClearAll();
    SoundPlayer::ClearCache();
    MusicPlayer::ClearCache(true);
    VocalsPlayer::ClearCache();

#ifdef _WIN32
    if (aggressive) {
        HANDLE process = GetCurrentProcess();
        EmptyWorkingSet(process);
        SetProcessWorkingSetSize(process, static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
    }
#endif

    Logger::Info(std::string("[MemoryManager] Cache cleanup completed") + (aggressive ? " (aggressive)" : ""));
}

} // namespace FNF