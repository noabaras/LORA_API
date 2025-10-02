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

using namespace std;
typedef struct {
    size_t Endseq;
    unsigned char seq;      // מספר רצף
    unsigned char payload[SIZE];
    uint8_t crc8;     // חישוב CRC על כל ההודעה לפניו
} lora_paket;
std::string hexToAscii(const std::string& hex);
char hexToChar(const std::string& hex);
size_t hexToSizeT(const std::string& hex);
std::string intToHexByte(int value);
std::string paketToHex(const lora_paket& p);
uint8_t hexToByte(const std::string& hex);
std::string toHex(const unsigned char* data, int len);
std::wstring StringToWString(const std::string& str);
std::string WStringToUtf8(const std::wstring& w);
std::string BytesToHex(const std::string& bytes);
bool isHexString(const std::string& s);
size_t hexToSizeT(const std::string& hex);
char hexToChar(const std::string& hex);
std::string hexToAscii(const std::string& hex);
std::string toHexSizeT(size_t value);