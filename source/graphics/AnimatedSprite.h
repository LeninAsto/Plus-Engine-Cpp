#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "SparrowAnimator.h"

class AnimatedSprite
{
public:
	bool loadAtlas(const std::filesystem::path& xmlPath);
	bool addByPrefix(const std::string& animationName, const std::string& prefix, float fps, bool loop);
	bool addAllFrames(const std::string& animationName, float fps, bool loop);
	bool addByFrameRange(const std::string& animationName, std::size_t startIndex, std::size_t endIndex, float fps, bool loop);
	bool addByIndices(const std::string& animationName, const std::vector<std::size_t>& indices, float fps, bool loop);
	bool play(const std::string& animationName, bool forceRestart = false);
	void update(float deltaSeconds);
	void render() const;

	void setPosition(float x, float y);
	void setScale(float scale);
	void setVisible(bool visible);

	float getX() const;
	float getY() const;
	bool isVisible() const;
	int getSourceWidth() const;
	int getSourceHeight() const;
	std::string getCurrentFrameName() const;
	std::size_t getFrameCount() const;

private:
	SparrowAnimator animator_;
	float x_ = 0.0f;
	float y_ = 0.0f;
	float scale_ = 1.0f;
	bool visible_ = true;
};