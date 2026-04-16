#include "Renderer.h"

#include <stdexcept>

#include "SparrowAnimator.h"

Renderer& Renderer::get()
{
	static Renderer renderer;
	return renderer;
}

bool Renderer::initialize(HWND windowHandle)
{
	Gdiplus::GdiplusStartupInput startupInput;
	if (Gdiplus::GdiplusStartup(&gdiplusToken_, &startupInput, nullptr) != Gdiplus::Ok) {
		return false;
	}

	windowHandle_ = windowHandle;
	windowDeviceContext_ = GetDC(windowHandle_);
	if (windowDeviceContext_ == nullptr) {
		Gdiplus::GdiplusShutdown(gdiplusToken_);
		gdiplusToken_ = 0;
		return false;
	}

	backbufferDeviceContext_ = CreateCompatibleDC(windowDeviceContext_);
	if (backbufferDeviceContext_ == nullptr) {
		ReleaseDC(windowHandle_, windowDeviceContext_);
		Gdiplus::GdiplusShutdown(gdiplusToken_);
		gdiplusToken_ = 0;
		windowDeviceContext_ = nullptr;
		windowHandle_ = nullptr;
		return false;
	}

	RECT clientRect = {};
	GetClientRect(windowHandle_, &clientRect);
	resizeBackbuffer(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
	return backbufferBitmap_ != nullptr;
}

void Renderer::shutdown()
{
	for (const std::wstring& fontPath : registeredFonts_) {
		RemoveFontResourceExW(fontPath.c_str(), FR_PRIVATE, nullptr);
	}
	registeredFonts_.clear();
	bitmapCache_.clear();

	if (backbufferDeviceContext_ != nullptr && previousBitmap_ != nullptr) {
		SelectObject(backbufferDeviceContext_, previousBitmap_);
		previousBitmap_ = nullptr;
	}

	if (backbufferBitmap_ != nullptr) {
		DeleteObject(backbufferBitmap_);
		backbufferBitmap_ = nullptr;
	}

	if (backbufferDeviceContext_ != nullptr) {
		DeleteDC(backbufferDeviceContext_);
		backbufferDeviceContext_ = nullptr;
	}

	if (windowHandle_ != nullptr && windowDeviceContext_ != nullptr) {
		ReleaseDC(windowHandle_, windowDeviceContext_);
	}

	windowDeviceContext_ = nullptr;
	windowHandle_ = nullptr;
	width_ = 0;
	height_ = 0;
	if (gdiplusToken_ != 0) {
		Gdiplus::GdiplusShutdown(gdiplusToken_);
		gdiplusToken_ = 0;
	}
}

void Renderer::beginFrame(COLORREF clearColor)
{
	RECT clientRect = {};
	GetClientRect(windowHandle_, &clientRect);
	const int width = clientRect.right - clientRect.left;
	const int height = clientRect.bottom - clientRect.top;
	if (width != width_ || height != height_) {
		resizeBackbuffer(width, height);
	}

	const HBRUSH backgroundBrush = CreateSolidBrush(clearColor);
	FillRect(backbufferDeviceContext_, &clientRect, backgroundBrush);
	DeleteObject(backgroundBrush);
	SetBkMode(backbufferDeviceContext_, TRANSPARENT);
}

void Renderer::endFrame()
{
	BitBlt(windowDeviceContext_, 0, 0, width_, height_, backbufferDeviceContext_, 0, 0, SRCCOPY);
}

void Renderer::drawText(const std::wstring& text, int x, int y, int fontSize, COLORREF color, const std::wstring& fontFamily, int weight)
{
	RECT bounds = {x, y, width_, height_};
	drawCenteredText(text, bounds, fontSize, color, fontFamily, weight);
}

void Renderer::drawCenteredText(const std::wstring& text, const RECT& bounds, int fontSize, COLORREF color, const std::wstring& fontFamily, int weight)
{
	if (backbufferDeviceContext_ == nullptr) {
		return;
	}

	const int logicalHeight = -MulDiv(fontSize, GetDeviceCaps(backbufferDeviceContext_, LOGPIXELSY), 72);
	HFONT font = CreateFontW(logicalHeight, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontFamily.c_str());
	HFONT oldFont = static_cast<HFONT>(SelectObject(backbufferDeviceContext_, font));
	SetTextColor(backbufferDeviceContext_, color);
	DrawTextW(backbufferDeviceContext_, text.c_str(), -1, const_cast<RECT*>(&bounds), DT_NOPREFIX | DT_WORDBREAK | DT_LEFT | DT_TOP);
	SelectObject(backbufferDeviceContext_, oldFont);
	DeleteObject(font);
}

void Renderer::drawImage(const std::filesystem::path& imagePath, int x, int y, float scale)
{
	Gdiplus::Bitmap* bitmap = getBitmap(imagePath);
	if (bitmap == nullptr) {
		return;
	}

	Gdiplus::Graphics graphics(backbufferDeviceContext_);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	graphics.DrawImage(bitmap, Gdiplus::Rect(x, y, static_cast<INT>(bitmap->GetWidth() * scale), static_cast<INT>(bitmap->GetHeight() * scale)));
}

void Renderer::drawAtlasFrame(const std::filesystem::path& imagePath, const SparrowFrame& frame, int x, int y, float scale)
{
	Gdiplus::Bitmap* bitmap = getBitmap(imagePath);
	if (bitmap == nullptr) {
		return;
	}

	Gdiplus::Graphics graphics(backbufferDeviceContext_);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	const float drawX = static_cast<float>(x) + (frame.offsetX * scale);
	const float drawY = static_cast<float>(y) + (frame.offsetY * scale);
	const float drawWidth = static_cast<float>(frame.width) * scale;
	const float drawHeight = static_cast<float>(frame.height) * scale;

	const Gdiplus::GraphicsState state = graphics.Save();
	if (frame.rotated) {
		graphics.TranslateTransform(drawX, drawY + drawWidth);
		graphics.RotateTransform(static_cast<Gdiplus::REAL>(frame.angleDegrees));
	} else {
		graphics.TranslateTransform(drawX, drawY);
	}

	if (frame.flipX || frame.flipY) {
		graphics.TranslateTransform(frame.flipX ? drawWidth : 0.0f, frame.flipY ? drawHeight : 0.0f, Gdiplus::MatrixOrderAppend);
		graphics.ScaleTransform(frame.flipX ? -1.0f : 1.0f, frame.flipY ? -1.0f : 1.0f, Gdiplus::MatrixOrderAppend);
	}

	graphics.DrawImage(
		bitmap,
		Gdiplus::RectF(0.0f, 0.0f, drawWidth, drawHeight),
		static_cast<Gdiplus::REAL>(frame.x),
		static_cast<Gdiplus::REAL>(frame.y),
		static_cast<Gdiplus::REAL>(frame.width),
		static_cast<Gdiplus::REAL>(frame.height),
		Gdiplus::UnitPixel);
	graphics.Restore(state);
}

bool Renderer::registerFontFile(const std::filesystem::path& fontPath)
{
	const std::wstring widePath = fontPath.wstring();
	if (registeredFonts_.find(widePath) != registeredFonts_.end()) {
		return true;
	}

	if (!std::filesystem::exists(fontPath)) {
		return false;
	}

	if (AddFontResourceExW(widePath.c_str(), FR_PRIVATE, nullptr) == 0) {
		return false;
	}

	registeredFonts_.insert(widePath);
	return true;
}

Gdiplus::Bitmap* Renderer::getBitmap(const std::filesystem::path& imagePath)
{
	const std::wstring widePath = imagePath.wstring();
	const auto found = bitmapCache_.find(widePath);
	if (found != bitmapCache_.end()) {
		return found->second.get();
	}

	auto bitmap = std::make_unique<Gdiplus::Bitmap>(widePath.c_str());
	if (bitmap->GetLastStatus() != Gdiplus::Ok) {
		return nullptr;
	}

	Gdiplus::Bitmap* bitmapPtr = bitmap.get();
	bitmapCache_.insert({widePath, std::move(bitmap)});
	return bitmapPtr;
}

HWND Renderer::getWindowHandle() const
{
	return windowHandle_;
}

int Renderer::getWidth() const
{
	return width_;
}

int Renderer::getHeight() const
{
	return height_;
}

void Renderer::resizeBackbuffer(int width, int height)
{
	if (backbufferDeviceContext_ == nullptr || windowDeviceContext_ == nullptr || width <= 0 || height <= 0) {
		return;
	}

	if (previousBitmap_ != nullptr) {
		SelectObject(backbufferDeviceContext_, previousBitmap_);
		previousBitmap_ = nullptr;
	}

	if (backbufferBitmap_ != nullptr) {
		DeleteObject(backbufferBitmap_);
		backbufferBitmap_ = nullptr;
	}

	backbufferBitmap_ = CreateCompatibleBitmap(windowDeviceContext_, width, height);
	if (backbufferBitmap_ == nullptr) {
		throw std::runtime_error("Failed to resize the renderer backbuffer.");
	}

	previousBitmap_ = static_cast<HBITMAP>(SelectObject(backbufferDeviceContext_, backbufferBitmap_));
	width_ = width;
	height_ = height;
}