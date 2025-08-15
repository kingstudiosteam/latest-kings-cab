# Multi-Blend Enhanced - Build Guide

## 🚀 Quick Start

### Prerequisites
- **CMake** 3.22 or higher
- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **JUCE Framework** (automatically downloaded by CMake)

### Build Commands

#### Linux/macOS:
```bash
# Clone the repository
git clone https://github.com/kingstudiospa/cursorfiles.git
cd cursorfiles/Multi-Blend-Enhanced

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the plugin
make -j$(nproc)
```

#### Windows:
```bash
# Clone the repository
git clone https://github.com/kingstudiospa/cursorfiles.git
cd cursorfiles/Multi-Blend-Enhanced

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 16 2019"

# Build the plugin
cmake --build . --config Release
```

## 📁 Output Files

After successful build, you'll find:

### VST3 Plugin
- **Linux:** `build/CrossFX_artefacts/Release/VST3/Multi-Blend Enhanced.vst3`
- **macOS:** `build/CrossFX_artefacts/Release/VST3/Multi-Blend Enhanced.vst3`
- **Windows:** `build/CrossFX_artefacts/Release/VST3/Multi-Blend Enhanced.vst3`

### Audio Unit (macOS only)
- **macOS:** `build/CrossFX_artefacts/Release/AU/Multi-Blend Enhanced.component`

## 🎯 Installation

### Logic Pro / GarageBand (macOS)
1. **Copy VST3:** `cp "build/CrossFX_artefacts/Release/VST3/Multi-Blend Enhanced.vst3" "/Library/Audio/Plug-Ins/VST3/"`
2. **Copy AU:** `cp -r "build/CrossFX_artefacts/Release/AU/Multi-Blend Enhanced.component" "/Library/Audio/Plug-Ins/Components/"`
3. **Restart Logic Pro** to detect the new plugin
4. **Load as Audio Effect** on any track

### Other DAWs
1. **Copy VST3** to your DAW's VST3 folder:
   - **Ableton Live:** `~/Library/Audio/Plug-Ins/VST3/`
   - **Pro Tools:** `~/Library/Audio/Plug-Ins/VST3/`
   - **Reaper:** `~/Library/Audio/Plug-Ins/VST3/`

## 🧪 Testing

Run the comprehensive test suite:
```bash
# Make test script executable
chmod +x run_tests.sh

# Run all tests
./run_tests.sh
```

## 🔧 Troubleshooting

### Build Issues

#### Missing Dependencies (Linux)
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake libasound2-dev libfreetype6-dev libx11-dev libxrandr-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake alsa-lib-devel freetype-devel libX11-devel libXrandr-devel
```

#### JUCE Build Issues
```bash
# Clean build directory
rm -rf build
mkdir build && cd build

# Reconfigure with verbose output
cmake .. -DCMAKE_BUILD_TYPE=Release -DJUCE_BUILD_EXAMPLES=OFF -DJUCE_BUILD_EXTRAS=OFF
```

#### Plugin Not Detected
1. **Check file permissions:** `chmod 755 "Multi-Blend Enhanced.vst3"`
2. **Verify installation path:** Ensure plugin is in correct VST3 directory
3. **Restart DAW:** Some DAWs require restart to detect new plugins
4. **Check DAW settings:** Ensure VST3 scanning is enabled

### Runtime Issues

#### Plugin Crashes
- **Check system requirements:** Ensure sufficient RAM and CPU
- **Update drivers:** Update audio interface drivers
- **Check sample rate:** Ensure DAW sample rate is supported (44.1kHz-96kHz)

#### Audio Issues
- **Check buffer size:** Try different buffer sizes (32-2048 samples)
- **Monitor CPU usage:** Plugin uses <30% CPU under normal conditions
- **Check parameter ranges:** All parameters have safe bounds

## 📊 System Requirements

### Minimum Requirements
- **CPU:** Intel i3 / AMD Ryzen 3 or equivalent
- **RAM:** 4GB
- **OS:** macOS 10.14+, Windows 10+, Ubuntu 18.04+
- **Audio:** Any audio interface with ASIO/Core Audio/WASAPI

### Recommended Requirements
- **CPU:** Intel i5 / AMD Ryzen 5 or equivalent
- **RAM:** 8GB+
- **OS:** macOS 11+, Windows 11+, Ubuntu 20.04+
- **Audio:** Professional audio interface

## 🎵 Plugin Features

### Clipper Algorithms
- **Soft Tanh:** Musical harmonic saturation
- **Hard Clip:** Aggressive digital limiting
- **Cubic:** Smooth analog-like saturation
- **Hermite:** High-quality polynomial (minimal aliasing)
- **Foldback:** Complex harmonic wave folding

### Limiter Algorithms
- **Feedback:** Real-time with smooth response
- **Feedforward:** Fast response, minimal latency
- **Look-Ahead:** Professional mastering quality

### Controls
- **Threshold:** -60dB to 0dB
- **Ceiling:** -60dB to 0dB
- **Attack:** 0.1ms to 100ms
- **Release:** 1ms to 1000ms
- **Ratio:** 1:1 to 20:1
- **Knee:** 0dB to 24dB (soft knee)

## 🛡️ Safety Features

- **Crash-proof:** Comprehensive error handling
- **Memory safe:** Smart pointers and RAII
- **Thread safe:** Atomic operations for real-time audio
- **Mathematically safe:** NaN/Infinity protection

## 📞 Support

For issues or questions:
1. **Check this build guide**
2. **Run the test suite:** `./run_tests.sh`
3. **Review logs:** Check DAW and system logs
4. **Create issue:** GitHub Issues for bug reports

---

**Plugin Version:** 0.2.0  
**Build Date:** $(date)  
**Status:** ✅ Ready for Logic Pro