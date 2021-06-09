#ifndef JoystickController_h
#define JoystickController_h

class JoystickController
{
public:
    struct AnalogStick
    {
        int8_t lx;
        int8_t ly;
        int8_t rx;
        int8_t ry;
    };

    struct AnalogButton
    {
        uint8_t up;
        uint8_t right;
        uint8_t down;
        uint8_t left;

        uint8_t l2;
        uint8_t r2;
        uint8_t l1;
        uint8_t r1;

        uint8_t triangle;
        uint8_t circle;
        uint8_t cross;
        uint8_t square;
    };

    struct Analog
    {
        AnalogStick stick;
        AnalogButton button;
    };

    struct Button
    {
        uint8_t select   : 1;
        uint8_t l3       : 1;
        uint8_t r3       : 1;
        uint8_t start    : 1;

        uint8_t up       : 1;
        uint8_t right    : 1;
        uint8_t down     : 1;
        uint8_t left     : 1;

        uint8_t upright  : 1;
        uint8_t upleft   : 1;
        uint8_t downright: 1;
        uint8_t downleft : 1;

        uint8_t l2       : 1;
        uint8_t r2       : 1;
        uint8_t l1       : 1;
        uint8_t r1       : 1;

        uint8_t triangle : 1;
        uint8_t circle   : 1;
        uint8_t cross    : 1;
        uint8_t square   : 1;

        uint8_t ps       : 1;
        uint8_t share    : 1;
        uint8_t options  : 1;
        uint8_t touchpad : 1;
    };

    enum BatteryStatus
    {
        kShutdown = 0x01,
        kDying    = 0x02,
        kLow      = 0x03,
        kHigh     = 0x04,
        kFull     = 0x05,
        kCharging = 0xEE
    };

    enum ConnectionStatus
    {
        kUSB,
        kBluetooth
    };

    struct Status
    {
        enum BatteryStatus battery;
        enum ConnectionStatus connection;
        uint8_t charging : 1;
        uint8_t rumbling : 1;
        uint8_t audio : 1;
        uint8_t mic : 1;
    };

    /********************/
    /*   S E N S O R S  */
    /********************/

    struct Gyroscope
    {
        int16_t z;
    };

    struct Accelerometer
    {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct Sensor
    {
        Accelerometer accelerometer;
        Gyroscope gyroscope;
    };

    struct Event
    {
        Button button_down;
        Button button_up;
        Button long_button_up;
        Analog analog_changed;
    };

    struct State
    {
        Analog analog;
        Button button;
        Status status;
        Sensor sensor;
    };

    State state;
    Event event;

    JoystickController() :
        fConnected(false),
        fConnecting(false),
        fCongested(false)
    {
    }

    inline bool isConnected() const  { return fConnected;  }
    inline bool isConnecting() const { return fConnecting; }
    inline bool isCongested() const  { return fCongested;  }

    virtual void disconnect() {}

    virtual void notify() {}
    virtual void onConnect() {}
    virtual void onDisconnect() {}

protected:
    bool fConnected;
    bool fConnecting;
    bool fCongested;
};

inline bool operator == (const JoystickController::AnalogStick& lhs, const JoystickController::AnalogStick& rhs)
{
	return (lhs.lx == rhs.lx &&
			lhs.ly == rhs.ly &&
			lhs.rx == rhs.rx &&
			lhs.ry == rhs.ry);
}

inline bool operator == (const JoystickController::AnalogButton& lhs, const JoystickController::AnalogButton& rhs)
{
	return (lhs.up		 == rhs.up &&
			lhs.right	 == rhs.right &&
			lhs.down	 == rhs.down &&
			lhs.left	 == rhs.left &&
			lhs.l2		 == rhs.l2 &&
			lhs.r2		 == rhs.r2 &&
			lhs.l1		 == rhs.l1 &&
			lhs.r1		 == rhs.r1 &&
			lhs.triangle == rhs.triangle &&
			lhs.circle	 == rhs.circle &&
			lhs.cross	 == rhs.cross &&
			lhs.square	 == rhs.square);
}

inline bool operator == (const JoystickController::Analog lhs, const JoystickController::Analog rhs)
{
	return (lhs.stick  == rhs.stick &&
			lhs.button == rhs.button);
}

inline bool operator == (const JoystickController::Button lhs, const JoystickController::Button rhs)
{
	return (lhs.select		 == rhs.select &&
			lhs.l3			 == rhs.l3 &&
			lhs.r3			 == rhs.r3 &&
			lhs.start		 == rhs.start &&
			lhs.up			 == rhs.up &&
			lhs.right		 == rhs.right &&
			lhs.down		 == rhs.down &&
			lhs.left		 == rhs.left &&
			lhs.upright		 == rhs.upright &&
			lhs.upleft		 == rhs.upleft &&
			lhs.downright	 == rhs.downright &&
			lhs.downleft	 == rhs.downleft &&
			lhs.l2			 == rhs.l2 &&
			lhs.r2			 == rhs.r2 &&
			lhs.l1			 == rhs.l1 &&
			lhs.r1			 == rhs.r1 &&
			lhs.triangle	 == rhs.triangle &&
			lhs.circle		 == rhs.circle &&
			lhs.cross		 == rhs.cross &&
			lhs.square		 == rhs.square &&
			lhs.ps			 == rhs.ps &&
			lhs.share		 == rhs.share &&
			lhs.options		 == rhs.options &&
			lhs.touchpad	 == rhs.touchpad);	
}

inline bool operator == (const JoystickController::Status lhs, const JoystickController::Status rhs)
{
	return (lhs.battery		 == rhs.battery &&
			lhs.connection	 == rhs.connection &&
			lhs.charging	 == rhs.charging &&
			lhs.rumbling	 == rhs.rumbling &&
			lhs.audio		 == rhs.audio &&
			lhs.mic			 == rhs.mic);
}

inline bool operator == (const JoystickController::Gyroscope lhs, const JoystickController::Gyroscope rhs)
{
	return (lhs.z == rhs.z);
}

inline bool operator == (const JoystickController::Accelerometer lhs, const JoystickController::Accelerometer rhs)
{
	return (lhs.x == rhs.x &&
			lhs.y == rhs.y &&
			lhs.z == rhs.z);
}

inline bool operator == (const JoystickController::Sensor lhs, const JoystickController::Sensor rhs)
{
	return (lhs.accelerometer == rhs.accelerometer &&
			lhs.gyroscope	  == rhs.gyroscope);
}

inline bool operator == (const JoystickController::Event lhs, const JoystickController::Event rhs)
{
	return (lhs.button_down	   == rhs.button_down &&
			lhs.button_up	   == rhs.button_up &&
			lhs.long_button_up == rhs.long_button_up &&
			lhs.analog_changed == rhs.analog_changed);
}


inline bool operator == (const JoystickController::State lhs, const JoystickController::State rhs)
{
	return (lhs.analog == rhs.analog &&
			lhs.button == rhs.button &&
			lhs.status == rhs.status &&
			lhs.sensor == rhs.sensor);
}

inline bool operator != (const JoystickController::AnalogStick& lhs, const JoystickController::AnalogStick& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::AnalogButton& lhs, const JoystickController::AnalogButton& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Analog& lhs, const JoystickController::Analog& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Button& lhs, const JoystickController::Button& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Status& lhs, const JoystickController::Status& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Gyroscope& lhs, const JoystickController::Gyroscope& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Accelerometer& lhs, const JoystickController::Accelerometer& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Sensor& lhs, const JoystickController::Sensor& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::Event& lhs, const JoystickController::Event& rhs) { return !(lhs == rhs); }
inline bool operator != (const JoystickController::State& lhs, const JoystickController::State& rhs) { return !(lhs == rhs); }

inline bool eq(const JoystickController::State lhs, const JoystickController::State rhs)
{
	return (lhs.analog == rhs.analog &&
			lhs.button == rhs.button);
}
inline bool ne(const JoystickController::State& lhs, const JoystickController::State& rhs)
{
    return ! eq(lhs, rhs);
}

#endif
