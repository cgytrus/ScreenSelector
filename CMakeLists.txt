cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME ScreenSelector)
project(${PROJECT_NAME})

find_file(WINDOWS_HEADER windows.h)
if(NOT WINDOWS_HEADER)
  message(FATAL_ERROR "Can't find windows.h!")
endif()

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  libraries/hackpro
  libraries/minhook/include
  libraries/gd.h/include
  libraries/gd.h
  libraries/cocos-headers/
  libraries/cocos-headers/extensions
  libraries/cocos-headers/cocos2dx
  libraries/cocos-headers/cocos2dx/include
  libraries/cocos-headers/cocos2dx/kazmath/include
  libraries/cocos-headers/cocos2dx/platform/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32
  libraries/cocos-headers/cocos2dx/platform/third_party/win32/OGLES
)

add_subdirectory(libraries/minhook)
target_link_libraries(${PROJECT_NAME} minhook)

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libraries/hackpro/hackpro.lib)
target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:hackpro.dll")

target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/cocos2dx/libcocos2d.lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/extensions/libExtensions.lib)
target_link_libraries(${PROJECT_NAME} opengl32 ${CMAKE_SOURCE_DIR}/libraries/glew32.lib)
target_link_libraries(${PROJECT_NAME} opengl32 ${CMAKE_SOURCE_DIR}/libraries/cocos-headers/cocos2dx/robtop/glfw/glfw3.lib)