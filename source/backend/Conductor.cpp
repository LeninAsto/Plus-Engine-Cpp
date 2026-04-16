#include "Conductor.h"

#include <algorithm>
#include <cmath>

std::vector<BPMChangeEvent> Conductor::bpmChanges_;
double Conductor::songPositionMs_ = 0.0;
double Conductor::bpm_ = 100.0;
double Conductor::crochetMs_ = 600.0;
double Conductor::stepCrochetMs_ = 150.0;

void Conductor::reset(double bpm)
{
	bpmChanges_.clear();
	setBpm(bpm);
	songPositionMs_ = 0.0;
	bpmChanges_.push_back(buildChangeEvent(0, 0.0, bpm_));
}

void Conductor::setBpm(double bpm)
{
	bpm_ = bpm;
	crochetMs_ = (60.0 / bpm_) * 1000.0;
	stepCrochetMs_ = crochetMs_ / 4.0;
}

void Conductor::setSongPosition(double songPositionMs)
{
	songPositionMs_ = songPositionMs;
}

void Conductor::update(double deltaSeconds)
{
	songPositionMs_ += deltaSeconds * 1000.0;
}

void Conductor::addBpmChange(int stepTime, double songTimeMs, double bpm)
{
	bpmChanges_.push_back(buildChangeEvent(stepTime, songTimeMs, bpm));
	std::sort(bpmChanges_.begin(), bpmChanges_.end(), [](const BPMChangeEvent& left, const BPMChangeEvent& right) {
		return left.songTimeMs < right.songTimeMs;
	});
}

BPMChangeEvent Conductor::getBpmFromSeconds(double songPositionMs)
{
	BPMChangeEvent result = bpmChanges_.empty() ? buildChangeEvent(0, 0.0, bpm_) : bpmChanges_.front();
	for (const BPMChangeEvent& change : bpmChanges_) {
		if (songPositionMs >= change.songTimeMs) {
			result = change;
		} else {
			break;
		}
	}

	return result;
}

int Conductor::getCurrentStep(double songPositionMs)
{
	const BPMChangeEvent currentChange = getBpmFromSeconds(songPositionMs);
	const double localTimeMs = songPositionMs - currentChange.songTimeMs;
	return currentChange.stepTime + static_cast<int>(std::floor(localTimeMs / currentChange.stepCrochetMs));
}

int Conductor::getCurrentBeat(double songPositionMs)
{
	return getCurrentStep(songPositionMs) / 4;
}

double Conductor::getSongPositionMs()
{
	return songPositionMs_;
}

double Conductor::getBpm()
{
	return bpm_;
}

double Conductor::getCrochetMs()
{
	return crochetMs_;
}

double Conductor::getStepCrochetMs()
{
	return stepCrochetMs_;
}

const std::vector<BPMChangeEvent>& Conductor::getBpmChanges()
{
	return bpmChanges_;
}

BPMChangeEvent Conductor::buildChangeEvent(int stepTime, double songTimeMs, double bpm)
{
	BPMChangeEvent event;
	event.stepTime = stepTime;
	event.songTimeMs = songTimeMs;
	event.bpm = bpm;
	event.crochetMs = (60.0 / bpm) * 1000.0;
	event.stepCrochetMs = event.crochetMs / 4.0;
	return event;
}