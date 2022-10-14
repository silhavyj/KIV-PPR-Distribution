# Minimum required version of cmake
cmake_minimum_required(VERSION 3.0)

# Specify the C++ standard used in the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories, so there is no need
# to specify relative paths
include_directories(lib)

# Add the name of the project, version, description,
# home URL, and a list of all languages used in the project
project(
    pprsolver
    VERSION 0.0.1
    DESCRIPTION "KIV/PPR - semester project"
    HOMEPAGE_URL "%%myurl%%"
    LANGUAGES CXX C)

# Add src folder
add_subdirectory(src)