#include "SoftwareLin.h"

#define RX_PIN (16)
#define TX_PIN (17)

SoftwareLin swLin(RX_PIN, TX_PIN);

void setup()
{
    Serial.begin(115200);
    swLin.begin(9600);
}

#define LIN_MASTER
// #define LIN_SLAVE
void loop()
{
    while (1) {
#ifdef LIN_MASTER
        swLin.sendBreak();
        const uint8_t data[] = {0x55, 0x00};
        swLin.write((const uint8_t*)data, sizeof(data));
        swLin.endFrame();
        delay(10);
#endif

#ifdef LIN_SLAVE
        uint8_t buf[8];
        if (swLin.checkBreak()) {
            const int data_bytes = 2;
            int bytes_read = 0;
            while (bytes_read < data_bytes) {
                bytes_read += swLin.read(buf + bytes_read, data_bytes - bytes_read);
            }
            swLin.endFrame();
            for (int i = 0; i < data_bytes; ++i) {
                Serial.printf("0x%02X ", buf[i]);
            }
            Serial.printf("\n\n");
        }
        delay(10);
#endif
    }
}
