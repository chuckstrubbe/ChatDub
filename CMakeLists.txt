cmake_minimum_required(VERSION 3.22)

project(Voice2MIDI VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Voice2MIDI
    COMPANY_NAME "Open Audio Tools"
    BUNDLE_ID com.openaudiotools.voice2midi
    PLUGIN_MANUFACTURER_CODE OAtl
    PLUGIN_CODE V2Md
    FORMATS VST3 Standalone
    PRODUCT_NAME "Voice2MIDI"
    DESCRIPTION "Real-time voice-to-MIDI audio effect"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(Voice2MIDI)

target_sources(Voice2MIDI PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/YinPitchDetector.cpp
    Source/YinPitchDetector.h
)

target_compile_features(Voice2MIDI PRIVATE cxx_std_17)
target_compile_definitions(Voice2MIDI PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Voice2MIDI PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
