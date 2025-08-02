cmake_minimum_required(VERSION 3.22)

project(ARTEFACT VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ──────────────────────────────────────────────────────────────────────────────
# JUCE setup
# ──────────────────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        origin/master
)
FetchContent_MakeAvailable(juce)

# ──────────────────────────────────────────────────────────────────────────────
# Plugin target: ARTEFACT
# ──────────────────────────────────────────────────────────────────────────────
juce_add_plugin(ARTEFACT
  COMPANY_NAME                "YourCompany"
  PRODUCT_NAME                "ARTEFACT"
  PLUGIN_MANUFACTURER_CODE   'MANU'
  PLUGIN_CODE                'ARTE'
  FORMATS                    VST3 Standalone
  IS_SYNTH                   TRUE
  NEEDS_MIDI_INPUT           TRUE
  NEEDS_MIDI_OUTPUT          FALSE
  IS_MIDI_EFFECT             FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD    TRUE
)

target_compile_definitions(ARTEFACT PRIVATE
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  BUILDING_ARTEFACT_PLUGIN
)

target_sources(ARTEFACT PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/Core/Commands.h
  Source/Core/ForgeProcessor.cpp
  Source/Core/ForgeProcessor.h
  Source/Core/ForgeVoice.cpp
  Source/Core/ForgeVoice.h
  Source/Core/CanvasProcessor.cpp
  Source/Core/CanvasProcessor.h
  Source/Core/ParameterBridge.h
  Source/Core/ModMatrix.cpp
  Source/Core/ModMatrix.h
  Source/Core/GrainPool.cpp
  Source/Core/GrainPool.h
  Source/GUI/PluginEditor.cpp
  Source/GUI/PluginEditor.h
  Source/GUI/ArtefactLookAndFeel.cpp
  Source/GUI/ArtefactLookAndFeel.h
  Source/GUI/HeaderBarComponent.cpp
  Source/GUI/HeaderBarComponent.h
  Source/GUI/ForgePanel.cpp
  Source/GUI/ForgePanel.h
  Source/GUI/CanvasPanel.cpp
  Source/GUI/CanvasPanel.h
  Source/GUI/SampleSlotComponent.cpp
  Source/GUI/SampleSlotComponent.h
)

target_include_directories(ARTEFACT PRIVATE
  Source
  Source/Core
  Source/GUI
  ${juce_SOURCE_DIR}/modules
)

target_link_libraries(ARTEFACT PRIVATE
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_audio_formats
  juce::juce_audio_plugin_client
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_core
  juce::juce_data_structures
  juce::juce_dsp
  juce::juce_events
  juce::juce_graphics
  juce::juce_gui_basics
  juce::juce_gui_extra
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)

# ──────────────────────────────────────────────────────────────────────────────
# Standalone SpectralCanvasApp
# ──────────────────────────────────────────────────────────────────────────────
add_executable(SpectralCanvasApp
  Source/Main.cpp
  Source/SpectralCanvasApp.h
  Source/MainComponent.cpp
  Source/MainComponent.h
  Source/CanvasComponent.cpp
  Source/CanvasComponent.h
  Source/SoundRenderer.cpp
  Source/SoundRenderer.h
)

target_include_directories(SpectralCanvasApp PRIVATE
  Source
  Source/Core
  Source/GUI
  ${juce_SOURCE_DIR}/modules
)

target_compile_definitions(SpectralCanvasApp PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
)

target_link_libraries(SpectralCanvasApp PRIVATE
  juce::juce_gui_basics
  juce::juce_audio_utils
  juce::juce_audio_formats
  juce::juce_core
  juce::juce_graphics
  juce::juce_events
  juce::juce_recommended_config_flags
  juce::juce_recommended_warning_flags
)

# ──────────────────────────────────────────────────────────────────────────────
# Compiler warnings and flags
# ──────────────────────────────────────────────────────────────────────────────
if(MSVC)
  target_compile_options(ARTEFACT           PRIVATE /W4 /permissive-)
  target_compile_options(SpectralCanvasApp PRIVATE /W4 /permissive-)
else()
  target_compile_options(ARTEFACT           PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(SpectralCanvasApp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ──────────────────────────────────────────────────────────────────────────────
# Optional debugger dir for standalone builds
# ──────────────────────────────────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set_target_properties(ARTEFACT_Standalone PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
  set_target_properties(SpectralCanvasApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  )
endif()

# ──────────────────────────────────────────────────────────────────────────────
# Done
# ──────────────────────────────────────────────────────────────────────────────
message(STATUS "✅ ARTEFACT + SpectralCanvasApp configured successfully.")
