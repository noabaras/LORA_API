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
#include "jason.h"
bool setupPort(HANDLE h);
bool resetMCU_like_WE(const char* comName);
void AppendOutputToGUI(HWND hwndEdit, const std::string& s);
std::string sendAT(HANDLE h, const char* cmd);
HANDLE openingPort(const char* portName);
void disconnected(const char* comName);
void connected(const char* comName);
void returnLoRaPorts();
int isLoRaDevice(const char* portName);
bool CorrectsyntaxAT(const std::string& cmd);
static void print_last_error(const char* where);
std::string readResponse(HANDLE hPort);
//vbbb
