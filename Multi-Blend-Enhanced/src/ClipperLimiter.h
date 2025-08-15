#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

class ClipperLimiter
{
public:
    enum class ClipperType
    {
        None = 0,
        SoftTanh,      // Musical soft clipping
        HardClip,      // Digital hard clipping
        Cubic,         // Smooth cubic saturation
        Hermite,       // High-quality polynomial
        Foldback       // Wave folding
    };

    enum class LimiterType
    {
        None = 0,
        Feedback,      // Real-time feedback limiter
        Feedforward,   // Fast feedforward limiter
        LookAhead      // Look-ahead limiter (simplified)
    };

    ClipperLimiter();
    ~ClipperLimiter() = default;

    // Setup
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    // Processing
    void processClipper(float* buffer, int numSamples);
    void processLimiter(float* buffer, int numSamples);
    void process(float* buffer, int numSamples);

    // Parameters
    void setClipperType(ClipperType type) { clipperType = type; }
    void setLimiterType(LimiterType type) { limiterType = type; }
    void setThreshold(float thresholdDb);
    void setCeiling(float ceilingDb);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setRatio(float ratio);
    void setKnee(float kneeDb);

    // Meters
    float getGainReduction() const { return gainReduction.load(); }
    float getInputLevel() const { return inputLevel.load(); }
    float getOutputLevel() const { return outputLevel.load(); }

private:
    // Clipper algorithms
    float processSoftTanh(float input, float threshold);
    float processHardClip(float input, float threshold);
    float processCubic(float input, float threshold);
    float processHermite(float input, float threshold);
    float processFoldback(float input, float threshold);

    // Limiter algorithms
    void processFeedbackLimiter(float* buffer, int numSamples);
    void processFeedforwardLimiter(float* buffer, int numSamples);
    void processLookAheadLimiter(float* buffer, int numSamples);

    // Helper functions
    float dbToGain(float db) const { return juce::Decibels::decibelsToGain(db); }
    float gainToDb(float gain) const { return juce::Decibels::gainToDecibels(gain); }
    float calculateKneeGain(float inputLevel, float threshold, float knee) const;

    // Parameters
    ClipperType clipperType = ClipperType::None;
    LimiterType limiterType = LimiterType::None;
    float thresholdGain = 1.0f;
    float ceilingGain = 1.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float ratio = 1.0f;
    float kneeDb = 0.0f;

    // State variables
    float sampleRate = 44100.0f;
    float envelope = 0.0f;
    float lookAheadBuffer[1024];
    int lookAheadIndex = 0;
    int lookAheadSize = 64;

    // Meters
    std::atomic<float> gainReduction{0.0f};
    std::atomic<float> inputLevel{0.0f};
    std::atomic<float> outputLevel{0.0f};

    // Constants
    static constexpr float MIN_THRESHOLD_DB = -60.0f;
    static constexpr float MAX_THRESHOLD_DB = 0.0f;
    static constexpr float MIN_CEILING_DB = -60.0f;
    static constexpr float MAX_CEILING_DB = 0.0f;
    static constexpr float MIN_ATTACK_MS = 0.1f;
    static constexpr float MAX_ATTACK_MS = 100.0f;
    static constexpr float MIN_RELEASE_MS = 1.0f;
    static constexpr float MAX_RELEASE_MS = 1000.0f;
    static constexpr float MIN_RATIO = 1.0f;
    static constexpr float MAX_RATIO = 20.0f;
    static constexpr float MIN_KNEE_DB = 0.0f;
    static constexpr float MAX_KNEE_DB = 24.0f;
};