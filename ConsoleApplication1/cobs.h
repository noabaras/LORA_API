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
#include "hex_ascii.h"
std::string decodeCobs(const std::string& cobs);
std::string crateCobs(std::string hexpaket);