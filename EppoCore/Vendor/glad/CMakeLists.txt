cmake_minimum_required(VERSION 3.14)

# Project
project(glad VERSION 0.1.36 LANGUAGES C)

# Files
set(SOURCES
    "src/glad.c"
)

set(INCLUDE_DIRS
    "include"
)

# Create target
add_library(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})