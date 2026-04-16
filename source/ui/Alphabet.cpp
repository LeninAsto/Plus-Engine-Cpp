#include "Alphabet.h"

#include <regex>

#include "../graphics/Renderer.h"

bool Alphabet::initialize(const std::filesystem::path& xmlPath, bool bold)
{
	bold_ = bold;
	characterToFrame_.clear();
	if (!atlas_.loadAtlas(xmlPath)) {
		return false;
	}

	const std::regex frameRegex(R"ALPHA(^(.{1})\s+(bold|normal)\s+instance\s+\d+)ALPHA");
	const std::vector<SparrowFrame>& frames = atlas_.getFrames();
	for (std::size_t index = 0; index < frames.size(); ++index) {
		std::smatch match;
		if (!std::regex_match(frames[index].name, match, frameRegex)) {
			continue;
		}

		const bool frameIsBold = match[2].str() == "bold";
		if (frameIsBold != bold_) {
			continue;
		}

		const char key = static_cast<char>(std::toupper(static_cast<unsigned char>(match[1].str()[0])));
		if (characterToFrame_.find(key) == characterToFrame_.end()) {
			characterToFrame_.insert({key, index});
		}
	}

	return !characterToFrame_.empty();
}

void Alphabet::setText(const std::string& text)
{
	text_ = text;
}

void Alphabet::setPosition(float x, float y)
{
	x_ = x;
	y_ = y;
}

void Alphabet::setScale(float scale)
{
	scale_ = scale;
}

void Alphabet::setVisible(bool visible)
{
	visible_ = visible;
}

bool Alphabet::isVisible() const
{
	return visible_;
}

void Alphabet::render() const
{
	if (!visible_ || text_.empty()) {
		return;
	}

	float cursorX = x_;
	float cursorY = y_;
	for (char rawCharacter : text_) {
		if (rawCharacter == '\n') {
			cursorX = x_;
			cursorY += 72.0f * scale_;
			continue;
		}

		if (rawCharacter == ' ') {
			cursorX += 30.0f * scale_;
			continue;
		}

		const SparrowFrame* frame = resolveFrame(rawCharacter);
		if (frame == nullptr) {
			cursorX += 28.0f * scale_;
			continue;
		}

		Renderer::get().drawAtlasFrame(atlas_.getImagePath(), *frame, static_cast<int>(cursorX), static_cast<int>(cursorY), scale_);
		cursorX += (static_cast<float>(frame->sourceWidth > 0 ? frame->sourceWidth : frame->width) + 6.0f) * scale_;
	}
}

const SparrowFrame* Alphabet::resolveFrame(char character) const
{
	const char key = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
	const auto found = characterToFrame_.find(key);
	if (found == characterToFrame_.end()) {
		return nullptr;
	}

	const std::vector<SparrowFrame>& frames = atlas_.getFrames();
	return &frames[found->second];
}