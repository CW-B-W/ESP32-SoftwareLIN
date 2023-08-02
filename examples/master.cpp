#include "SoftwareLin.h"

#define RX_PIN (4)
#define TX_PIN (5)

SoftwareLin swLin(RX_PIN, TX_PIN);

void setup()
{
    Serial.begin(115200);
    swLin.begin(9600);
}

void loop()
{
    while (1) {
        const int frame_data_bytes = 5;
        swLin.sendBreak();
        const uint8_t buf[3+frame_data_bytes] = {0x55, 0x02, 'h', 'e', 'l', 'l', 'o', 0xA7};
        // 3 bytes for SYNC, PID and CHECKSUM
        swLin.write((const uint8_t*)buf, sizeof(buf));
        swLin.endFrame();
        delay(10);
    }
}
