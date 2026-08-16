cmake_minimum_required(VERSION 3.22)

project(DOL_BKL VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Copy the built plugin to the DAW plugin folder after build.
# ON for local dev (macOS). Turn OFF when packaging on Windows so the
# build doesn't need an elevated shell (the installer handles installation):
#   cmake -DDOL_COPY_AFTER_BUILD=OFF ...
set(DOL_COPY_AFTER_BUILD ON CACHE BOOL "Copy built plugin to the plugin folder after build")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE SDK location.
# Override on other machines (e.g. Windows) with:
#   cmake -DDOL_JUCE_ROOT=C:/path/to/JUCE ..
if(NOT DEFINED DOL_JUCE_ROOT)
    set(DOL_JUCE_ROOT
        "/Volumes/SSD Ex 1 Tb/Development/JUCE"
        CACHE PATH "Path to the JUCE SDK")
endif()
add_subdirectory("${DOL_JUCE_ROOT}" JUCE)

# foleys_gui_magic defaults to building its examples/tests, which would make
# it FetchContent a SECOND copy of JUCE (duplicate targets -> CMake error).
# Disable them so only our JUCE (DOL_JUCE_ROOT) is used.
set(FOLEYS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FOLEYS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FOLEYS_RUN_PLUGINVAL OFF CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/foleys_gui_magic"
    foleys_gui_magic
)
juce_add_plugin(DOL_BKL
    COMPANY_NAME "Digiethnica"
    BUNDLE_ID com.digiethnica.dolbengkulu
    PLUGIN_MANUFACTURER_CODE Dedi
    PLUGIN_CODE TST2
    FORMATS VST3
    PRODUCT_NAME "DOL BENGKULU"
    VST3_CATEGORIES Instrument Sampler
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD ${DOL_COPY_AFTER_BUILD}
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

# --- Distribution: bundle the samples inside the VST3 bundle so the
# installer works on machines that don't have the development folder.
# Same bundle layout on macOS & Windows VST3:
#   <bundle>.vst3/Contents/Resources/Samples
# The plugin binary sits at <bundle>/Contents/MacOS (macOS) or
# <bundle>/Contents/x86_64-win (Windows), so the bundle root is two
# levels up from the target file directory.
set(DOL_SAMPLES_DIR "$<TARGET_FILE_DIR:DOL_BKL_VST3>/../..")
add_custom_command(
    TARGET DOL_BKL_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Samples/V1"
            "${DOL_SAMPLES_DIR}/Contents/Resources/Samples"
    COMMAND ${CMAKE_COMMAND}
            -DCLEAN_DIR="${DOL_SAMPLES_DIR}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/clean_apple_double.cmake"
    COMMENT "Bundling Samples/V1 into the VST3 bundle"
)
