#pragma once

#include <cctype>
#include <filesystem>
#include <string>
#include <unordered_map>

#include "../graphics/SparrowAnimator.h"

class Alphabet
{
public:
	bool initialize(const std::filesystem::path& xmlPath, bool bold);
	void setText(const std::string& text);
	void setPosition(float x, float y);
	void setScale(float scale);
	void setVisible(bool visible);
	bool isVisible() const;
	void render() const;

private:
	const SparrowFrame* resolveFrame(char character) const;

	SparrowAnimator atlas_;
	std::unordered_map<char, std::size_t> characterToFrame_;
	std::string text_;
	float x_ = 0.0f;
	float y_ = 0.0f;
	float scale_ = 1.0f;
	bool bold_ = true;
	bool visible_ = true;
};