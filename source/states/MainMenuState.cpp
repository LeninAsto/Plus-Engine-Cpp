#include "MainMenuState.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>

#include "../backend/AudioPlayer.h"
#include "../backend/Input.h"
#include "../backend/Paths.h"
#include "../graphics/Renderer.h"

MainMenuState::MainMenuState()
	: State("MainMenuState")
{
}

void MainMenuState::create()
{
	State::create();
	backgroundPath_ = Paths::image("shared/images/menuBG");
	magentaPath_ = Paths::image("shared/images/menuDesat");
	playMenuMusicIfNeeded();

	const std::vector<std::string> centerOptions = {"story_mode", "freeplay", "mods", "credits"};
	menuItems_.clear();
	menuItems_.reserve(centerOptions.size());
	for (std::size_t index = 0; index < centerOptions.size(); ++index) {
		MenuItem item;
		item.name = centerOptions[index];
		item.y = static_cast<float>((index * 140) + 90);
		item.sprite.loadAtlas(Paths::xml("shared/images/mainmenu/menu_" + item.name));
		item.sprite.addByPrefix("idle", item.name + " idle", 24.0f, true);
		item.sprite.addByPrefix("selected", item.name + " selected", 24.0f, true);
		item.sprite.play("idle", true);
		menuItems_.push_back(std::move(item));
	}

	hasLeftItem_ = leftItem_.loadAtlas(Paths::xml("shared/images/mainmenu/menu_achievements"));
	if (hasLeftItem_) {
		leftItem_.addByPrefix("idle", "achievements idle", 24.0f, true);
		leftItem_.addByPrefix("selected", "achievements selected", 24.0f, true);
		leftItem_.play("idle", true);
	}

	hasRightItem_ = rightItem_.loadAtlas(Paths::xml("shared/images/mainmenu/menu_options"));
	if (hasRightItem_) {
		rightItem_.addByPrefix("idle", "options idle", 24.0f, true);
		rightItem_.addByPrefix("selected", "options selected", 24.0f, true);
		rightItem_.play("idle", true);
	}

	if (boyfriendSprite_.loadAtlas(Paths::xml("shared/images/menucharacters/Menu_BF"))) {
		boyfriendSprite_.addByPrefix("idle", "M BF Idle", 24.0f, true);
		boyfriendSprite_.addByPrefix("hey", "M bf HEY", 24.0f, false);
		boyfriendSprite_.play("idle", true);
	}

	if (girlfriendSprite_.loadAtlas(Paths::xml("shared/images/menucharacters/Menu_GF"))) {
		girlfriendSprite_.addByPrefix("idle", "M GF Idle", 24.0f, true);
		girlfriendSprite_.play("idle", true);
	}

	selectedColumn_ = MenuColumn::Center;
	selectedIndex_ = 0;
	layoutMenuItems();
	refreshMenuAnimations(false);
	log("MainMenuState created.");
}

void MainMenuState::update(float deltaSeconds)
{
	playMenuMusicIfNeeded();
	updateAnimatedSprites(deltaSeconds);
	if (confirming_) {
		confirmTimer_ -= deltaSeconds;
		flashTimer_ += deltaSeconds;
		if (confirmTimer_ <= 0.0f) {
			confirming_ = false;
			AnimatedSprite* selectedSprite = getSelectedSprite();
			if (selectedSprite != nullptr) {
				selectedSprite->play("selected", true);
			}
			boyfriendSprite_.play("idle", true);
		}
		return;
	}

	const bool pressedUp = Input::isKeyJustPressed(VK_UP) || Input::isKeyJustPressed('W');
	const bool pressedDown = Input::isKeyJustPressed(VK_DOWN) || Input::isKeyJustPressed('S');
	const bool pressedLeft = Input::isKeyJustPressed(VK_LEFT) || Input::isKeyJustPressed('A');
	const bool pressedRight = Input::isKeyJustPressed(VK_RIGHT) || Input::isKeyJustPressed('D');
	const bool pressedAccept = Input::isKeyJustPressed(VK_RETURN) || Input::isKeyJustPressed(VK_SPACE);
	const bool pressedBack = Input::isKeyJustPressed(VK_ESCAPE) || Input::isKeyJustPressed(VK_BACK);

	if (pressedUp && selectedColumn_ == MenuColumn::Center) {
		selectedIndex_ = (selectedIndex_ + static_cast<int>(menuItems_.size()) - 1) % static_cast<int>(menuItems_.size());
		refreshMenuAnimations(true);
	}

	if (pressedDown && selectedColumn_ == MenuColumn::Center) {
		selectedIndex_ = (selectedIndex_ + 1) % static_cast<int>(menuItems_.size());
		refreshMenuAnimations(true);
	}

	if (pressedLeft) {
		if (selectedColumn_ == MenuColumn::Center && hasLeftItem_) {
			selectedColumn_ = MenuColumn::Left;
			refreshMenuAnimations(true);
		} else if (selectedColumn_ == MenuColumn::Right) {
			selectedColumn_ = MenuColumn::Center;
			refreshMenuAnimations(true);
		}
	}

	if (pressedRight) {
		if (selectedColumn_ == MenuColumn::Center && hasRightItem_) {
			selectedColumn_ = MenuColumn::Right;
			refreshMenuAnimations(true);
		} else if (selectedColumn_ == MenuColumn::Left) {
			selectedColumn_ = MenuColumn::Center;
			refreshMenuAnimations(true);
		}
	}

	if (pressedBack) {
		AudioPlayer::playSound(Paths::sound("cancelMenu"));
		requestStateChange("TitleState");
		return;
	}

	if (pressedAccept) {
		confirmSelection();
	}
}

void MainMenuState::render()
{
	Renderer& renderer = Renderer::get();
	if (!backgroundPath_.empty()) {
		renderer.drawImage(backgroundPath_, -80, -40, 1.18f);
	}

	if (confirming_ && !magentaPath_.empty() && static_cast<int>(flashTimer_ * 20.0f) % 2 == 0) {
		renderer.drawImage(magentaPath_, -80, -40, 1.18f);
	}

	girlfriendSprite_.render();
	boyfriendSprite_.render();
	for (const MenuItem& item : menuItems_) {
		item.sprite.render();
	}
	if (hasLeftItem_) {
		leftItem_.render();
	}
	if (hasRightItem_) {
		rightItem_.render();
	}
}

void MainMenuState::playMenuMusicIfNeeded()
{
	if (!AudioPlayer::isPlaying()) {
		AudioPlayer::playMusic(Paths::music("freakyMenu"), true);
	}
}

void MainMenuState::layoutMenuItems()
{
	const int width = Renderer::get().getWidth();
	const int height = Renderer::get().getHeight();
	for (MenuItem& item : menuItems_) {
		const int sourceWidth = item.sprite.getSourceWidth() > 0 ? item.sprite.getSourceWidth() : 1;
		const float spriteWidth = static_cast<float>(sourceWidth);
		item.sprite.setPosition((width - spriteWidth) * 0.5f, item.y);
	}

	if (hasLeftItem_) {
		leftItem_.setPosition(60.0f, 490.0f);
	}

	if (hasRightItem_) {
		const int sourceWidth = rightItem_.getSourceWidth() > 0 ? rightItem_.getSourceWidth() : 1;
		const float spriteWidth = static_cast<float>(sourceWidth);
		rightItem_.setPosition(static_cast<float>(width) - 60.0f - spriteWidth, 490.0f);
	}

	girlfriendSprite_.setPosition(70.0f, static_cast<float>(height) - 400.0f);
	boyfriendSprite_.setPosition(static_cast<float>(width) - 420.0f, static_cast<float>(height) - 410.0f);
}

void MainMenuState::refreshMenuAnimations(bool playScrollSound)
{
	if (playScrollSound) {
		AudioPlayer::playSound(Paths::sound("scrollMenu"));
	}

	for (MenuItem& item : menuItems_) {
		item.sprite.play("idle", true);
	}

	if (hasLeftItem_) {
		leftItem_.play("idle", true);
	}

	if (hasRightItem_) {
		rightItem_.play("idle", true);
	}

	AnimatedSprite* selectedSprite = getSelectedSprite();
	if (selectedSprite != nullptr) {
		selectedSprite->play("selected", true);
	}

	layoutMenuItems();
}

void MainMenuState::updateAnimatedSprites(float deltaSeconds)
{
	for (MenuItem& item : menuItems_) {
		item.sprite.update(deltaSeconds);
	}
	if (hasLeftItem_) {
		leftItem_.update(deltaSeconds);
	}
	if (hasRightItem_) {
		rightItem_.update(deltaSeconds);
	}
	boyfriendSprite_.update(deltaSeconds);
	girlfriendSprite_.update(deltaSeconds);
}

void MainMenuState::confirmSelection()
{
	AudioPlayer::playSound(Paths::sound("confirmMenu"));
	confirming_ = true;
	confirmTimer_ = 0.65f;
	flashTimer_ = 0.0f;
	boyfriendSprite_.play("hey", true);
}

std::string MainMenuState::getSelectedOption() const
{
	switch (selectedColumn_) {
	case MenuColumn::Left:
		return hasLeftItem_ ? "achievements" : std::string();

	case MenuColumn::Right:
		return hasRightItem_ ? "options" : std::string();

	case MenuColumn::Center:
	default:
		return menuItems_.empty() ? std::string() : menuItems_[selectedIndex_].name;
	}
}

AnimatedSprite* MainMenuState::getSelectedSprite()
{
	switch (selectedColumn_) {
	case MenuColumn::Left:
		return hasLeftItem_ ? &leftItem_ : nullptr;

	case MenuColumn::Right:
		return hasRightItem_ ? &rightItem_ : nullptr;

	case MenuColumn::Center:
	default:
		return menuItems_.empty() ? nullptr : &menuItems_[selectedIndex_].sprite;
	}
}