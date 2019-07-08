cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(airkit-host-sdl LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include_directories(
  $ENV{AIRBUILD_PREFIX}/include/
)

add_library(
  airkit-host-sdl
  SHARED
  host.cpp
)
target_link_libraries(airkit-host-sdl SDL2)

install(
  TARGETS airkit-host-sdl
  LIBRARY DESTINATION lib
)