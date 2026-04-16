#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct SparrowFrame
{
	std::string name;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int frameX = 0;
	int frameY = 0;
	int frameWidth = 0;
	int frameHeight = 0;
	int sourceWidth = 0;
	int sourceHeight = 0;
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float pivotX = 0.0f;
	float pivotY = 0.0f;
	int angleDegrees = 0;
	bool flipX = false;
	bool flipY = false;
	bool rotated = false;
	bool trimmed = false;
};

struct SparrowAnimation
{
	std::string name;
	std::vector<std::size_t> frameIndices;
	float fps = 24.0f;
	bool loop = true;
};

class SparrowAnimator
{
public:
	bool loadAtlas(const std::filesystem::path& xmlPath);
	bool addByPrefix(const std::string& animationName, const std::string& prefix, float fps, bool loop);
	bool addAllFrames(const std::string& animationName, float fps, bool loop);
	bool addByFrameRange(const std::string& animationName, std::size_t startIndex, std::size_t endIndex, float fps, bool loop);
	bool addByIndices(const std::string& animationName, const std::vector<std::size_t>& indices, float fps, bool loop);
	bool play(const std::string& animationName, bool forceRestart = false);
	void update(float deltaSeconds);

	const SparrowFrame* getCurrentFrame() const;
	std::string getCurrentFrameName() const;
	std::size_t getFrameCount() const;
	bool hasAnimation(const std::string& animationName) const;
	const std::vector<SparrowFrame>& getFrames() const;
	const std::filesystem::path& getImagePath() const;

private:
	static bool parseBool(const std::string& value);
	static int parseInt(const std::unordered_map<std::string, std::string>& attributes, const std::string& key, int fallbackValue = 0);
	static float parseFloat(const std::unordered_map<std::string, std::string>& attributes, const std::string& key, float fallbackValue = 0.0f);
	static int extractTrailingNumber(const std::string& value, std::string& stem);

	std::vector<SparrowFrame> frames_;
	std::vector<SparrowAnimation> animations_;
	std::unordered_map<std::string, std::size_t> animationLookup_;
	std::size_t currentAnimationIndex_ = static_cast<std::size_t>(-1);
	std::size_t currentAnimationFrame_ = 0;
	float currentFrameTimer_ = 0.0f;
	std::filesystem::path imagePath_;
};