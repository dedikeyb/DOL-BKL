cmake_minimum_required(VERSION 3.22)

project(DOL_BKL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(
    "/Volumes/SSD Ex 1 Tb/Development/JUCE"
    JUCE
)

set(FOLEYS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FOLEYS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FOLEYS_RUN_PLUGINVAL OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/foleys_gui_magic"
    foleys_gui_magic
)
juce_add_plugin(DOL_BKL
    COMPANY_NAME "DOL BKL"
    BUNDLE_ID com.dedikeyb.dolbkl.test02
    PLUGIN_MANUFACTURER_CODE Dedi
    PLUGIN_CODE TST2
    FORMATS VST3
    PRODUCT_NAME "DOL BENGKULU PROTOTYPE"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(DOL_BKL)

juce_add_binary_data(DOL_BKL_UI_ASSETS
    SOURCES
        Source/UI_ASSETS_EXTRACTED/background.png
        Source/UI_ASSETS_EXTRACTED/knobFace.png
)

target_sources(DOL_BKL
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SampleManager.cpp
        Source/SampleManager.h
        Source/DOLVoice.cpp
        Source/DOLVoice.h
)

target_compile_definitions(DOL_BKL
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        DOL_SAMPLE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Samples/V1"
)

target_link_libraries(DOL_BKL
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_basics
        juce::juce_core
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_audio_formats
        DOL_BKL_UI_ASSETS
        foleys::foleys_gui_magic
)

target_compile_features(DOL_BKL PRIVATE cxx_std_17)
