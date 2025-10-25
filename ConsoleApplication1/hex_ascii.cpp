#include "hex_ascii.h"
bool isHexString(const std::string& s) {
    return !s.empty() &&
        std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return std::isxdigit(c); // ספרה או A–F
            });
}
std::string BytesToHex(const std::string& bytes) {
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    out.reserve(bytes.size() * 2);
    for (unsigned char b : bytes) {
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}
std::string WStringToUtf8(const std::wstring& w) {
    if (w.empty()) return {};
    int need = WideCharToMultiByte(CP_UTF8, 0,
        w.c_str(), (int)w.size(),
        nullptr, 0, nullptr, nullptr);

    std::string out(need, '\0'); // מאתחל מחרוזת עם מקום ריק
    WideCharToMultiByte(CP_UTF8, 0,
        w.c_str(), (int)w.size(),
        &out[0], need, nullptr, nullptr); // כאן השינוי
    return out;
}
std::wstring StringToWString(const std::string& str)
{
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
std::string toHex(const unsigned char* data, int len) {
    static const char* hex = "0123456789ABCDEF";
    std::string out;
    out.reserve(len * 2);

    for (int i = 0; i < len; i++) {
        unsigned char b = data[i];
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }

    return out;
}
uint8_t hexToByte(const std::string& hex) {
    if (hex.size() == 0) return 0;
    return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
}

std::string paketToHex(const lora_paket& p) {
    AppendOutputToGUI(hwndOutput2, "paket to HEX" + p.payload[1]);
    std::string out;
    out += toHexSizeT(p.Endseq);
    out += "7E";
    out += toHex(&p.seq, 1);
    out += "7E";
    out += toHex(p.payload, SIZE);

    //for (int i = 0; i < SIZE; i++) {
     //out += toHex(&p.payload[i], 1);
      // out +=p.payload[i];
   // }


    out += "7E";
    out += toHex(&p.crc8, 1);

    return out;
}
std::string intToHexByte(int value) {
    std::stringstream ss;
    ss << std::uppercase    // אותיות גדולות (A–F)
        << std::setfill('0') // מוסיף אפסים מובילים
        << std::setw(2)      // תמיד 2 תווים
        << std::hex
        << (value & 0xFF);   // רק בית אחד
    return ss.str();
}
size_t hexToSizeT(const std::string& hex) {
    return static_cast<size_t>(strtoull(hex.c_str(), nullptr, 16));
}
/*
char hexToChar(const std::string& hex) {
    return static_cast<char>(strtol(hex.c_str(), nullptr, 16));
}
*/
uint8_t hexToChar(const std::string& hex) {
    return static_cast<uint8_t>(std::stoul(hex, nullptr, 16));
}

std::string hexToAscii(const std::string& hex) {
    std::string ascii;
    ascii.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char chr = (char)strtol(byteString.c_str(), nullptr, 16);
        ascii.push_back(chr);
    }

    return ascii;
}
std::string toHexSizeT(size_t value) {
    std::ostringstream oss;
    oss << std::uppercase      // אותיות גדולות A-F
        << std::hex            // מצב הקסאדצימלי
        << std::setw(2)        // לפחות 2 תווים
        << std::setfill('0')   // ריפוד באפסים אם צריך
        << value;
    return oss.str();
}