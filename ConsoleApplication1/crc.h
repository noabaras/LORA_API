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
uint8_t calcCRC8(const std::vector<uint8_t>& data);