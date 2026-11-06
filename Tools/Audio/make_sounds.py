"""Aetheria sound set, synthesized from scratch (no samples, no third-party audio).

Writes 48 kHz 16-bit WAVs to Source_Art/Audio for import to /Game/Aetheria/Audio. Pure Python (wave, math,
array): every sound is built from oscillators, filtered noise and envelopes, seeded so the output is repeatable.

  A_*  ambience loops (seamless)       S_*  one-shot effects       M_*  music stings (stereo)

Run:  python Tools/Audio/make_sounds.py [out_dir] [name ...]
"""
import array
import math
import os
import random
import sys
import wave

SR = 48000
TAU = 2.0 * math.pi


# ---- building blocks -----------------------------------------------------------------------------

def n_of(seconds):
    return int(seconds * SR)


def zeros(n):
    return [0.0] * n


def sweep_freq(f0, f1, n, curve="exp"):
    """Per-sample frequency gliding from f0 to f1."""
    if n <= 1:
        return [f0] * n
    if curve == "exp":
        r = math.log(f1 / f0)
        return [f0 * math.exp(r * i / (n - 1)) for i in range(n)]
    return [f0 + (f1 - f0) * i / (n - 1) for i in range(n)]


def osc(freq, n, shape="sine", phase=0.0, fm=None, fm_index=None):
    """Oscillator; freq is a number or per-sample list. Optional FM: fm is a modulator signal, fm_index its depth."""
    out = [0.0] * n
    ph = phase
    for i in range(n):
        f = freq[i] if isinstance(freq, list) else freq
        p = ph
        if fm is not None:
            p += (fm_index[i] if isinstance(fm_index, list) else fm_index) * fm[i]
        if shape == "sine":
            out[i] = math.sin(p)
        else:
            x = (p / TAU) % 1.0
            if shape == "saw":
                out[i] = 2.0 * x - 1.0
            elif shape == "square":
                out[i] = 1.0 if x < 0.5 else -1.0
            elif shape == "tri":
                out[i] = 4.0 * abs(x - 0.5) - 1.0
        ph += TAU * f / SR
    return out


def noise(n, seed):
    rng = random.Random(seed)
    return [rng.random() * 2.0 - 1.0 for _ in range(n)]


def lowpass(x, cutoff, passes=1):
    """One-pole lowpass; cutoff is a number or per-sample list (Hz)."""
    for _ in range(passes):
        y = 0.0
        out = [0.0] * len(x)
        for i, v in enumerate(x):
            fc = cutoff[i] if isinstance(cutoff, list) else cutoff
            a = 1.0 - math.exp(-TAU * fc / SR)
            y += a * (v - y)
            out[i] = y
        x = out
    return x


def highpass(x, cutoff):
    lp = lowpass(x, cutoff)
    return [a - b for a, b in zip(x, lp)]


def bandpass(x, lo, hi):
    return highpass(lowpass(x, hi, 2), lo)


def env_ad(n, attack, decay_tau):
    """Linear attack then exponential decay (seconds)."""
    a = max(1, n_of(attack))
    out = [0.0] * n
    for i in range(n):
        if i < a:
            out[i] = i / a
        else:
            out[i] = math.exp(-(i - a) / (decay_tau * SR))
    return out


def env_points(n, points):
    """Piecewise-linear envelope through (time_s, level) points."""
    out = [0.0] * n
    pts = [(n_of(t), v) for t, v in points]
    k = 0
    for i in range(n):
        while k + 1 < len(pts) - 1 and i >= pts[k + 1][0]:
            k += 1
        (i0, v0), (i1, v1) = pts[k], pts[min(k + 1, len(pts) - 1)]
        out[i] = v0 if i1 == i0 else v0 + (v1 - v0) * min(1.0, max(0.0, (i - i0) / (i1 - i0)))
    return out


def mul(a, b):
    return [x * y for x, y in zip(a, b)]


def gain(a, g):
    return [x * g for x in a]


def mix(*signals):
    n = max(len(s) for s in signals)
    out = [0.0] * n
    for s in signals:
        for i, v in enumerate(s):
            out[i] += v
    return out


def pad_to(x, n):
    return x + [0.0] * (n - len(x)) if len(x) < n else x[:n]


def place(dst, src, start):
    for i, v in enumerate(src):
        if 0 <= start + i < len(dst):
            dst[start + i] += v
    return dst


def softclip(x, drive=1.0):
    return [math.tanh(v * drive) for v in x]


def reverb(x, wet=0.25, size=1.0, tail=1.5):
    """Small Schroeder reverb (four combs, two allpasses), mono."""
    n = len(x) + n_of(tail)
    dry = pad_to(x, n)
    combs = [int(d * size) for d in (1557, 1617, 1491, 1422)]
    out = [0.0] * n
    for d in combs:
        buf = [0.0] * d
        fb, lp_state = 0.82, 0.0
        for i in range(n):
            y = buf[i % d]
            lp_state = 0.75 * y + 0.25 * lp_state
            buf[i % d] = dry[i] + fb * lp_state
            out[i] += y * 0.25
    for d in (225, 556):
        buf = [0.0] * d
        for i in range(n):
            b = buf[i % d]
            v = out[i]
            y = -v + b
            buf[i % d] = v + b * 0.5
            out[i] = y
    return [d * (1 - wet) + w * wet for d, w in zip(dry, out)]


def loopify(x, xfade):
    """Make x loop seamlessly: crossfade its last `xfade` samples into its start, then drop them."""
    n = len(x) - xfade
    out = x[:n]
    for i in range(xfade):
        t = i / xfade
        out[i] = x[n + i] * (1 - t) + x[i] * t
    return out


def normalize(x, peak_db=-1.0):
    m = max(1e-9, max(abs(v) for v in x))
    g = 10 ** (peak_db / 20) / m
    return [v * g for v in x]


def fade(x, fade_in=0.002, fade_out=0.01):
    n = len(x)
    fi, fo = max(1, n_of(fade_in)), max(1, n_of(fade_out))
    out = list(x)
    for i in range(min(fi, n)):
        out[i] *= i / fi
    for i in range(min(fo, n)):
        out[n - 1 - i] *= i / fo
    return out


def write_wav(path, left, right=None):
    data = array.array("h")
    if right is None:
        for v in left:
            data.append(int(max(-1.0, min(1.0, v)) * 32767))
        channels = 1
    else:
        for l, r in zip(left, right):
            data.append(int(max(-1.0, min(1.0, l)) * 32767))
            data.append(int(max(-1.0, min(1.0, r)) * 32767))
        channels = 2
    with wave.open(path, "wb") as w:
        w.setnchannels(channels)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(data.tobytes())


def note(name):
    """'A4' -> Hz (equal temperament)."""
    names = {"C": -9, "C#": -8, "Db": -8, "D": -7, "D#": -6, "Eb": -6, "E": -5, "F": -4, "F#": -3, "Gb": -3,
             "G": -2, "G#": -1, "Ab": -1, "A": 0, "A#": 1, "Bb": 1, "B": 2}
    pitch, octave = name[:-1], int(name[-1])
    return 440.0 * 2 ** ((names[pitch] + 12 * (octave - 4)) / 12)


# ---- ambience ------------------------------------------------------------------------------------

def bridge_hum():
    """12 s loop: the ship's deep machinery drone, mains hum and ventilation air."""
    loop = 12.0
    n = n_of(loop) + n_of(1.0)
    # partials with whole cycles over the loop, slightly detuned pairs for a slow beat
    drone = mix(gain(osc(55.0, n), 0.5), gain(osc(55.25, n), 0.35), gain(osc(110.0, n), 0.22),
                gain(osc(165.0833, n), 0.08), gain(osc(120.0, n, "tri"), 0.04))
    air = gain(bandpass(noise(n, 11), 90, 900), 1.6)
    swell = osc(1 / 6.0, n)                       # breathing of the vents, two per loop
    air = [a * (0.75 + 0.25 * s) for a, s in zip(air, swell)]
    return normalize(loopify(mix(drone, air), n_of(1.0)), -6)


def combat_bed():
    """12 s loop under combat: very low rumble and a faint hiss."""
    n = n_of(12.0) + n_of(1.0)
    rumble = mix(gain(osc(36.0, n), 0.5), gain(lowpass(noise(n, 21), 120, 2), 5.0))
    hiss = gain(highpass(noise(n, 22), 4000), 0.05)
    return normalize(loopify(mix(rumble, hiss), n_of(1.0)), -9)


def engine_loop():
    """6 s loop: a ship's drive rumble with a faint turbine whine (pitch set per ship)."""
    n = n_of(6.0) + n_of(0.5)
    body = gain(lowpass(noise(n, 31), 380, 2), 4.0)
    tone = mix(gain(osc(48.0, n), 0.5), gain(osc(96.0, n), 0.25))
    whine = gain(osc(1000.0, n), 0.03)
    am = osc(0.5, n)
    sig = [(b + t) * (0.85 + 0.15 * a) + w for b, t, w, a in zip(body, tone, whine, am)]
    return normalize(loopify(sig, n_of(0.5)), -3)


# ---- weapons -------------------------------------------------------------------------------------

def blaster(seed, f0, f1, dur, ratio, index, noise_amt, cutoff, drive=1.0, shimmer=0.0):
    rng = random.Random(seed)
    k = rng.uniform(0.94, 1.06)
    n = n_of(dur)
    freq = sweep_freq(f0 * k, f1 * k, n)
    mod = osc([f * ratio for f in freq], n)
    idx = [index * math.exp(-i / (0.18 * n)) for i in range(n)]
    tone = osc(freq, n, fm=mod, fm_index=idx)
    if shimmer:
        tone = mix(tone, gain(osc([f * 2.01 for f in freq], n), shimmer))
    burst = mul(lowpass(noise(n, seed + 100), cutoff), env_ad(n, 0.001, 0.02))
    sig = mix(tone, gain(burst, noise_amt))
    sig = mul(softclip(sig, drive), env_ad(n, 0.002, dur * 0.28))
    return normalize(fade(sig), -1)


def impact(seed):
    n = n_of(0.35)
    crack = mul(lowpass(noise(n, seed), 3500), env_ad(n, 0.001, 0.03))
    thump = mul(osc(sweep_freq(90, 40, n), n), env_ad(n, 0.002, 0.08))
    return normalize(fade(mix(gain(crack, 0.8), thump)), -2)


def explosion(seed):
    n = n_of(2.8)
    cut = sweep_freq(7000, 250, n)
    body = mul(lowpass(noise(n, seed), cut, 2), env_ad(n, 0.004, 0.7))
    sub = mul(osc(sweep_freq(70, 28, n), n), env_ad(n, 0.003, 0.5))
    rng = random.Random(seed + 7)
    crackle = zeros(n)
    for _ in range(140):
        t = int((rng.random() ** 2) * n * 0.8)
        crackle[t] = rng.uniform(-1, 1) * math.exp(-t / (0.9 * SR))
    crackle = highpass(crackle, 1500)
    sig = mix(gain(body, 1.6), gain(sub, 1.0), gain(crackle, 2.5))
    return normalize(fade(softclip(sig, 1.5), 0.001, 0.4), -1)


def beam_charge():
    """Prismatic Stellar Beam charging: rising shimmering whine, 1.6 s."""
    n = n_of(1.6)
    voices = []
    for det in (1.0, 1.007, 0.993, 2.0):
        f = sweep_freq(180 * det, 1500 * det, n)
        voices.append(gain(osc(f, n), 0.5 if det != 2.0 else 0.18))
    trem_rate = sweep_freq(4, 28, n)
    trem = osc(trem_rate, n)
    sig = [v * (0.75 + 0.25 * t) for v, t in zip(mix(*voices), trem)]
    sig = mul(sig, env_points(n, [(0, 0), (1.3, 0.8), (1.58, 1.0), (1.6, 0.0)]))
    return normalize(sig, -2)


def beam_fire():
    """The beam discharging: roar of the lance and a crystalline ring, 2.8 s."""
    n = n_of(2.8)
    lance = lowpass(mix(osc(55, n, "saw"), osc(110.5, n, "saw"), gain(osc(165.3, n, "saw"), 0.6)), 900, 2)
    roar = bandpass(noise(n, 41), 250, 2500)
    vib = osc(5.5, n)
    ring = osc([2200 + 40 * v for v in vib], n)
    sig = mix(gain(lance, 0.9), gain(roar, 2.2), gain(ring, 0.2))
    sig = mul(softclip(sig, 1.6), env_points(n, [(0, 0), (0.03, 1.0), (1.8, 0.8), (2.8, 0.0)]))
    return normalize(sig, -1)


def pulse_telegraph():
    """Silence Pulse building, 1.5 s: a glassy chord swelling over a falling sub tone."""
    n = n_of(1.5)
    glass = mix(*(gain(osc(f, n), 0.33) for f in (880, 884, 1318.5, 1760)))
    sub = gain(osc(sweep_freq(120, 60, n), n), 0.6)
    sig = mul(mix(glass, sub), env_points(n, [(0, 0), (1.3, 0.7), (1.48, 1.0), (1.5, 0.0)]))
    return normalize(sig, -2)


def pulse_release():
    """Silence Pulse release, 1.4 s: a deep whump, a short inward rush, then a high ring dying away."""
    n = n_of(1.4)
    whump = mul(osc(sweep_freq(95, 28, n), n), env_ad(n, 0.004, 0.25))
    rush = mul(bandpass(noise(n, 51), 300, 5000), env_points(n, [(0, 1.0), (0.12, 0.0), (1.4, 0.0)]))
    ring = mul(mix(osc(2637, n), gain(osc(2650, n), 0.7)), env_ad(n, 0.005, 0.35))
    return normalize(fade(mix(gain(whump, 1.2), gain(rush, 0.9), gain(ring, 0.18)), 0.001, 0.2), -1)


def surge():
    """Chrono Surge: a rising tape-speed whoosh, 1.3 s."""
    n = n_of(1.3)
    whoosh = mul(lowpass(noise(n, 61), sweep_freq(300, 9000, n), 2), env_points(n, [(0, 0), (1.0, 1.0), (1.3, 0)]))
    tone = mul(osc(sweep_freq(140, 1100, n), n, "tri"), env_points(n, [(0, 0), (1.1, 0.6), (1.3, 0)]))
    return normalize(mix(gain(whoosh, 2.5), gain(tone, 0.4)), -2)


# ---- bridge consoles and UI ----------------------------------------------------------------------

SCALE = [1046.5, 1174.7, 1396.9, 1568.0, 1760.0, 2093.0, 2349.3, 2793.8, 3136.0, 3520.0]


def chirp(seed):
    """A console readout: a short phrase of electronic blips."""
    rng = random.Random(seed)
    kind = seed % 4
    out = zeros(n_of(0.9))
    t = 0
    if kind == 3:                                        # data chatter
        for _ in range(rng.randint(14, 22)):
            d = n_of(0.014)
            b = mul(osc(rng.choice(SCALE) * 1.5, d, "square"), env_ad(d, 0.001, 0.006))
            place(out, gain(lowpass(b, 6000), 0.5), t)
            t += d + n_of(0.006)
    else:
        for _ in range(rng.randint(2, 6)):
            dur = rng.uniform(0.03, 0.09)
            d = n_of(dur)
            f0 = rng.choice(SCALE)
            f1 = f0 * rng.choice((1.0, 1.0, 1.5, 0.75, 2.0)) if kind == 1 else f0
            b = mul(osc(sweep_freq(f0, f1, d), d, "sine" if kind != 2 else "tri"), env_ad(d, 0.002, dur * 0.5))
            place(out, b, t)
            t += d + n_of(rng.uniform(0.01, 0.06))
    out = out[:t + n_of(0.05)]
    return normalize(fade(out), -3)


def relay_click():
    n = n_of(0.12)
    out = zeros(n)
    for start in (0, n_of(0.045)):
        c = mul(highpass(noise(n_of(0.006), 71 + start), 1500), env_ad(n_of(0.006), 0.0005, 0.0015))
        place(out, c, start)
    return normalize(out, -4)


def ui_click():
    n = n_of(0.05)
    tick = mul(osc(2600, n), env_ad(n, 0.0005, 0.006))
    return normalize(fade(tick), -6)


# ---- music stings (stereo) -----------------------------------------------------------------------

def brass(freqs, n, env, bright=2200.0, seed=0):
    """Swelling synth brass: detuned saws per voice, the filter opening with the envelope."""
    rng = random.Random(seed)
    voices = []
    for f in freqs:
        for det in (1.0, 1.004, 0.996):
            voices.append(osc(f * det, n, "saw", phase=rng.random() * TAU))
    body = gain(mix(*voices), 1.0 / len(voices))
    cutoff = [300 + bright * e for e in env]
    return mul(lowpass(body, cutoff, 2), env)


def strings(freqs, n, env, seed=0):
    rng = random.Random(seed)
    voices = [osc(f * d, n, "saw", phase=rng.random() * TAU) for f in freqs for d in (1.0, 1.006, 0.994, 2.003)]
    return mul(lowpass(gain(mix(*voices), 1.0 / len(voices)), 1600, 2), env)


def timpani(f, n):
    d = n_of(1.6)
    hit = mix(mul(osc(sweep_freq(f * 1.3, f, d), d), env_ad(d, 0.003, 0.5)),
              gain(mul(lowpass(noise(d, 81), 900), env_ad(d, 0.001, 0.05)), 0.5))
    return pad_to(hit, n)


def stereo(sig, width=0.006, seed=0):
    """Slightly different left/right: a short delay on one side."""
    d = n_of(width)
    left = sig
    right = [0.0] * d + sig[:len(sig) - d]
    return left, right


def finish_sting(sig, peak=-3):
    sig = normalize(reverb(sig, wet=0.3, size=1.2, tail=1.6), peak)
    sig = fade(sig, 0.002, 0.6)
    return stereo(sig)


def sting_council():
    """A council convenes: a grave minor chord swelling over a timpani stroke."""
    n = n_of(4.5)
    env = env_points(n, [(0, 0), (0.9, 1.0), (2.6, 0.8), (4.5, 0)])
    chord = [note("D3"), note("F3"), note("A3"), note("D4")]
    sig = mix(brass(chord, n, env, 1600, 1), gain(strings([note("D2"), note("A2")], n, env, 2), 0.8),
              gain(timpani(note("D2"), n), 0.9))
    return finish_sting(sig)


def sting_victory():
    """Victory: a rising major call, then the full chord."""
    n = n_of(5.0)
    sig = zeros(n)
    for i, (nm, t) in enumerate((("G4", 0.0), ("C5", 0.28), ("E5", 0.56), ("G5", 0.84))):
        d = n_of(0.5)
        place(sig, gain(brass([note(nm)], d, env_points(d, [(0, 0), (0.04, 1), (0.4, 0.7), (0.5, 0)]), 3200, i), 0.8),
              n_of(t))
    d = n_of(4.0)
    env = env_points(d, [(0, 0), (0.1, 1.0), (2.2, 0.75), (4.0, 0)])
    place(sig, brass([note("C4"), note("E4"), note("G4"), note("C5")], d, env, 3000, 9), n_of(1.15))
    place(sig, gain(strings([note("C3"), note("G3")], d, env, 10), 0.7), n_of(1.15))
    place(sig, gain(timpani(note("C2"), n_of(1.6)), 1.0), n_of(1.15))
    return finish_sting(sig)


def sting_defeat():
    """Defeat: a falling minor line into a dark low chord."""
    n = n_of(5.0)
    sig = zeros(n)
    for i, (nm, t) in enumerate((("G4", 0.0), ("Eb4", 0.45), ("C4", 0.9))):
        d = n_of(0.7)
        place(sig, gain(strings([note(nm)], d, env_points(d, [(0, 0), (0.1, 1), (0.6, 0.6), (0.7, 0)]), i), 0.9),
              n_of(t))
    d = n_of(3.6)
    env = env_points(d, [(0, 0), (0.4, 1.0), (2.0, 0.7), (3.6, 0)])
    place(sig, brass([note("Ab2"), note("C3"), note("Eb3")], d, env, 900, 5), n_of(1.4))
    place(sig, gain(timpani(note("C2"), n_of(1.6)), 0.8), n_of(1.4))
    return finish_sting(sig)


def sting_leap():
    """A leap: a rising rush into an open, shimmering fifth."""
    n = n_of(3.8)
    rush = mul(lowpass(noise(n, 91), sweep_freq(200, 8000, n), 2), env_points(n, [(0, 0), (0.85, 1.0), (1.1, 0), (3.8, 0)]))
    d = n_of(2.8)
    env = env_points(d, [(0, 0), (0.15, 1.0), (1.4, 0.7), (2.8, 0)])
    shimmer = mix(strings([note("C4"), note("G4"), note("C5")], d, env, 3),
                  gain(mul(mix(osc(note("G6"), d), osc(note("C7"), d)), env), 0.08))
    sig = mix(gain(rush, 1.6), pad_to([0.0] * n_of(0.9) + shimmer, n))
    return finish_sting(sig)


def sting_act_end():
    """Act end: a slow cadence, Dm - Bb - F - C."""
    chords = [("D3", "F3", "A3"), ("Bb2", "D3", "F3"), ("F3", "A3", "C4"), ("C3", "E3", "G3", "C4")]
    step = 1.7
    n = n_of(step * 4 + 2.5)
    sig = zeros(n)
    for i, ch in enumerate(chords):
        d = n_of(step + 0.6 if i < 3 else 3.0)
        env = env_points(d, [(0, 0), (0.35, 1.0), (d / SR - 0.4, 0.8), (d / SR, 0)])
        place(sig, brass([note(x) for x in ch], d, env, 1800, 20 + i), n_of(i * step))
        place(sig, gain(strings([note(ch[0]) / 2], d, env, 30 + i), 0.7), n_of(i * step))
    place(sig, gain(timpani(note("C2"), n_of(1.6)), 0.8), n_of(3 * step))
    return finish_sting(sig)


# ---- catalogue -----------------------------------------------------------------------------------

def catalogue():
    c = {
        "A_Bridge_Hum": bridge_hum,
        "A_Combat_Bed": combat_bed,
        "S_Engine_Loop": engine_loop,
        "S_Bolt_Impact": lambda: impact(5),
        "S_Explosion_Ship": lambda: explosion(6),
        "S_Beam_Charge": beam_charge,
        "S_Beam_Fire": beam_fire,
        "S_Pulse_Telegraph": pulse_telegraph,
        "S_Pulse_Release": pulse_release,
        "S_Surge": surge,
        "S_Relay_Click": relay_click,
        "S_UI_Click": ui_click,
        "M_Sting_Council": sting_council,
        "M_Sting_Victory": sting_victory,
        "M_Sting_Defeat": sting_defeat,
        "M_Sting_Leap": sting_leap,
        "M_Sting_ActEnd": sting_act_end,
    }
    for i in range(3):
        # fleet (red): bright falling FM zap; Iron Tithe (green): lower, harsher; Severant (blue): glassy
        c[f"S_Blaster_Fleet_{i + 1:02d}"] = (lambda s=i: blaster(100 + s, 2400, 380, 0.32, 1.5, 3.0, 0.5, 5000))
        c[f"S_Blaster_Tithe_{i + 1:02d}"] = (lambda s=i: blaster(200 + s, 1500, 220, 0.4, 0.5, 4.0, 0.9, 3000, 2.2))
        c[f"S_Blaster_Severant_{i + 1:02d}"] = (lambda s=i: blaster(300 + s, 3200, 900, 0.45, 2.0, 1.5, 0.25, 8000, 1.0, 0.35))
    for i in range(8):
        c[f"S_Console_Chirp_{i + 1:02d}"] = (lambda s=i: chirp(400 + s))
    return c


def main():
    args = sys.argv[1:]
    out = os.path.abspath(args[0]) if args else os.path.join(os.path.dirname(__file__), "..", "..", "Source_Art", "Audio")
    only = set(args[1:])
    os.makedirs(out, exist_ok=True)
    for name, build in catalogue().items():
        if only and name not in only:
            continue
        result = build()
        path = os.path.join(out, name + ".wav")
        if isinstance(result, tuple):
            write_wav(path, *result)
        else:
            write_wav(path, result)
        frames = len(result[0]) if isinstance(result, tuple) else len(result)
        print(f"[audio] {name}: {frames / SR:.2f} s -> {path}")


if __name__ == "__main__":
    main()
