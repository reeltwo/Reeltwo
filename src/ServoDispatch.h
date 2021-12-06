#ifndef ServoDispatch_h
#define ServoDispatch_h

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "ServoEasing.h"

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
      * Start pulse value
      */
    uint16_t startPulse;
    /**
      * End pulse value
      */
    uint16_t endPulse;
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
    virtual uint8_t getPin(uint16_t num) = 0;
    virtual uint16_t getStart(uint16_t num) = 0;
    virtual uint16_t getEnd(uint16_t num) = 0;
    virtual uint16_t getMinimum(uint16_t num) = 0;
    virtual uint16_t getMaximum(uint16_t num) = 0;
    virtual uint16_t getNeutral(uint16_t num) = 0;
    virtual uint32_t getGroup(uint16_t num) = 0;
    virtual uint16_t currentPos(uint16_t num) = 0;
    virtual uint16_t scaleToPos(uint16_t num, float scale) = 0;
    virtual bool isActive(uint16_t num) = 0;
    virtual void disable(uint16_t num) = 0;
    virtual void setNeutral(uint16_t num, uint16_t neutralPulse) = 0;
    virtual void setServo(uint16_t num, uint8_t pin, uint16_t startPulse, uint16_t endPulse, uint16_t neutralPulse, uint32_t group) = 0;
    virtual void setPWM(uint16_t num, uint16_t targetLength) = 0;

    // Stop all servo movement
    virtual void stop() = 0;

    /////////////////////////////////////////////////////////////////////////////////
    //
    // Range functions
    //
    // Position is 0.0 - 1.0 as a scale for startPulse - endPulse
    //
    /////////////////////////////////////////////////////////////////////////////////

    void moveTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, float pos)
    {
        _moveServoToPulse(num, startDelay, moveTime, currentPos(num), scaleToPos(num, pos));
    }

    void moveTo(uint16_t num, uint32_t moveTime, float pos)
    {
        _moveServoToPulse(num, 0, moveTime, currentPos(num), scaleToPos(num, pos));
    }

    void moveTo(uint16_t num, float pos)
    {
        _moveServoToPulse(num, 0, 0, currentPos(num), scaleToPos(num, pos));
    }

    void moveTo(uint16_t num, uint32_t startDelay, uint32_t moveTime, float startPos, float pos)
    {
        _moveServoToPulse(num, startDelay, moveTime, scaleToPos(num, startPos), scaleToPos(num, pos));
    }

    void moveServosTo(uint32_t servoGroupMask, float pos)
    {
        _moveServosTo(servoGroupMask, 0, 0, 0, pos);
    }

    void moveServosTo(uint32_t servoGroupMask, uint32_t moveTime, float pos)
    {
        _moveServosTo(servoGroupMask, 0, moveTime, moveTime, pos);
    }

    void moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTime, float pos)
    {
        _moveServosTo(servoGroupMask, startDelay, moveTime, moveTime, pos);
    }

    void moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float pos)
    {
        _moveServosTo(servoGroupMask, startDelay, moveTimeMin, moveTimeMax, pos);
    }

    void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, float onPos, float offPos, float (*onEasingMethod)(float) = NULL, float (*offEasingMethod)(float) = NULL)
    {
        _moveServoSetTo(servoGroupMask, servoSetMask, 0, 0, 0, onPos, offPos, onEasingMethod, offEasingMethod);
    }

    void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t moveTime, float onPos, float offPos, float (*onEasingMethod)(float) = NULL, float (*offEasingMethod)(float) = NULL)
    {
        _moveServoSetTo(servoGroupMask, servoSetMask, 0, moveTime, moveTime, onPos, offPos, onEasingMethod, offEasingMethod);
    }

    void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTime, float onPos, float offPos, float (*onEasingMethod)(float) = NULL, float (*offEasingMethod)(float) = NULL)
    {
        _moveServoSetTo(servoGroupMask, servoSetMask, startDelay, moveTime, moveTime, onPos, offPos, onEasingMethod, offEasingMethod);
    }

    void moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float onPos, float offPos, float (*onEasingMethod)(float) = NULL, float (*offEasingMethod)(float) = NULL)
    {
        _moveServoSetTo(servoGroupMask, servoSetMask, startDelay, moveTimeMin, moveTimeMax, onPos, offPos, onEasingMethod, offEasingMethod);
    }

    /////////////////////////////////////////////////////////////////////////////////
    //
    // Pulse value functions
    //
    /////////////////////////////////////////////////////////////////////////////////

    void moveToPulse(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t pos)
    {
        _moveServoToPulse(num, startDelay, moveTime, currentPos(num), pos);
    }

    void moveToPulse(uint16_t num, uint32_t moveTime, uint16_t pos)
    {
        _moveServoToPulse(num, 0, moveTime, currentPos(num), pos);
    }

    void moveToPulse(uint16_t num, uint16_t pos)
    {
        _moveServoToPulse(num, 0, 0, currentPos(num), pos);
    }

    void moveToPulse(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos)
    {
        _moveServoToPulse(num, startDelay, moveTime, startPos, pos);
    }

    void moveByPulse(uint16_t num, uint32_t moveTime, int16_t pos)
    {
        uint16_t curpos = currentPos(num);
        _moveServoToPulse(num, 0, moveTime, curpos, curpos + pos);
    }

    void moveByPulse(uint16_t num, int16_t pos)
    {
        uint16_t curpos = currentPos(num);
        _moveServoToPulse(num, 0, 0, curpos, curpos + pos);
    }

    void moveByPulse(uint16_t num, uint32_t startDelay, uint32_t moveTime, int16_t pos)
    {
        uint16_t curpos = currentPos(num);
        _moveServoToPulse(num, startDelay, moveTime, curpos, curpos + pos);
    }

    /////////////////////////////////////////////////////////////////////////////////

    void moveServosToPulse(uint32_t servoGroupMask, uint16_t pos)
    {
        _moveServosToPulse(servoGroupMask, 0, 0, 0, pos);
    }

    void moveServosToPulse(uint32_t servoGroupMask, uint32_t moveTime, uint16_t pos)
    {
        _moveServosToPulse(servoGroupMask, 0, moveTime, moveTime, pos);
    }

    void moveServosToPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTime, uint16_t pos)
    {
        _moveServosToPulse(servoGroupMask, startDelay, moveTime, moveTime, pos);
    }

    void moveServosToPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos)
    {
        _moveServosToPulse(servoGroupMask, startDelay, moveTimeMin, moveTimeMax, pos);
    }

    void moveServosByPulse(uint32_t servoGroupMask, uint32_t moveTime, uint16_t pos)
    {
        _moveServosByPulse(servoGroupMask, 0, moveTime, moveTime, pos);
    }

    void moveServosByPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTime, int16_t pos)
    {
        _moveServosByPulse(servoGroupMask, startDelay, moveTime, moveTime, pos);
    }

    void moveServosByPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos)
    {
        _moveServosByPulse(servoGroupMask, startDelay, moveTimeMin, moveTimeMax, pos);
    }

    /////////////////////////////////////////////////////////////////////////////////

    void moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint16_t onPos, uint16_t offPos)
    {
        _moveServoSetToPulse(servoGroupMask, servoSetMask, 0, 0, 0, onPos, offPos);
    }

    void moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t moveTime, uint16_t onPos, uint16_t offPos)
    {
        _moveServoSetToPulse(servoGroupMask, servoSetMask, 0, moveTime, moveTime, onPos, offPos);
    }

    void moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTime, uint16_t onPos, uint16_t offPos)
    {
        _moveServoSetToPulse(servoGroupMask, servoSetMask, startDelay, moveTime, moveTime, onPos, offPos);
    }

    void moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos)
    {
        _moveServoSetToPulse(servoGroupMask, servoSetMask, startDelay, moveTimeMin, moveTimeMax, onPos, offPos);
    }

    void moveServoSetByPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t moveTime, int16_t onPos, int16_t offPos)
    {
        _moveServoSetByPulse(servoGroupMask, servoSetMask, 0, moveTime, moveTime, onPos, offPos);
    }

    void moveServoSetByPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTime, int16_t onPos, int16_t offPos)
    {
        _moveServoSetByPulse(servoGroupMask, servoSetMask, startDelay, moveTime, moveTime, onPos, offPos);
    }

    void moveServoSetByPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos)
    {
        _moveServoSetByPulse(servoGroupMask, servoSetMask, startDelay, moveTimeMin, moveTimeMax, onPos, offPos);
    }

    /////////////////////////////////////////////////////////////////////////////////

    void setServoEasingMethod(uint16_t num, float (*easingMethod)(float completion))
    {
        _setServoEasingMethod(num, easingMethod);
    }

    void setServosEasingMethod(uint32_t servoGroupMask, float (*easingMethod)(float completion))
    {
        _setServosEasingMethod(servoGroupMask, easingMethod);
    }

    void setEasingMethod(float (*easingMethod)(float completion))
    {
        _setEasingMethod(easingMethod);
    }

protected:
    virtual void _moveServoToPulse(uint16_t num, uint32_t startDelay, uint32_t moveTime, uint16_t startPos, uint16_t pos) = 0;
    virtual void _moveServosToPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t pos) = 0;
    virtual void _moveServosByPulse(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t pos) = 0;
    virtual void _moveServoSetToPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, uint16_t onPos, uint16_t offPos) = 0;
    virtual void _moveServoSetByPulse(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, int16_t onPos, int16_t offPos) = 0;
    virtual void _moveServosTo(uint32_t servoGroupMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float pos) = 0;
    virtual void _moveServoSetTo(uint32_t servoGroupMask, uint32_t servoSetMask, uint32_t startDelay, uint32_t moveTimeMin, uint32_t moveTimeMax, float onPos, float offPos, float (*onEasingMethod)(float), float (*offEasingMethod)(float)) = 0;
    virtual void _setServoEasingMethod(uint16_t num, float (*easingMethod)(float completion)) = 0;
    virtual void _setServosEasingMethod(uint32_t servoGroupMask, float (*easingMethod)(float completion)) = 0;
    virtual void _setEasingMethod(float (*easingMethod)(float completion)) = 0;
};

#endif
