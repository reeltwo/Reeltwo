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
  *     { 1,  1000, 1650, SMALL_PANEL   },  // 0: door 4
  *     { 2,  1500, 2300, SMALL_PANEL   },  // 1: door 3
  *     { 4,   900, 1650, SMALL_PANEL   },  // 2: door 2
  *     { 6,  1200, 1900, SMALL_PANEL   },  // 3: door 1
  *     { 17, 1200, 2000, MEDIUM_PANEL  },  // 4: door 5
  *     { 9,  1200, 2000, BIG_PANEL     },  // 5: door 9
  *     { 8,  1275, 1975, MINI_PANEL    },  // 6: mini door 2
  *     { 7,  1550, 1900, MINI_PANEL    },  // 7: mini front psi door
  *     { 3,  1250, 1900, PIE_PANEL     },  // 8: pie panel 1
  *     { 10, 1075, 1700, PIE_PANEL     },  // 9: pie panel 2
  *     { 11, 1200, 2000, PIE_PANEL     },  // 10: pie panel 3
  *     { 12,  750, 1450, PIE_PANEL     },  // 11: pie panel 4
  *     { 5,  1250, 1850, TOP_PIE_PANEL },  // 12: dome top panel
  *     { 13,  800, 1600, HOLO_HSERVO   },  // 13: horizontal front holo
  *     { 14,  800, 1800, HOLO_VSERVO   },  // 14: vertical front holo
  *     { 15,  800, 1600, HOLO_HSERVO   },  // 15: horizontal top holo
  *     { 16,  800, 1325, HOLO_VSERVO   },  // 16: vertical top holo
  *     { 25,  900, 1000, HOLO_VSERVO   },  // 17: vertical rear holo
  *     { 26, 1300, 1600, HOLO_HSERVO   },  // 18: horizontal rear holo
  * };
  * \endcode
  */
struct ServoSettings {
    /**
      * Pin number
      */
    uint16_t pinNum;
    /**
      * Mininum allowed pulse value
      */
    uint16_t minPulse;
    /**
      * Maximum allowed pulse value
      */
    uint16_t maxPulse;
    /**
      * Servo group mask
      */
    uint32_t group;
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
