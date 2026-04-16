#include "AudioPlayer.h"

#include <cstdlib>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

bool AudioPlayer::playing_ = false;

bool AudioPlayer::playMusic(const std::filesystem::path& filePath, bool loop)
{
	stopMusic();
	if (!openMusic(filePath)) {
		return false;
	}

	const std::wstring playCommand = std::wstring(L"play ") + aliasName_ + (loop ? L" repeat" : L"");
	if (mciSendStringW(playCommand.c_str(), nullptr, 0, nullptr) != 0) {
		stopMusic();
		return false;
	}

	playing_ = true;
	return true;
}

bool AudioPlayer::playSound(const std::filesystem::path& filePath)
{
	if (!std::filesystem::exists(filePath)) {
		return false;
	}

	return PlaySoundW(filePath.wstring().c_str(), nullptr, SND_ASYNC | SND_FILENAME | SND_NODEFAULT);
}

void AudioPlayer::stopMusic()
{
	mciSendStringW((std::wstring(L"stop ") + aliasName_).c_str(), nullptr, 0, nullptr);
	mciSendStringW((std::wstring(L"close ") + aliasName_).c_str(), nullptr, 0, nullptr);
	playing_ = false;
}

bool AudioPlayer::isPlaying()
{
	return playing_;
}

double AudioPlayer::getPlaybackPositionMs()
{
	if (!playing_) {
		return 0.0;
	}

	wchar_t buffer[64] = {};
	if (mciSendStringW((std::wstring(L"status ") + aliasName_ + L" position").c_str(), buffer, static_cast<UINT>(std::size(buffer)), nullptr) != 0) {
		return 0.0;
	}

	return std::wcstod(buffer, nullptr);
}

bool AudioPlayer::openMusic(const std::filesystem::path& filePath)
{
	if (!std::filesystem::exists(filePath)) {
		return false;
	}

	const std::wstring command = std::wstring(L"open \"") + filePath.wstring() + L"\" type mpegvideo alias " + aliasName_;
	if (mciSendStringW(command.c_str(), nullptr, 0, nullptr) != 0) {
		return false;
	}

	return mciSendStringW((std::wstring(L"set ") + aliasName_ + L" time format milliseconds").c_str(), nullptr, 0, nullptr) == 0;
}