# Multi-Blend Enhanced

An enhanced version of the Multi-Blend VST 3 plugin with additional features.

## Original Features
- Dual input blending (Main Input + Sidechain)
- Three fade modes: Linear, Smooth S-curve, Equal Power
- Independent gain controls for both inputs (-24dB to +24dB)
- Real-time metering with peak hold and RMS
- Auto-gain matching feature
- Clip detection

## New Features - Clipper/Limiter System
- **Professional Clipper Algorithms:**
  - Soft Tanh: Musical harmonic saturation
  - Hard Clip: Aggressive digital limiting
  - Cubic: Smooth analog-like saturation
  - Hermite: High-quality polynomial (minimal aliasing)
  - Foldback: Complex harmonic wave folding

- **Advanced Limiter Types:**
  - Feedback Limiter: Real-time with smooth response
  - Feedforward Limiter: Fast response, minimal latency
  - Look-Ahead Limiter: Professional mastering quality

- **Comprehensive Controls:**
  - Threshold: -60dB to 0dB
  - Ceiling: -60dB to 0dB
  - Attack: 0.1ms to 100ms
  - Release: 1ms to 1000ms
  - Ratio: 1:1 to 20:1
  - Knee: 0dB to 24dB (soft knee)

- **Real-time Metering:**
  - Gain reduction meters for both channels
  - Input/Output level monitoring
  - Visual feedback for all parameters

## Planned Enhancements

### Phase 1: Advanced Blending
- [ ] Phase correlation meter
- [ ] Phase alignment controls
- [ ] Frequency-dependent blending
- [ ] Mid/Side processing mode

### Phase 2: Effects Integration
- [ ] Built-in EQ for each input
- [ ] Compression controls
- [ ] Stereo width enhancement
- [ ] Harmonic saturation

### Phase 3: Advanced Features
- [ ] LFO modulation of blend parameter
- [ ] Envelope follower for dynamic blending
- [ ] Preset management system
- [ ] A/B comparison mode

### Phase 4: UI Improvements
- [ ] Spectrum analyzer
- [ ] Advanced metering options
- [ ] Customizable color themes
- [ ] Resizable interface

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## License
This enhanced version maintains the same license as the original Multi-Blend plugin.