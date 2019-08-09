cmake_minimum_required(VERSION 3.5)
project(codec2jni)

add_subdirectory(codec2)
include_directories(codec2/src)

set(SOURCES codec2_jni.c)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_link_libraries(${PROJECT_NAME} log android codec2)


