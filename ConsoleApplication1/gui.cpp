#include "Globals.h"
#include <windows.h>
#include "gui.h"
#define BUFFER_SIZE 256
#define INITIAL_SIZE 10
#define IDC_EDIT_PAYLOAD  1001
#define IDC_BTN_SEND      1002
#define MCU 1003


wchar_t gSelectedCOM[64] = L"";  // שם ה-COM שנבחר

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDC_BTN_SEND: {
            int len = GetWindowTextLengthW(hwndEditPayload);//מחזיר את מס התווים של הטקסט שהמשתמש הקליד
            std::wstring wtext;
            wtext.resize(len + 1); //הקצאת מקום במחרוזת        
            GetWindowTextW(hwndEditPayload, &wtext[0], len + 1);  // &wtext[0] = LPWSTR
            wtext.resize(wcslen(wtext.c_str()));                  // לגזור את ה־NUL

            std::string utf8 = WStringToUtf8(wtext);
            std::string hex = BytesToHex(utf8);//המרה להקסא
           // TX(hex);
            TX(utf8);
            break;
        }

        case 2: // כפתור COM
            returnLoRaPorts();
            ShowWindow(hwndComboBox, SW_SHOW);
            SendMessage(hwndComboBox, CB_SHOWDROPDOWN, TRUE, 0);
            break;

        case 3: // בחירת COM
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                int sel = SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0);
                if (sel != CB_ERR) {
                    SendMessage(hwndComboBox, CB_GETLBTEXT, sel, (LPARAM)gSelectedCOM);
                    SetWindowText(hwndButtonCOM, gSelectedCOM);
                    ShowWindow(hwndComboBox, SW_HIDE);
                    SetWindowText(hwndStatus, L"Status: COM selected");
                }

                if (!hwndButtonConnect) {
                    hwndButtonConnect = CreateWindow(
                        L"BUTTON", L"Connect",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        200, 120, 100, 30,
                        hwnd, (HMENU)4,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                }

                if (!hwndButtonDisconnect) {
                    hwndButtonDisconnect = CreateWindow(
                        L"BUTTON", L"Disconnect",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        310, 120, 100, 30,
                        hwnd, (HMENU)5,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                }

                if (!hwndButtonRX) {
                    hwndButtonRX = CreateWindow(
                        L"BUTTON", L"RX",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        10, 10, 100, 30,
                        hwnd, (HMENU)7,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                    EnableWindow(hwndButtonRX, FALSE);

                }
                if (!hwndbuttonMCU) {
                    hwndbuttonMCU = CreateWindow(
                        L"BUTTON", L"MCU",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        60, 60, 30, 40,
                        hwnd, (HMENU)MCU,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                }
                // יצירת תיבת קלט למטען (Payload)
                if (!hwndEditPayload) {
                    hwndEditPayload = CreateWindowEx(
                        WS_EX_CLIENTEDGE, L"EDIT", L"",
                        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
                        20, 160, 390, 24,
                        hwnd, (HMENU)IDC_EDIT_PAYLOAD,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                }
                // יצירת כפתור Send
                if (!hwndButtonSend) {
                    hwndButtonSend = CreateWindow(
                        L"BUTTON", L"Send",
                        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                        420, 160, 90, 24,
                        hwnd, (HMENU)IDC_BTN_SEND,
                        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                        NULL
                    );
                }

            }
            break;

        case 4: { // Connect
            char selectedCOM_char[64];
            size_t converted = 0;
            wcstombs_s(&converted, selectedCOM_char, sizeof(selectedCOM_char), gSelectedCOM, _TRUNCATE);
            connected(selectedCOM_char);
        }
              break;

        case 5: { // Disconnect
            char selectedCOM_char[64];
            size_t converted = 0;
            wcstombs_s(&converted, selectedCOM_char, sizeof(selectedCOM_char), gSelectedCOM, _TRUNCATE);
            disconnected(selectedCOM_char);
            SetWindowText(hwndStatus, L"Status: Disconnected");
        }
              break;

        case 6: // TX
            // TODO: להוסיף לוגיקה ל-TX
            break;
        case MCU: {
            char selectedCOM_char[64];
            size_t converted = 0;
            wcstombs_s(&converted, selectedCOM_char, sizeof(selectedCOM_char), gSelectedCOM, _TRUNCATE);
            resetMCU_like_WE(selectedCOM_char);
        }
                break;
        case 7: { // RX
            
            char selectedCOM_char[64];
            size_t converted = 0;
            wcstombs_s(&converted, selectedCOM_char, sizeof(selectedCOM_char), gSelectedCOM, _TRUNCATE);
            RX(selectedCOM_char);
        }
              break;
        }
    }
                   break; // <-- סוגר את case WM_COMMAND כמו שצריך

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        const wchar_t text[] = L"";
        TextOut(hdc, 50, 50, text, wcslen(text));
        EndPaint(hwnd, &ps);
    }
                 return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SimpleWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // רקע לבן

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"My Simple Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600,600,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;
    // יצירת כפתור הCOM
     hwndButtonCOM = CreateWindow(
        L"BUTTON",          // סוג חלון: כפתור
        L"COM",             // טקסט הכפתור
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 120, 100, 30,  // מיקום וגודל הכפתור
        hwnd,               // חלון אב
        (HMENU)2,           // מזהה הכפתור
        hInstance,
        NULL
    );
    // רשימת הCOM 
    hwndComboBox = CreateWindow(
        L"COMBOBOX",           // סוג חלון: רשימה נפתחת
        NULL,                  // אין טקסט ראשוני
        WS_CHILD  | CBS_DROPDOWNLIST,
        30, 20, 150, 200,    // מיקום וגודל
        hwnd,                  // חלון אב
        (HMENU)3,              // מזהה ה־ComboBox
        hInstance,
        NULL
    );
  
    hwndStatus = CreateWindow(
        L"STATIC",              // סוג חלון סטטי (label)
        L"Status: Disconnected", // טקסט התחלתי
        WS_CHILD | WS_VISIBLE,
        50, 200, 200, 30,      // מיקום וגודל
        hwnd,                   // חלון אב
        NULL,                   // אין ID
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        NULL
    );
   
    hwndOutput = CreateWindow(
        L"EDIT",                 // סוג חלון: Edit
        NULL,                     // תוכן התחלתי
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        50, 250, 500, 300,       // מיקום וגודל
        hwnd,                     // חלון אב
        NULL,                     // אין ID
        hInstance,
        NULL
    );
     hwndOutput2 = CreateWindow(
        L"EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        50, 560, 500, 300,   // אותו X, Y=560 מתחת לחלון הראשון
        hwnd,
        NULL,
        hInstance,
        NULL
    );


    
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}