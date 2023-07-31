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
    SoftwareLin(int8_t rxPin, int8_t txPin);

    void sendBreak(int breakBits = 13, int delimiterBits = 1);
    bool checkBreak();
    
    void endFrame();

    int read(uint8_t* buffer, size_t size);
    size_t write(const uint8_t* buffer, size_t size) override;

protected:
    bool m_inFrame;
};

#endif