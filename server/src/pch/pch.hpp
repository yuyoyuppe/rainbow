#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <iostream>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <array>
#include <numeric>
#include <fstream>
#include <bitset>
#include <optional>
#include <cassert>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#else
#include <locale>
#include <clocale>
#endif

#define NOMINMAX
#include <windows.h>

namespace fs = std::filesystem;
