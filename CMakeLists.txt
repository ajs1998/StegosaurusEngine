# CMakeList.txt: CMake project

# Minimum CMake Version
cmake_minimum_required(VERSION 3.19)

# Project Name
project("StegosaurusEngine")
set(CMAKE_CXX_STANDARD 20)

# Create a list of source files
set(SRC_DIR "src")
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.h")

# Add the source files to this project's binary
add_executable(${PROJECT_NAME} ${SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
include_directories("include")

# Link submodule libraries
set(LIB_DIR "lib")
list(APPEND LIBS "argon2")
list(APPEND LIBS "lodepng")
foreach(LIB_NAME ${LIBS})
    set(LIB_PATH "${LIB_DIR}/${LIB_NAME}")
    message(STATUS "Found Library: ${LIB_NAME} in ${LIB_PATH}")
#    add_subdirectory(${LIB_PATH})
    target_link_libraries(${PROJECT_NAME} ${LIB_NAME})
    include_directories(${PROJECT_NAME} ${LIB_PATH})
endforeach()
