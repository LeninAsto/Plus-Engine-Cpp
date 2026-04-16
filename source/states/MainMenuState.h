#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../backend/State.h"
#include "../graphics/AnimatedSprite.h"

class MainMenuState : public State
{
public:
	MainMenuState();

	void create() override;
	void update(float deltaSeconds) override;
	void render() override;

private:
	enum class MenuColumn
	{
		Left,
		Center,
		Right
	};

	struct MenuItem
	{
		std::string name;
		AnimatedSprite sprite;
		float y = 0.0f;
	};

	void playMenuMusicIfNeeded();
	void layoutMenuItems();
	void refreshMenuAnimations(bool playScrollSound);
	void updateAnimatedSprites(float deltaSeconds);
	void confirmSelection();
	std::string getSelectedOption() const;
	AnimatedSprite* getSelectedSprite();

	std::vector<MenuItem> menuItems_;
	AnimatedSprite leftItem_;
	AnimatedSprite rightItem_;
	AnimatedSprite boyfriendSprite_;
	AnimatedSprite girlfriendSprite_;
	std::filesystem::path backgroundPath_;
	std::filesystem::path magentaPath_;
	MenuColumn selectedColumn_ = MenuColumn::Center;
	int selectedIndex_ = 0;
	bool hasLeftItem_ = false;
	bool hasRightItem_ = false;
	bool confirming_ = false;
	float confirmTimer_ = 0.0f;
	float flashTimer_ = 0.0f;
};