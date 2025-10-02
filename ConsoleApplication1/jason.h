#pragma once
#include "Globals.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include "C:\Users\jason\json.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <regex>
#include <algorithm>
#include "shared.h"

using json = nlohmann::json;
static json load_json_file(const std::wstring& path);
json openJason();