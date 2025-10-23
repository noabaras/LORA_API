#include "crc.h"
uint8_t calcCRC8(const std::vector<uint8_t>& data) {
    uint8_t crc = 0x00;
    for (uint8_t b : data) {
        crc ^= b;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07; // τεμιπεν
            else
                crc <<= 1;
        }
    }
    AppendOutputToGUI(hwndOutput, "crc" + crc);
    return crc;
}