#pragma once

#include <filesystem>

class AudioPlayer
{
public:
	static bool playMusic(const std::filesystem::path& filePath, bool loop = true);
	static bool playSound(const std::filesystem::path& filePath);
	static void stopMusic();
	static bool isPlaying();
	static double getPlaybackPositionMs();

private:
	static bool openMusic(const std::filesystem::path& filePath);
	static constexpr const wchar_t* aliasName_ = L"PlusEngineMusic";
	static bool playing_;
};