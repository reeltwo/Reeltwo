#ifndef ServoEase_h
#define ServoEase_h

#include "ServoDispatch.h"

class Easing
{
public:
    typedef float (*Method)(float);

    enum
    {
        kLinearInterpolation = 0,
        kQuadraticEaseIn = 1,
        kQuadraticEaseOut = 2,
        kQuadraticEaseInOut = 3,
        kCubicEaseIn = 4,
        kCubicEaseOut = 5,
        kCubicEaseInOut = 6,
        kQuarticEaseIn = 7,
        kQuarticEaseOut = 8,
        kQuarticEaseInOut = 9,
        kQuinticEaseIn = 10,
        kQuinticEaseOut = 11,
        kQuinticEaseInOut = 12,
        kSineEaseIn = 13,
        kSineEaseOut = 14,
        kSineEaseInOut = 15,
        kCircularEaseIn = 16,
        kCircularEaseOut = 17,
        kCircularEaseInOut = 18,
        kExponentialEaseIn = 19,
        kExponentialEaseOut = 20,
        kExponentialEaseInOut = 21,
        kElasticEaseIn = 22,
        kElasticEaseOut = 23,
        kElasticEaseInOut = 24,
        kBackEaseIn = 25,
        kBackEaseOut = 26,
        kBackEaseInOut = 27,
        kBounceEaseIn = 28,
        kBounceEaseOut = 29,
        kBounceEaseInOut = 30
    };

    // Modeled after the line y = x
    static float LinearInterpolation(float p)
    {
        return p;
    }

    // Modeled after the parabola y = x^2
    static float QuadraticEaseIn(float p)
    {
        return p * p;
    }

    // Modeled after the parabola y = -x^2 + 2x
    static float QuadraticEaseOut(float p)
    {
        return -(p * (p - 2));
    }

    // Modeled after the piecewise quadratic
    // y = (1/2)((2x)^2)             ; [0, 0.5)
    // y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
    static float QuadraticEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 2 * p * p;
        }
        else
        {
            return (-2 * p * p) + (4 * p) - 1;
        }
    }

    // Modeled after the cubic y = x^3
    static float CubicEaseIn(float p)
    {
        return p * p * p;
    }

    // Modeled after the cubic y = (x - 1)^3 + 1
    static float CubicEaseOut(float p)
    {
        float f = (p - 1);
        return f * f * f + 1;
    }

    // Modeled after the piecewise cubic
    // y = (1/2)((2x)^3)       ; [0, 0.5)
    // y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
    static float CubicEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 4 * p * p * p;
        }
        else
        {
            float f = ((2 * p) - 2);
            return 0.5 * f * f * f + 1;
        }
    }

    // Modeled after the quartic x^4
    static float QuarticEaseIn(float p)
    {
        return p * p * p * p;
    }

    // Modeled after the quartic y = 1 - (x - 1)^4
    static float QuarticEaseOut(float p)
    {
        float f = (p - 1);
        return f * f * f * (1 - p) + 1;
    }

    // Modeled after the piecewise quartic
    // y = (1/2)((2x)^4)        ; [0, 0.5)
    // y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
    static float QuarticEaseInOut(float p) 
    {
        if (p < 0.5)
        {
            return 8 * p * p * p * p;
        }
        else
        {
            float f = (p - 1);
            return -8 * f * f * f * f + 1;
        }
    }

    // Modeled after the quintic y = x^5
    static float QuinticEaseIn(float p) 
    {
        return p * p * p * p * p;
    }

    // Modeled after the quintic y = (x - 1)^5 + 1
    static float QuinticEaseOut(float p) 
    {
        float f = (p - 1);
        return f * f * f * f * f + 1;
    }

    // Modeled after the piecewise quintic
    // y = (1/2)((2x)^5)       ; [0, 0.5)
    // y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
    static float QuinticEaseInOut(float p) 
    {
        if (p < 0.5)
        {
            return 16 * p * p * p * p * p;
        }
        else
        {
            float f = ((2 * p) - 2);
            return  0.5 * f * f * f * f * f + 1;
        }
    }

    // Modeled after quarter-cycle of sine wave
    static float SineEaseIn(float p)
    {
        return sin((p - 1) * M_PI_2) + 1;
    }

    // Modeled after quarter-cycle of sine wave (different phase)
    static float SineEaseOut(float p)
    {
        return sin(p * M_PI_2);
    }

    // Modeled after half sine wave
    static float SineEaseInOut(float p)
    {
        return 0.5 * (1 - cos(p * M_PI));
    }

    // Modeled after shifted quadrant IV of unit circle
    static float CircularEaseIn(float p)
    {
        return 1 - sqrt(1 - (p * p));
    }

    // Modeled after shifted quadrant II of unit circle
    static float CircularEaseOut(float p)
    {
        return sqrt((2 - p) * p);
    }

    // Modeled after the piecewise circular function
    // y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
    // y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
    static float CircularEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
        }
        else
        {
            return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
        }
    }

    // Modeled after the exponential function y = 2^(10(x - 1))
    static float ExponentialEaseIn(float p)
    {
        return (p == 0.0) ? p : pow(2, 10 * (p - 1));
    }

    // Modeled after the exponential function y = -2^(-10x) + 1
    static float ExponentialEaseOut(float p)
    {
        return (p == 1.0) ? p : 1 - pow(2, -10 * p);
    }

    // Modeled after the piecewise exponential
    // y = (1/2)2^(10(2x - 1))         ; [0,0.5)
    // y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
    static float ExponentialEaseInOut(float p)
    {
        if (p == 0.0 || p == 1.0)
        {
            return p;
        }
        else if (p < 0.5)
        {
            return 0.5 * pow(2, (20 * p) - 10);
        }
        return -0.5 * pow(2, (-20 * p) + 10) + 1;
    }

    // Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
    static float ElasticEaseIn(float p)
    {
        return sin(13 * M_PI_2 * p) * pow(2, 10 * (p - 1));
    }

    // Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
    static float ElasticEaseOut(float p)
    {
        return sin(-13 * M_PI_2 * (p + 1)) * pow(2, -10 * p) + 1;
    }

    // Modeled after the piecewise exponentially-damped sine wave:
    // y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
    // y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
    static float ElasticEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * sin(13 * M_PI_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
        }
        else
        {
            return 0.5 * (sin(-13 * M_PI_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
        }
    }

    // Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
    static float BackEaseIn(float p)
    {
        return p * p * p - p * sin(p * M_PI);
    }

    // Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
    static float BackEaseOut(float p)
    {
        float f = (1 - p);
        return 1 - (f * f * f - f * sin(f * M_PI));
    }

    // Modeled after the piecewise overshooting cubic function:
    // y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
    // y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
    static float BackEaseInOut(float p)
    {
        if (p < 0.5)
        {
            float f = 2 * p;
            return 0.5 * (f * f * f - f * sin(f * M_PI));
        }
        else
        {
            float f = (1 - (2*p - 1));
            return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
        }
    }

    static float BounceEaseIn(float p)
    {
        return 1 - BounceEaseOut(1 - p);
    }

    static float BounceEaseOut(float p)
    {
        if (p < 4/11.0)
        {
            return (121 * p * p)/16.0;
        }
        else if (p < 8/11.0)
        {
            return (363/40.0 * p * p) - (99/10.0 * p) + 17/5.0;
        }
        else if (p < 9/10.0)
        {
            return (4356/361.0 * p * p) - (35442/1805.0 * p) + 16061/1805.0;
        }
        return (54/5.0 * p * p) - (513/25.0 * p) + 268/25.0;
    }

    static float BounceEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * BounceEaseIn(p*2);
        }
        else
        {
            return 0.5 * BounceEaseOut(p * 2 - 1) + 0.5;
        }
    }

    static Method getEasingMethod(uint8_t i)
    {
        switch (i)
        {
            case kLinearInterpolation:
                return LinearInterpolation;
            case kQuadraticEaseIn:
                return QuadraticEaseIn;
            case kQuadraticEaseOut:
                return QuadraticEaseOut;
            case kQuadraticEaseInOut:
                return QuadraticEaseInOut;
            case kCubicEaseIn:
                return CubicEaseIn;
            case kCubicEaseOut:
                return CubicEaseOut;
            case kCubicEaseInOut:
                return CubicEaseInOut;
            case kQuarticEaseIn:
                return QuarticEaseIn;
            case kQuarticEaseOut:
                return QuarticEaseOut;
            case kQuarticEaseInOut:
                return QuarticEaseInOut;
            case kQuinticEaseIn:
                return QuinticEaseIn;
            case kQuinticEaseOut:
                return QuinticEaseOut;
            case kQuinticEaseInOut:
                return QuinticEaseInOut;
            case kSineEaseIn:
                return SineEaseIn;
            case kSineEaseOut:
                return SineEaseOut;
            case kSineEaseInOut:
                return SineEaseInOut;
            case kCircularEaseIn:
                return CircularEaseIn;
            case kCircularEaseOut:
                return CircularEaseOut;
            case kCircularEaseInOut:
                return CircularEaseInOut;
            case kExponentialEaseIn:
                return ExponentialEaseIn;
            case kExponentialEaseOut:
                return ExponentialEaseOut;
            case kExponentialEaseInOut:
                return ExponentialEaseInOut;
            case kElasticEaseIn:
                return ElasticEaseIn;
            case kElasticEaseOut:
                return ElasticEaseOut;
            case kElasticEaseInOut:
                return ElasticEaseInOut;
            case kBackEaseIn:
                return BackEaseIn;
            case kBackEaseOut:
                return BackEaseOut;
            case kBackEaseInOut:
                return BackEaseInOut;
            case kBounceEaseIn:
                return BounceEaseIn;
            case kBounceEaseOut:
                return BounceEaseOut;
            case kBounceEaseInOut:
                return BounceEaseInOut;
        }
        return NULL;
    }
};

#endif
