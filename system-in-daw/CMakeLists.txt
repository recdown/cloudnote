cmake_minimum_required(VERSION 3.24)
project(SystemInDawOut VERSION 0.2.0 LANGUAGES CXX)

if(NOT WIN32)
  message(FATAL_ERROR "This project currently targets Windows audio APIs.")
endif()

include(CTest)

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
