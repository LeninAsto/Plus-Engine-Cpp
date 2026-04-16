#include <chrono>
#include <filesystem>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "backend/Input.h"
#include "backend/Paths.h"
#include "backend/State.h"
#include "graphics/Renderer.h"
#include "states/MainMenuState.h"
#include "states/TitleState.h"
#include "ui/FPSCounter.h"

struct EngineConfig {
	int windowWidth = 1280;
	int windowHeight = 720;
	int targetFps = 60;
	bool startFullscreen = false;
	std::string title = "Plus Engine C++";
	std::string initialState = "TitleState";
};

class Engine {
public:
	explicit Engine(EngineConfig config)
		: config_(std::move(config)) {}

	int run()
	{
		try {
			installCrashHandler();
			initializeCoreSystems();
			loadPreferences();
			createWindow();
			loadInitialState();
			mainLoop();
			shutdown();
			return 0;
		} catch (const std::exception& error) {
			std::cerr << "Fatal error: " << error.what() << '\n';
			shutdown();
			return 1;
		}
	}

    void trace(const std::string& message)
    {
        std::cout << "[Trace] " << message << '\n';
    }

private:
	void installCrashHandler()
	{
		trace("Installing crash handler");
		// TODO: Replace this with a platform-specific crash handler.
	}

	void initializeCoreSystems()
	{
		trace("Initializing paths, memory, audio, scripting and renderer");
		Paths::initialize(std::filesystem::current_path());
	}

	void loadPreferences()
	{
		trace("Loading preferences and keybinds");
		// TODO: Load save data, options, keybinds and user profile.
	}

	void createWindow()
	{
		trace("Creating window: " + config_.title + " " + std::to_string(config_.windowWidth) + "x" + std::to_string(config_.windowHeight) + " @ " + std::to_string(config_.targetFps) + " FPS" + (config_.startFullscreen ? " fullscreen" : " windowed"));

		windowInstance_ = GetModuleHandleW(nullptr);
		windowClassName_ = L"PlusEngineWindowClass";

		WNDCLASSEXW windowClass = {};
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = Engine::windowProc;
		windowClass.hInstance = windowInstance_;
		windowClass.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		windowClass.lpszClassName = windowClassName_;

		if (!RegisterClassExW(&windowClass)) {
			throw std::runtime_error("Failed to register the game window class.");
		}

		RECT desiredRect = {0, 0, config_.windowWidth, config_.windowHeight};
		AdjustWindowRect(&desiredRect, WS_OVERLAPPEDWINDOW, FALSE);

		const std::wstring windowTitle(config_.title.begin(), config_.title.end());
		windowHandle_ = CreateWindowExW(
			0,
			windowClassName_,
			windowTitle.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			desiredRect.right - desiredRect.left,
			desiredRect.bottom - desiredRect.top,
			nullptr,
			nullptr,
			windowInstance_,
			this);

		if (windowHandle_ == nullptr) {
			UnregisterClassW(windowClassName_, windowInstance_);
			throw std::runtime_error("Failed to create the game window.");
		}

		ShowWindow(windowHandle_, SW_SHOW);
		UpdateWindow(windowHandle_);

		if (!Renderer::get().initialize(windowHandle_)) {
			throw std::runtime_error("Failed to initialize the renderer.");
		}

		fpsCounter_.initialize();
	}

	void loadInitialState()
	{
		trace("Loading initial state: " + config_.initialState);
		if (config_.initialState == "TitleState") {
			changeState(std::make_unique<TitleState>());
			return;
		}

		if (config_.initialState == "MainMenuState") {
			changeState(std::make_unique<MainMenuState>());
			return;
		}

		changeState(std::make_unique<State>(config_.initialState));
	}

	void mainLoop()
	{
		using clock = std::chrono::steady_clock;
		const auto frameTime = std::chrono::milliseconds(1000 / config_.targetFps);

		isRunning_ = true;
		auto previousFrame = clock::now();

		while (isRunning_) {
			const auto frameStart = clock::now();
			const float deltaSeconds = std::chrono::duration<float>(frameStart - previousFrame).count();
			previousFrame = frameStart;

			Input::beginFrame();
			processInput();
			if (!isRunning_) {
				break;
			}
			update(deltaSeconds);
			handlePendingStateChange();
			render();

			const auto frameEnd = clock::now();
			const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
			if (elapsed < frameTime) {
				std::this_thread::sleep_for(frameTime - elapsed);
			}
		}
	}

	void processInput()
	{
		MSG message = {};
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
			if (message.message == WM_QUIT) {
				isRunning_ = false;
				return;
			}

			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
	}

	void update(float deltaSeconds)
	{
		fpsCounter_.update(deltaSeconds);
		if (currentState_ != nullptr && currentState_->isActive()) {
			currentState_->update(deltaSeconds);
		}
	}

	void render()
	{
		Renderer::get().beginFrame(RGB(18, 24, 36));
		if (currentState_ != nullptr && currentState_->isVisible()) {
			currentState_->render();
		}
		fpsCounter_.render(12, 8);
		Renderer::get().endFrame();
	}

	void shutdown()
	{
		if (!didShutdown_) {
			didShutdown_ = true;
			if (currentState_ != nullptr) {
				currentState_->onExit();
				currentState_->ensureDestroyed();
				currentState_.reset();
			}

			Renderer::get().shutdown();

			if (windowHandle_ != nullptr) {
				DestroyWindow(windowHandle_);
				windowHandle_ = nullptr;
			}

			if (windowClassName_ != nullptr && windowInstance_ != nullptr) {
				UnregisterClassW(windowClassName_, windowInstance_);
				windowClassName_ = nullptr;
			}

			trace("Releasing engine resources");
		}
	}

	void handlePendingStateChange()
	{
		if (!State::hasPendingStateChange()) {
			return;
		}

		changeState(createStateByName(State::consumePendingStateChange()));
	}

	std::unique_ptr<State> createStateByName(const std::string& stateName)
	{
		if (stateName == "TitleState") {
			return std::make_unique<TitleState>();
		}

		if (stateName == "MainMenuState") {
			return std::make_unique<MainMenuState>();
		}

		return std::make_unique<State>(stateName);
	}

	static LRESULT CALLBACK windowProc(HWND windowHandle, UINT message, WPARAM wordParam, LPARAM longParam)
	{
		Engine* engine = nullptr;
		if (message == WM_NCCREATE) {
			CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(longParam);
			engine = static_cast<Engine*>(createStruct->lpCreateParams);
			SetWindowLongPtrW(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(engine));
		} else {
			engine = reinterpret_cast<Engine*>(GetWindowLongPtrW(windowHandle, GWLP_USERDATA));
		}

		switch (message) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			Input::setKeyDown(static_cast<unsigned int>(wordParam), true);
			return 0;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			Input::setKeyDown(static_cast<unsigned int>(wordParam), false);
			return 0;

		case WM_KILLFOCUS:
			Input::clear();
			return 0;

		case WM_CLOSE:
			DestroyWindow(windowHandle);
			return 0;

		case WM_DESTROY:
			if (engine != nullptr) {
				engine->windowHandle_ = nullptr;
				engine->isRunning_ = false;
			}
			PostQuitMessage(0);
			return 0;

		default:
			return DefWindowProcW(windowHandle, message, wordParam, longParam);
		}
	}

	void changeState(std::unique_ptr<State> nextState)
	{
		if (nextState == nullptr) {
			return;
		}

		if (currentState_ != nullptr) {
			currentState_->onExit();
			currentState_->ensureDestroyed();
		}

		currentState_ = std::move(nextState);
		trace("Switched to state: " + currentState_->getName());
		currentState_->ensureCreated();
		currentState_->onEnter();
	}

	EngineConfig config_;
	bool isRunning_ = false;
	bool didShutdown_ = false;
	std::unique_ptr<State> currentState_;
	FPSCounter fpsCounter_;
	HWND windowHandle_ = nullptr;
	HINSTANCE windowInstance_ = nullptr;
	const wchar_t* windowClassName_ = nullptr;
};

int main()
{
	EngineConfig config;
	Engine engine(config);
	return engine.run();
}
