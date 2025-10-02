#pragma once
#include <windows.h>
#include <string>

extern HWND hwndComboBox;
extern HWND hwndButtonCOM;
extern HWND hwndButtonConnect;
extern HWND hwndButtonDisconnect;
extern HWND hwndStatus;
extern HWND hwndOutput;
extern HWND hwndRadioTx;
extern HWND hwndRadioRx;
extern HWND hwndButtonRX;
extern HWND hwndEditPayload;
extern HWND hwndButtonSend;
extern HWND hwndbuttonMCU;
extern  HWND hwndOutput2;

extern HANDLE      hConnectedPort;
extern bool        isConnected;
extern bool        flagRadio;
extern std::string connectedCOM;
