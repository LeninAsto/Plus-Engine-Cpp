#include "Input.h"

std::array<bool, 256> Input::currentKeys_ = {};
std::array<bool, 256> Input::previousKeys_ = {};

void Input::beginFrame()
{
	previousKeys_ = currentKeys_;
}

void Input::setKeyDown(unsigned int virtualKey, bool isDown)
{
	if (virtualKey < currentKeys_.size()) {
		currentKeys_[virtualKey] = isDown;
	}
}

bool Input::isKeyPressed(unsigned int virtualKey)
{
	return virtualKey < currentKeys_.size() ? currentKeys_[virtualKey] : false;
}

bool Input::isKeyJustPressed(unsigned int virtualKey)
{
	return virtualKey < currentKeys_.size() ? currentKeys_[virtualKey] && !previousKeys_[virtualKey] : false;
}

void Input::clear()
{
	currentKeys_.fill(false);
	previousKeys_.fill(false);
}