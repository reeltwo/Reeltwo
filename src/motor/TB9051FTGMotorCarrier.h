#pragma once

#include "ReelTwo.h"

/**
  * \ingroup Motor
  *
  * \class TB9051FTGMotorCarrier
  *
  * \brief Implements support for the Pololu TB9051FTG MotorCarrier (https://www.pololu.com/product/2997)
*/
class TB9051FTGMotorCarrier {
public:
    static constexpr uint8_t kPinNotUsed{255};
    // static constexpr uint8_t kPWM1_Timer0{5};
    // static constexpr uint8_t kPWM2_Timer0{6};
    // static constexpr uint8_t kPWM1_Timer1{9};
    // static constexpr uint8_t kPWM2_Timer1{10};
    // static constexpr uint8_t kPWM1_Timer2{3};
    // static constexpr uint8_t kPWM2_Timer2{11};

    /** \brief Constructor
      *
      * Construct a new instance of TB9051FTGMotorCarrier.
      *
      * \param pwm1 The pin number connected to the PWM1 input on the motor driver carrier
      * \param pwm2 The pin number connected to the PWM2 input on the motor driver carrier
      * \param en (Optional) The pin number connected to the EN input on the motor driver carrier. Defaults to kPinNotUsed
      * \param enb (Optional) The pin number connected to the ENB input on the motor driver carrier. Defaults to kPinNotUsed
      * \param ocm (Optional) The pin number connected to the OCM output on the motor driver carrier. Defaults to kPinNotUsed
      * \param diag (Optional) The pin number connected to the DIAG output on the motor driver carrier. Defaults to kPinNotUsed
      * \param occ (Optional) The pin number connected to the OCC input on the motor driver carrier. Defaults to kPinNotUsed
      */
    TB9051FTGMotorCarrier(
            uint8_t pwm1,
            uint8_t pwm2,
            uint8_t en = kPinNotUsed,
            uint8_t enb = kPinNotUsed,
            uint8_t ocm = kPinNotUsed,
            uint8_t diag = kPinNotUsed,
            uint8_t occ = kPinNotUsed) :
       fPWM1(pwm1),
       fPWM2(pwm2),
       fOCM(ocm),
       fDiag(diag),
       fOCC(occ),
       fEN(en),
       fENB(enb)
    {
        pinMode(fPWM1, OUTPUT);
        pinMode(fPWM2, OUTPUT);
        pinMode(fOCM, INPUT);
        pinMode(fDiag, INPUT);
        pinMode(fOCC, OUTPUT);
        pinMode(fEN, OUTPUT);
        pinMode(fENB, OUTPUT);
        // if (fPWM1 == kPWM1_Timer0 && fPWM2 == kPWM2_Timer0)
        // {
        //     /* Enable fast PWM mode */
        //     TCCR0A |= (1<<WGM00);
        //     TCCR0A |= (1<<WGM01);
        //     TCCR0B &= ~(1<<WGM02);
            
        //     /* Enable non-inverting mode on OC0A */
        //     TCCR0A |= (1<<COM0A1);
        //     TCCR0A &= ~(1<<COM0A0);

        //     /* Enable non-inverting mode on OC0B */
        //     TCCR0A |= (1<<COM0B1);
        //     TCCR0A &= ~(1<<COM0B0);
            
        //     /* Disable Force Output Compare A */
        //     TCCR0B &= ~(1<<FOC0A);

        //     /* Disable Force Output Compare B */
        //     TCCR0B &= ~(1<<FOC0B);
            
        //     /* Disable all Counter0 interrupts */
        //     TIMSK0 = 0;

        //     auto CS_bits = getPrescaleCSBits(0);

        //     /* Use dummy variable to set the right CS02:0 bits */
        //     TCCR0B |= CS_bits;

        //     /* Use dummy variable to clear the right CS02:0 bits, masking out the other bits */
        //     TCCR0B &= (0b11111000 | CS_bits);
        // }
        // else if (fPWM1 == kPWM1_Timer1 && fPWM2 == kPWM2_Timer1)
        // {
        //     /* Enable fast PWM mode */
        //     TCCR1A |= (1<<WGM10);
        //     TCCR1A |= (1<<WGM11);
        //     TCCR1B &= ~(1<<WGM12);
            
        //     /* Enable non-inverting mode on OC1A */
        //     TCCR1A |= (1<<COM1A1);
        //     TCCR1A &= ~(1<<COM1A0);

        //     /* Enable non-inverting mode on OC1B */
        //     TCCR1A |= (1<<COM1B1);
        //     TCCR1A &= ~(1<<COM1B0);
            
        //     /* Disable Force Output Compare A */
        //     TCCR1B &= ~(1<<FOC1A);

        //     /* Disable Force Output Compare B */
        //     TCCR1B &= ~(1<<FOC1B);
            
        //     /* Disable all Counter1 interrupts */
        //     TIMSK1 = 0;

        //     ICR1 = 799;

        //     // prescale 0
        //     auto CS_bits = getPrescaleCSBits(0);

        //     /* Use dummy variable to set the right CS02:0 bits */
        //     TCCR1B |= CS_bits;

        //     /* Use dummy variable to clear the right CS02:0 bits, masking out the other bits */
        //     TCCR1B &= (0b11111000 | CS_bits);
        // }
        // else if (fPWM1 == kPWM1_Timer2 && fPWM2 == kPWM2_Timer2)
        // {
        //     /* Enable fast PWM mode */
        //     TCCR2A |= (1<<WGM20);
        //     TCCR2A |= (1<<WGM21);
        //     TCCR2B &= ~(1<<WGM22);
            
        //     /* Enable non-inverting mode on OC2A */
        //     TCCR2A |= (1<<COM2A1);
        //     TCCR2A &= ~(1<<COM2A0);

        //     /* Enable non-inverting mode on OC2B */
        //     TCCR2A |= (1<<COM2B1);
        //     TCCR2A &= ~(1<<COM2B0);
            
        //     /* Disable Force Output Compare A */
        //     TCCR2B &= ~(1<<FOC2A);

        //     /* Disable Force Output Compare B */
        //     TCCR2B &= ~(1<<FOC2B);
            
        //     /* Disable all Counter2 interrupts */
        //     TIMSK2 = 0;

        //     // prescale 0
        //     auto CS_bits = getPrescaleCSBits(0);

        //     /* Use dummy variable to set the right CS02:0 bits */
        //     TCCR1B |= CS_bits;

        //     /* Use dummy variable to clear the right CS02:0 bits, masking out the other bits */
        //     TCCR1B &= (0b11111000 | CS_bits);
        // }
    }

    /**
      * Reads an estimate of the motor current from the motor driver carrier. Output is in
      * milliamps (mA). Note: This measurement is a coarse approximation only. Consider using
      * an external sensor for fine current measurements.
      *
      * \returns Current measurement in mA
      */
    float getCurrent(void) const
    {
        // 4.9 mV per analog unit (for Arduino Uno)
        // 500 mV per A on TB9051FTG current sense
        // 1000 mA per A
        return (analogRead(fOCM) * 4.9) / 500 * 1000;
    }

    /**
      * Reads the DIAG pin on the motor driver carrier. If there is a fault, or if the motor
      * driver carrier is disabled by the EN or ENB pins, it returns logical low (0).
      * Otherwise, it returns logical high (1)
      * 
      * \return 0 if fault or disabled. 1 otherwise.
      */
    uint8_t getDiagnostic(void) const
    {
        return digitalRead(fDiag, HIGH);
    }

    /**
      * Sets the dead band range. When the throttle output value is within this range, the
      * motor driver carrier stops the motor according to the current brake mode configuration
      * 
      * \param lower Lower dead band value (exclusively)
      * \param upper Upper dead band value (exclusively)
      */
    void setDeadband(float lower, float upper)
    {
        fDeadBandLower = lower;
        fDeadBandUpper = upper;
    }

    /**
      * Sets the brake mode for the motor driver carrier. If the carrier is set to brake mode
      * (mode 1), the carrier outputs will be shorted to ground when the motor stops. This acts as a
      * brake. If the carrier is not set to brake mode (0), the carrier outputs will be left floating,
      * so the motor will coast to a stop.
      * 
      * \param mode Brake mode (true) or non brake mode (false).
      */
    void setBrakeMode(bool mode)
    {
        fBrakeMode = mode;
    }

    /**
      * Sets the throttle output percentage [-1, 1], which is the percentage of the driver carrier's
      * input voltage. When the output percentage is negative, the motor will turn in reverse
      * 
      * \param percent output throttle percentage (-1.0 - 1.0)
      */
    void setOutput(float percent) const
    {
        if (withinDeadband(percent))
        {
            if (fBrakeMode)
            {
                dualAnalogWrite(fPWM1, 0, fPWM2, 0);
            }
            else
            {
                disableOutputs();
            }
        }
        else if (fEnabled && percent < 0)
        {
            enableOutputs();
            dualAnalogWrite(fPWM1, 0, fPWM2, fabs(percent));
        }
        else if (fEnabled)
        {
            enableOutputs();
            dualAnalogWrite(fPWM1, fabs(percent), fPWM2, 0);
        }
    }

    /**
      * Sets the over-current response. If false, the motor driver carrier is disabled when an
      * over-current event happens. The motor driver carrier can be reset by toggling either the EN
      * pin or the ENB pin. If true, the motor driver carrier will re-enable after a short
      * period following an over-current event.
      * 
      * \param automatic over-current response
      */
    void setOccResponse(bool automatic) const
    {
        digitalWrite(fOCC, automatic ? 1 : 0);
    }

    /**
      * Enable the motor driver carrier's outputs. The motor driver carrier Initially is disabled, so it
      * needs to be manually enabled using this function
      */
    void enable(void)
    {
        fEnabled = true;
        enableOutputs();
    }

    /**
      * Disable the motor driver carrier's outputs
      */
    void disable(void)
    {
        fEnabled = false;
        disableOutputs();
    }

private:
    static constexpr uint8_t getPrescaleCSBits(unsigned prescale)
    {
        return (prescale == 0) ? 0b00000001 :
               (prescale == 8) ? 0b00000010 :
               (prescale == 64) ? 0b00000011 :
               (prescale == 256) ? 0b00000100 :
               (prescale == 1024) ? 0b00000101 : 0;
    }

    static inline void pinMode(uint8_t pin, uint8_t mode)
    {
        if (pin != kPinNotUsed)
        {
            ::pinMode(pin, mode);
        }
    }

    static inline void digitalWrite(uint8_t pin, uint8_t val)
    {
        if (pin != kPinNotUsed)
        {
            Serial.println("digitalWrite");
            ::digitalWrite(pin, val);
        }
    }

    static inline uint8_t digitalRead(uint8_t pin, uint8_t defaultValue = 0)
    {
        return (pin != kPinNotUsed) ? ::digitalRead(pin) : defaultValue;
    }

    static inline int analogRead(uint8_t pin)
    {
        return (pin != kPinNotUsed) ? ::analogRead(pin) : 0;
    }

    static inline void dualAnalogWrite(uint8_t m1, float v1, uint8_t m2, float v2)
    {
        const auto output1{static_cast<uint16_t>(abs(v1) * 1023)};
        const auto output2{static_cast<uint16_t>(abs(v2) * 1023)};
        // if (m1 == kPWM1_Timer0 && m2 == kPWM2_Timer0)
        // {
        //     OCR0A = output1;
        //     OCR0B = output2;
        // }
        // else if (m1 == kPWM1_Timer1 && m2 == kPWM2_Timer1)
        // {
        //     const auto output1{static_cast<uint16_t>(abs(v1) * 799)};
        //     const auto output2{static_cast<uint16_t>(abs(v2) * 799)};
        //     OCR1A = output1;
        //     OCR1B = output2;
        // }
        // else if (m1 == kPWM1_Timer2 && m2 == kPWM2_Timer2)
        // {
        //     OCR2A = output1;
        //     OCR2B = output2;
        // }
        // else
        {
            Serial.print("dualAnalogWrite "); Serial.print(output1); Serial.print(", "); Serial.println(output2);
            ::analogWrite(m1, output1, 1023);
            ::analogWrite(m2, output2, 1023);
        }
        Serial.println();
    }

    bool withinDeadband(float value) const
    {
        return value < fDeadBandUpper && value > fDeadBandLower;
    }

    void enableOutputs(void) const
    {
        digitalWrite(fEN, 1);
        digitalWrite(fENB, 0);
    }

    void disableOutputs(void) const
    {
        digitalWrite(fEN, 0);
        digitalWrite(fENB, 1);
    }

    float fDeadBandLower = 0;
    float fDeadBandUpper = 0;

    bool fBrakeMode = false;  // true if motor in brake mode
    bool fEnabled = false;  // true if driver enabled

    // Pin values
    uint8_t fPWM1;  // PWM for OUT1 (PWM1)
    uint8_t fPWM2;  // PWM for OUT2 (PWM2)
    uint8_t fOCM;  // Output current monitor (OCM)
    uint8_t fDiag;  // Diagnostic error (DIAG)
    uint8_t fOCC;  // Over current (OCC) response
    uint8_t fEN;  // Enable (EN)
    uint8_t fENB;  // Inverted enable (ENB)
};
