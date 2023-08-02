#include "SoftwareLin.h"

#define RX_PIN (4)
#define TX_PIN (5)

SoftwareLin swLin(RX_PIN, TX_PIN);

void setup()
{
    Serial.begin(115200);
    swLin.begin(9600);
}

#define LIN_SLAVE
void loop()
{
    while (1) {
        const int frame_data_bytes = 5;
        uint8_t buf[3+frame_data_bytes]; // 3 bytes for SYNC, PID and CHECKSUM
        if (swLin.checkBreak()) {
            const int read_timeout = 100000; // 100ms timeout
            int start_time = micros();

            int bytes_to_read = sizeof(buf) / sizeof(buf[0]);
            int bytes_read = 0;
            while (bytes_read < bytes_to_read && micros() - start_time <= read_timeout) {
                bytes_read += swLin.read(buf + bytes_read, bytes_to_read - bytes_read);
                delay(0); // yield for other tasks
            }
            swLin.endFrame();

            if (bytes_read < bytes_to_read) {
                Serial.printf("Timeout: only %d bytes is read\n", bytes_read);
            }
            for (int i = 0; i < bytes_to_read; ++i) {
                Serial.printf("0x%02X ", buf[i]);
            }
            Serial.printf("\n\n");
        }
    }
}