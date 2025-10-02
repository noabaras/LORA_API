#include "Globals.h"

HWND hwndComboBox = nullptr;
HWND hwndButtonCOM = nullptr;
HWND hwndButtonConnect = nullptr;
HWND hwndButtonDisconnect = nullptr;
HWND hwndStatus = nullptr;
HWND hwndOutput = nullptr;
HWND hwndRadioTx = nullptr;
HWND hwndRadioRx = nullptr;
HWND hwndButtonRX = nullptr;
HWND hwndEditPayload = nullptr;
HWND hwndButtonSend = nullptr;
HWND hwndbuttonMCU = nullptr;
HWND hwndOutput2 = nullptr;
HANDLE      hConnectedPort = INVALID_HANDLE_VALUE;
bool        isConnected = false;
bool        flagRadio = false;
std::string connectedCOM;
