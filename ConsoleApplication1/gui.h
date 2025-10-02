#pragma once

#include <windows.h>
#include <string>
#include "C:\Users\jason\json.hpp"
using json = nlohmann::json;





// הצהרות של הפונקציות
int isLoRaDevice(const char* portName);
void returnLoRaPorts();
void disconnected(const char* comName);
void connected(const char* comName);
HANDLE openingPort(const char* portName);
void trys(const char* portName);
void RX(const char* comName);
 std::string WStringToUtf8(const std::wstring& w);
 std::string BytesToHex(const std::string& bytes);
 void TX(const std::string& dataHex);
void SEND(const std::string& dataHex);
bool resetMCU_like_WE(const char* comName);


 

