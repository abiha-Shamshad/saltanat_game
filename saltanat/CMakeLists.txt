cmake_minimum_required(VERSION 3.10)
project(Saltanat CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find SFML (Graphics, Window, System)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(Saltanat ${SOURCES})

target_link_libraries(Saltanat PRIVATE sfml-graphics sfml-window sfml-system)

# Copy assets folder to output directory if needed, or assume it's run from the project root.
