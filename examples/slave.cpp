#include "SoftwareLin.h"

#define RX_PIN (4)
#define TX_PIN (5)

#define LIN_AUTOBAUD
#define LIN_BAUD_MAX (20000)

SoftwareLin swLin(RX_PIN, TX_PIN);

void setup()
{
    Serial.begin(115200);

#ifdef LIN_AUTOBAUD    
    // swLIN.setAutoBaud() can detect and set the correct baud automatically.
    // Just set the initial baud rate to LIN_BAUD_MAX (20000)
    swLin.begin(LIN_BAUD_MAX);
#else
    swLin.begin(9600);
#endif
}

#define LIN_SLAVE
void loop()
{
    while (1) {
        const int frame_data_bytes = 5;

#ifdef LIN_AUTOBAUD
        uint8_t buf[2+frame_data_bytes]; // 2 bytes for PID and CHECKSUM. !!! The SYNC is consumed by swLin.setAutoBaud()
#else
        uint8_t buf[3+frame_data_bytes]; // 2 bytes for SYNC, PID and CHECKSUM.
#endif


        if (swLin.checkBreak()) {
            // sw_lin.checkBreak() blocks until UART ISR gives the semaphore.

#ifdef LIN_AUTOBAUD
            const uint32_t commonBaud[] = {110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200};
            uint32_t autobaud = swLin.setAutoBaud(commonBaud, sizeof(commonBaud)/sizeof(commonBaud[0]));
            if (autobaud) {
                Serial.printf("autobaud detection succeeded. Set baud = %lu\n\n", autobaud);
            }
            else {
                Serial.printf("autobaud detection failed. baud is not changed = %lu\n\n", swLin.baudRate());
            }
#else

#endif

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
