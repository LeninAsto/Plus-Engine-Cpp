#pragma once

#include <vector>

struct BPMChangeEvent
{
	int stepTime = 0;
	double songTimeMs = 0.0;
	double bpm = 100.0;
	double crochetMs = 600.0;
	double stepCrochetMs = 150.0;
};

class Conductor
{
public:
	static void reset(double bpm = 100.0);
	static void setBpm(double bpm);
	static void setSongPosition(double songPositionMs);
	static void update(double deltaSeconds);

	static void addBpmChange(int stepTime, double songTimeMs, double bpm);
	static BPMChangeEvent getBpmFromSeconds(double songPositionMs);

	static int getCurrentStep(double songPositionMs);
	static int getCurrentBeat(double songPositionMs);

	static double getSongPositionMs();
	static double getBpm();
	static double getCrochetMs();
	static double getStepCrochetMs();

	static const std::vector<BPMChangeEvent>& getBpmChanges();

private:
	static BPMChangeEvent buildChangeEvent(int stepTime, double songTimeMs, double bpm);

	static std::vector<BPMChangeEvent> bpmChanges_;
	static double songPositionMs_;
	static double bpm_;
	static double crochetMs_;
	static double stepCrochetMs_;
};