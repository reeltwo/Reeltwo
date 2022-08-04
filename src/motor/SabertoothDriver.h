/*
Arduino Library for SyRen/Sabertooth Packet Serial
Copyright (c) 2012-2013 Dimension Engineering LLC
http://www.dimensionengineering.com/arduino

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef SabertoothDriver_h
#define SabertoothDriver_h   

#include "ReelTwo.h"

/**
  * \ingroup drive
  *
  * \class Sabertooth
  *
  * \brief Controls a %Sabertooth or %SyRen motor driver running in Packet Serial mode.
  */
class SabertoothDriver
{
public:
    /*!
    Initializes a new instance of the Sabertooth class.
    The driver address is set to the value given, and the specified serial port is used.
    \param address The driver address.
    \param port    The port to use.
    */
    SabertoothDriver(byte address, Stream& port) :
        fAddress(address),
        fPort(&port)
    {
    }

    /*!
    Gets the driver address.
    \return The driver address.
    */
    inline byte address() const
    {
        return fAddress;
    }

    /*!
    Sets the driver address.
    \return The driver address.
    */
    inline void setAddress(byte addr)
    {
        fAddress = addr;
    }

    /*!
    Sends the autobaud character.
    \param dontWait If false, a delay is added to give the driver time to start up.
    */
    void autobaud(boolean dontWait = false) const
    {
        if (fPort != nullptr)
        {
            autobaud(*fPort, dontWait);
        }
    }

    /*!
    Sends the autobaud character.
    \param port     The port to use.
    \param dontWait If false, a delay is added to give the driver time to start up.
    */
    static void autobaud(Stream& port, boolean dontWait = false)
    {
        if (!dontWait)
        {
            delay(1500);
        }
        port.write(0xAA);
        port.flush();
        if (!dontWait)
        {
            delay(500);
        }
    }

    /*!
    Sends a packet serial command to the motor driver.
    \param command The number of the command.
    \param value   The command's value.
    */
    void command(byte command, byte value) const
    {
        if (fPort != nullptr)
        {
            // printf("CMD{%d}:%d:%d:%d\n", address(), command, value, (address() + command + value) & B01111111);
            fPort->write(address());
            fPort->write(command);
            fPort->write(value);
            fPort->write((address() + command + value) & B01111111);
        }
    }

    /*!
    Sets the power of motor 1.
    \param power The power, between -127 and 127.
    */
    void motor(int power) const
    {
        motor(1, power);
    }

    /*!
    Sets the power of the specified motor.
    \param motor The motor number, 1 or 2.
    \param power The power, between -127 and 127.
    */
    void motor(byte motor, int power) const
    {
        if (motor >= 1 && motor <= 2)
        {
            throttleCommand((motor == 2 ? 4 : 0) + (power < 0 ? 1 : 0), power);
        }
    }

    /*!
    Sets the driving power.
    \param power The power, between -127 and 127.
    */
    void drive(int power) const
    {
        throttleCommand(power < 0 ? 9 : 8, power);
    }

    /*!
    Sets the turning power.
    \param power The power, between -127 and 127.
    */
    void turn(int power) const
    {
        throttleCommand(power < 0 ? 11 : 10, power);
    }

    /*!
    Stops.
    */
    void stop() const
    {
        motor(1, 0);
        motor(2, 0);
    }

    /*!
    Sets the minimum voltage.
    \param value The voltage. The units of this value are driver-specific and are specified in the Packet Serial chapter of the driver's user manual.
    */
    void setMinVoltage(byte value) const
    {
        command(2, (byte)min((int)value, 120));
    }

    /*!
    Sets the maximum voltage.
    Maximum voltage is stored in EEPROM, so changes persist between power cycles.
    \param value The voltage. The units of this value are driver-specific and are specified in the Packet Serial chapter of the driver's user manual.
    */
    void setMaxVoltage(byte value) const
    {
        command(3, (byte)min((int)value, 127));
    }

    /*!
    Sets the baud rate.
    Baud rate is stored in EEPROM, so changes persist between power cycles.
    \param baudRate The baud rate. This can be 2400, 9600, 19200, 38400, or on some drivers 115200.
    */
    void setBaudRate(long baudRate) const
    {
    #if defined(ARDUINO) && ARDUINO >= 100
        fPort->flush();
    #endif

        byte value;
        switch (baudRate)
        {
            case 2400:
                value = 1;
                break;
            default:
            case 9600:
                value = 2;
                break;
            case 19200:
                value = 3;
                break;
            case 38400:
                value = 4;
                break;
            case 115200:
                value = 5;
                break;
        }
        command(15, value);

    #if defined(ARDUINO) && ARDUINO >= 100
        fPort->flush();
    #endif

        // (1) flush() does not seem to wait until transmission is complete.
        //     As a result, a Serial.end() directly after this appears to
        //     not always transmit completely. So, we manually add a delay.
        // (2) Sabertooth takes about 200 ms after setting the baud rate to
        //     respond to commands again (it restarts).
        // So, this 500 ms delay should deal with this.
        delay(500);
    }

    /*!
    Sets the deadband.
    Deadband is stored in EEPROM, so changes persist between power cycles.
    \param value The deadband value.
                 Motor powers in the range [-deadband, deadband] will be considered in the deadband, and will
                 not prevent the driver from entering nor cause the driver to leave an idle brake state.
                 0 resets to the default, which is 3.
    */
    void setDeadband(byte value) const
    {
        command(17, (byte)min(int(value), 127));
    }

    /*!
    Sets the ramping.
    Ramping is stored in EEPROM, so changes persist between power cycles.
    \param value The ramping value. Consult the user manual for possible values.
    */
    void setRamping(byte value) const
    {
        command(16, (byte)min(int(value), 80));
    }

    /*!
    Sets the serial timeout.
    \param milliseconds The maximum time in milliseconds between packets. If this time is exceeded,
                        the driver will stop the motors. This value is rounded up to the nearest 100 milliseconds.
                        This library assumes the command value is in units of 100 milliseconds. This is true for
                        most drivers, but not all. Check the packet serial chapter of the driver's user manual
                        to make sure.
    */
    void setTimeout(int milliseconds) const
    {
        command(14, (byte)((constrain(milliseconds, 0, 12700) + 99) / 100));
    }

private:
    void throttleCommand(byte command, int power) const
    {
        power = constrain(power, -126, 126);
        this->command(command, (byte)abs(power));
    }

private:
    byte fAddress;
    Stream* fPort = nullptr; 
};

#endif
