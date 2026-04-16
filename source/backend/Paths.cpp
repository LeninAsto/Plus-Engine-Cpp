#include "Paths.h"

#include <stdexcept>

bool Paths::initialized_ = false;
std::filesystem::path Paths::projectRoot_;
std::filesystem::path Paths::assetsRoot_;

void Paths::initialize(const std::filesystem::path& hintPath)
{
	projectRoot_ = findProjectRoot(hintPath);
	assetsRoot_ = projectRoot_ / "assets";
	initialized_ = true;
}

bool Paths::isInitialized()
{
	return initialized_;
}

const std::filesystem::path& Paths::projectRoot()
{
	if (!initialized_) {
		throw std::runtime_error("Paths::initialize() must be called before using projectRoot().");
	}

	return projectRoot_;
}

const std::filesystem::path& Paths::assetsRoot()
{
	if (!initialized_) {
		throw std::runtime_error("Paths::initialize() must be called before using assetsRoot().");
	}

	return assetsRoot_;
}

std::filesystem::path Paths::get(const std::string& relativePath)
{
	return assetsRoot() / relativePath;
}

std::filesystem::path Paths::font(const std::string& fileName)
{
	return assetsRoot() / "fonts" / fileName;
}

std::filesystem::path Paths::image(const std::string& relativePath)
{
	return assetsRoot() / withExtensionIfMissing(relativePath, ".png");
}

std::filesystem::path Paths::xml(const std::string& relativePath)
{
	return assetsRoot() / withExtensionIfMissing(relativePath, ".xml");
}

std::filesystem::path Paths::sound(const std::string& relativePath)
{
	return assetsRoot() / withExtensionIfMissing(relativePath, ".ogg");
}

std::filesystem::path Paths::music(const std::string& relativePath)
{
	std::filesystem::path path(relativePath);
	if (!path.has_extension()) {
		const std::filesystem::path mp3Path = assetsRoot() / "shared" / "music" / (relativePath + ".mp3");
		if (std::filesystem::exists(mp3Path)) {
			return mp3Path;
		}

		return assetsRoot() / "shared" / "music" / (relativePath + ".ogg");
	}

	return assetsRoot() / "shared" / "music" / relativePath;
}

bool Paths::exists(const std::filesystem::path& path)
{
	return std::filesystem::exists(path);
}

std::filesystem::path Paths::withExtensionIfMissing(const std::string& relativePath, const std::string& extension)
{
	std::filesystem::path path(relativePath);
	if (!path.has_extension()) {
		path += extension;
	}

	return path;
}

std::filesystem::path Paths::findProjectRoot(const std::filesystem::path& hintPath)
{
	std::filesystem::path current = hintPath;
	if (!std::filesystem::is_directory(current)) {
		current = current.parent_path();
	}

	while (!current.empty()) {
		if (std::filesystem::exists(current / "assets") && std::filesystem::exists(current / "source")) {
			return current;
		}

		const std::filesystem::path parent = current.parent_path();
		if (parent == current) {
			break;
		}

		current = parent;
	}

	throw std::runtime_error("Failed to locate the project root from the provided path.");
}