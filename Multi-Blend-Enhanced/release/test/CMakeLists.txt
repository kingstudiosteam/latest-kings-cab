cmake_minimum_required(VERSION 3.22)
project(PluginTestSuite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE
find_package(JUCE REQUIRED)

# Create test executable
add_executable(PluginTestRunner
    TestRunner.cpp
    PluginTestSuite.cpp
    PluginTestSuite.h
)

# Link JUCE modules
target_link_libraries(PluginTestRunner PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_dsp
    juce::juce_audio_basics
    juce::juce_audio_processors
)

# Include directories
target_include_directories(PluginTestRunner PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Compile definitions
target_compile_definitions(PluginTestRunner PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_PLUGINHOST_VST=0
    JUCE_PLUGINHOST_VST3=0
)