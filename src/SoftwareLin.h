/*
SoftwareLin.h - Implementation of the software LIN bus emulation for ESP32.
Copyright (c) 2023 CW-B-W All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef __SoftwareLin_h
#define __SoftwareLin_h

#include "espsoftwareserial/src/SoftwareSerial.h"

class SoftwareLin : public EspSoftwareSerial::UART
{
public:
    /**
     * @brief Construct a new Software Lin object
     * 
     * @param rxPin The designated GPIO pin number of Rx pin
     * @param txPin The designated GPIO pin number of Tx pin
     */
    SoftwareLin(int8_t rxPin, int8_t txPin);

    /**
     * @brief Send Break Field and Break Delimiter to the bus
     * 
     * @param breakBits Number of dominant bits in the break field
     * @param delimiterBits Number of recessive bits in the break delimiter
     */
    void sendBreak(int breakBits = 13, int delimiterBits = 1);

    /**
     * @brief Check whether there is Break Field sent on the bus
     * 
     * @return Whether the Break Field has been detected
     */
    bool checkBreak();

    /**
     * @brief
     * setAutoBaud() is used for automatically detect and 
     * set baud rate after the break field has been detected. 
     * When setAutoBaud() is called, it's assumed that the Rx pin 
     * is receiving a SYNC byte (0x55). setAutoBaud() will detect 
     * the baud rate with SYNC byte and set the baud rate to 
     * the closest one in commonBaud[].
     * 
     * @param commonBaud commonBaud The array of common baud rates. 
     * When baud rate is detected, setAutoBaud() will set the 
     * baud rate to the closest one in commandBaud[]
     * @param commonBaudSize commonBaudSize The size of commonBaud[]
     * @return (uint32_t) The baud rate set by setAutoBaud(). 
     * Return 0 if setAutoBaud() failed, and the baud rate will
     * not be changed.
     */
    uint32_t setAutoBaud(const uint32_t commonBaud[], int commonBaudSize);
    
    /**
     * @brief Notify SoftwareLin the LIN frame has ended.
     * This is for SoftwareLin to reset the internal state
     * registers and prepare for next frame.
     * 
     */
    void endFrame();

    /**
     * @brief Read bytes to buffer
     * 
     * @param buffer The buffer for the data read
     * @param size The number of bytes to read
     * @return (int) The number of that has been successfully read
     */
    int read(uint8_t* buffer, size_t size);

    /**
     * @brief Write bytes to the bus
     * 
     * @param buffer The buffer for the data write
     * @param size The number of bytes to write
     * @return (size_t) The number of that has been successfully write
     */
    size_t write(const uint8_t* buffer, size_t size) override;

protected:
    /**
     * @brief Indicate whether SoftwareLin is still processing a frame.
     * Initially, m_inFrame is false.
     * After sendBreak() is executed or detectBreak() is true,
     * m_inFrame is set to true.
     * Only when m_inFrame is true, we can execute read()/write(),
     * otherwise the assertion should failed.
     * When the LIN frame has ended, endFrame() should be executed to
     * reset m_inFrame to false, so that the next LIN frame can be processed.
     */
    bool m_inFrame;
};

#endif