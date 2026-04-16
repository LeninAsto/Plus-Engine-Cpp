#include "FPSCounter.h"

#include <iomanip>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#include "../backend/Paths.h"
#include "../graphics/Renderer.h"

void FPSCounter::initialize()
{
	fontReady_ = Renderer::get().registerFontFile(Paths::font("inter.otf"));
	updateMemoryUsage();
}

void FPSCounter::update(float deltaSeconds)
{
	++frameCount_;
	accumulator_ += deltaSeconds;
	lastDelayMs_ = deltaSeconds * 1000.0f;
	if (accumulator_ >= 1.0f) {
		fps_ = frameCount_;
		frameCount_ = 0;
		accumulator_ -= 1.0f;
		updateMemoryUsage();
	}
}

void FPSCounter::render(int x, int y) const
{
	if (!visible_) {
		return;
	}

	std::wstringstream stream;
	stream << L"FPS: " << fps_
		<< L"\nDelay: " << std::fixed << std::setprecision(2) << lastDelayMs_ << L" ms"
		<< L"\nGC: N/A"
		<< L"\nMemory: " << std::fixed << std::setprecision(2) << memoryMb_ << L" MB";

	Renderer::get().drawText(stream.str(), x, y, 10, RGB(180, 255, 180), fontReady_ ? L"Inter" : L"Segoe UI", FW_BOLD);
}

void FPSCounter::setVisible(bool visible)
{
	visible_ = visible;
}

bool FPSCounter::isVisible() const
{
	return visible_;
}

void FPSCounter::updateMemoryUsage()
{
	PROCESS_MEMORY_COUNTERS_EX memoryCounters = {};
	if (GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memoryCounters), sizeof(memoryCounters))) {
		memoryMb_ = static_cast<double>(memoryCounters.WorkingSetSize) / (1024.0 * 1024.0);
	}
}