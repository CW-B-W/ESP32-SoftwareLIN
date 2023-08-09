# ESP32-SoftwareLIN
[Doxygen docs](https://cw-b-w.github.io/ESP32-SoftwareLIN/)
# Introduction
This project is to make ESP32 support LIN (Local Interconnect Network) bus serial.  
ESP32 doesn't have a dedicated hardware for LIN bus communication, therefore we have to use software emulation with GPIO pins to emulate the LIN communication.

## Why can't we use UART to do LIN communication?
In LIN bus, the break field should consist of at least 13 dominant bits (zeros) and at least 1 recessive bits (ones). (Refer to the figure below)  
But for UART, sending/receiving 13 consecutive zeros is considered error.  
Therefore, generally for LIN bus communication we will need dedicated hardware in MCU to support it.  
Refer to [Texas Instruments - LIN Protocol and Physical Layer Requirements](https://www.ti.com/lit/an/slla383a/slla383a.pdf)

<img src="https://github.com/CW-B-W/ESP32-SoftwareLIN/assets/76680670/19d90946-3f73-4156-8f06-243790baa16e" style="width:75%;"/>  
(The signal is analyzed with [EUA/ESP32_LogicAnalyzer](https://github.com/EUA/ESP32_LogicAnalyzer))

## Common implementations and their limitation
For LIN master to send a break field, a smart trick is:  
When sending the break field, they halve the baud rate, and send 0x00.  
In this way, the receiver, whose baud rate is not halved, actually receives 18 dominant bits (zeros) and 2 recessive bits (stop bits).  
For example the implementation of [mestrode/Lin-Interface-Library](https://github.com/mestrode/Lin-Interface-Library/blob/76f9d4c31d0e90f9c053faf287fa81837a453ba9/src/Lin_Interface.cpp#L229)  

This method should work fine for LIN master.  
But if we use the same trick to implement LIN slave, the LIN slave cannot be flexible enough to receive all possible combinations of break field.  
For example, it cannot detect break field with 13 dominant bits and 1 recessive bit.  

Therefore, to make ESP32 able to be a LIN slave, this project emerges.

## This project
Based on [espsoftwareserial](https://github.com/plerup/espsoftwareserial), which is the software emulation of UART with GPIO pins, we add `break field` detection and `auto baud` for LIN bus.  

***espsoftwareserial*** uses GPIO pins to emulate UART. At the rising edge and falling edge of the GPIO pins, it triggers interrupt, and the ISR records the timestamp and the rising / falling edge of this interrupt.  

Making use of the timestamp and rising / falling edge information recorded by ***espsoftwareserial***, we can check whether the `break field` is received, no matter how many bits the `break field` contains.  

Using the same idea, `auto baud` is also implemented in this project.

### Features
* Send/Detect break field of LIN bus
* Autobaud detection and set

# Quick start guide
1. Use VS Code with extension PlatformIO to open the project folder
2. Build and upload the ESP32 firmware with PlatformIO
    * If this device is acting as a LIN master, copy the `examples/master.cpp` to `src/`
    * If this device is acting as a LIN slave, copy the `examples/slave.cpp` to `src/`
3. Connect the UART pins of the ESP32 LIN master and ESP32 LIN slave

# Usage example
## For LIN master
Only two functions need to be used: `sendBreak()` and `endFrame()`.  
`sendBreak()` is used to send the break field to the LIN bus.  
`endFrame()` is to notify the `SoftwareLin` the frame has ended, and it resets the internal value of `SoftwareLin`
```C++
SoftwareLin swLin(RX_PIN, TX_PIN);
swLin.begin(baud);
swLin.sendBreak();
.
. // Send data to the bus
.
swLin.endFrame();
```

## For LIN slave

Only three functions need to be used: `checkBreak()`, `setAutoBaud()` and `endFrame()`.  
`sendBreak()` is used to check whether the break field has been detected  
`setAutoBaud()` is used to automatically detect and set baud rate  
`endFrame()` is to notify the `SoftwareLin` the frame has ended, and it resets the internal value of `SoftwareLin`
```C++
SoftwareLin swLin(RX_PIN, TX_PIN);
#define LIN_BAUD_MAX (20000)
swLin.begin(LIN_BAUD_MAX);
if (swLin.checkBreak()) {
    const uint32_t commonBaud[3] = {9600, 14400, 19200};
    uint32_t autobaud = swLin.setAutoBaud(commonBaud, 3);
    if (autobaud > 0) {
        // baud rate is set to autobaud
    }
    else {
        // autobaud detection failed
    }
    .
    . // Receive from the bus
    . // !!! Note that the SYNC byte was consumed by setAutoBaud()
    .
    swLin.endFrame();
}
```

# Testing
Refer to [CW-B-W/ESP32-openLIN#testing](https://github.com/CW-B-W/ESP32-openLIN/tree/master#testing)
