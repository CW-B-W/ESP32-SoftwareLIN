/*
SoftwareLin.cpp - Implementation of the software LIN bus emulation for ESP32.
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

#include "SoftwareLin.h"

SoftwareLin::SoftwareLin(int8_t rxPin, int8_t txPin) : BasicUART<GpioCapabilities>(rxPin, txPin, false)
{
    m_inFrame = false;
}

void IRAM_ATTR SoftwareLin::sendBreak(int breakBits, int delimiterBits)
{
    assert(false == m_inFrame);

    EspSoftwareSerial::UART::writePeriod(0, breakBits * m_bitTicks, false);
    EspSoftwareSerial::UART::writePeriod(delimiterBits * m_bitTicks, 0, true);

    m_inFrame = true;
}

bool IRAM_ATTR SoftwareLin::checkBreak()
{
    assert(false == m_inFrame);

    bool breakDetected = false;
    while (m_isrBuffer->available()) {
        // This section is copied from `void UARTBase::rxBits(const uint32_t isrTick)`
        uint32_t isrTick = m_isrBuffer->pop();
        const bool level = (m_isrLastTick & 1) ^ m_invert; // level in the duration of `m_isrLastTick -> isrTick`
        uint32_t ticks = isrTick - m_isrLastTick;
        m_isrLastTick = isrTick;
        uint32_t bits = ticks / m_bitTicks;
        if (ticks % m_bitTicks > (m_bitTicks >> 1)) ++bits;

        if (level) {
            // `m_isrLastTick -> isrTick` is high.
            // At isrTick, it transitted to low.
            // These should be all stopbits, ignore them.
            // DO NOTHING
        }
        else {
            // `m_isrLastTick -> isrTick` is low (break field).
            // At isrTick, it transitted to high (break delimiter).
            // This is the breaking field.

            // break field should be at least 13 bits
            if(bits >= 13) {
                breakDetected = true;
                break;
            }
        }
    }

    if (breakDetected) {
        m_inFrame = true;
    }

    return breakDetected;
}

uint32_t IRAM_ATTR SoftwareLin::setAutoBaud(const uint32_t commonBaud[], int commonBaudSize)
{
    assert(true == m_inFrame);

    uint32_t m_bitTicks_bak = m_bitTicks;
    
    m_bitTicks = (microsToTicks(1000000UL) + 1 / 2) / 1;
    // This is to satisfy the assertion in detectBaud();
    
    uint32_t detectedBaud = detectBaud();
    for (int i = 0; i < 3; ++i) {
        // After detectBaud() finished, there should be totally
        // 3 rising/falling edges of the SYNC byte left in the buffer.
        // Popping the rest rising/falling edges of the SYNC bytes.

        while (!m_isrBuffer->available()) {
            ; // wait until can pop
        }
        uint32_t isrTick = m_isrBuffer->pop();
        const bool level = (m_isrLastTick & 1) ^ m_invert;
        m_isrLastTick = isrTick;
    }
    if (detectedBaud == 0) {
        m_bitTicks = m_bitTicks_bak;
        return 0;
    }

    int32_t diff_min = 0x7FFFFFFFUL;
    uint32_t baud = 0;
    for (int i = 0; i < commonBaudSize; ++i) {
        int32_t diff = abs((int32_t)commonBaud[i] - (int32_t)detectedBaud);
        if (diff < diff_min) {
            diff_min = diff;
            baud = commonBaud[i];
        }
    }

    const uint32_t LIN_BAUD_MAX = 20000;
    if (baud > LIN_BAUD_MAX) {
        m_bitTicks = m_bitTicks_bak;
        return 0;
    }

    // Set baud rate to the detected baud rate
    m_bitTicks = (microsToTicks(1000000UL) + baud / 2) / baud;

    return baud;    
}

void SoftwareLin::endFrame()
{
    m_inFrame = false;
}

int SoftwareLin::read(uint8_t* buffer, size_t size)
{
    assert(true == m_inFrame);

    return UART::read(buffer, size);
}

size_t SoftwareLin::write(const uint8_t* buffer, size_t size)
{
    assert(true == m_inFrame);

    return UART::write(buffer, size);
}
