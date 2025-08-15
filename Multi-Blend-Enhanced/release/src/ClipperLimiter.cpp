#include "ClipperLimiter.h"
#include <juce_audio_basics/juce_audio_basics.h>

ClipperLimiter::ClipperLimiter()
{
    reset();
}

void ClipperLimiter::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
}

void ClipperLimiter::reset()
{
    envelope = 0.0f;
    lookAheadIndex = 0;
    std::fill(std::begin(lookAheadBuffer), std::end(lookAheadBuffer), 0.0f);
    gainReduction.store(0.0f);
    inputLevel.store(0.0f);
    outputLevel.store(0.0f);
}

void ClipperLimiter::setThreshold(float thresholdDb)
{
    thresholdDb = juce::jlimit(MIN_THRESHOLD_DB, MAX_THRESHOLD_DB, thresholdDb);
    thresholdGain = dbToGain(thresholdDb);
}

void ClipperLimiter::setCeiling(float ceilingDb)
{
    ceilingDb = juce::jlimit(MIN_CEILING_DB, MAX_CEILING_DB, ceilingDb);
    ceilingGain = dbToGain(ceilingDb);
}

void ClipperLimiter::setAttack(float attackMs)
{
    attackMs = juce::jlimit(MIN_ATTACK_MS, MAX_ATTACK_MS, attackMs);
    attackCoeff = std::exp(-1000.0f / (attackMs * sampleRate));
}

void ClipperLimiter::setRelease(float releaseMs)
{
    releaseMs = juce::jlimit(MIN_RELEASE_MS, MAX_RELEASE_MS, releaseMs);
    releaseCoeff = std::exp(-1000.0f / (releaseMs * sampleRate));
}

void ClipperLimiter::setRatio(float newRatio)
{
    ratio = juce::jlimit(MIN_RATIO, MAX_RATIO, newRatio);
}

void ClipperLimiter::setKnee(float kneeDb)
{
    kneeDb = juce::jlimit(MIN_KNEE_DB, MAX_KNEE_DB, kneeDb);
    this->kneeDb = kneeDb;
}

// ============================================================================
// Clipper Algorithms
// ============================================================================

void ClipperLimiter::processClipper(float* buffer, int numSamples)
{
    if (clipperType == ClipperType::None)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float input = buffer[i];
        float output = 0.0f;

        switch (clipperType)
        {
            case ClipperType::SoftTanh:
                output = processSoftTanh(input, thresholdGain);
                break;
            case ClipperType::HardClip:
                output = processHardClip(input, thresholdGain);
                break;
            case ClipperType::Cubic:
                output = processCubic(input, thresholdGain);
                break;
            case ClipperType::Hermite:
                output = processHermite(input, thresholdGain);
                break;
            case ClipperType::Foldback:
                output = processFoldback(input, thresholdGain);
                break;
            default:
                output = input;
                break;
        }

        buffer[i] = output;
    }
}

float ClipperLimiter::processSoftTanh(float input, float threshold)
{
  // Musical soft clipping using hyperbolic tangent
  // Provides harmonic saturation with minimal aliasing
  if (!std::isfinite(input) || !std::isfinite(threshold) || threshold <= 0.0f)
    return 0.0f;
    
  const float drive = 1.0f / threshold;
  const float saturated = std::tanh(input * drive);
  return saturated * threshold;
}

float ClipperLimiter::processHardClip(float input, float threshold)
{
  // Digital hard clipping - aggressive limiting
  if (!std::isfinite(input) || !std::isfinite(threshold) || threshold <= 0.0f)
    return 0.0f;
    
  return juce::jlimit(-threshold, threshold, input);
}

float ClipperLimiter::processCubic(float input, float threshold)
{
  // Smooth cubic saturation - analog-like characteristics
  if (!std::isfinite(input) || !std::isfinite(threshold) || threshold <= 0.0f)
    return 0.0f;
    
  const float drive = 1.0f / threshold;
  const float x = input * drive;
    
    if (std::abs(x) < 2.0f / 3.0f)
    {
        return input;
    }
    else if (std::abs(x) < 4.0f / 3.0f)
    {
        const float sign = (x > 0.0f) ? 1.0f : -1.0f;
        const float absX = std::abs(x);
        const float cubic = 1.0f - std::pow(2.0f - 3.0f * absX, 2.0f) / 3.0f;
        return sign * cubic * threshold;
    }
    else
    {
        return (x > 0.0f) ? threshold : -threshold;
    }
}

float ClipperLimiter::processHermite(float input, float threshold)
{
  // High-quality Hermite polynomial interpolation
  // Minimizes aliasing while providing smooth saturation
  if (!std::isfinite(input) || !std::isfinite(threshold) || threshold <= 0.0f)
    return 0.0f;
    
  const float drive = 1.0f / threshold;
  const float x = input * drive;
    const float absX = std::abs(x);
    
    if (absX <= 1.0f)
    {
        // Smooth polynomial curve
        const float x2 = x * x;
        const float x3 = x2 * x;
        return (2.0f * x3 - 3.0f * x2 + 1.0f) * threshold;
    }
    else
    {
        return (x > 0.0f) ? threshold : -threshold;
    }
}

float ClipperLimiter::processFoldback(float input, float threshold)
{
  // Wave folding - creates complex harmonics
  if (!std::isfinite(input) || !std::isfinite(threshold) || threshold <= 0.0f)
    return 0.0f;
    
  const float drive = 1.0f / threshold;
  const float x = input * drive;
    const float absX = std::abs(x);
    
    if (absX <= 1.0f)
    {
        return input;
    }
    else
    {
        // Fold the wave back
        const float folded = 2.0f - absX;
        const float sign = (x > 0.0f) ? 1.0f : -1.0f;
        return sign * folded * threshold;
    }
}

// ============================================================================
// Limiter Algorithms
// ============================================================================

void ClipperLimiter::processLimiter(float* buffer, int numSamples)
{
    if (limiterType == LimiterType::None)
        return;

    switch (limiterType)
    {
        case LimiterType::Feedback:
            processFeedbackLimiter(buffer, numSamples);
            break;
        case LimiterType::Feedforward:
            processFeedforwardLimiter(buffer, numSamples);
            break;
        case LimiterType::LookAhead:
            processLookAheadLimiter(buffer, numSamples);
            break;
        default:
            break;
    }
}

void ClipperLimiter::processFeedbackLimiter(float* buffer, int numSamples)
{
    // Real-time feedback limiter with smooth response
    for (int i = 0; i < numSamples; ++i)
    {
        const float input = buffer[i];
        const float inputAbs = std::abs(input);
        
        // Update envelope
        if (inputAbs > envelope)
            envelope = attackCoeff * (envelope - inputAbs) + inputAbs;
        else
            envelope = releaseCoeff * (envelope - inputAbs) + inputAbs;
        
        // Calculate gain reduction
        float gainReductionDb = 0.0f;
        if (envelope > thresholdGain)
        {
            const float overThreshold = envelope / thresholdGain;
            gainReductionDb = calculateKneeGain(envelope, thresholdGain, kneeDb);
        }
        
        const float gainReductionLinear = dbToGain(gainReductionDb);
        const float output = input * gainReductionLinear;
        
        // Apply ceiling
        buffer[i] = juce::jlimit(-ceilingGain, ceilingGain, output);
        
        // Update meters
        gainReduction.store(gainReductionDb);
        inputLevel.store(gainToDb(inputAbs));
        outputLevel.store(gainToDb(std::abs(buffer[i])));
    }
}

void ClipperLimiter::processFeedforwardLimiter(float* buffer, int numSamples)
{
    // Fast feedforward limiter with minimal latency
    for (int i = 0; i < numSamples; ++i)
    {
        const float input = buffer[i];
        const float inputAbs = std::abs(input);
        
        // Calculate required gain reduction
        float gainReductionDb = 0.0f;
        if (inputAbs > thresholdGain)
        {
            const float overThreshold = inputAbs / thresholdGain;
            gainReductionDb = calculateKneeGain(inputAbs, thresholdGain, kneeDb);
        }
        
        // Apply gain reduction
        const float gainReductionLinear = dbToGain(gainReductionDb);
        const float output = input * gainReductionLinear;
        
        // Apply ceiling
        buffer[i] = juce::jlimit(-ceilingGain, ceilingGain, output);
        
        // Update meters
        gainReduction.store(gainReductionDb);
        inputLevel.store(gainToDb(inputAbs));
        outputLevel.store(gainToDb(std::abs(buffer[i])));
    }
}

void ClipperLimiter::processLookAheadLimiter(float* buffer, int numSamples)
{
    // Simplified look-ahead limiter using delay buffer
    for (int i = 0; i < numSamples; ++i)
    {
        // Store input in look-ahead buffer
        lookAheadBuffer[lookAheadIndex] = buffer[i];
        
        // Find peak in look-ahead window
        float peak = 0.0f;
        for (int j = 0; j < lookAheadSize; ++j)
        {
            const int idx = (lookAheadIndex - j + 1024) % 1024;
            peak = std::max(peak, std::abs(lookAheadBuffer[idx]));
        }
        
        // Calculate gain reduction based on look-ahead peak
        float gainReductionDb = 0.0f;
        if (peak > thresholdGain)
        {
            gainReductionDb = calculateKneeGain(peak, thresholdGain, kneeDb);
        }
        
        // Apply gain reduction to delayed signal
        const float gainReductionLinear = dbToGain(gainReductionDb);
        const int delayedIndex = (lookAheadIndex - lookAheadSize + 1024) % 1024;
        const float delayedInput = lookAheadBuffer[delayedIndex];
        const float output = delayedInput * gainReductionLinear;
        
        // Apply ceiling
        buffer[i] = juce::jlimit(-ceilingGain, ceilingGain, output);
        
        // Update look-ahead index
        lookAheadIndex = (lookAheadIndex + 1) % 1024;
        
        // Update meters
        gainReduction.store(gainReductionDb);
        inputLevel.store(gainToDb(std::abs(delayedInput)));
        outputLevel.store(gainToDb(std::abs(buffer[i])));
    }
}

// ============================================================================
// Main Processing
// ============================================================================

void ClipperLimiter::process(float* buffer, int numSamples)
{
  try
  {
    // Validate input
    if (buffer == nullptr || numSamples <= 0)
      return;
    
    // Process clipper first, then limiter
    processClipper(buffer, numSamples);
    processLimiter(buffer, numSamples);
  }
  catch (const std::exception& e)
  {
    // Log error and continue with safe defaults
    juce::Logger::writeToLog("ClipperLimiter::process exception: " + juce::String(e.what()));
  }
  catch (...)
  {
    // Catch any other exceptions to prevent crashes
    juce::Logger::writeToLog("ClipperLimiter::process unknown exception");
  }
}

// ============================================================================
// Helper Functions
// ============================================================================

float ClipperLimiter::calculateKneeGain(float inputLevel, float threshold, float knee) const
{
    if (knee <= 0.0f)
    {
        // Hard knee
        if (inputLevel > threshold)
        {
            const float overThreshold = inputLevel / threshold;
            return gainToDb(1.0f / overThreshold) * (1.0f - 1.0f / ratio);
        }
        return 0.0f;
    }
    else
    {
        // Soft knee
        const float kneeGain = dbToGain(knee);
        const float kneeStart = threshold / kneeGain;
        const float kneeEnd = threshold * kneeGain;
        
        if (inputLevel <= kneeStart)
        {
            return 0.0f;
        }
        else if (inputLevel >= kneeEnd)
        {
            const float overThreshold = inputLevel / threshold;
            return gainToDb(1.0f / overThreshold) * (1.0f - 1.0f / ratio);
        }
        else
        {
            // Soft knee region
            const float kneeRatio = 1.0f + (ratio - 1.0f) * 
                (inputLevel - kneeStart) / (kneeEnd - kneeStart);
            const float overThreshold = inputLevel / threshold;
            return gainToDb(1.0f / overThreshold) * (1.0f - 1.0f / kneeRatio);
        }
    }
}