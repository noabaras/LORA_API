#include "cobs.h"
std::string decodeCobs(const std::string& cobs) {
    std::string decoded;
    size_t i = 0;

    AppendOutputToGUI(hwndOutput, std::string("paket ") + cobs);
    AppendOutputToGUI(hwndOutput, "[decodeCobs] start, len=" + std::to_string(cobs.length()));

    auto is_hex_pair = [&](size_t pos) {
        return pos + 2 <= cobs.length()
            && std::isxdigit((unsigned char)cobs[pos])
            && std::isxdigit((unsigned char)cobs[pos + 1]);
    };

    while (i + 2 <= cobs.length()) {
        if (!is_hex_pair(i)) {
            AppendOutputToGUI(hwndOutput, "[decodeCobs] bad code hex at i=" + std::to_string(i));
            break;
        }
        uint8_t code = hexToByte(cobs.substr(i, 2));
        AppendOutputToGUI(hwndOutput, "[decodeCobs] i=" + std::to_string(i) +
            " code=" + std::to_string(code));

        if (code == 0) { // 0x00 � ������� ��� ����
            AppendOutputToGUI(hwndOutput, "[decodeCobs] end-of-packet (code=0)");
            break;
        }
        i += 2; // ����� �� ����� �� code

        // ����� �� ������ ���� code-1 ���� (�� ���=2 ���� ����)
        size_t need_chars = (size_t)(code - 1) * 2;
        if (i + need_chars > cobs.length()) {
            AppendOutputToGUI(hwndOutput, "[decodeCobs] not enough hex for block, need=" +
                std::to_string(need_chars) + " have=" +
                std::to_string(cobs.length() - i));
            break; // �� ����� �����
        }

        // ����� code-1 ���� (��2 ����� ��� ���)
        for (int j = 1; j < code; ++j) {
            decoded += cobs[i];
            decoded += cobs[i + 1];
            i += 2;
        }

        // ����� ��� ����� �� �� ��� ������� ��� ��� ������ ����
        if (code < 0xFF && i < cobs.length()) {
            decoded += "00";
        }
    }
    

    AppendOutputToGUI(hwndOutput, "decod" + decoded);
    return decoded;
}
std::string crateCobs(std::string hexpaket) {
    AppendOutputToGUI(hwndOutput, "create cobs" + hexpaket);
    int count = 1; // ���� �� �� ��counter ����
    std::string cobs = "00"; // placeholder �����
    size_t placeholder = 0;

    for (int i = 0; i < hexpaket.length(); i += 2) {
        if (hexpaket[i] == '0' && hexpaket[i + 1] == '0') {
            cobs.replace(placeholder, 2, intToHexByte(count));
            cobs += "00";
            placeholder = cobs.size() - 2;
            count = 1;
            continue; // �� ������� �� "00" �����
        }


        cobs += hexpaket[i];
        cobs += hexpaket[i + 1];
        count++;
    }
    cobs.replace(placeholder, 2, intToHexByte(count));


    cobs += "00";

    return cobs;
}