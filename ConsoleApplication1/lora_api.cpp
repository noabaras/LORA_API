#include "lora_api.h"
bool setupPort(HANDLE h) {
    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(h, &dcb)) {
        print_last_error("GetCommState");
        return false;
    }

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    if (!SetCommState(h, &dcb)) {
        print_last_error("SetCommState");
        AppendOutputToGUI(hwndOutput, "setup faild");
        return false;
    }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = 50;
    to.ReadTotalTimeoutConstant = 500;
    to.ReadTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant = 500;
    to.WriteTotalTimeoutMultiplier = 0;
    if (!SetCommTimeouts(h, &to)) {
        print_last_error("SetCommTimeouts");
        return false;
    }

    EscapeCommFunction(h, CLRDTR);
    EscapeCommFunction(h, CLRRTS);
    Sleep(50);
    EscapeCommFunction(h, SETDTR);
    EscapeCommFunction(h, SETRTS);
    Sleep(100);
    PurgeComm(h, PURGE_RXCLEAR | PURGE_TXCLEAR);
    AppendOutputToGUI(hwndOutput, "succesfull connected");
    // std::cout << "succesfull connected";
    return true;
}
bool resetMCU_like_WE(const char* comName) {
    HANDLE hPort = openingPort(comName);
    if (!isConnected) {
        return false;
    }
    json c = openJason();
    if (!isConnected) {
        return false;
    }
    // 1. ניקוי הבאפרים
    PurgeComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);//ניקוי תור הפיפו
    PurgeComm(hConnectedPort, PURGE_RXCLEAR | PURGE_TXCLEAR);


    // 2. שליחת ATZ
    std::string resp = sendAT(hPort, "ATZ\r\n");

    // 3. המתנה ל-READY
    DWORD startTick = GetTickCount();
    bool readyFound = false;
    while (GetTickCount() - startTick < 5000) { // עד 5 שניות
        std::string r = readResponse(hPort); // פונקציה שמחזירה מהגיע
        if (r.find("READY") != std::string::npos) {
            readyFound = true;
            break;
        }
        Sleep(50);
    }
    if (!readyFound) {
        AppendOutputToGUI(hwndOutput, "[ERR] לא התקבל READY אחרי ATZ");
        return false;
    }

    // 4. השהייה קטנה
    Sleep(300);

    // 5. שליחת פקודות INIT מחדש
    for (auto& item : c["AT_Commands"]["init1"]) {
        std::string cmd = item["command"].get<std::string>();
        if (cmd.back() != '\n') cmd += "\r\n";
        sendAT(hPort, cmd.c_str());
    }

    return true;
}
std::string readResponse(HANDLE hPort) {
    std::string response;
    char buf[256];
    DWORD bytesRead = 0;

    // בדיקה כמה בייטים זמינים בתור
    COMSTAT status;
    DWORD errors;
    if (!ClearCommError(hPort, &errors, &status)) {
        return "";
    }

    if (status.cbInQue > 0) {
        DWORD toRead = (status.cbInQue < sizeof(buf)) ? status.cbInQue : sizeof(buf);
        if (ReadFile(hPort, buf, toRead, &bytesRead, NULL)) {
            response.assign(buf, bytesRead);
        }
    }

    return response;
}
std::string sendAT(HANDLE h, const char* cmd) {

    if (!CorrectsyntaxAT(cmd)) {
        AppendOutputToGUI(hwndOutput, std::string("[INVALID CMD] ") + cmd);
        return {}; // מחזיר מחרוזת ריקה
    }


    if (h == INVALID_HANDLE_VALUE || !cmd) return {};

    DWORD written = 0;
    PurgeComm(h, PURGE_RXCLEAR); // מינימום נדרש

    // לוג TX
    {
        std::string tx = std::string("[TX] ") + cmd;
        AppendOutputToGUI(hwndOutput, tx);
    }

    std::string line = std::string(cmd) + "\r\n";

    // כתיבה – עם CRLF כחלק מהפקודה
    if (!WriteFile(h, line.c_str(), (DWORD)line.size(), &written, NULL)) {
        AppendOutputToGUI(hwndOutput, "[ERR] WriteFile(AT) failed");
        return {}; // ← היה return; לא חוקי לפונקציה שמחזירה std::string
    }

    // קורא תגובה
    std::string resp;
    DWORD startTick = GetTickCount();
    for (;;) {
        COMSTAT st{}; DWORD errors = 0;
        if (!ClearCommError(h, &errors, &st)) break;

        if (st.cbInQue > 0) {
            char buf[600];
            DWORD toRead = st.cbInQue < sizeof(buf) ? st.cbInQue : (DWORD)sizeof(buf);
            DWORD got = 0;
            if (ReadFile(h, buf, toRead, &got, NULL) && got > 0) {
                resp.append(buf, buf + got);
            }
        }
        /*
        // מסיים כשהגיעו CRLF כלשהם או טיימאאוט 1s
        if (!resp.empty() && resp.find("\r\n") != std::string::npos) break;
        if (GetTickCount() - startTick > 1000) break;

        Sleep(500);
        */

        // ✅ מסיים כשהתקבלה תשובה מלאה או טיימאאוט גדול יותר
        if (resp.find("OK\r\n") != std::string::npos ||
            resp.find("AT_ERROR") != std::string::npos ||
            resp.find("ERROR") != std::string::npos ||
            resp.find("+SYSNOTF:READY") != std::string::npos) {
            break;
        }
        if (GetTickCount() - startTick > 3000) break; // 3 שניות
        Sleep(20);
    }

    // לוג RX (raw)
    AppendOutputToGUI(hwndOutput, std::string("[RX] ") + resp);

    return resp;
}
HANDLE openingPort(const char* portName) {

    HANDLE h = CreateFileA(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (h == INVALID_HANDLE_VALUE) {
        print_last_error("CreateFileA");
        std::cout << "INVALID_HANDLE_VALUE";
        return INVALID_HANDLE_VALUE;
    }
    cout << "opening port" << portName;
    SetupComm(h, 4096, 4096);
    PurgeComm(h, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
    return h;
}
void disconnected(const char* comName) {

    if (isConnected) {

        CloseHandle(hConnectedPort);
        hConnectedPort = INVALID_HANDLE_VALUE;
        isConnected = false;
        SetWindowText(hwndStatus, L"Status: Disconnected");


    }
    else {
        SetWindowText(hwndStatus, L"Status:  Already Disconnected");
    }
    string resp = sendAT(hConnectedPort, "AT");
    AppendOutputToGUI(hwndOutput, "resp");
    AppendOutputToGUI(hwndOutput, resp);

}
void connected(const char* comName) {
    json c;
    if (isConnected) {
        SetWindowText(hwndStatus, L"Status: Already Connected");
        UpdateWindow(hwndStatus);
        return;
    }

    // 1) פותחים פורט
    hConnectedPort = openingPort(comName);
    EnableWindow(hwndButtonRX, TRUE);



    if (hConnectedPort == INVALID_HANDLE_VALUE) {
        SetWindowText(hwndStatus, L"Status: Connection Failed (open)");
        UpdateWindow(hwndStatus);
        return;
    }

    // 2) מגדירים פרמטרי פורט (DCB/Timeouts)
    if (!setupPort(hConnectedPort)) {
        CloseHandle(hConnectedPort);
        hConnectedPort = INVALID_HANDLE_VALUE;
        SetWindowText(hwndStatus, L"Status: Connection Failed (setup)");
        UpdateWindow(hwndStatus);
        return;
    }
    PurgeComm(hConnectedPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
    c = openJason();

    // ריצה על כל הפקודות תחת init
    // לוקחים את המערך init1 בלבד
    auto& at = c["AT_Commands"]["init1"];

    for (const auto& item : at) {
        if (!item.contains("command")) {
            AppendOutputToGUI(hwndOutput, "[WARN] פריט בלי שדה 'command' דולג");
            continue;
        }

        std::string cmd = item["command"].get<std::string>();
        std::string resp = sendAT(hConnectedPort, cmd.c_str());

        // אם זו פקודה שמשנה מצב, ודאי READY
        if (cmd == "ATZ" || cmd == "AT+RUS=APPLY") {
            DWORD waitStart = GetTickCount();
            while (resp.find("+SYSNOTF:READY") == std::string::npos &&
                GetTickCount() - waitStart < 5000) {
                // עוד קריאה קצרה להשלמות
                COMSTAT st{}; DWORD errors = 0;
                ClearCommError(hConnectedPort, &errors, &st);
                if (st.cbInQue) {
                    char buf[256]; DWORD got = 0;
                    DWORD toRead = st.cbInQue < sizeof(buf) ? st.cbInQue : (DWORD)sizeof(buf);
                    if (ReadFile(hConnectedPort, buf, toRead, &got, NULL) && got)
                        resp.append(buf, buf + got);
                }
                if (resp.find("+SYSNOTF:READY") != std::string::npos) break;
                Sleep(20);
            }
            AppendOutputToGUI(hwndOutput, std::string("[RX] ") + resp);
        }

        // ריווח קטן בין פקודות (למודולים שצריכים לנשום)
        Sleep(80);
    }

    isConnected = true;
    connectedCOM = comName;
    SetWindowText(hwndStatus, L"Status: Connected");
    UpdateWindow(hwndStatus);


}
void returnLoRaPorts() {
    if (!flagRadio) {

        char** loraDevice = NULL;
        char output[4096] = ""; // מחרוזת לאגירת הפלט
        int count = 0;
        char devices[65536];
        DWORD charsReturned = QueryDosDeviceA(NULL, devices, sizeof(devices));
        if (charsReturned == 0) {
            printf("QueryDosDevice failed. Error: %lu\n", GetLastError());
            return;
        }

        char* ptr = devices;
        while (*ptr) {
            if (strncmp(ptr, "COM", 3) == 0) {
                char fullPortName[64];
                printf("%s\n", ptr);
                sprintf_s(fullPortName, "\\\\.\\%s", ptr);
                AppendOutputToGUI(hwndOutput, fullPortName);

                if (isLoRaDevice(fullPortName)) {

                    char** tmp = (char**)realloc(loraDevice, sizeof(char*) * (count + 1));
                    AppendOutputToGUI(hwndOutput, fullPortName);
                    if (!tmp) {
                        // שחרור זיכרון אם נכשל
                        for (int i = 0; i < count; i++) free(loraDevice[i]);
                        free(loraDevice);
                        return;
                    }
                    loraDevice = tmp;
                    strcat_s(output, fullPortName);
                    strcat_s(output, "\n");

                    // הקצאת מחרוזת חדשה
                    loraDevice[count] = (char*)malloc(strlen(fullPortName) + 1);
                    strcpy_s(loraDevice[count], strlen(fullPortName) + 1, fullPortName);

                    printf("LoRa device found on %s\n", fullPortName);
                    count++;


                    printf("LoRa device found on %s\n", ptr);
                }
            }
            ptr += strlen(ptr) + 1;
        }

        // מציג את כל הפורטים שנמצאו בתיבת הודעה
        if (strlen(output) == 0)
            strcpy_s(output, "No LoRa devices found.");
        for (int i = 0; i < count; i++) {
            wchar_t wPort[64];
            MultiByteToWideChar(CP_ACP, 0, loraDevice[i], -1, wPort, 64); // המרה ל־Unicode
            SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)wPort);     // הוספה ל־ComboBox
        }

        int i = 0;
        for (; i < count; i++) {
            free(loraDevice[i]);
        }
        free(loraDevice);
        loraDevice = NULL;
        flagRadio = true;
    }
    return;
}
int isLoRaDevice(const char* portName) {


    printf("%s", portName);
    HANDLE hPort = CreateFileA(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,      // אין שיתוף
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPort == INVALID_HANDLE_VALUE) {
        cout << "isLoRaDevice falied";
        return 0; // לא ניתן לפתוח את הפורט, כנראה תפוס או לא קיים
    }
    DWORD w = 0;
    WriteFile(hPort, "\r\n", 2, &w, NULL);
    Sleep(60);

    // שולחים פקודת AT לבדיקה
    const char* cmd = "AT\r\n";
    string respons;
    setupPort(hPort);
    PurgeComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
    respons = sendAT(hPort, cmd);
    if (respons.find("OK") != std::string::npos) {
        //if (respons.find("OK")|| respons.find("OK") != std::string::npos){

        AppendOutputToGUI(hwndOutput, "this LoRa device");
        //AppendOutputToGUI(hwndOutput, x);
        setupPort(hPort);
        CloseHandle(hPort);

        return 1;
    }

    else
    {
        return 0;
    }
}
bool CorrectsyntaxAT(const std::string& cmd) {
    // 1. פקודות בסיסיות: "AT" או "ATX" או "AT123"
    //static const std::regex re_basic("^AT(Z)?$");
    static const std::regex re_basic("^AT(Z)?\\s*$");


    // 2. פקודות עם + וללא פרמטרים: AT+NAME
    static const std::regex re_plus("^AT\\+[A-Z0-9]+\\s*$");

    // 3. פקודות עם ערך: AT+NAME=VALUE (ערך יכול להיות ספרות, HEX או אותיות)
    static const std::regex re_assign("^AT\\+[A-Z0-9]+=[A-Za-z0-9]+\\s*$");

    // 4. פקודות שאלה: AT+NAME? או AT+NAME=?
    static const std::regex re_query("^AT\\+[A-Z0-9]+(=)?\\?\\s*$");

    // 5. פקודות עם פסיקים (למשל יוניקסט/ברודקאסט)
    static const std::regex re_multi("^AT\\+[A-Z0-9]+=[A-Za-z0-9,]+\\s*$");

    return std::regex_match(cmd, re_basic) ||
        std::regex_match(cmd, re_plus) ||
        std::regex_match(cmd, re_assign) ||
        std::regex_match(cmd, re_query) ||
        std::regex_match(cmd, re_multi);
}
static void print_last_error(const char* where) {
    DWORD e = GetLastError();
    std::cerr << where << " failed. GetLastError=" << e << "\n";
}
