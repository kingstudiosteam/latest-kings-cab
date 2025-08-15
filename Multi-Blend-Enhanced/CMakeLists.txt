cmake_minimum_required(VERSION 3.22)
project(CrossFX VERSION 0.2.0 LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON CACHE BOOL "" FORCE)
set(JUCE_FETCH_VST3_SDK ON CACHE BOOL "" FORCE)

FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG master
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(CrossFX
  COMPANY_NAME "King Studios"
  PLUGIN_MANUFACTURER "King Studios"
  PLUGIN_MANUFACTURER_CODE KngS
  PLUGIN_CODE Mlb3
  BUNDLE_ID com.kingstudios.multiblend.enhanced
  PRODUCT_NAME "Multi-Blend Enhanced"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  FORMATS VST3 AU
  VST3_CATEGORIES "Fx"
  AU_MAIN_TYPE "kAudioUnitType_Effect"
)

juce_generate_juce_header(CrossFX)

target_sources(CrossFX PRIVATE
  src/PluginProcessor.cpp
  src/PluginEditor.cpp
  src/LookAndFeel.cpp
  src/ClipperLimiter.cpp
)

# Embed assets (logo) into the binary
juce_add_binary_data(MultiBlenderEnhancedAssets
  SOURCES
    assets/multiblend_logo.png
)

target_link_libraries(CrossFX PRIVATE MultiBlenderEnhancedAssets)

target_compile_definitions(CrossFX PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_PLUGINHOST_VST=0
  JUCE_PLUGINHOST_VST3=0
)

target_link_libraries(CrossFX PRIVATE
  juce::juce_audio_utils
  juce::juce_audio_plugin_client
  juce::juce_dsp
)


