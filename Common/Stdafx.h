#pragma once

// 이 헤더 파일은 모든 프로젝트의 Stdafx.h에서 가장 첫번째로 포함한다.
// 모든 프로젝트에서 공통으로 사용하는 헤더들을 포함한다.

// C/C++
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <ranges>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
using namespace std::chrono_literals;

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#ifdef DrawText
#undef DrawText
#endif

// Project
#include "Delegate.h"
#include "Singleton.h"
#include "StringTable.h"
#include "Types.h"
