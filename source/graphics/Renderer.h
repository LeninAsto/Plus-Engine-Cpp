#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#include "SparrowAnimator.h"

class Renderer
{
public:
	static Renderer& get();

	bool initialize(HWND windowHandle);
	void shutdown();

	void beginFrame(COLORREF clearColor);
	void endFrame();

	void drawText(const std::wstring& text, int x, int y, int fontSize, COLORREF color, const std::wstring& fontFamily = L"Segoe UI", int weight = FW_NORMAL);
	void drawCenteredText(const std::wstring& text, const RECT& bounds, int fontSize, COLORREF color, const std::wstring& fontFamily = L"Segoe UI", int weight = FW_NORMAL);
	void drawImage(const std::filesystem::path& imagePath, int x, int y, float scale = 1.0f);
	void drawAtlasFrame(const std::filesystem::path& imagePath, const SparrowFrame& frame, int x, int y, float scale = 1.0f);

	bool registerFontFile(const std::filesystem::path& fontPath);

	HWND getWindowHandle() const;
	int getWidth() const;
	int getHeight() const;

private:
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Gdiplus::Bitmap* getBitmap(const std::filesystem::path& imagePath);
	void resizeBackbuffer(int width, int height);

	HWND windowHandle_ = nullptr;
	HDC windowDeviceContext_ = nullptr;
	HDC backbufferDeviceContext_ = nullptr;
	HBITMAP backbufferBitmap_ = nullptr;
	HBITMAP previousBitmap_ = nullptr;
	int width_ = 0;
	int height_ = 0;
	ULONG_PTR gdiplusToken_ = 0;
	std::unordered_set<std::wstring> registeredFonts_;
	std::unordered_map<std::wstring, std::unique_ptr<Gdiplus::Bitmap>> bitmapCache_;
};