#ifndef __INCLUDES_H
#define __INCLUDES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <iostream>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
// shut up cocos // lmao
#pragma warning(push, 0)
#include <cocos2d.h>
#pragma warning(pop)
#include <MinHook.h>
#include <gd.h>
#include "utils.hpp"

using std::uintptr_t;

using namespace cocos2d;

#endif