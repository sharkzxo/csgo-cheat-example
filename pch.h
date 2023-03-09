// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN 

//windows
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>

//directx
#include <d3d9types.h>
#include <d3d9helper.h>

//important 
#include <string>
#include <thread>
#include <chrono>

//encryption
#include "encryption/xorstr.h"
#include "encryption/lazy.h"

//sdk
#include "sdk/interfaces/interfaces.h"

//hooking
#pragma comment(lib, "minhook")
#include "minhook/minhook.h"

//imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#endif //PCH_H
