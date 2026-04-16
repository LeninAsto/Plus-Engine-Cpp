#include "AnimatedSprite.h"

#include "Renderer.h"

bool AnimatedSprite::loadAtlas(const std::filesystem::path& xmlPath)
{
	return animator_.loadAtlas(xmlPath);
}

bool AnimatedSprite::addByPrefix(const std::string& animationName, const std::string& prefix, float fps, bool loop)
{
	return animator_.addByPrefix(animationName, prefix, fps, loop);
}

bool AnimatedSprite::addAllFrames(const std::string& animationName, float fps, bool loop)
{
	return animator_.addAllFrames(animationName, fps, loop);
}

bool AnimatedSprite::addByFrameRange(const std::string& animationName, std::size_t startIndex, std::size_t endIndex, float fps, bool loop)
{
	return animator_.addByFrameRange(animationName, startIndex, endIndex, fps, loop);
}

bool AnimatedSprite::addByIndices(const std::string& animationName, const std::vector<std::size_t>& indices, float fps, bool loop)
{
	return animator_.addByIndices(animationName, indices, fps, loop);
}

bool AnimatedSprite::play(const std::string& animationName, bool forceRestart)
{
	return animator_.play(animationName, forceRestart);
}

void AnimatedSprite::update(float deltaSeconds)
{
	animator_.update(deltaSeconds);
}

void AnimatedSprite::render() const
{
	if (!visible_) {
		return;
	}

	const SparrowFrame* frame = animator_.getCurrentFrame();
	if (frame == nullptr) {
		return;
	}

	Renderer::get().drawAtlasFrame(animator_.getImagePath(), *frame, static_cast<int>(x_), static_cast<int>(y_), scale_);
}

void AnimatedSprite::setPosition(float x, float y)
{
	x_ = x;
	y_ = y;
}

void AnimatedSprite::setScale(float scale)
{
	scale_ = scale;
}

void AnimatedSprite::setVisible(bool visible)
{
	visible_ = visible;
}

float AnimatedSprite::getX() const
{
	return x_;
}

float AnimatedSprite::getY() const
{
	return y_;
}

bool AnimatedSprite::isVisible() const
{
	return visible_;
}

int AnimatedSprite::getSourceWidth() const
{
	const SparrowFrame* frame = animator_.getCurrentFrame();
	return frame == nullptr ? 0 : frame->sourceWidth;
}

int AnimatedSprite::getSourceHeight() const
{
	const SparrowFrame* frame = animator_.getCurrentFrame();
	return frame == nullptr ? 0 : frame->sourceHeight;
}

std::string AnimatedSprite::getCurrentFrameName() const
{
	return animator_.getCurrentFrameName();
}

std::size_t AnimatedSprite::getFrameCount() const
{
	return animator_.getFrameCount();
}