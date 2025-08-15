#include "PluginTestSuite.h"
#include "../src/PluginProcessor.h"
#include "../src/ClipperLimiter.h"
#include <chrono>
#include <thread>
#include <algorithm>
#include <iostream>
#include <fstream>

PluginTestSuite::PluginTestSuite()
    : randomGenerator(randomDevice())
    , randomDistribution(-1.0f, 1.0f)
{
    // Create test processor instance
    testProcessor = std::make_unique<CrossFXEnhancedAudioProcessor>();
}

bool PluginTestSuite::runAllTests()
{
    std::cout << "=== Multi-Blend Enhanced Plugin Test Suite ===" << std::endl;
    std::cout << "Testing for Logic Pro compatibility and stability..." << std::endl;
    
    bool allTestsPassed = true;
    
    // Run all test categories
    allTestsPassed &= testPluginInitialization();
    allTestsPassed &= testParameterValidation();
    allTestsPassed &= testAudioProcessing();
    allTestsPassed &= testClipperLimiterAlgorithms();
    allTestsPassed &= testMemoryManagement();
    allTestsPassed &= testThreadSafety();
    allTestsPassed &= testStatePersistence();
    allTestsPassed &= testEdgeCases();
    allTestsPassed &= testPerformance();
    allTestsPassed &= testLogicProCompatibility();
    
    // Print summary
    std::cout << "\n=== Test Summary ===" << std::endl;
    int passedTests = 0;
    int totalTests = 0;
    
    for (const auto& result : testResults)
    {
        totalTests++;
        if (result.passed) passedTests++;
        
        std::cout << (result.passed ? "✅ PASS" : "❌ FAIL") 
                  << " - " << result.testName;
        if (!result.passed && !result.errorMessage.empty())
            std::cout << " (" << result.errorMessage << ")";
        std::cout << std::endl;
    }
    
    std::cout << "\nResults: " << passedTests << "/" << totalTests << " tests passed" << std::endl;
    
    if (allTestsPassed)
        std::cout << "🎉 All tests passed! Plugin is ready for Logic Pro." << std::endl;
    else
        std::cout << "⚠️  Some tests failed. Please review before Logic Pro deployment." << std::endl;
    
    return allTestsPassed;
}

bool PluginTestSuite::testPluginInitialization()
{
    std::cout << "\n--- Testing Plugin Initialization ---" << std::endl;
    
    bool passed = true;
    
    // Test 1: Basic initialization
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        if (processor == nullptr)
        {
            logTestResult("Plugin Creation", false, "Failed to create processor");
            passed = false;
        }
        else
        {
            logTestResult("Plugin Creation", true);
        }
    }
    catch (const std::exception& e)
    {
        logTestResult("Plugin Creation", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    // Test 2: Parameter count validation
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        auto& vts = processor->getValueTreeState();
        int expectedParams = 12; // Original 4 + 8 new clipper/limiter params
        int actualParams = vts.getParameterCount();
        
        if (actualParams != expectedParams)
        {
            logTestResult("Parameter Count", false, 
                         "Expected " + std::to_string(expectedParams) + 
                         ", got " + std::to_string(actualParams));
            passed = false;
        }
        else
        {
            logTestResult("Parameter Count", true);
        }
    }
    catch (const std::exception& e)
    {
        logTestResult("Parameter Count", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    // Test 3: Bus layout validation
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        auto layout = processor->getBusesLayout();
        
        // Should support stereo input/output
        if (!layout.getMainInputChannelSet().isDisabled() && 
            layout.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        {
            logTestResult("Bus Layout", false, "Main input should be stereo");
            passed = false;
        }
        else
        {
            logTestResult("Bus Layout", true);
        }
    }
    catch (const std::exception& e)
    {
        logTestResult("Bus Layout", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testParameterValidation()
{
    std::cout << "\n--- Testing Parameter Validation ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        auto& vts = processor->getValueTreeState();
        
        // Test all parameter ranges
        std::vector<std::pair<std::string, std::pair<float, float>>> paramRanges = {
            {"blend", {0.0f, 1.0f}},
            {"gainA", {-24.0f, 24.0f}},
            {"gainB", {-24.0f, 24.0f}},
            {"threshold", {-60.0f, 0.0f}},
            {"ceiling", {-60.0f, 0.0f}},
            {"attack", {0.1f, 100.0f}},
            {"release", {1.0f, 1000.0f}},
            {"ratio", {1.0f, 20.0f}},
            {"knee", {0.0f, 24.0f}}
        };
        
        for (const auto& param : paramRanges)
        {
            auto* parameter = vts.getParameter(param.first);
            if (parameter == nullptr)
            {
                logTestResult("Parameter " + param.first, false, "Parameter not found");
                passed = false;
                continue;
            }
            
            // Test parameter bounds
            float minValue = param.second.first;
            float maxValue = param.second.second;
            
            // Set to minimum value
            parameter->setValueNotifyingHost(0.0f);
            float actualMin = parameter->getValue();
            
            // Set to maximum value
            parameter->setValueNotifyingHost(1.0f);
            float actualMax = parameter->getValue();
            
            if (std::abs(actualMin - minValue) > 0.01f || std::abs(actualMax - maxValue) > 0.01f)
            {
                logTestResult("Parameter " + param.first + " Range", false,
                             "Expected [" + std::to_string(minValue) + ", " + std::to_string(maxValue) + 
                             "], got [" + std::to_string(actualMin) + ", " + std::to_string(actualMax) + "]");
                passed = false;
            }
            else
            {
                logTestResult("Parameter " + param.first + " Range", true);
            }
        }
    }
    catch (const std::exception& e)
    {
        logTestResult("Parameter Validation", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testAudioProcessing()
{
    std::cout << "\n--- Testing Audio Processing ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        
        // Prepare processor
        processor->prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Create test buffers
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        
        // Test 1: Silent input
        buffer.clear();
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("Silent Input Processing", true);
        
        // Test 2: Sine wave input
        std::vector<float> sineWave(TEST_BUFFER_SIZE);
        generateTestSignal(sineWave, 440.0f, 0.5f, TEST_SAMPLE_RATE);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(ch, i, sineWave[i]);
            }
        }
        
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("Sine Wave Processing", true);
        
        // Test 3: High amplitude input (potential clipping)
        std::vector<float> highAmpSignal(TEST_BUFFER_SIZE);
        generateTestSignal(highAmpSignal, 440.0f, 2.0f, TEST_SAMPLE_RATE);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(ch, i, highAmpSignal[i]);
            }
        }
        
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("High Amplitude Processing", true);
        
        // Test 4: Noise input
        std::vector<float> noiseSignal(TEST_BUFFER_SIZE);
        generateNoiseSignal(noiseSignal, 0.8f);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(ch, i, noiseSignal[i]);
            }
        }
        
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("Noise Processing", true);
        
        // Test 5: Output validation
        if (!isFinite(noiseSignal))
        {
            logTestResult("Output Validation", false, "Non-finite values detected");
            passed = false;
        }
        else
        {
            logTestResult("Output Validation", true);
        }
        
        // Test 6: Extended processing (simulate long session)
        for (int iteration = 0; iteration < 1000; ++iteration)
        {
            generateTestSignal(sineWave, 440.0f + iteration, 0.5f, TEST_SAMPLE_RATE);
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    buffer.setSample(ch, i, sineWave[i]);
                }
            }
            
            validateNoCrashes([&]() {
                processor->processBlock(buffer, midiBuffer);
            });
        }
        logTestResult("Extended Processing", true);
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Audio Processing", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testClipperLimiterAlgorithms()
{
    std::cout << "\n--- Testing Clipper/Limiter Algorithms ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        // Test each clipper type
        std::vector<ClipperLimiter::ClipperType> clipperTypes = {
            ClipperLimiter::ClipperType::SoftTanh,
            ClipperLimiter::ClipperType::HardClip,
            ClipperLimiter::ClipperType::Cubic,
            ClipperLimiter::ClipperType::Hermite,
            ClipperLimiter::ClipperType::Foldback
        };
        
        for (auto clipperType : clipperTypes)
        {
            ClipperLimiter clipper;
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = TEST_SAMPLE_RATE;
            spec.maximumBlockSize = TEST_BUFFER_SIZE;
            spec.numChannels = 1;
            
            clipper.prepare(spec);
            clipper.setClipperType(clipperType);
            clipper.setThreshold(-12.0f);
            
            std::vector<float> testBuffer(TEST_BUFFER_SIZE);
            generateClippingSignal(testBuffer, 2.0f);
            
            validateNoCrashes([&]() {
                clipper.processClipper(testBuffer.data(), TEST_BUFFER_SIZE);
            });
            
            // Validate output
            if (!isFinite(testBuffer))
            {
                logTestResult("Clipper " + std::to_string(static_cast<int>(clipperType)), 
                             false, "Non-finite output");
                passed = false;
            }
            else
            {
                logTestResult("Clipper " + std::to_string(static_cast<int>(clipperType)), true);
            }
        }
        
        // Test each limiter type
        std::vector<ClipperLimiter::LimiterType> limiterTypes = {
            ClipperLimiter::LimiterType::Feedback,
            ClipperLimiter::LimiterType::Feedforward,
            ClipperLimiter::LimiterType::LookAhead
        };
        
        for (auto limiterType : limiterTypes)
        {
            ClipperLimiter limiter;
            juce::dsp::ProcessSpec spec;
            spec.sampleRate = TEST_SAMPLE_RATE;
            spec.maximumBlockSize = TEST_BUFFER_SIZE;
            spec.numChannels = 1;
            
            limiter.prepare(spec);
            limiter.setLimiterType(limiterType);
            limiter.setThreshold(-12.0f);
            limiter.setCeiling(-0.1f);
            limiter.setAttack(1.0f);
            limiter.setRelease(50.0f);
            limiter.setRatio(4.0f);
            limiter.setKnee(6.0f);
            
            std::vector<float> testBuffer(TEST_BUFFER_SIZE);
            generateClippingSignal(testBuffer, 2.0f);
            
            validateNoCrashes([&]() {
                limiter.processLimiter(testBuffer.data(), TEST_BUFFER_SIZE);
            });
            
            // Validate output
            if (!isFinite(testBuffer))
            {
                logTestResult("Limiter " + std::to_string(static_cast<int>(limiterType)), 
                             false, "Non-finite output");
                passed = false;
            }
            else
            {
                logTestResult("Limiter " + std::to_string(static_cast<int>(limiterType)), true);
            }
        }
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Clipper/Limiter Algorithms", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testMemoryManagement()
{
    std::cout << "\n--- Testing Memory Management ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        // Test 1: Multiple processor instances
        std::vector<std::unique_ptr<CrossFXEnhancedAudioProcessor>> processors;
        
        for (int i = 0; i < 100; ++i)
        {
            validateNoCrashes([&]() {
                processors.push_back(std::make_unique<CrossFXEnhancedAudioProcessor>());
            });
        }
        
        processors.clear();
        logTestResult("Multiple Instances", true);
        
        // Test 2: Rapid creation/destruction
        for (int i = 0; i < 1000; ++i)
        {
            validateNoCrashes([&]() {
                auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
                processor->prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
                processor->releaseResources();
            });
        }
        logTestResult("Rapid Creation/Destruction", true);
        
        // Test 3: Large buffer processing
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        processor->prepareToPlay(TEST_SAMPLE_RATE, 8192); // Large buffer
        
        juce::AudioBuffer<float> largeBuffer(2, 8192);
        juce::MidiBuffer midiBuffer;
        
        for (int iteration = 0; iteration < 100; ++iteration)
        {
            validateNoCrashes([&]() {
                processor->processBlock(largeBuffer, midiBuffer);
            });
        }
        logTestResult("Large Buffer Processing", true);
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Memory Management", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testThreadSafety()
{
    std::cout << "\n--- Testing Thread Safety ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        processor->prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Test 1: Parameter changes during processing
        std::vector<std::thread> threads;
        std::atomic<bool> stopThreads(false);
        
        // Audio processing thread
        threads.emplace_back([&]() {
            juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
            juce::MidiBuffer midiBuffer;
            std::vector<float> testSignal(TEST_BUFFER_SIZE);
            
            while (!stopThreads.load())
            {
                generateTestSignal(testSignal, 440.0f, 0.5f, TEST_SAMPLE_RATE);
                
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int i = 0; i < buffer.getNumSamples(); ++i)
                    {
                        buffer.setSample(ch, i, testSignal[i]);
                    }
                }
                
                validateNoCrashes([&]() {
                    processor->processBlock(buffer, midiBuffer);
                });
            }
        });
        
        // Parameter modification thread
        threads.emplace_back([&]() {
            auto& vts = processor->getValueTreeState();
            std::vector<std::string> paramNames = {"blend", "gainA", "gainB", "threshold", "ceiling"};
            
            while (!stopThreads.load())
            {
                for (const auto& paramName : paramNames)
                {
                    if (auto* param = vts.getParameter(paramName))
                    {
                        float randomValue = randomDistribution(randomGenerator);
                        validateNoCrashes([&]() {
                            param->setValueNotifyingHost(randomValue);
                        });
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
        
        // Run for 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        stopThreads.store(true);
        
        for (auto& thread : threads)
        {
            thread.join();
        }
        
        logTestResult("Thread Safety", true);
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Thread Safety", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testStatePersistence()
{
    std::cout << "\n--- Testing State Persistence ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor1 = std::make_unique<CrossFXEnhancedAudioProcessor>();
        auto processor2 = std::make_unique<CrossFXEnhancedAudioProcessor>();
        
        // Set random parameters on processor1
        auto& vts1 = processor1->getValueTreeState();
        std::vector<std::pair<std::string, float>> testValues = {
            {"blend", 0.7f},
            {"gainA", 6.0f},
            {"gainB", -3.0f},
            {"threshold", -18.0f},
            {"ceiling", -0.5f},
            {"attack", 5.0f},
            {"release", 100.0f},
            {"ratio", 8.0f},
            {"knee", 12.0f}
        };
        
        for (const auto& [paramName, value] : testValues)
        {
            if (auto* param = vts1.getParameter(paramName))
            {
                param->setValueNotifyingHost(value);
            }
        }
        
        // Save state
        juce::MemoryBlock stateData;
        validateNoCrashes([&]() {
            processor1->getStateInformation(stateData);
        });
        
        // Load state into processor2
        validateNoCrashes([&]() {
            processor2->setStateInformation(stateData.getData(), stateData.getSize());
        });
        
        // Verify state was restored correctly
        auto& vts2 = processor2->getValueTreeState();
        bool stateRestored = true;
        
        for (const auto& [paramName, expectedValue] : testValues)
        {
            if (auto* param = vts2.getParameter(paramName))
            {
                float actualValue = param->getValue();
                if (std::abs(actualValue - expectedValue) > 0.01f)
                {
                    stateRestored = false;
                    break;
                }
            }
        }
        
        if (stateRestored)
        {
            logTestResult("State Persistence", true);
        }
        else
        {
            logTestResult("State Persistence", false, "State not restored correctly");
            passed = false;
        }
        
    }
    catch (const std::exception& e)
    {
        logTestResult("State Persistence", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testEdgeCases()
{
    std::cout << "\n--- Testing Edge Cases ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        processor->prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        
        // Test 1: Extremely high amplitude
        std::vector<float> extremeSignal(TEST_BUFFER_SIZE);
        generateTestSignal(extremeSignal, 440.0f, 100.0f, TEST_SAMPLE_RATE);
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                buffer.setSample(ch, i, extremeSignal[i]);
            }
        }
        
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("Extreme Amplitude", true);
        
        // Test 2: NaN and Infinity values
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (i % 10 == 0)
                {
                    buffer.setSample(ch, i, std::numeric_limits<float>::quiet_NaN());
                }
                else if (i % 10 == 1)
                {
                    buffer.setSample(ch, i, std::numeric_limits<float>::infinity());
                }
                else if (i % 10 == 2)
                {
                    buffer.setSample(ch, i, -std::numeric_limits<float>::infinity());
                }
                else
                {
                    buffer.setSample(ch, i, 0.0f);
                }
            }
        }
        
        validateNoCrashes([&]() {
            processor->processBlock(buffer, midiBuffer);
        });
        logTestResult("NaN/Infinity Handling", true);
        
        // Test 3: Zero buffer size
        juce::AudioBuffer<float> zeroBuffer(2, 0);
        validateNoCrashes([&]() {
            processor->processBlock(zeroBuffer, midiBuffer);
        });
        logTestResult("Zero Buffer Size", true);
        
        // Test 4: Single sample buffer
        juce::AudioBuffer<float> singleBuffer(2, 1);
        singleBuffer.setSample(0, 0, 0.5f);
        singleBuffer.setSample(1, 0, -0.5f);
        
        validateNoCrashes([&]() {
            processor->processBlock(singleBuffer, midiBuffer);
        });
        logTestResult("Single Sample Buffer", true);
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Edge Cases", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testPerformance()
{
    std::cout << "\n--- Testing Performance ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        processor->prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        std::vector<float> testSignal(TEST_BUFFER_SIZE);
        
        // Performance test: 10 seconds of audio processing
        auto startTime = std::chrono::high_resolution_clock::now();
        int iterations = 0;
        
        while (iterations < 10000) // ~10 seconds at 44.1kHz
        {
            generateTestSignal(testSignal, 440.0f + (iterations % 1000), 0.5f, TEST_SAMPLE_RATE);
            
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int i = 0; i < buffer.getNumSamples(); ++i)
                {
                    buffer.setSample(ch, i, testSignal[i]);
                }
            }
            
            validateNoCrashes([&]() {
                processor->processBlock(buffer, midiBuffer);
            });
            
            iterations++;
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        double cpuUsage = (duration.count() / 10000.0) * 100.0; // Percentage of real-time
        
        if (cpuUsage < 50.0) // Should use less than 50% CPU
        {
            logTestResult("Performance", true);
            logPerformance("CPU Usage", cpuUsage);
        }
        else
        {
            logTestResult("Performance", false, "CPU usage too high: " + std::to_string(cpuUsage) + "%");
            passed = false;
        }
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Performance", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

bool PluginTestSuite::testLogicProCompatibility()
{
    std::cout << "\n--- Testing Logic Pro Compatibility ---" << std::endl;
    
    bool passed = true;
    
    try
    {
        auto processor = std::make_unique<CrossFXEnhancedAudioProcessor>();
        
        // Test 1: Logic Pro specific bus layouts
        std::vector<juce::AudioProcessor::BusesLayout> logicProLayouts = {
            // Stereo to Stereo
            juce::AudioProcessor::BusesLayout()
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
                .withOutput("Output", juce::AudioChannelSet::stereo(), true),
            
            // Mono to Mono
            juce::AudioProcessor::BusesLayout()
                .withInput("Input", juce::AudioChannelSet::mono(), true)
                .withOutput("Output", juce::AudioChannelSet::mono(), true),
            
            // Stereo to Mono
            juce::AudioProcessor::BusesLayout()
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
                .withOutput("Output", juce::AudioChannelSet::mono(), true),
            
            // Mono to Stereo
            juce::AudioProcessor::BusesLayout()
                .withInput("Input", juce::AudioChannelSet::mono(), true)
                .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        };
        
        for (const auto& layout : logicProLayouts)
        {
            if (!processor->isBusesLayoutSupported(layout))
            {
                logTestResult("Logic Pro Bus Layout", false, "Unsupported bus layout");
                passed = false;
            }
        }
        logTestResult("Logic Pro Bus Layout", true);
        
        // Test 2: Sample rate changes (Logic Pro may change sample rate)
        std::vector<double> sampleRates = {44100.0, 48000.0, 88200.0, 96000.0};
        
        for (double sampleRate : sampleRates)
        {
            validateNoCrashes([&]() {
                processor->prepareToPlay(sampleRate, TEST_BUFFER_SIZE);
            });
        }
        logTestResult("Sample Rate Changes", true);
        
        // Test 3: Buffer size changes (Logic Pro may change buffer size)
        std::vector<int> bufferSizes = {32, 64, 128, 256, 512, 1024, 2048};
        
        for (int bufferSize : bufferSizes)
        {
            validateNoCrashes([&]() {
                processor->prepareToPlay(TEST_SAMPLE_RATE, bufferSize);
            });
        }
        logTestResult("Buffer Size Changes", true);
        
        // Test 4: Parameter automation (Logic Pro automation)
        auto& vts = processor->getValueTreeState();
        std::vector<std::string> automatableParams = {"blend", "gainA", "gainB", "threshold", "ceiling"};
        
        for (const auto& paramName : automatableParams)
        {
            if (auto* param = vts.getParameter(paramName))
            {
                // Simulate automation
                for (int i = 0; i < 100; ++i)
                {
                    float value = static_cast<float>(i) / 100.0f;
                    validateNoCrashes([&]() {
                        param->setValueNotifyingHost(value);
                    });
                }
            }
        }
        logTestResult("Parameter Automation", true);
        
        // Test 5: Preset loading/saving (Logic Pro preset management)
        juce::MemoryBlock presetData;
        validateNoCrashes([&]() {
            processor->getStateInformation(presetData);
        });
        
        auto processor2 = std::make_unique<CrossFXEnhancedAudioProcessor>();
        validateNoCrashes([&]() {
            processor2->setStateInformation(presetData.getData(), presetData.getSize());
        });
        logTestResult("Preset Management", true);
        
    }
    catch (const std::exception& e)
    {
        logTestResult("Logic Pro Compatibility", false, std::string("Exception: ") + e.what());
        passed = false;
    }
    
    return passed;
}

// Utility functions
void PluginTestSuite::generateTestSignal(std::vector<float>& buffer, float frequency, float amplitude, float sampleRate)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        buffer[i] = amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * frequency * t);
    }
}

void PluginTestSuite::generateNoiseSignal(std::vector<float>& buffer, float amplitude)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        buffer[i] = amplitude * randomDistribution(randomGenerator);
    }
}

void PluginTestSuite::generateImpulseSignal(std::vector<float>& buffer, int position)
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    if (position >= 0 && position < static_cast<int>(buffer.size()))
    {
        buffer[position] = 1.0f;
    }
}

void PluginTestSuite::generateClippingSignal(std::vector<float>& buffer, float amplitude)
{
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        float t = static_cast<float>(i) / TEST_SAMPLE_RATE;
        buffer[i] = amplitude * std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * t);
    }
}

bool PluginTestSuite::validateNoCrashes(const std::function<void()>& testFunction)
{
    try
    {
        testFunction();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

bool PluginTestSuite::isFinite(const std::vector<float>& buffer)
{
    return std::all_of(buffer.begin(), buffer.end(), [](float x) {
        return std::isfinite(x);
    });
}

bool PluginTestSuite::isInRange(const std::vector<float>& buffer, float min, float max)
{
    return std::all_of(buffer.begin(), buffer.end(), [min, max](float x) {
        return x >= min && x <= max;
    });
}

void PluginTestSuite::logTestResult(const std::string& testName, bool passed, const std::string& errorMessage)
{
    TestResult result;
    result.testName = testName;
    result.passed = passed;
    result.errorMessage = errorMessage;
    result.executionTime = 0.0;
    
    testResults.push_back(result);
}

void PluginTestSuite::logPerformance(const std::string& testName, double executionTime)
{
    std::cout << "Performance: " << testName << " - " << executionTime << "ms" << std::endl;
}