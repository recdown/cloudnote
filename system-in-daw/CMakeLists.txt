cmake_minimum_required(VERSION 3.25)
project(SystemInDawOut VERSION 0.3.0 LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "This project currently targets Windows audio APIs.")
endif()

include(CTest)
include(FetchContent)

# Pin the official Steinberg SDK revision. Its git submodules provide the
# matching base, pluginterfaces, public SDK and CMake support repositories.
set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VSTGUI_SUPPORT OFF CACHE BOOL "" FORCE)
set(SMTG_CREATE_PLUGIN_LINK OFF CACHE BOOL "" FORCE)

FetchContent_Declare(vst3sdk
  GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
  GIT_TAG 3cdf9ca5d1f5b1b21e0a86832aa4abe55607bd96
  GIT_SUBMODULES_RECURSE TRUE
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(vst3sdk)
smtg_enable_vst3_sdk()

add_executable(SystemInProbe src/system_in_probe.cpp)
target_compile_features(SystemInProbe PRIVATE cxx_std_20)
target_compile_definitions(SystemInProbe PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(SystemInProbe PRIVATE ole32 uuid propsys)
if(MSVC)
  target_compile_options(SystemInProbe PRIVATE /W4 /permissive- /EHsc)
endif()

add_executable(AudioRingBufferTest tests/audio_ring_buffer_test.cpp)
target_include_directories(AudioRingBufferTest PRIVATE src)
target_compile_features(AudioRingBufferTest PRIVATE cxx_std_20)
if(MSVC)
  target_compile_options(AudioRingBufferTest PRIVATE /W4 /permissive- /EHsc)
endif()
add_test(NAME AudioRingBufferTest COMMAND AudioRingBufferTest)

smtg_add_vst3plugin(SystemIn
  vst3/cids.h
  vst3/controller.cpp
  vst3/entry.cpp
  vst3/processor.cpp
  vst3/version.h
  src/shared_audio_mapping.hpp
)
target_include_directories(SystemIn PRIVATE src vst3)
target_compile_features(SystemIn PRIVATE cxx_std_20)
target_link_libraries(SystemIn PRIVATE sdk)
if(MSVC)
  target_compile_options(SystemIn PRIVATE /W4 /permissive- /EHsc)
endif()
smtg_target_configure_version_file(SystemIn)
