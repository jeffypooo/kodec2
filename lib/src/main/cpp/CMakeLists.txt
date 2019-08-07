cmake_minimum_required(VERSION 3.5)

# Name top level CMake project
project(beartooth-jni)

set(BUILD_TESTS OFF CACHE BOOL "enable/disable libbeartooth unit test build")

add_subdirectory(libbeartooth)
include_directories(libbeartooth/ext/RxCpp/Rx/v2/src)

add_subdirectory(codec2)
include_directories(codec2/src)

set(BEARTOOTH_JNI_SRC beartooth_jni.cpp jni_data_output.cpp jni_callbacks.cpp codec2_jni.c)

add_library(${PROJECT_NAME} SHARED ${BEARTOOTH_JNI_SRC})
target_link_libraries(${PROJECT_NAME} log android beartooth codec2)


