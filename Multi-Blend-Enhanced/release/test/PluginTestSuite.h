#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <random>

class PluginTestSuite
{
public:
    PluginTestSuite();
    ~PluginTestSuite() = default;

    // Main test runner
    bool runAllTests();

    // Individual test categories
    bool testPluginInitialization();
    bool testParameterValidation();
    bool testAudioProcessing();
    bool testClipperLimiterAlgorithms();
    bool testMemoryManagement();
    bool testThreadSafety();
    bool testStatePersistence();
    bool testEdgeCases();
    bool testPerformance();
    bool testLogicProCompatibility();

private:
    // Test utilities
    void generateTestSignal(std::vector<float>& buffer, float frequency, float amplitude, float sampleRate);
    void generateNoiseSignal(std::vector<float>& buffer, float amplitude);
    void generateImpulseSignal(std::vector<float>& buffer, int position);
    void generateClippingSignal(std::vector<float>& buffer, float amplitude);
    
    // Validation helpers
    bool validateParameterRanges();
    bool validateAudioOutput(const std::vector<float>& buffer);
    bool validateNoCrashes(const std::function<void()>& testFunction);
    bool validateMemoryLeaks();
    
    // Test data
    std::unique_ptr<juce::AudioProcessor> testProcessor;
    std::random_device randomDevice;
    std::mt19937 randomGenerator;
    std::uniform_real_distribution<float> randomDistribution;
    
    // Test constants
    static constexpr float TEST_SAMPLE_RATE = 44100.0f;
    static constexpr int TEST_BUFFER_SIZE = 512;
    static constexpr int TEST_DURATION_SECONDS = 10;
    static constexpr int MAX_ITERATIONS = 10000;
    
    // Test results
    struct TestResult
    {
        std::string testName;
        bool passed;
        std::string errorMessage;
        double executionTime;
    };
    
    std::vector<TestResult> testResults;
    
    // Helper functions
    void logTestResult(const std::string& testName, bool passed, const std::string& errorMessage = "");
    void logPerformance(const std::string& testName, double executionTime);
    bool isFinite(const std::vector<float>& buffer);
    bool isInRange(const std::vector<float>& buffer, float min, float max);
};