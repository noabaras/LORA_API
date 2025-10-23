#include "jason.h"
//���� �� ������
json openJason() {
    try {
        std::ifstream f("config.json");

        if (!f.is_open()) {
            AppendOutputToGUI(hwndOutput, "[ERR] cannot open config.json");
            return json(); // ����� JSON ��� ����� return ���
        }

        json cfg = json::parse(f);
        return cfg; // ����� �� �-JSON ������
    }
    catch (const std::exception& ex) {
        AppendOutputToGUI(hwndOutput, std::string("[EXCEPTION] ") + ex.what());
        SetWindowText(hwndStatus, L"Status: Connection Failed (JSON)");
        UpdateWindow(hwndStatus);
        return json(); // ����� ����� � ����� JSON ���
    }
}
static json load_json_file(const std::wstring& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        AppendOutputToGUI(hwndOutput, "[ERR] Cannot open config file");
        throw std::runtime_error("config open failed");
    }
    return json::parse(f, nullptr, true, true); // ignore_comments=true
}