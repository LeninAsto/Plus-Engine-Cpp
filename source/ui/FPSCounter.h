#pragma once

class FPSCounter
{
public:
	void initialize();
	void update(float deltaSeconds);
	void render(int x, int y) const;

	void setVisible(bool visible);
	bool isVisible() const;

private:
	void updateMemoryUsage();

	bool visible_ = true;
	bool fontReady_ = false;
	int fps_ = 0;
	int frameCount_ = 0;
	float accumulator_ = 0.0f;
	float lastDelayMs_ = 0.0f;
	double memoryMb_ = 0.0;
};