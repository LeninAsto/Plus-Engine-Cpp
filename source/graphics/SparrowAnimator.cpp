#include "SparrowAnimator.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
#include <sstream>

bool SparrowAnimator::loadAtlas(const std::filesystem::path& xmlPath)
{
	frames_.clear();
	animations_.clear();
	animationLookup_.clear();
	currentAnimationIndex_ = static_cast<std::size_t>(-1);
	currentAnimationFrame_ = 0;
	currentFrameTimer_ = 0.0f;
	imagePath_.clear();

	std::ifstream file(xmlPath);
	if (!file.is_open()) {
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	const std::string xmlContent = buffer.str();

	const std::regex atlasImageRegex(R"IMG(<TextureAtlas\s+imagePath="([^"]+)")IMG");
	const std::regex subTextureRegex(R"(<SubTexture\s+([^>]+)/?>)");
	const std::regex attributeRegex(R"ATTR((\w+)="([^"]*)")ATTR");

	std::smatch atlasMatch;
	if (std::regex_search(xmlContent, atlasMatch, atlasImageRegex)) {
		imagePath_ = xmlPath.parent_path() / atlasMatch[1].str();
	}

	for (std::sregex_iterator subTextureIt(xmlContent.begin(), xmlContent.end(), subTextureRegex); subTextureIt != std::sregex_iterator(); ++subTextureIt) {
		std::unordered_map<std::string, std::string> attributes;
		const std::string rawAttributes = (*subTextureIt)[1].str();
		for (std::sregex_iterator attributeIt(rawAttributes.begin(), rawAttributes.end(), attributeRegex); attributeIt != std::sregex_iterator(); ++attributeIt) {
			attributes.emplace((*attributeIt)[1].str(), (*attributeIt)[2].str());
		}

		SparrowFrame frame;
		frame.name = attributes["name"];
		frame.x = parseInt(attributes, "x");
		frame.y = parseInt(attributes, "y");
		frame.width = parseInt(attributes, "width");
		frame.height = parseInt(attributes, "height");
		frame.frameX = parseInt(attributes, "frameX");
		frame.frameY = parseInt(attributes, "frameY");
		frame.frameWidth = parseInt(attributes, "frameWidth", frame.width);
		frame.frameHeight = parseInt(attributes, "frameHeight", frame.height);
		frame.pivotX = parseFloat(attributes, "pivotX");
		frame.pivotY = parseFloat(attributes, "pivotY");
		frame.rotated = attributes.find("rotated") != attributes.end() ? parseBool(attributes["rotated"]) : false;
		frame.flipX = attributes.find("flipX") != attributes.end() ? parseBool(attributes["flipX"]) : false;
		frame.flipY = attributes.find("flipY") != attributes.end() ? parseBool(attributes["flipY"]) : false;
		frame.trimmed = attributes.find("frameX") != attributes.end() || attributes.find("frameY") != attributes.end() || attributes.find("frameWidth") != attributes.end() || attributes.find("frameHeight") != attributes.end();

		const int sizeLeft = frame.trimmed ? frame.frameX : 0;
		const int sizeTop = frame.trimmed ? frame.frameY : 0;
		const int sizeWidth = frame.trimmed ? frame.frameWidth : frame.width;
		const int sizeHeight = frame.trimmed ? frame.frameHeight : frame.height;

		frame.offsetX = -static_cast<float>(sizeLeft) - frame.pivotX;
		frame.offsetY = -static_cast<float>(sizeTop) - frame.pivotY;
		frame.sourceWidth = sizeWidth;
		frame.sourceHeight = sizeHeight;
		frame.angleDegrees = frame.rotated ? -90 : 0;
		if (frame.rotated && !frame.trimmed) {
			frame.sourceWidth = sizeHeight;
			frame.sourceHeight = sizeWidth;
		}

		frames_.push_back(frame);
	}

	return !frames_.empty();
}

bool SparrowAnimator::addByPrefix(const std::string& animationName, const std::string& prefix, float fps, bool loop)
{
	std::vector<std::size_t> frameIndices;
	for (std::size_t index = 0; index < frames_.size(); ++index) {
		if (frames_[index].name.rfind(prefix, 0) == 0) {
			frameIndices.push_back(index);
		}
	}

	std::sort(frameIndices.begin(), frameIndices.end(), [this](std::size_t left, std::size_t right) {
		std::string leftStem;
		std::string rightStem;
		const int leftNumber = extractTrailingNumber(frames_[left].name, leftStem);
		const int rightNumber = extractTrailingNumber(frames_[right].name, rightStem);
		if (leftStem == rightStem && leftNumber >= 0 && rightNumber >= 0) {
			return leftNumber < rightNumber;
		}

		return frames_[left].name < frames_[right].name;
	});

	if (frameIndices.empty()) {
		return false;
	}

	animations_.push_back({animationName, frameIndices, fps, loop});
	animationLookup_[animationName] = animations_.size() - 1;
	return true;
}

bool SparrowAnimator::addAllFrames(const std::string& animationName, float fps, bool loop)
{
	if (frames_.empty()) {
		return false;
	}

	std::vector<std::size_t> frameIndices;
	for (std::size_t index = 0; index < frames_.size(); ++index) {
		frameIndices.push_back(index);
	}

	animations_.push_back({animationName, frameIndices, fps, loop});
	animationLookup_[animationName] = animations_.size() - 1;
	return true;
}

bool SparrowAnimator::addByFrameRange(const std::string& animationName, std::size_t startIndex, std::size_t endIndex, float fps, bool loop)
{
	if (frames_.empty() || startIndex >= frames_.size() || endIndex >= frames_.size() || startIndex > endIndex) {
		return false;
	}

	std::vector<std::size_t> frameIndices;
	for (std::size_t index = startIndex; index <= endIndex; ++index) {
		frameIndices.push_back(index);
	}

	animations_.push_back({animationName, frameIndices, fps, loop});
	animationLookup_[animationName] = animations_.size() - 1;
	return true;
}

bool SparrowAnimator::addByIndices(const std::string& animationName, const std::vector<std::size_t>& indices, float fps, bool loop)
{
	if (frames_.empty() || indices.empty()) {
		return false;
	}

	std::vector<std::size_t> frameIndices;
	frameIndices.reserve(indices.size());
	for (std::size_t index : indices) {
		if (index >= frames_.size()) {
			return false;
		}

		frameIndices.push_back(index);
	}

	animations_.push_back({animationName, frameIndices, fps, loop});
	animationLookup_[animationName] = animations_.size() - 1;
	return true;
}

bool SparrowAnimator::play(const std::string& animationName, bool forceRestart)
{
	if (animationLookup_.find(animationName) == animationLookup_.end()) {
		return false;
	}

	const std::size_t animationIndex = animationLookup_[animationName];
	if (currentAnimationIndex_ == animationIndex && !forceRestart) {
		return true;
	}

	currentAnimationIndex_ = animationIndex;
	currentAnimationFrame_ = 0;
	currentFrameTimer_ = 0.0f;
	return true;
}

void SparrowAnimator::update(float deltaSeconds)
{
	if (currentAnimationIndex_ == static_cast<std::size_t>(-1)) {
		return;
	}

	SparrowAnimation& animation = animations_[currentAnimationIndex_];
	if (animation.frameIndices.empty() || animation.fps <= 0.0f) {
		return;
	}

	const float frameDuration = 1.0f / animation.fps;
	currentFrameTimer_ += deltaSeconds;
	while (currentFrameTimer_ >= frameDuration) {
		currentFrameTimer_ -= frameDuration;
		++currentAnimationFrame_;
		if (currentAnimationFrame_ >= animation.frameIndices.size()) {
			if (animation.loop) {
				currentAnimationFrame_ = 0;
			} else {
				currentAnimationFrame_ = animation.frameIndices.size() - 1;
				break;
			}
		}
	}
}

const SparrowFrame* SparrowAnimator::getCurrentFrame() const
{
	if (currentAnimationIndex_ == static_cast<std::size_t>(-1)) {
		return nullptr;
	}

	const SparrowAnimation& animation = animations_[currentAnimationIndex_];
	if (animation.frameIndices.empty()) {
		return nullptr;
	}

	return &frames_[animation.frameIndices[currentAnimationFrame_]];
}

std::string SparrowAnimator::getCurrentFrameName() const
{
	const SparrowFrame* frame = getCurrentFrame();
	return frame == nullptr ? std::string() : frame->name;
}

std::size_t SparrowAnimator::getFrameCount() const
{
	return frames_.size();
}

bool SparrowAnimator::hasAnimation(const std::string& animationName) const
{
	return animationLookup_.find(animationName) != animationLookup_.end();
}

const std::vector<SparrowFrame>& SparrowAnimator::getFrames() const
{
	return frames_;
}

const std::filesystem::path& SparrowAnimator::getImagePath() const
{
	return imagePath_;
}

bool SparrowAnimator::parseBool(const std::string& value)
{
	return value == "true" || value == "1";
}

int SparrowAnimator::parseInt(const std::unordered_map<std::string, std::string>& attributes, const std::string& key, int fallbackValue)
{
	if (attributes.find(key) == attributes.end()) {
		return fallbackValue;
	}

	return std::stoi(attributes.at(key));
}

float SparrowAnimator::parseFloat(const std::unordered_map<std::string, std::string>& attributes, const std::string& key, float fallbackValue)
{
	if (attributes.find(key) == attributes.end()) {
		return fallbackValue;
	}

	return std::stof(attributes.at(key));
}

int SparrowAnimator::extractTrailingNumber(const std::string& value, std::string& stem)
{
	if (value.empty()) {
		stem.clear();
		return -1;
	}

	std::size_t splitIndex = value.size();
	while (splitIndex > 0 && std::isdigit(static_cast<unsigned char>(value[splitIndex - 1]))) {
		--splitIndex;
	}

	stem = value.substr(0, splitIndex);
	if (splitIndex == value.size()) {
		return -1;
	}

	return std::stoi(value.substr(splitIndex));
}