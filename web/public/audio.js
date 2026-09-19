// Tiny synthesized sound effects via the Web Audio API. No external audio
// files are used, so there's nothing to license or ship as a binary asset.

let audioCtx = null;

function getContext() {
  if (!audioCtx) {
    const AudioContextClass = window.AudioContext || window.webkitAudioContext;
    audioCtx = new AudioContextClass();
  }
  if (audioCtx.state === "suspended") {
    audioCtx.resume().catch(() => {});
  }
  return audioCtx;
}

// Call synchronously from inside a click handler (before any `await`) to
// satisfy strict autoplay policies (notably Safari) that require the
// AudioContext to be created/resumed within the actual user gesture, even
// if the sound itself is only played later via a timer.
export function unlockAudio() {
  getContext();
}

// A short percussive "thud/pop" for a piece landing: a quick pitch drop
// combined with a fast volume envelope.
export function playDropSound() {
  try {
    const ctx = getContext();
    const now = ctx.currentTime;

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();

    osc.type = "sine";
    osc.frequency.setValueAtTime(240, now);
    osc.frequency.exponentialRampToValueAtTime(60, now + 0.14);

    gain.gain.setValueAtTime(0.0001, now);
    gain.gain.exponentialRampToValueAtTime(0.35, now + 0.008);
    gain.gain.exponentialRampToValueAtTime(0.0001, now + 0.2);

    osc.connect(gain).connect(ctx.destination);
    osc.start(now);
    osc.stop(now + 0.22);
  } catch {
    // Audio is a nice-to-have; never let it break gameplay.
  }
}

// A brief upward chirp for a win/draw banner appearing.
export function playChime() {
  try {
    const ctx = getContext();
    const now = ctx.currentTime;

    [523.25, 659.25, 783.99].forEach((freq, i) => {
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      const start = now + i * 0.09;

      osc.type = "triangle";
      osc.frequency.setValueAtTime(freq, start);

      gain.gain.setValueAtTime(0.0001, start);
      gain.gain.exponentialRampToValueAtTime(0.25, start + 0.02);
      gain.gain.exponentialRampToValueAtTime(0.0001, start + 0.35);

      osc.connect(gain).connect(ctx.destination);
      osc.start(start);
      osc.stop(start + 0.4);
    });
  } catch {
    // Audio is a nice-to-have; never let it break gameplay.
  }
}
