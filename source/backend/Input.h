#pragma once

#include <array>

class Input
{
public:
	static void beginFrame();
	static void setKeyDown(unsigned int virtualKey, bool isDown);
	static bool isKeyPressed(unsigned int virtualKey);
	static bool isKeyJustPressed(unsigned int virtualKey);
	static void clear();

private:
	static std::array<bool, 256> currentKeys_;
	static std::array<bool, 256> previousKeys_;
};