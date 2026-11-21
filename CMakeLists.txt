cmake_minimum_required(VERSION 3.22)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build Universal Binary" FORCE)
endif()

project(ICStandardGuitar VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JUCE_DIR "" CACHE PATH "Path to local JUCE")
if(NOT JUCE_DIR AND DEFINED ENV{JUCE_DIR})
    set(JUCE_DIR "$ENV{JUCE_DIR}")
endif()
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    foreach(_cand
        "${CMAKE_CURRENT_SOURCE_DIR}/JUCE"
        "$ENV{HOME}/ClaudeCode/chord-pad-au/JUCE"
        "$ENV{HOME}/JUCE")
        if(EXISTS "${_cand}/CMakeLists.txt")
            set(JUCE_DIR "${_cand}")
            break()
        endif()
    endforeach()
endif()
if(NOT EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found. Pass -DJUCE_DIR=/path/to/JUCE")
endif()

# Strip absolute local workspace/home paths from __FILE__ and debug metadata
add_compile_options(
    "-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=."
    "-ffile-prefix-map=${JUCE_DIR}=JUCE"
    "-ffile-prefix-map=$ENV{HOME}=~"
)

add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

# Core DSP & sample engine (shared between the plugin and automated tests)
add_library(ICStandardGuitarCore STATIC
    Source/SampleBank.cpp
    Source/GuitarSynthEngine.cpp)

target_include_directories(ICStandardGuitarCore PUBLIC Source)
target_compile_definitions(ICStandardGuitarCore PUBLIC
    JUCE_USE_FLAC=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)

target_link_libraries(ICStandardGuitarCore
    PUBLIC
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

juce_add_plugin(ICStandardGuitar
    COMPANY_NAME "InstaChordTools"
    BUNDLE_ID "com.instachordtools.icstandardguitar"
    PLUGIN_MANUFACTURER_CODE Icht
    PLUGIN_CODE Icsg
    FORMATS AU Standalone
    PRODUCT_NAME "IC Standard Guitar"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    COPY_PLUGIN_AFTER_BUILD TRUE)

target_sources(ICStandardGuitar PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(ICStandardGuitar PUBLIC
    JUCE_USE_FLAC=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(ICStandardGuitar
    PRIVATE
        ICStandardGuitarCore
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

enable_testing()

juce_add_console_app(ICStandardGuitarTest PRODUCT_NAME "ICStandardGuitarTest")
target_sources(ICStandardGuitarTest PRIVATE Tests/EngineVerificationTest.cpp)
target_compile_definitions(ICStandardGuitarTest PRIVATE
    JUCE_USE_FLAC=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)
target_link_libraries(ICStandardGuitarTest
    PRIVATE
        ICStandardGuitarCore
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
add_test(NAME ICStandardGuitarTest COMMAND ICStandardGuitarTest)
