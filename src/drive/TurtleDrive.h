class TurtleDrive
{
public:
    inline double getDroidDiameterMM()      { return fDroidDiameterMM;      }
    inline double getWheelCircumferenceMM() { return fWheelCircumferenceMM; }
    inline double getWheelTurnCount()       { return fWheelTurnCount;       }

    inline double getDroidCircumference()
    {
        return 2 * M_PI * getDroidDiameterMM() / 2;
    }

    inline int getMoveDistanceCount(double millimeters)
    {
        return millimeters / getWheelCircumferenceMM() * getWheelTurnCount();
    }

    inline int getTurnDistanceCount(double turnDegrees) 
    {
        return getMoveDistanceCount(getDroidCircumference() / 360 * turnDegrees);
    }

    inline void moveInches(double inches)       { moveMillimeters(inches * 25.4); }
    inline void moveFeet(double feet)           { moveMillimeters(feet * 304.8);  }
    inline void moveMeters(double meters)       { moveMillimeters(meters * 1000); }
    inline void moveCentimeters(double meters)  { moveMillimeters(meters * 10);   }

    virtual bool enterCommandMode() = 0;
    virtual void leaveCommandMode() = 0;
    virtual bool isCommandModeActive() = 0;

    virtual void moveMillimeters(double mm, float speed = 0.1) = 0;
    virtual void turnDegrees(double degrees, float speed = 0.1) = 0;
    virtual void stop() = 0;

protected:
    double  fDroidDiameterMM = 558.8;
    double  fWheelCircumferenceMM = 381;
    double  fWheelTurnCount = 610;
};