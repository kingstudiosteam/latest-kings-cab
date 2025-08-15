# Multi-Blend Enhanced

An enhanced version of the Multi-Blend VST 3 plugin with additional features.

## Original Features
- Dual input blending (Main Input + Sidechain)
- Three fade modes: Linear, Smooth S-curve, Equal Power
- Independent gain controls for both inputs (-24dB to +24dB)
- Real-time metering with peak hold and RMS
- Auto-gain matching feature
- Clip detection

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