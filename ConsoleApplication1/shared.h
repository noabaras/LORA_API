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
#define SIZE 5
using namespace std;
void AppendOutputToGUI(HWND hwndEdit, const std::string& s);
std::wstring SafeStringForGUI(const std::string& str);
int divCeil(int a, int b);