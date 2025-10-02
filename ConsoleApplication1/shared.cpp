#include "shared.h"
void AppendOutputToGUI(HWND hwndEdit, const std::string& s)
{
    if (!hwndEdit) return;
    // ���� �-wstring ���� ������ (�� �� ��� SafeStringForGUI)
    std::wstring w = SafeStringForGUI(s);

    // ����� CRLF �� ���
    if (w.empty() || (w.size() >= 1 && w.back() != L'\n')) w += L"\r\n";

    // ����� ���� �-EDIT
    int len = GetWindowTextLengthW(hwndEdit);
    SendMessage(hwndEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)w.c_str());
}
std::wstring SafeStringForGUI(const std::string& str)
{
    std::wstring wstr;
    for (unsigned char c : str)
    {
        if (c >= 32 && c <= 126) // ����� ������
            wstr += static_cast<wchar_t>(c);
        else
            wstr += L'.'; // �� ������ �� �� ����
    }
    return wstr;
}
int divCeil(int a, int b) {
    return (a + b - 1) / b;
}// ������ ����� �� ��� ������� ��� �� ����� ���� �1