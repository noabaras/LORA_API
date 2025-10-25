#define _CRT_SECURE_NO_WARNINGS
#define IDC_EDIT_PAYLOAD  1001
#define IDC_BTN_SEND      1002

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
#include <cassert>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define SIZE 5

using namespace std;

#include "cobs.h"
#include"crc.h"
#include"hex_ascii.h"
#include"lora_api.h"

#include "jason.h"


class PacketHandler {
    unsigned char lastSeq = 0;
    uint8_t lastcrc8;
    lora_paket* paket;
    bool check_cobs(const std::string& paket) {
        if (paket.size() < 2 || (paket.size() % 2) != 0) {
            AppendOutputToGUI(hwndOutput, "[ERR] Packet length invalid (must be >=2 and even)");
            return false;
        }
        if (paket.compare(paket.size() - 2, 2, "00") != 0) {
            AppendOutputToGUI(hwndOutput, "[ERR] Packet missing 00 terminator");
            return false;
        }
        for (unsigned char c : paket) {
            if (!std::isxdigit(c)) {
                AppendOutputToGUI(hwndOutput, "[ERR] Non-hex character in packet");
                return false;
            }
        }
        return true;
    }

    bool check_squ(const lora_paket& p, uint8_t /*crc_from_prev*/) {
        if ((uint8_t)(lastSeq + 1) == p.seq) {
            lastSeq = p.seq;                // עדכון רק כשבסדר
            AppendOutputToGUI(hwndOutput, "OK sequence");
            return true;
        }
        AppendOutputToGUI(hwndOutput, "Missing/Out-of-order: expected " +
            std::to_string((uint8_t)(lastSeq + 1)) + " got " + std::to_string(p.seq));
        return false;
    }


    bool checkCrc(uint8_t crc8, vector<uint8_t>data) {
        AppendOutputToGUI(hwndOutput, "function:checkCrc" +to_string(crc8));
        for (int i = 0; i < data.size(); i++) {
            AppendOutputToGUI(hwndOutput, "function:checkCrc data" + data[i]);
        }
        std::ostringstream oss;
        for (uint8_t b : data)
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)b << " ";
        AppendOutputToGUI(hwndOutput, "Receiver bytes for CRC: " + oss.str());
        AppendOutputToGUI(hwndOutput, "Receiver expected CRC: " + std::to_string(crc8));
        AppendOutputToGUI(hwndOutput, "Receiver calculated CRC: " + std::to_string(calcCRC8(data)));

       // AppendOutputToGUI(hwndOutput, "function:checkCrc"+ to);
        if (calcCRC8(data) == crc8) {
            return true;
        }
        return false;


    }
public:
    string HandlingCompletedPackage(string paket) {

        AppendOutputToGUI(hwndOutput, "HandlingCompletedPackage" + paket);
        // AppendOutputToGUI(hwndOutput, "HandlingCompletedPackage");
        string decodingPaket;
        lora_paket pakets;

        // הורדת הקובס
        decodingPaket = decodeCobs(paket);
        AppendOutputToGUI(hwndOutput, " decodingPaket" + decodingPaket);
        
        if (check_cobs(decodingPaket)) {

            // שחזור הפקטה
            pakets = PacketaRecovery(decodingPaket);

            // AppendOutputToGUI(hwndOutput, paket.data);
            for (int i = 0; i < 5; i++) {
                AppendOutputToGUI(hwndOutput, "Recovery payload:" + to_string(pakets.payload[i]));

            }
            //AppendOutputToGUI(hwndOutput, "Recovery payload:" + to_string(pakets.payload[i]));
            // חישוב CRC על התוכן
            std::vector<uint8_t> data;
            for (int i = 0; i < SIZE; i++) {
                data.push_back(pakets.payload[i]);
                AppendOutputToGUI(hwndOutput, " build data" + pakets.payload[i]);
            }
            AppendOutputToGUI(hwndOutput, "pakets.crc8" + to_string(pakets.crc8));
            if (check_squ(pakets, lastcrc8)) {
                if (checkCrc(pakets.crc8, data)) {
                    lastcrc8 = pakets.crc8;     // ← חשוב
                    AppendOutputToGUI(hwndOutput2, "compatible crc");
                    return "OK";
                }
                else {
                    AppendOutputToGUI(hwndOutput2, "not compatible crc");
                }
            }

        }
        //AppendOutputToGUI(hwndOutput, " end Recovery package");
        //lastcrc8 = pakets.crc8;
        return "";
    }

    lora_paket PacketaRecovery(const std::string& data) {
        lora_paket paket{};
        paket.Endseq = 0;
        paket.seq = 0;
        std::memset(paket.payload, 0, SIZE);
        paket.crc8 = 0;

        std::vector<std::string> parts;
        size_t start = 0, pos;
        while ((pos = data.find("7E", start)) != std::string::npos) {
            parts.push_back(data.substr(start, pos - start));
            start = pos + 2;
        }
        parts.push_back(data.substr(start));
        /*
        if (parts.size() < 4) {
            AppendOutputToGUI(hwndOutput, "[ERR] not enough parts in data");
            return paket;
        }
        */
        auto require_hex_even = [&](const std::string& s, const char* name) -> bool {
            if (s.size() % 2 != 0) {
                AppendOutputToGUI(hwndOutput, std::string("[ERR] ") + name + " odd length");
                return false;
            }
            for (char ch : s) {
                if (!std::isxdigit((unsigned char)ch)) {
                    AppendOutputToGUI(hwndOutput, std::string("[ERR] ") + name + " bad hex char");
                    return false;
                }
            }
            return true;
        };
        if (parts.size() < 4) {
            AppendOutputToGUI(hwndOutput, "[ERR] not enough parts in data");
            return paket;
        }


        if (!require_hex_even(parts[0], "Endseq") ||
            !require_hex_even(parts[1], "seq") ||
            !require_hex_even(parts[2], "payload") ||
            !require_hex_even(parts[3], "crc8")) {
            return paket;
        }

        paket.Endseq = hexToSizeT(parts[0]);
        paket.seq = hexToChar(parts[1]);

        std::string d = parts[2];
        for (int i = 0; !d.empty() && i < SIZE; ++i) {
            std::string hexPair = d.substr(0, 2);
            d.erase(0, 2);
            paket.payload[i] = hexToChar(hexPair);
            AppendOutputToGUI(hwndOutput2, std::string("payload ") + std::to_string((int)paket.payload[i]));
        }
        AppendOutputToGUI(hwndOutput, std::string("CRC part raw: '") + parts[3] + "'");
        std::string crcPart = parts[3];
        if (crcPart.size() > 2)
            crcPart = crcPart.substr(0, 2); // ניקוי ה-00 העודפים

        paket.crc8 = hexToChar(crcPart);
        AppendOutputToGUI(hwndOutput, "CRC part raw (clean): '" + crcPart + "'");

       

        AppendOutputToGUI(hwndOutput, std::string("Endseq=") + std::to_string(paket.Endseq));
        AppendOutputToGUI(hwndOutput, std::string("seq=") + std::to_string((int)paket.seq));
        AppendOutputToGUI(hwndOutput, std::string("crc8=") + std::to_string((int)paket.crc8));

        return paket;
    }

};

std::string extractRxPacket(const std::string& line);
lora_paket PacketaRecovery(const std::string& data);
class PacketHandler;
void TX(const std::string& dataHex);
void RX(const char* comName);
bool sendChunkWithRetry2(lora_paket paket, string baseCmd);
void SEND(const std::string& dataHex);
lora_paket create_paket(unsigned char seq, const std::string& message, size_t Endseq) {
    lora_paket paket{};

    // מספר רצף
    paket.seq = seq;

    // Endseq
    paket.Endseq = Endseq;

    // העתקת המידע לתוך payload (מקסימום SIZE תווים)
    size_t len = MIN(message.size(), (size_t)SIZE);
    for (size_t i = 0; i < len; i++) {
        paket.payload[i] = (unsigned char)message[i];
    }
    /*
    // אם ההודעה קצרה → נרפד בשאר אפסים
    for (size_t i = len; i < SIZE; i++) {
        paket.payload[i] = 0;
    }
    */
    // חישוב CRC על התוכן
    std::vector<uint8_t> data;
    for (int i = 0; i < SIZE; i++) {
        data.push_back(paket.payload[i]);
    }
    std::ostringstream oss;
    for (uint8_t b : data)
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)b << " ";
    AppendOutputToGUI(hwndOutput2, "Sender bytes for CRC: " + oss.str());
    AppendOutputToGUI(hwndOutput2, "Sender CRC result: " + std::to_string(calcCRC8(data)));

    paket.crc8 = calcCRC8(data);
    
    /*
    std::vector<uint8_t> data;
    for (size_t i = 0; i < message.size(); i += 2) {
        std::string hexPair = message.substr(i, 2);
        data.push_back(hexToChar(hexPair)); // ממיר 2 תווים לערך אמיתי
    }
    paket.crc8 = calcCRC8(data);
    */
    AppendOutputToGUI(hwndOutput2, "ffffffffffff " + paket.payload[1]);
    return paket;
}
void RXThred(const char* comName) {
    PacketHandler handler;
    EnableWindow(hwndButtonRX, TRUE);
    json c = openJason();
    string paket;
    // שולחים את הפקודות ברשימת listen (כמו AT+P2PRX=1)
    if (c.contains("AT_Commands") && c["AT_Commands"].contains("listen")) {
        for (auto& item : c["AT_Commands"]["listen"]) {
            std::string cmd = item["command"].get<std::string>();
            if (cmd.back() != '\n') cmd += "\r\n";
            sendAT(hConnectedPort, cmd.c_str());
        }
    }

    isConnected = true;
    connectedCOM = comName;
    UpdateWindow(hwndStatus);

    AppendOutputToGUI(hwndOutput, "[INFO] Listening on " + std::string(comName));

    char buf[512];
    DWORD got = 0;
    static std::string rxBuffer;  // מצטבר בין קריאות

    while (isConnected) {
        COMSTAT st{};
        DWORD errors = 0;
        if (!ClearCommError(hConnectedPort, &errors, &st)) break;

        if (st.cbInQue > 0) {
            char ch;
            DWORD got = 0;

            // קוראים תמיד בייט אחד
            if (ReadFile(hConnectedPort, &ch, 1, &got, NULL) && got > 0) {
                paket += ch;
                // מציגים את התו שהתקבל
                AppendOutputToGUI(hwndOutput, std::string(1, ch));

                rxBuffer.push_back(ch);
                if (rxBuffer.size() >= 2) {
                    if (rxBuffer.substr(rxBuffer.size() - 2) == "00") {
                        AppendOutputToGUI(hwndOutput, "Package completed");
                        string onlyPacket = extractRxPacket(paket);
                       
                        string re= handler.HandlingCompletedPackage(onlyPacket);
                        //בדיקת תקינות פקטה
                       if (re != "OK") {
                           AppendOutputToGUI(hwndOutput, "faulty package");
                           
                       }
                    }
                }

            }
        }

        Sleep(10);
    }

    AppendOutputToGUI(hwndOutput, "[INFO] RX thread stopped");
    if (hConnectedPort != INVALID_HANDLE_VALUE) {
        CloseHandle(hConnectedPort);
        hConnectedPort = INVALID_HANDLE_VALUE;
    }
}
void SEND(const std::string& dataHex) {
    // מקבלים בהקסא את החבילה שרוצים לשלוח ואז מחלקים את החבילה ל10 תווים 
    lora_paket paket;
    string data = dataHex;
    string response = "";
    string paketa = "";
    int count = 0;
    bool flagHex = false;
    size_t dataEnd = divCeil(dataHex.length(), 10);
    // size_t dataEnd = (dataHex.length() + 9) / 10;
    AppendOutputToGUI(hwndOutput2, "data " + dataHex);
    AppendOutputToGUI(hwndOutput2, "num package neet to send" + dataEnd);
    data.erase(std::remove(data.begin(), data.end(), ' '), data.end());
    if (!isConnected) return;
    // std::vector<uint8_t> payload(dataHex.begin(), dataHex.end());
    std::string chunk = "";
    json cfg = openJason();
    while (!data.empty()) {
        count++;
        AppendOutputToGUI(hwndOutput2, "[DEBUG] Sending chunk #" + std::to_string(count) + " : " + chunk);
        chunk = data.substr(0, std::min<size_t>(10, data.size())); // 5 תווים ASCII פירוק החבילה ל-
        data.erase(0, chunk.size());
        
        paket = create_paket(count, chunk, dataEnd);// בניית החבילה שתכלול את כל מנגנוני האבטחה
        auto& at = cfg["AT_Commands"]["send"];
        for (const auto& item : at) {
            if (!item.contains("command")) continue;
            // לקיחת הפקודה שמתאימה לשליחה
            std::string base = item["command"].get<std::string>();
            /*
            if (!sendChunkWithRetry(chunk, base)) {
                AppendOutputToGUI(hwndOutput2, "[ERROR] Could not send chunk after retries.");
                return;
            }
            */
            // ניסיון שליחת החבילה הכוללת
            if (!sendChunkWithRetry2(paket, base)) {
                AppendOutputToGUI(hwndOutput2, "[ERROR] Could not send chunk after retries.");
                return;
            }
        }
        if (data.empty()) {
            AppendOutputToGUI(hwndOutput2, "[ERROR] Empty data, nothing to send");
            return;
        }

    }
    AppendOutputToGUI(hwndOutput2, " end send succesful");
}
bool sendChunkWithRetry2(lora_paket paket, string baseCmd) {
    char buf[8];
    // std::string crcHex(buf);
    string cobs = crateCobs(paketToHex(paket));
    std::string fullCmd = baseCmd + cobs;

    int maxRetries = 3;
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        AppendOutputToGUI(hwndOutput2, " nums send " + to_string(attempt));
        std::string resp;
        resp = sendAT(hConnectedPort, fullCmd.c_str());
        AppendOutputToGUI(hwndOutput2, "[RX] " + resp);
        if (resp.find("SUCCESS") != std::string::npos &&
            resp.find("SENT") != std::string::npos) {
            return true;
        }

        else {

        }
        AppendOutputToGUI(hwndOutput2, "[WARNING] add try");
    }

    return false;
}
void RX(const char* comName) {
    std::thread listener(RXThred, comName);
    listener.detach(); // מנתקים כדי שלא יחסום את ה־GUI
}
void TX(const std::string& dataHex) {
    std::thread listener(SEND, dataHex);
    listener.detach(); // מנתקים כדי שלא יחסום את ה־GUI

}

std::string extractRxPacket(const std::string& line) {
    size_t pos = line.rfind(',');
    if (pos == std::string::npos) {
        return "";
    }
    return line.substr(pos + 1);  // מה שבא אחרי הפסיק האחרון
}
lora_paket PacketaRecovery(const std::string& data) {
    lora_paket paket{};
    paket.Endseq = 0;
    paket.seq = 0;
    std::memset(paket.payload, 0, SIZE);
    paket.crc8 = 0;

    std::vector<std::string> parts;
    size_t start = 0, pos;
    while ((pos = data.find("7E", start)) != std::string::npos) {
        parts.push_back(data.substr(start, pos - start));
        start = pos + 2;
    }
    parts.push_back(data.substr(start));

    if (parts.size() < 4) {
        AppendOutputToGUI(hwndOutput, "[ERR] not enough parts in data");
        return paket;
    }

    auto require_hex_even = [&](const std::string& s, const char* name) -> bool {
        if (s.size() % 2 != 0) {
            AppendOutputToGUI(hwndOutput, std::string("[ERR] ") + name + " odd length");
            return false;
        }
        for (char ch : s) {
            if (!std::isxdigit((unsigned char)ch)) {
                AppendOutputToGUI(hwndOutput, std::string("[ERR] ") + name + " bad hex char");
                return false;
            }
        }
        return true;
    };

    if (!require_hex_even(parts[0], "Endseq") ||
        !require_hex_even(parts[1], "seq") ||
        !require_hex_even(parts[2], "payload") ||
        !require_hex_even(parts[3], "crc8")) {
        return paket;
    }

    paket.Endseq = hexToSizeT(parts[0]);
    paket.seq = hexToChar(parts[1]);

    std::string d = parts[2];
    for (int i = 0; !d.empty() && i < SIZE; ++i) {
        std::string hexPair = d.substr(0, 2);
        d.erase(0, 2);
        paket.payload[i] = hexToChar(hexPair);
        AppendOutputToGUI(hwndOutput2, std::string("payload ") + std::to_string((int)paket.payload[i]));
    }

    paket.crc8 = hexToChar(parts[3]);

    AppendOutputToGUI(hwndOutput, std::string("Endseq=") + std::to_string(paket.Endseq));
    AppendOutputToGUI(hwndOutput, std::string("seq=") + std::to_string((int)paket.seq));
    AppendOutputToGUI(hwndOutput, std::string("crc8=") + std::to_string((int)paket.crc8));

    return paket;
}





























































//std::string sendAT(HANDLE h, const char* cmd);










//  lora_paket ומפרקת אותה לחבילת של הקסא לוקחת פקטה מסוג 

//שחזור פקטה לצד מקבל המידע


//בודקת שתחביר הפקודות תקין


// פונקציה שבודקת אם CRC שקיבלנו תואם לנתון
bool verifyCRC(const std::vector<uint8_t>& payload, uint8_t crcFromSender) {
    uint8_t crcLocal = calcCRC8(payload);
    return (crcLocal == crcFromSender);
}

// ממיר std::string ל-wstring

/*
void returnLoRaPorts() {
    char** loraDevice = NULL;
    int count = 0;
    char devices[65536];

    DWORD charsReturned = QueryDosDeviceA(NULL, devices, sizeof(devices));
    if (charsReturned == 0) {
        SetWindowText(hwndOutput, L"QueryDosDevice failed.");
        return;
    }

    char* ptr = devices;
    while (*ptr) {
        if (strncmp(ptr, "COM", 3) == 0) {
            char fullPortName[64];
            sprintf_s(fullPortName, "\\\\.\\%s", ptr);

            if (isLoRaDevice(fullPortName)) {  // משתמשים בפונקציה שלך ללא שינוי
                char** tmp = (char**)realloc(loraDevice, sizeof(char*) * (count + 1));
                if (!tmp) {
                    for (int i = 0; i < count; i++) free(loraDevice[i]);
                    free(loraDevice);
                    return;
                }
                loraDevice = tmp;
                loraDevice[count] = (char*)malloc(strlen(fullPortName) + 1);
                strcpy(loraDevice[count], fullPortName);

                wchar_t wPort[64];
                MultiByteToWideChar(CP_ACP, 0, fullPortName, -1, wPort, 64);
                SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)wPort);

                count++;
            }
        }
        ptr += strlen(ptr) + 1;
    }

    if (count == 0) {
        //SetWindowText(hwndOutput, L"No LoRa devices found.");
    }

    for (int i = 0; i < count; i++) {
        free(loraDevice[i]);
    }
    free(loraDevice);
}
*/

//הפיכת תווים אסקיים לערך מספרי





    // פונקציה שמדפיסה את כל פורטי LoRa המחוברים


   



  


 

        
  

// מנתקת פורט


// טוען קובץ JSON



// פונקציה לעזור בהדפסת שגיאות







// פתיחת פורט


// הגדרת פרמטרים של פורט קצב שליחת הנתונים הינול קווי וכל הגדרות שקשורות לכך שנוכל לשלוח פקודות AT



// שולח פקודה ומוודא שיש "OK" בתשובה
static bool at_ok(HANDLE h, const std::string& cmd, int retries = 1, int delay_ms = 80) {
    for (int i = 0; i <= retries; ++i) {
        std::string resp = sendAT(h, (cmd + "\r\n").c_str());
        AppendOutputToGUI(hwndOutput, std::string("[TX] ") + cmd);
        AppendOutputToGUI(hwndOutput, std::string("[RX] ") + resp);
        if (resp.find("OK") != std::string::npos) return true;
        Sleep(delay_ms);
    }
    return false;
}
// מריץ קבוצת פקודות לפי המפתח ("init" / "status")
static bool run_at_group_from_json(const json& root, const char* groupKey, HANDLE h) {
    if (!root.contains("AT_Commands") || !root["AT_Commands"].contains(groupKey)) {
        AppendOutputToGUI(hwndOutput, std::string("[WARN] Missing AT_Commands/") + groupKey);
        return true; // לא קריטי; ממשיכים
    }
    const auto& arr = root["AT_Commands"][groupKey];
    if (!arr.is_array()) {
        AppendOutputToGUI(hwndOutput, std::string("[ERR] AT_Commands/") + groupKey + " must be an array");
        return false;
    }
    for (const auto& item : arr) {
        if (!item.contains("command")) continue;
        std::string cmd = item["command"].get<std::string>();
        if (!at_ok(h, cmd)) {
            AppendOutputToGUI(hwndOutput, std::string("[FAIL] No OK from: ") + cmd);
            return false;
        }
    }
    return true;
}




// Wide → UTF-8

// bytes → HEX (A..F)

 
 //הפונקציה  מקבלת פקטות ושולחת אותם  לרכיב השני בתקשורת
 /*
 bool sendChunkWithRetry(const std::string& chunk, const std::string& baseCmd, int maxRetries = 3) {
     char buf[512];
     DWORD got = 0;
     for (int attempt = 1; attempt <= maxRetries; ++attempt) {
         //std::string cmd = buildCommandWithCRC(baseCmd, chunk);
         std::string hex = BytesToHex(chunk);
         std::string cmd = baseCmd+hex;
         std::string resp = sendAT(hConnectedPort, cmd.c_str());

         AppendOutputToGUI(hwndOutput2, "[TX] " + cmd);
         
     
     //כניסה למצב ההאזנה
     
     while (isConnected) {
         // PurgeComm(hConnectedPort, PURGE_TXCLEAR | PURGE_RXCLEAR);

         COMSTAT st{}; DWORD errors = 0;
         if (!ClearCommError(hConnectedPort, &errors, &st)) break;

         if (st.cbInQue > 0) {
             DWORD toRead = st.cbInQue < sizeof(buf) ? st.cbInQue : sizeof(buf);
             if (ReadFile(hConnectedPort, buf, toRead, &got, NULL) && got > 0) {
                 std::string resp(buf, buf + got);
                 AppendOutputToGUI(hwndOutput2, "[RX] " + resp);
                 AppendOutputToGUI(hwndOutput2, "[RAW RX] " + resp);


                 // אפשר לזהות הודעת קבלה
                 if (resp.find("+P2PTXCONF") != std::string::npos) {
                     AppendOutputToGUI(hwndOutput, "[RECEIVED PACKET] " + resp);
                 }
                 if (resp.find("+P2PTXCONF:SUCCESS") != std::string::npos) {
                     return true; // הצלחה
                 }
                 else {
                     AppendOutputToGUI(hwndOutput2, "[WARN] Failed attempt ");
                     Sleep(100); // המתנה קצרה בין נסיונות
                 }
             }

         }


         Sleep(50); // לא לחנוק CPU
     }
     }
     return false; // נכשל אחרי כל הניסיונות
 }
 */
 
 bool sendChunkWithRetry(const std::string& chunk, const std::string& baseCmd, int maxRetries = 3) {
   

     // חישוב CRC
     uint8_t crcVal = calcCRC8(std::vector<uint8_t>(chunk.begin(), chunk.end()));

     char buf[8];
     sprintf(buf, "%02X", crcVal);
     std::string crcHex(buf);
     std::string fullCmd = baseCmd + chunk ;
   




     for (int attempt = 1; attempt <= maxRetries; ++attempt) {
         std::string resp;
        resp= sendAT(hConnectedPort, fullCmd.c_str());
         AppendOutputToGUI(hwndOutput2, "[TX] send" + chunk);
       
         AppendOutputToGUI(hwndOutput2, "[RX] " + resp);
         if (resp.find("SUCCESS") != std::string::npos &&
             resp.find("SENT") != std::string::npos) {
             return true;
         }

         else {

         }
         AppendOutputToGUI(hwndOutput2, "[WARNING] add try"  );
      }
      
     return false;
         }
 


 /*
 void SEND(const std::string& dataHex) {
     if (!isConnected || hConnectedPort == INVALID_HANDLE_VALUE) {
         AppendOutputToGUI(hwndOutput, "[ERR] No active connection");
         return;
     }

     // פקודת broadcast ישירה (dataHex זה כבר HEX מוכן)
    std::string cmd = "AT+P2PBROADCASTTX=" + dataHex;
    
     // שליחה למודם
     std::string resp = sendAT(hConnectedPort, cmd.c_str());
     // כניסה למצב האזנה:
     char buf[512];
     DWORD got = 0;
 
     while (isConnected) {
         PurgeComm(hConnectedPort, PURGE_TXCLEAR | PURGE_RXCLEAR);

         COMSTAT st{}; DWORD errors = 0;
         if (!ClearCommError(hConnectedPort, &errors, &st)) break;

         if (st.cbInQue > 0) {
             DWORD toRead = st.cbInQue < sizeof(buf) ? st.cbInQue : sizeof(buf);
             if (ReadFile(hConnectedPort, buf, toRead, &got, NULL) && got > 0) {
                 std::string resp(buf, buf + got);
                 AppendOutputToGUI(hwndOutput, "[RX] " + resp);
                 AppendOutputToGUI(hwndOutput, "[RAW RX] " + resp);


                 // אפשר לזהות הודעת קבלה
                 if (resp.find("+P2PTXCONF") != std::string::npos) {
                     AppendOutputToGUI(hwndOutput, "[RECEIVED PACKET] " + resp);
                 }
             }
         }




         Sleep(50); // לא לחנוק CPU
     }

     // לוג לפלט
     AppendOutputToGUI(hwndOutput, "[TX] " + cmd);
     AppendOutputToGUI(hwndOutput, "[RX] " + resp);
 }
 */


// הגדרת תפקיד היחידה
void RoleDefinition(HANDLE h, const std::string& role) {
    std::string setCmd = "AT+P2PROLE=" + role + "\r\n";
    std::string respSet = sendAT(h, setCmd.c_str());
    std::cout << "RoleDefinition response: " << respSet << "\n";
}
void readPortLive(HANDLE h) {
    char buf[1];
    DWORD got = 0;

    std::cout << "Starting live read..." << std::endl;

    while (true) {
        COMSTAT st{};
        DWORD errors = 0;
        if (!ClearCommError(h, &errors, &st)) {
            std::cout << "ClearCommError failed!" << std::endl;
            break;
        }

        if (st.cbInQue > 0) {
            if (ReadFile(h, buf, 1, &got, NULL) && got > 0) {
                // מדפיס כל בייט כ־hex
                std::cout << std::hex << (int)(unsigned char)buf[0] << " ";
            }
        }
        Sleep(10); // מנוחה קצרה כדי לא "לתקוע" את הלולאה
    }
}


// ---------- MAIN ----------
int main() {
  
    const char* portName1 = R"(\\.\COM3)";
    const char* portName2 = R"(\\.\COM3)";

    HANDLE h1 = INVALID_HANDLE_VALUE;
    HANDLE h2 = INVALID_HANDLE_VALUE;
    /*
   h1 = openingPort(portName1);
    if (h1 == INVALID_HANDLE_VALUE) return 1;
    setupPort(h1);
    if (!setupPort(h1)) {
       CloseHandle(h1);
        return 1;
    }
    std::string resp = sendAT(h1, "AT\r\n");
     std::cout << "respons :" << resp;
    */
    //readPortLive(h1);
    
   //checkLoRaPort();
     //h2 = openingPort(portName2);
   // if (h2 == INVALID_HANDLE_VALUE) return 1;
   //  if (!setupPort(h2)) { CloseHandle(h2); return 1; }
    //Json();
    std::string myID1 = "01:02:09:A2";
    std::string myID2 = "01:02:09:04";

  //returnLoRaPorts();
    //
    //sendMessageToID(h2, "01:02:09:04", "hello");

   // CloseHandle(h1);
    // CloseHandle(h2);

    return 0;
}
