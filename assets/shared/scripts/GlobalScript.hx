// Advanced scripted-class test for GlobalScript.
// Demonstrates: class definitions, onUpdate, onBeatHit, onSwitchState.
// Output goes to the debug console.

// ------------------------------------------------------------------
// Stopwatch — tracks real elapsed seconds, supports laps
// ------------------------------------------------------------------
class Stopwatch {
    var label:String;
    var elapsed:Float;
    var running:Bool;
    var laps:Array<Float>;

    function new(lbl:String) {
        label   = lbl;
        elapsed = 0.0;
        running = false;
        laps    = [];
    }

    function start() {
        running = true;
        trace('[Stopwatch] ' + label + ' started.');
    }

    function stop() {
        running = false;
        trace('[Stopwatch] ' + label + ' stopped at ' + _fmt(elapsed) + 's');
    }

    function reset() {
        elapsed = 0.0;
        running = false;
        laps    = [];
        trace('[Stopwatch] ' + label + ' reset.');
    }

    // Called from onUpdate — advances time only when running
    function tick(dt:Float) {
        if (running) elapsed += dt;
    }

    function lap() {
        laps.push(elapsed);
        trace('[Stopwatch] ' + label + ' lap ' + laps.length + ' -> ' + _fmt(elapsed) + 's');
    }

    function getElapsed():Float {
        return elapsed;
    }

    // Internal helper: round to 3 decimals
    function _fmt(v:Float):Float {
        return Math.round(v * 1000) / 1000;
    }
}

// ------------------------------------------------------------------
// BeatTracker — records beat timestamps, estimates live BPM
// ------------------------------------------------------------------
class BeatTracker {
    var beatTimes:Array<Float>;   // elapsed seconds at each beat
    var totalTime:Float;          // shared reference updated from onUpdate
    var windowSize:Int;           // how many recent beats to average

    function new(window:Int) {
        beatTimes  = [];
        totalTime  = 0.0;
        windowSize = window;
    }

    function tick(dt:Float) {
        totalTime += dt;
    }

    // Call this from onBeatHit
    function recordBeat(beat:Int) {
        beatTimes.push(totalTime);
        // Keep only the last windowSize+1 entries (need pairs to get intervals)
        while (beatTimes.length > windowSize + 1)
            beatTimes.shift();
    }

    // Returns estimated BPM from recent beats, or -1 if not enough data
    function getBPM():Float {
        var len = beatTimes.length;
        if (len < 2) return -1;

        var totalInterval:Float = beatTimes[len - 1] - beatTimes[0];
        if (totalInterval <= 0) return -1;

        var avgInterval:Float = totalInterval / (len - 1);
        return Math.round((60.0 / avgInterval) * 10) / 10; // 1 decimal
    }

    function reset() {
        beatTimes = [];
        totalTime = 0.0;
        trace('[BeatTracker] Reset.');
    }
}

// ------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------
var sw      = new Stopwatch("main");
var tracker = new BeatTracker(8); // average over last 8 beats

sw.start();
trace('[Test] Stopwatch and BeatTracker created. Listening to callbacks...');

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------
var _lapDone = false; // lap at ~2s only once per state

function onUpdate(elapsed:Float) {
    sw.tick(elapsed);
    tracker.tick(elapsed);

    // Take one lap at ~2 seconds to show it works mid-run
    if (!_lapDone && sw.getElapsed() >= 2.0) {
        sw.lap();
        _lapDone = true;
    }
}

function onBeatHit(beat:Int) {
    tracker.recordBeat(beat);

    var bpm:Float = tracker.getBPM();
    if (bpm > 0) {
        trace('[BeatTracker] beat ' + beat
            + ' | estimated BPM: ' + bpm
            + ' | conductor BPM: ' + Conductor.bpm);
    } else {
        trace('[BeatTracker] beat ' + beat + ' (collecting data...)');
    }
}

function onSwitchState(nextState:String) {
    // Report final stopwatch time before leaving the state
    sw.stop();
    trace('[Test] Switching to ' + nextState
        + ' after ' + sw.getElapsed() + 's'
        + ' | laps recorded: ' + sw.laps.length);

    // Reset everything for the next state
    sw.reset();
    tracker.reset();
    _lapDone = false;
    sw.start();
}
