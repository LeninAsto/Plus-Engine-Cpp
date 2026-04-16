#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../backend/State.h"
#include "../graphics/AnimatedSprite.h"
#include "../ui/Alphabet.h"

class TitleState : public State
{
public:
	TitleState();

	void create() override;
	void update(float deltaSeconds) override;
	void render() override;
	void destroy() override;

private:
	struct TitleLayout
	{
		float logoX = -150.0f;
		float logoY = -100.0f;
		float enterX = 100.0f;
		float enterY = 576.0f;
		float gfX = 512.0f;
		float gfY = 40.0f;
		double bpm = 102.0;
		std::string animationName = "gfDance";
		std::vector<std::size_t> danceLeftFrames = {30, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
		std::vector<std::size_t> danceRightFrames = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
		bool useIdle = false;
		std::filesystem::path backgroundPath;
	};

	void loadTitleLayout();
	void updateBeatState();
	void updateBeatAnimations();
	void advanceIntroBeat();
	void skipIntro();
	void startTransitionToMainMenu();
	void renderIntroOverlay() const;

	TitleLayout layout_;
	AnimatedSprite logoSprite_;
	AnimatedSprite gfSprite_;
	AnimatedSprite titleEnterSprite_;
	Alphabet titleAlphabet_;
	std::vector<std::wstring> introLines_;
	std::vector<std::wstring> wackyLines_ = {L"ENGINEERING CHAOS", L"WITH BETTER BPMS"};
	std::filesystem::path backgroundPath_;
	double elapsedTime_ = 0.0;
	double introTime_ = 0.0;
	int currentBeat_ = 0;
	int currentStep_ = 0;
	int lastBeat_ = -1;
	int sickBeats_ = 0;
	bool danceLeft_ = false;
	bool skippedIntro_ = false;
	bool transitioning_ = false;
	bool showNewgroundsLine_ = false;
	float transitionTimer_ = 0.0f;
	float titlePulseTimer_ = 0.0f;
	float titleEnterBaseScale_ = 0.55f;
	float titleAlphabetBaseScale_ = 0.5f;
	bool keepMusicOnDestroy_ = false;
	bool fontReady_ = false;
};