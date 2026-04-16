#pragma once

#include <filesystem>
#include <string>

class Paths
{
public:
	static void initialize(const std::filesystem::path& hintPath);
	static bool isInitialized();

	static const std::filesystem::path& projectRoot();
	static const std::filesystem::path& assetsRoot();

	static std::filesystem::path get(const std::string& relativePath);
	static std::filesystem::path font(const std::string& fileName);
	static std::filesystem::path image(const std::string& relativePath);
	static std::filesystem::path xml(const std::string& relativePath);
	static std::filesystem::path sound(const std::string& relativePath);
	static std::filesystem::path music(const std::string& relativePath);

	static bool exists(const std::filesystem::path& path);

private:
	static std::filesystem::path withExtensionIfMissing(const std::string& relativePath, const std::string& extension);
	static std::filesystem::path findProjectRoot(const std::filesystem::path& hintPath);

	static bool initialized_;
	static std::filesystem::path projectRoot_;
	static std::filesystem::path assetsRoot_;
};