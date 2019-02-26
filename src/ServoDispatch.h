#ifndef ServoDispatch_h
#define ServoDispatch_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"

//#define SERVO_DEBUG

/**
  * \struct ServoSettings
  *
  * \brief Settings for individual servos.
  *
  * \code
  * #define SMALL_PANEL        0x0001
  * #define MEDIUM_PANEL       0x0002
  * #define BIG_PANEL          0x0004
  * #define PIE_PANEL          0x0008
  * #define TOP_PIE_PANEL      0x0010
  * #define MINI_PANEL         0x0020
  * 
  * #define HOLO_HSERVO        0x1000
  * #define HOLO_VSERVO        0x2000
  * 
  * #define DOME_PANELS_MASK        (SMALL_PANEL|MEDIUM_PANEL|BIG_PANEL)
  * #define PIE_PANELS_MASK         (PIE_PANEL)
  * #define ALL_DOME_PANELS_MASK    (MINI_PANEL|DOME_PANELS_MASK|PIE_PANELS_MASK|TOP_PIE_PANEL)
  * #define HOLO_SERVOS_MASK        (HOLO_HSERVO|HOLO_VSERVO)
  * 
  * const ServoSettings servoSettings[] PROGMEM = {
  *     { 1,  SMALL_PANEL,   1000, 1650 },  // 0: door 4
  *     { 2,  SMALL_PANEL,   1500, 2300 },  // 1: door 3
  *     { 4,  SMALL_PANEL,    900, 1650 },  // 2: door 2
  *     { 6,  SMALL_PANEL,   1200, 1900 },  // 3: door 1
  *     { 17, MEDIUM_PANEL,  1200, 2000 },  // 4: door 5
  *     { 9,  BIG_PANEL,     1200, 2000 },  // 5: door 9
  *     { 8,  MINI_PANEL,    1275, 1975 },  // 6: mini door 2
  *     { 7,  MINI_PANEL,    1550, 1900 },  // 7: mini front psi door
  *     { 3,  PIE_PANEL,     1250, 1900 },  // 8: pie panel 1
  *     { 10, PIE_PANEL,     1075, 1700 },  // 9: pie panel 2
  *     { 11, PIE_PANEL,     1200, 2000 },  // 10: pie panel 3
  *     { 12, PIE_PANEL,      750, 1450 },  // 11: pie panel 4
  *     { 5,  TOP_PIE_PANEL, 1250, 1850 },  // 12: dome top panel
  *     { 13, HOLO_HSERVO,    800, 1600 },  // 13: horizontal front holo
  *     { 14, HOLO_VSERVO,    800, 1800 },  // 14: vertical front holo
  *     { 15, HOLO_HSERVO,    800, 1600 },  // 15: horizontal top holo
  *     { 16, HOLO_VSERVO,    800, 1325 },  // 16: vertical top holo
  *     { 25, HOLO_VSERVO,    900, 1000 },  // 17: vertical rear holo
  *     { 26, HOLO_HSERVO,   1300, 1600 },  // 18: horizontal rear holo
  * };
  * \endcode
  */
struct ServoSettings {
	/**
	  * Pin number
	  */
	uint16_t pinNum;
	/**
	  * Servo group mask
	  */
	uint32_t group;
	/**
	  * Mininum allowed pulse value
	  */
	uint16_t minPulse;
	/**
	  * Maximum allowed pulse value
	  */
	uint16_t maxPulse;
};

/**
  * \ingroup Core
  *
  * \class ServoDispatch
  *
  * \brief Servo interace implemented eitehr by ServoDispatchPCA9685 or ServoDispatchDirect.
  */
class ServoDispatch
{
public:
	virtual uint16_t getNumServos() = 0;
	virtual uint16_t getMinimum(uint16_t num) = 0;
	virtual uint16_t getMaximum(uint16_t num) = 0;
	virtual uint16_t currentPos(uint16_t num) = 0;

	/////////////////////////////////////////////////////////////////////////////////

	void moveTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t pos)
	{
		_moveServoTo(num, startDelay, moveTime, currentPos(num), pos);
	}

	void moveTo(uint16_t num, uint32_t moveTime, uint16_t pos)
	{
		_moveServoTo(num, 0, moveTime, currentPos(num), pos);
	}

	void moveTo(uint16_t num, uint16_t pos)
	{
		_moveServoTo(num, 0, 0, currentPos(num), pos);
	}

	void moveTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos)
	{
		_moveServoTo(num, startDelay, moveTime, startPos, pos);
	}

	void moveBy(uint16_t num, uint32_t moveTime, int16_t pos)
	{
		uint16_t curpos = currentPos(num);
		_moveServoTo(num, 0, moveTime, curpos, curpos + pos);
	}

	void moveBy(uint16_t num, int16_t pos)
	{
		uint16_t curpos = currentPos(num);
		_moveServoTo(num, 0, 0, curpos, curpos + pos);
	}

	void moveBy(uint16_t num, uint32_t startDelay, uint32_t moveTime, int16_t pos)
	{
		uint16_t curpos = currentPos(num);
		_moveServoTo(num, startDelay, moveTime, curpos, curpos + pos);
	}

	/////////////////////////////////////////////////////////////////////////////////

	void moveServosTo(uint32_t servoGroupMask, uint16_t pos)
	{
		_moveServosTo(servoGroupMask, 0, 0, 0, pos);
	}

	void moveServosTo(uint32_t servoGroupMask, uint32_t moveTime, uint16_t pos)
	{
		_moveServosTo(servoGroupMask, 0, moveTime, moveTime, pos);
	}

	void moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTime, uint16_t pos)
	{
		_moveServosTo(servoGroupMask, startDelay, moveTime, moveTime, pos);
	}

	void moveServosBy(uint32_t servoGroupMask, uint32_t moveTime, uint16_t pos)
	{
		_moveServosBy(servoGroupMask, 0, moveTime, moveTime, pos);
	}

	void moveServosBy(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTime, int16_t pos)
	{
		_moveServosBy(servoGroupMask, startDelay, moveTime, moveTime, pos);
	}

	void moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos)
	{
		_moveServosTo(servoGroupMask, startDelay, moveTimeMin, moveTimeMax, pos);
	}

	void moveServosBy(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos)
	{
		_moveServosBy(servoGroupMask, startDelay, moveTimeMin, moveTimeMax, pos);
	}

	/////////////////////////////////////////////////////////////////////////////////

	void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint16_t onPos, uint16_t offPos)
	{
		_moveServoSetTo(servoGroupMask, servoSetMask, 0, 0, 0, onPos, offPos);
	}

	void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t moveTime, uint16_t onPos, uint16_t offPos)
	{
		_moveServoSetTo(servoGroupMask, servoSetMask, 0, moveTime, moveTime, onPos, offPos);
	}

	void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTime, uint16_t onPos, uint16_t offPos)
	{
		_moveServoSetTo(servoGroupMask, servoSetMask, startDelay, moveTime, moveTime, onPos, offPos);
	}

	void moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t moveTime, int16_t onPos, int16_t offPos)
	{
		_moveServoSetBy(servoGroupMask, servoSetMask, 0, moveTime, moveTime, onPos, offPos);
	}

	void moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTime, int16_t onPos, int16_t offPos)
	{
		_moveServoSetBy(servoGroupMask, servoSetMask, startDelay, moveTime, moveTime, onPos, offPos);
	}

	void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos)
	{
		_moveServoSetTo(servoGroupMask, servoSetMask, startDelay, moveTimeMin, moveTimeMax, onPos, offPos);
	}

	void moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos)
	{
		_moveServoSetBy(servoGroupMask, servoSetMask, startDelay, moveTimeMin, moveTimeMax, onPos, offPos);
	}

protected:
	virtual void _moveServoTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos) = 0;
	virtual void _moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t pos) = 0;
	virtual void _moveServosBy(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos) = 0;
	virtual void _moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t onPos, uint16_t offPos) = 0;
	virtual void _moveServoSetBy(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos) = 0;
};

#endif
