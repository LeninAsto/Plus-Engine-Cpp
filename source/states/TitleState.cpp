#include "TitleState.h"

#include <cmath>
#include <fstream>
#include <regex>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../backend/AudioPlayer.h"
#include "../backend/Conductor.h"
#include "../backend/Input.h"
#include "../backend/Paths.h"
#include "../graphics/Renderer.h"

namespace
{
	std::string readTextFile(const std::filesystem::path& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open()) {
			return {};
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	bool parseFloatValue(const std::string& json, const std::string& key, float& output)
	{
		const std::regex regex("\\\"" + key + "\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
		std::smatch match;
		if (!std::regex_search(json, match, regex)) {
			return false;
		}

		output = std::stof(match[1].str());
		return true;
	}

	bool parseDoubleValue(const std::string& json, const std::string& key, double& output)
	{
		const std::regex regex("\\\"" + key + "\\\"\\s*:\\s*(-?\\d+(?:\\.\\d+)?)");
		std::smatch match;
		if (!std::regex_search(json, match, regex)) {
			return false;
		}

		output = std::stod(match[1].str());
		return true;
	}

	bool parseStringValue(const std::string& json, const std::string& key, std::string& output)
	{
		const std::regex regex("\\\"" + key + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");
		std::smatch match;
		if (!std::regex_search(json, match, regex)) {
			return false;
		}

		output = match[1].str();
		return true;
	}

	bool parseBoolValue(const std::string& json, const std::string& key, bool& output)
	{
		const std::regex regex("\\\"" + key + "\\\"\\s*:\\s*(true|false)");
		std::smatch match;
		if (!std::regex_search(json, match, regex)) {
			return false;
		}

		output = match[1].str() == "true";
		return true;
	}

	bool parseIndexArray(const std::string& json, const std::string& key, std::vector<std::size_t>& output)
	{
		const std::regex regex("\\\"" + key + "\\\"\\s*:\\s*\\[([^\\]]*)\\]");
		std::smatch match;
		if (!std::regex_search(json, match, regex)) {
			return false;
		}

		output.clear();
		const std::string rawValues = match[1].str();
		const std::regex numberRegex("-?\\d+");
		for (std::sregex_iterator iterator(rawValues.begin(), rawValues.end(), numberRegex); iterator != std::sregex_iterator(); ++iterator) {
			const int value = std::stoi((*iterator).str());
			if (value >= 0) {
				output.push_back(static_cast<std::size_t>(value));
			}
		}

		return !output.empty();
	}
}

TitleState::TitleState()
	: State("TitleState")
{
}

void TitleState::create()
{
	State::create();
	fontReady_ = Renderer::get().registerFontFile(Paths::font("inter.otf"));
	loadTitleLayout();
	Conductor::reset(layout_.bpm);
	AudioPlayer::playMusic(Paths::music("freakyMenu"), true);

	const bool logoLoaded = logoSprite_.loadAtlas(Paths::xml("shared/images/logoBumpin"));
	const bool gfLoaded = gfSprite_.loadAtlas(Paths::xml("shared/images/gfDanceTitle"));
	const bool enterLoaded = titleEnterSprite_.loadAtlas(Paths::xml("shared/images/titleEnter"));
	const bool alphabetLoaded = titleAlphabet_.initialize(Paths::xml("shared/images/alphabet"), true);

	if (logoLoaded) {
		logoSprite_.addByPrefix("bump", "logo bumpin", 24.0f, false);
		logoSprite_.play("bump", true);
		logoSprite_.setPosition(layout_.logoX, layout_.logoY);
		logoSprite_.setScale(0.45f);
	}

	if (gfLoaded) {
		if (layout_.useIdle) {
			gfSprite_.addByPrefix("idle", layout_.animationName, 24.0f, false);
			gfSprite_.play("idle", true);
		} else {
			gfSprite_.addByIndices("danceLeft", layout_.danceLeftFrames, 24.0f, false);
			gfSprite_.addByIndices("danceRight", layout_.danceRightFrames, 24.0f, false);
			gfSprite_.play("danceRight", true);
		}
		gfSprite_.setPosition(layout_.gfX, layout_.gfY);
		gfSprite_.setScale(0.45f);
	}

	if (enterLoaded) {
		titleEnterSprite_.addByPrefix("idle", "ENTER IDLE", 24.0f, true);
		titleEnterSprite_.addByPrefix("freeze", "ENTER FREEZE", 24.0f, false);
		titleEnterSprite_.addByPrefix("press", "ENTER PRESSED", 24.0f, false);
		titleEnterSprite_.play("idle", true);
		titleEnterSprite_.setPosition(layout_.enterX, layout_.enterY);
		titleEnterSprite_.setScale(titleEnterBaseScale_);
		titleEnterSprite_.setVisible(false);
	}

	if (alphabetLoaded) {
		titleAlphabet_.setText("PRESS ENTER");
		titleAlphabet_.setPosition(layout_.enterX + 138.0f, layout_.enterY - 72.0f);
		titleAlphabet_.setScale(titleAlphabetBaseScale_);
		titleAlphabet_.setVisible(false);
	}

	if (!(logoLoaded && gfLoaded && enterLoaded && alphabetLoaded)) {
		log("One or more title assets failed to load.");
	}

	introLines_ = {L"PSYCH ENGINE BY", L"SHADOW MARIO"};
	log("TitleState created successfully.");
}

void TitleState::update(float deltaSeconds)
{
	elapsedTime_ += deltaSeconds;
	introTime_ += deltaSeconds;
	titlePulseTimer_ += deltaSeconds;

	if (AudioPlayer::isPlaying()) {
		Conductor::setSongPosition(AudioPlayer::getPlaybackPositionMs());
	} else {
		Conductor::update(deltaSeconds);
	}

	currentBeat_ = Conductor::getCurrentBeat(Conductor::getSongPositionMs());
	currentStep_ = Conductor::getCurrentStep(Conductor::getSongPositionMs());
	updateBeatState();
	logoSprite_.update(deltaSeconds);
	gfSprite_.update(deltaSeconds);
	titleEnterSprite_.update(deltaSeconds);

	if (skippedIntro_ && !transitioning_) {
		const float pulse = 1.0f + std::sin(titlePulseTimer_ * 4.0f) * 0.03f;
		titleEnterSprite_.setScale(titleEnterBaseScale_ * pulse);
		titleAlphabet_.setScale(titleAlphabetBaseScale_ * pulse);
	}

	if (!transitioning_ && Input::isKeyJustPressed(VK_RETURN)) {
		if (!skippedIntro_) {
			skipIntro();
		} else {
			startTransitionToMainMenu();
		}
	}

	if (transitioning_) {
		transitionTimer_ -= deltaSeconds;
		if (transitionTimer_ <= 0.0f) {
			requestStateChange("MainMenuState");
		}
	}
}

void TitleState::render()
{
	Renderer& renderer = Renderer::get();
	const int width = renderer.getWidth();

	if (!backgroundPath_.empty()) {
		renderer.drawImage(backgroundPath_, 0, 0, 1.0f);
	}

	logoSprite_.render();
	gfSprite_.render();
	titleEnterSprite_.render();
	titleAlphabet_.render();
	renderIntroOverlay();
}

void TitleState::destroy()
{
	if (!keepMusicOnDestroy_) {
		AudioPlayer::stopMusic();
	}
	log("TitleState destroyed.");
	State::destroy();
}

void TitleState::loadTitleLayout()
{
	const std::filesystem::path jsonPath = Paths::get("shared/images/gfDanceTitle.json");
	const std::string json = readTextFile(jsonPath);
	if (json.empty()) {
		backgroundPath_.clear();
		return;
	}

	parseFloatValue(json, "titlex", layout_.logoX);
	parseFloatValue(json, "titley", layout_.logoY);
	parseFloatValue(json, "startx", layout_.enterX);
	parseFloatValue(json, "starty", layout_.enterY);
	parseFloatValue(json, "gfx", layout_.gfX);
	parseFloatValue(json, "gfy", layout_.gfY);
	parseDoubleValue(json, "bpm", layout_.bpm);
	parseStringValue(json, "animation", layout_.animationName);
	parseIndexArray(json, "dance_left", layout_.danceLeftFrames);
	parseIndexArray(json, "dance_right", layout_.danceRightFrames);
	parseBoolValue(json, "idle", layout_.useIdle);

	std::string backgroundSprite;
	if (parseStringValue(json, "backgroundSprite", backgroundSprite) && !backgroundSprite.empty()) {
		backgroundPath_ = Paths::image(backgroundSprite);
	}
}

void TitleState::updateBeatState()
{
	while (lastBeat_ < currentBeat_) {
		++lastBeat_;
		updateBeatAnimations();
		advanceIntroBeat();
	}
}

void TitleState::updateBeatAnimations()
{
	logoSprite_.play("bump", true);
	if (layout_.useIdle) {
		if (currentBeat_ % 2 == 0) {
			gfSprite_.play("idle", true);
		}
		return;
	}

	danceLeft_ = !danceLeft_;
	gfSprite_.play(danceLeft_ ? "danceRight" : "danceLeft", true);
}

void TitleState::advanceIntroBeat()
{
	if (lastBeat_ < 0 || transitioning_) {
		return;
	}

	if (!skippedIntro_) {
		++sickBeats_;
		switch (sickBeats_) {
		case 1:
			if (!AudioPlayer::isPlaying()) {
				AudioPlayer::playMusic(Paths::music("freakyMenu"), true);
			}
			break;

		case 2:
			introLines_ = {L"PSYCH ENGINE BY", L"SHADOW MARIO"};
			break;

		case 4:
			introLines_ = {L"PLUS ENGINE BY", L"LENIN ASTO"};
			break;

		case 5:
			introLines_.clear();
			break;

		case 6:
			introLines_ = {L"NOT ASSOCIATED", L"WITH"};
			showNewgroundsLine_ = false;
			break;

		case 8:
			introLines_ = {L"NEWGROUNDS"};
			showNewgroundsLine_ = true;
			break;

		case 9:
			introLines_.clear();
			showNewgroundsLine_ = false;
			break;

		case 10:
			introLines_ = {wackyLines_[0]};
			break;

		case 12:
			introLines_ = {wackyLines_[0], wackyLines_[1]};
			break;

		case 13:
			introLines_.clear();
			break;

		case 14:
			introLines_ = {L"FRIDAY"};
			break;

		case 15:
			introLines_ = {L"FRIDAY", L"NIGHT"};
			break;

		case 16:
			introLines_ = {L"FRIDAY", L"NIGHT", L"FUNKIN"};
			break;

		case 17:
			skipIntro();
			break;

		default:
			break;
		}
	}
}

void TitleState::skipIntro()
{
	if (skippedIntro_) {
		return;
	}

	skippedIntro_ = true;
	introLines_.clear();
	showNewgroundsLine_ = false;
	titleEnterSprite_.setVisible(true);
	titleAlphabet_.setVisible(true);
	titleEnterSprite_.play("idle", true);
}

void TitleState::startTransitionToMainMenu()
{
	if (transitioning_) {
		return;
	}

	transitioning_ = true;
	keepMusicOnDestroy_ = true;
	transitionTimer_ = 1.0f;
	AudioPlayer::playSound(Paths::sound("confirmMenu"));
	titleEnterSprite_.setScale(titleEnterBaseScale_ * 1.05f);
	titleAlphabet_.setScale(titleAlphabetBaseScale_ * 1.05f);
	titleEnterSprite_.play("press", true);
}

void TitleState::renderIntroOverlay() const
{
	if (skippedIntro_ || introLines_.empty()) {
		return;
	}

	Renderer& renderer = Renderer::get();
	const int width = renderer.getWidth();
	int y = 270;
	for (const std::wstring& line : introLines_) {
		RECT bounds = {120, y, width - 120, y + 56};
		renderer.drawCenteredText(line, bounds, 34, RGB(255, 255, 255), fontReady_ ? L"Inter" : L"Segoe UI", FW_BOLD);
		y += 56;
	}

	if (showNewgroundsLine_) {
		RECT bounds = {120, y + 12, width - 120, y + 56};
		renderer.drawCenteredText(L"newgrounds", bounds, 24, RGB(255, 185, 80), fontReady_ ? L"Inter" : L"Segoe UI", FW_BOLD);
	}
}