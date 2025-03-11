/*
Arduino Library for Cytron SmartDriveDuo

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
#ifndef CytronSmartDriveDuoDriver_h
#define CytronSmartDriveDuoDriver_h

#include "ReelTwo.h"

#define MDDS10 0x55
#define MDDS30 0x80
#define MDDS60 0x55

#ifdef USE_DRIVER_DEBUG
#define DRIVER_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define DRIVER_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define DRIVER_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define DRIVER_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#define DRIVER_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__)
#else
#define DRIVER_DEBUG_PRINT(s)
#define DRIVER_DEBUG_PRINTLN(s)
#define DRIVER_DEBUG_PRINT_HEX(s)
#define DRIVER_DEBUG_PRINTLN_HEX(s)
#define DRIVER_DEBUG_PRINTF(...)
#endif

class CytronSmartDriveDuoDriver
{
public:
    /*!
    Initializes a new instance of the Sabertooth class.
    The driver address is set to the value given, and the specified serial port is used.
    \param address The driver address.
    \param port    The port to use.
    */
    CytronSmartDriveDuoDriver(byte address, Stream& port, uint8_t initialByte = 0x80) :
        fPort(&port),
        fAddress(address & 0x7),
        fInitialByte(initialByte)
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
        fAddress = (addr & 0x7);
    }

    /*!
    Sends the autobaud character.
    \param dontWait If false, a delay is added to give the driver time to start up.
    */
    void autobaud(boolean dontWait = false) const
    {
        if (fPort != nullptr)
        {
	        if (!dontWait)
	        {
	            delay(1000);
	        }
	        fPort->write(&fInitialByte, 1);
	        fPort->flush();
	        if (!dontWait)
	        {
	            delay(500);
	        }
        }
    }

    /*!
    Sets the turning power.
    \param power The power, between -127 and 127.
    */
    void turn(int power)
    {
        fTurnPower = power;
    }

    /*!
    Sets the driving power.
    \param power The power, between -127 and 127.
    */
    void drive(int power)
    {
		int turn = map(fTurnPower, -127, 127, -255, 255);
		int throttle = map(power, -127, 127, -255, 255);
		int left = throttle + turn;
		int right = throttle - turn;
		float scaleLeft = abs(left/255.0);
		float scaleRight = abs(right/255.0);
		float scaleMax = max(scaleLeft, scaleRight);
		scaleMax = max(1.0f, scaleMax);
		left = int(constrain(left/scaleMax, -255, 255));
  		right = int(constrain(right/scaleMax, -255, 255));
  		motor(map(left, -255, 255, -127, 127),
  				map(right, -255, 255, -127, 127));
    }

    void motor(int power)
    {
    	const uint8_t headerByte = 0x55;

		// Left motor
		uint8_t addressByte = fAddress;
		uint8_t commandByte = map(power, -127, 127, 0, 255);
		uint8_t checksum = headerByte + addressByte + commandByte;
		fPort->write(&headerByte, 1);
		fPort->write(&addressByte, 1);
		fPort->write(&commandByte, 1);
		fPort->write(&checksum, 1);
    }

    /*!
    Independent motor command.
    \param power The power, between -127 and 127.
    */
    void motor(int powerLeft, int powerRight)
    {
        DRIVER_DEBUG_PRINTF("MOTOR{%d}:%d:%d\n", address(), powerLeft, powerRight);
    	const uint8_t headerByte = 0x55;

		// Left motor
		uint8_t addressByte = fAddress;
		uint8_t commandByte = map(powerLeft, -127, 127, 0, 255);
		uint8_t checksum = headerByte + addressByte + commandByte;
		DRIVER_DEBUG_PRINTF("  LEFT: address=%02X cmd=%02X checksum=%02X\n", addressByte, commandByte, checksum);
		fPort->write(&headerByte, 1);
		fPort->write(&addressByte, 1);
		fPort->write(&commandByte, 1);
		fPort->write(&checksum, 1);

		// Right motor
		addressByte = (fAddress | 0x8);
		commandByte = map(powerRight, -127, 127, 0, 255);
		checksum = headerByte + addressByte + commandByte;
		DRIVER_DEBUG_PRINTF("  RIGHT: address=%02X cmd=%02X checksum=%02X\n", addressByte, commandByte, checksum);
		fPort->write(&headerByte, 1);
		fPort->write(&addressByte, 1);
		fPort->write(&commandByte, 1);
		fPort->write(&checksum, 1);

  		fTurnPower = 0;
  	}

  	void stop()
  	{
  		motor(0, 0);
  	}

  	// Not supported
    void setDeadband(byte value)
    {
    }

  	// Not supported
    void setTimeout(int milliseconds)
    {
    }

private:
	Stream* fPort;
	uint8_t fAddress;
	uint8_t fInitialByte;
	uint32_t fStartTime;
	int fTurnPower = 0;
};

class CytronSmartDriveDuoMDDS10Driver : public CytronSmartDriveDuoDriver
{
public:
	CytronSmartDriveDuoMDDS10Driver(byte address, Stream& port) :
		CytronSmartDriveDuoDriver(address, port, MDDS10) {}
};

class CytronSmartDriveDuoMDDS30Driver : public CytronSmartDriveDuoDriver
{
public:
	CytronSmartDriveDuoMDDS30Driver(byte address, Stream& port) :
		CytronSmartDriveDuoDriver(address, port, MDDS30) {}
};

class CytronSmartDriveDuoMDDS60Driver : public CytronSmartDriveDuoDriver
{
public:
	CytronSmartDriveDuoMDDS60Driver(byte address, Stream& port) :
		CytronSmartDriveDuoDriver(address, port, MDDS60) {}
};
#endif