/*
 * Derivative work based on FFmpeg.
 *
 * CTFFT is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * CTFFT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with CTFFT; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef CTFFT_MINIMAL
 #if defined(ARDUINO) && !defined(ESP32)
  #define CTFFT_MINIMAL 1
  #ifndef CTFFT_MAXBITS
   #define CTFFT_MAXBITS 8
  #endif
  #ifndef PROGMEM
   #define PROGMEM
  #else
   #define READ_COS(x) pgm_read_float(&(x))
  #endif
 #endif
 #ifndef CTFFT_MINIMAL
  #define CTFFT_MINIMAL 0
 #endif
#endif

#ifndef READ_COS
 #define READ_COS(x) x
#endif

#ifndef HAVE_TYPE_TRAITS
 #ifndef CTFFT_MINIMAL
  #define HAVE_TYPE_TRAITS 1
 #endif
#endif

#if HAVE_TYPE_TRAITS
 #include <type_traits>
#endif

#if defined(__GNUC__) && defined(ESP32)
 #if !__GNUC_PREREQ(8,4)
  #define GCC_OPTIMIZER_BUG
 #endif
#endif

#include <math.h>

#ifndef CTFFT_COMPILETIME
 #if defined(__GNUC__) && __cplusplus >= 201300
  #define CTFFT_COMPILETIME
 #endif
#endif

#ifndef CTFFT_MAXBITS
// Setting CTFFT_MAXBITS to higher than 19 will increase compilation time.
// You may also need to increase the constexpr loop limit -fconstexpr-loop-limit=<value>
// MAXBITS==19 : 5 seconds
// MAXBITS==20 : 16 seconds
// MAXBITS==21 : 56 seconds
// MAXBITS==22 : 5 minutes
//  ....
#define CTFFT_MAXBITS 19
#endif

namespace CTFFT
{
constexpr long double pi = 3.1415926535897932384626433832795028841972L;
constexpr long double sqrt1_2 = 1.4142135623730950488016887242096980785697L/2;

constexpr unsigned log2(unsigned x)
{
    return (x == 1) ? 0 : log2(x >> 1) + 1;
}

#ifdef CTFFT_COMPILETIME
 #define CTFFT_COSTABLE_CONSTRUCTOR constexpr Data():sample
 #define CTFFT_COSTABLE_CONSTEXPR   constexpr
#else
 #define CTFFT_COSTABLE_CONSTRUCTOR Data
 #define CTFFT_COSTABLE_CONSTEXPR
#endif

template<typename T, unsigned nbits>
struct CosTable
{
#if HAVE_TYPE_TRAITS
    static_assert(std::is_floating_point<T>(), "Only floating point supported");
#endif
    static_assert(nbits >= 4 && nbits <= CTFFT_MAXBITS, "Number of bits not supported");
#if !CTFFT_MINIMAL
    template<unsigned NUMBITS>
    struct Data
    {
        static constexpr unsigned size = 1<<NUMBITS;
        T sample[size];

        CTFFT_COSTABLE_CONSTRUCTOR()
        {
            constexpr long double freq = 2*pi/size;
            for (unsigned i = 0; i <= size / 4; i++)
                sample[i] = cos(i*freq);
            for (unsigned i = 1; i < size / 4; i++)
                sample[size / 2 - i] = sample[i];
        }
    };
#endif

    static const T* get(unsigned i)
    {
    #if CTFFT_MINIMAL
    #if CTFFT_MAXBITS >= 4
        static const T costable4[16] PROGMEM = {
            1, 0.92388, 0.707107, 0.382683, -2.50828e-20, 0.382683, 0.707107, 0.92388
        };
    #endif
    #if CTFFT_MAXBITS >= 5
        static const T costable5[32] PROGMEM = {
            1, 0.980785, 0.92388, 0.83147, 0.707107, 0.55557, 0.382683, 0.19509, -2.50828e-20,
            0.19509, 0.382683, 0.55557, 0.707107, 0.83147, 0.92388, 0.980785
        };
    #endif
    #if CTFFT_MAXBITS >= 6
        static const T costable6[64] PROGMEM = {
            1, 0.995185, 0.980785, 0.95694, 0.92388, 0.881921, 0.83147, 0.77301, 0.707107,
            0.634393, 0.55557, 0.471397, 0.382683, 0.290285, 0.19509, 0.0980171, -2.50828e-20,
            0.0980171, 0.19509, 0.290285, 0.382683, 0.471397, 0.55557, 0.634393, 0.707107,
            0.77301, 0.83147, 0.881921, 0.92388, 0.95694, 0.980785, 0.995185
        };
    #endif
    #if CTFFT_MAXBITS >= 7
        static const T costable7[128] PROGMEM = {
            1, 0.998795, 0.995185, 0.989177, 0.980785, 0.970031, 0.95694, 0.941544, 0.92388,
            0.903989, 0.881921, 0.857729, 0.83147, 0.803208, 0.77301, 0.740951, 0.707107,
            0.671559, 0.634393, 0.595699, 0.55557, 0.514103, 0.471397, 0.427555, 0.382683,
            0.33689, 0.290285, 0.24298, 0.19509, 0.14673, 0.0980171, 0.0490677, -2.50828e-20,
            0.0490677, 0.0980171, 0.14673, 0.19509, 0.24298, 0.290285, 0.33689, 0.382683,
            0.427555, 0.471397, 0.514103, 0.55557, 0.595699, 0.634393, 0.671559, 0.707107,
            0.740951, 0.77301, 0.803208, 0.83147, 0.857729, 0.881921, 0.903989, 0.92388,
            0.941544, 0.95694, 0.970031, 0.980785, 0.989177, 0.995185, 0.998795
        };
    #endif
    #if CTFFT_MAXBITS >= 8
        static const T costable8[256] PROGMEM = {
            1, 0.999699, 0.998795, 0.99729, 0.995185, 0.99248, 0.989177, 0.985278, 0.980785,
            0.975702, 0.970031, 0.963776, 0.95694, 0.949528, 0.941544, 0.932993, 0.92388,
            0.91421, 0.903989, 0.893224, 0.881921, 0.870087, 0.857729, 0.844854, 0.83147,
            0.817585, 0.803208, 0.788346, 0.77301, 0.757209, 0.740951, 0.724247, 0.707107,
            0.689541, 0.671559, 0.653173, 0.634393, 0.615232, 0.595699, 0.575808, 0.55557,
            0.534998, 0.514103, 0.492898, 0.471397, 0.449611, 0.427555, 0.405241, 0.382683,
            0.359895, 0.33689, 0.313682, 0.290285, 0.266713, 0.24298, 0.219101, 0.19509,
            0.170962, 0.14673, 0.122411, 0.0980171, 0.0735646, 0.0490677, 0.0245412,
            -2.50828e-20, 0.0245412, 0.0490677, 0.0735646, 0.0980171, 0.122411, 0.14673,
            0.170962, 0.19509, 0.219101, 0.24298, 0.266713, 0.290285, 0.313682, 0.33689,
            0.359895, 0.382683, 0.405241, 0.427555, 0.449611, 0.471397, 0.492898, 0.514103,
            0.534998, 0.55557, 0.575808, 0.595699, 0.615232, 0.634393, 0.653173, 0.671559,
            0.689541, 0.707107, 0.724247, 0.740951, 0.757209, 0.77301, 0.788346, 0.803208,
            0.817585, 0.83147, 0.844854, 0.857729, 0.870087, 0.881921, 0.893224, 0.903989,
            0.91421, 0.92388, 0.932993, 0.941544, 0.949528, 0.95694, 0.963776, 0.970031,
            0.975702, 0.980785, 0.985278, 0.989177, 0.99248, 0.995185, 0.99729, 0.998795, 0.999699
        };
    #endif
        switch (i)
        {
        #if CTFFT_MAXBITS >= 4
            case 4:
                return costable4;
        #endif
        #if CTFFT_MAXBITS >= 5
            case 5:
                return costable5;
        #endif
        #if CTFFT_MAXBITS >= 6
            case 6:
                return costable6;
        #endif
        #if CTFFT_MAXBITS >= 7
            case 7:
                return costable7;
        #endif
        #if CTFFT_MAXBITS >= 8
            case 8:
                return costable8;
        #endif
            default:
                return NULL;
        }
    #else
        static const T* tables[nbits-3];
        if (i-4 >= sizeof(tables)/sizeof(tables[0]))
            return NULL;
    #define DECLARE_COSTABLE(nb) \
        if (nbits >= nb && !tables[nb-4]) \
        { \
            static const CTFFT_COSTABLE_CONSTEXPR Data<nb> table_ = Data<nb>(); \
            tables[nb-4] = table_.sample; \
        }
        DECLARE_COSTABLE(4)
    #if CTFFT_MAXBITS >= 5
        DECLARE_COSTABLE(5)
    #endif
    #if CTFFT_MAXBITS >= 7
        DECLARE_COSTABLE(6)
    #endif
    #if CTFFT_MAXBITS >= 7
        DECLARE_COSTABLE(7)
    #endif
    #if CTFFT_MAXBITS >= 8
        DECLARE_COSTABLE(8)
    #endif
    #if CTFFT_MAXBITS >= 9
        DECLARE_COSTABLE(9)
    #endif
    #if CTFFT_MAXBITS >= 10
        DECLARE_COSTABLE(10)
    #endif
    #if CTFFT_MAXBITS >= 11
        DECLARE_COSTABLE(11)
    #endif
    #if CTFFT_MAXBITS >= 12
        DECLARE_COSTABLE(12)
    #endif
    #if CTFFT_MAXBITS >= 13
        DECLARE_COSTABLE(13)
    #endif
    #if CTFFT_MAXBITS >= 14
        DECLARE_COSTABLE(14)
    #endif
    #if CTFFT_MAXBITS >= 15
        DECLARE_COSTABLE(15)
    #endif
    #if CTFFT_MAXBITS >= 16
        DECLARE_COSTABLE(16)
    #endif
    #if CTFFT_MAXBITS >= 17
        DECLARE_COSTABLE(17)
    #endif
    #if CTFFT_MAXBITS >= 18
        DECLARE_COSTABLE(18)
    #endif
    #if CTFFT_MAXBITS >= 19
        DECLARE_COSTABLE(19)
    #endif
    #if CTFFT_MAXBITS >= 20
        DECLARE_COSTABLE(20)
    #endif
    #if CTFFT_MAXBITS >= 21
        DECLARE_COSTABLE(21)
    #endif
    #if CTFFT_MAXBITS >= 22
        DECLARE_COSTABLE(22)
    #endif
    #if CTFFT_MAXBITS >= 23
        DECLARE_COSTABLE(23)
    #endif
    #if CTFFT_MAXBITS >= 24
        DECLARE_COSTABLE(24)
    #endif
    #undef DECLARE_COSTABLE
        return tables[i-4];
    #endif
    }
};

#undef CTFFT_COSTABLE_CONSTRUCTOR
#undef CTFFT_COSTABLE_CONSTEXPR

template<typename T>
struct Complex
{
    T real;
    T img;
};

template<typename T, unsigned SIZE, typename CT>
class FourierTransform
{
public:
    FourierTransform(bool inverseTransform = false) :
        inverse(inverseTransform)
    {
        unsigned n = 1 << nbits;
        for (unsigned i = 0; i < n; i++)
            rev[-split_radix_permutation(i, n, inverse) & (n - 1)] = i;
    }

    void permute(Complex<T>* data)
    {
        unsigned np = 1 << nbits;
        for (unsigned i = 0; i < np; i++)
            tmp[rev[i]] = data[i];
        for (unsigned i = 0; i < np; i++)
            data[i] = tmp[i];
    }

    void calculate(Complex<T>* data)
    {
        dispatch(nbits)(data, nbits);
    }

private:
    typedef void (*FFT)(Complex<T>* data, unsigned nbits);
    static constexpr int nbits = log2(SIZE);
    int inverse;
    int mdct_size;
    int mdct_bits;
    unsigned rev[SIZE];
    Complex<T> tmp[SIZE];
    const T* tcos = CT::get(nbits);
    const T* tsin = tcos + (SIZE >> 2);

    static int split_radix_permutation(int i, int n, int inverse)
    {
        int m;
        if (n <= 2)
            return i & 1;
        m = n >> 1;
        if (!(i&m))
            return split_radix_permutation(i, m, inverse) * 2;
        m >>= 1;
        if (inverse == !(i&m))
            return split_radix_permutation(i, m, inverse) * 4 + 1;
        return split_radix_permutation(i, m, inverse) * 4 - 1;
    }

    #define BF(x, y, a, b) do {                     \
            x = a - b;                              \
            y = a + b;                              \
        } while (0)

    #define CMUL(dre, dim, are, aim, bre, bim) do { \
            (dre) = (are) * (bre) - (aim) * (bim);  \
            (dim) = (are) * (bim) + (aim) * (bre);  \
        } while (0)

    #define BUTTERFLIES(a0,a1,a2,a3) {\
        BF(t3, t5, t5, t1);\
        BF(a2.real, a0.real, a0.real, t5);\
        BF(a3.img, a1.img, a1.img, t3);\
        BF(t4, t6, t2, t6);\
        BF(a3.real, a1.real, a1.real, t4);\
        BF(a2.img, a0.img, a0.img, t6);\
    }

    // force loading all the inputs before storing any.
    // this is slightly slower for small data, but avoids store->load aliasing
    // for addresses separated by large powers of 2.
    #define BUTTERFLIES_BIG(a0,a1,a2,a3) {\
        T r0=a0.real, i0=a0.img, r1=a1.real, i1=a1.img;\
        BF(t3, t5, t5, t1);\
        BF(a2.real, a0.real, r0, t5);\
        BF(a3.img, a1.img, i1, t3);\
        BF(t4, t6, t2, t6);\
        BF(a3.real, a1.real, r1, t4);\
        BF(a2.img, a0.img, i0, t6);\
    }

    #define TRANSFORM(a0,a1,a2,a3,wre,wim) {\
        CMUL(t1, t2, a2.real, a2.img, wre, -wim);\
        CMUL(t5, t6, a3.real, a3.img, wre,  wim);\
        BUTTERFLIES(a0,a1,a2,a3)\
    }

    #define TRANSFORM_ZERO(a0,a1,a2,a3) {\
        t1 = a2.real;\
        t2 = a2.img;\
        t5 = a3.real;\
        t6 = a3.img;\
        BUTTERFLIES(a0,a1,a2,a3)\
    }

    static void fft4(Complex<T>* z, unsigned n)
    {
        // argument not used
        (void)n;

        T t1, t2, t3, t4, t5, t6, t7, t8;

        BF(t3, t1, z[0].real, z[1].real);
        BF(t8, t6, z[3].real, z[2].real);
        BF(z[2].real, z[0].real, t1, t6);
        BF(t4, t2, z[0].img, z[1].img);
        BF(t7, t5, z[2].img, z[3].img);
        BF(z[3].img, z[1].img, t4, t8);
        BF(z[3].real, z[1].real, t3, t7);
        BF(z[2].img, z[0].img, t2, t5);
    }

    static void fft8(Complex<T>* z, unsigned n)
    {
        // argument not used
        (void)n;

        T t1, t2, t3, t4, t5, t6;

        fft4(z, 2);

        BF(t1, z[5].real, z[4].real, -z[5].real);
        BF(t2, z[5].img, z[4].img, -z[5].img);
        BF(t5, z[7].real, z[6].real, -z[7].real);
        BF(t6, z[7].img, z[6].img, -z[7].img);

        BUTTERFLIES(z[0],z[2],z[4],z[6]);
        TRANSFORM(z[1],z[3],z[5],z[7],sqrt1_2,sqrt1_2);
    }

    static void fft16(Complex<T>* z, unsigned n)
    {
        // argument not used
        (void)n;

        T t1, t2, t3, t4, t5, t6;
        const T* cos16 = CT::get(4);
        T cos_16_1 = READ_COS(cos16[1]);
        T cos_16_3 = READ_COS(cos16[3]);

        fft8(z, 3);
        fft4(z+8, 2);
        fft4(z+12, 2);

        TRANSFORM_ZERO(z[0],z[4],z[8],z[12]);
        TRANSFORM(z[2],z[6],z[10],z[14],sqrt1_2,sqrt1_2);
        TRANSFORM(z[1],z[5],z[9],z[13],cos_16_1,cos_16_3);
        TRANSFORM(z[3],z[7],z[11],z[15],cos_16_3,cos_16_1);
    }
#ifdef GCC_OPTIMIZER_BUG
 #pragma GCC optimize ("O0")
#endif
    static void pass(Complex<T> *z, const T* wre, unsigned int n)
    {
        T t1, t2, t3, t4, t5, t6;
        int o1 = 2*n;
        int o2 = 4*n;
        int o3 = 6*n;
        const T* wim = wre+o1;

        n--;
        TRANSFORM_ZERO(z[0],z[o1],z[o2],z[o3]);
        TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],READ_COS(wre[1]),READ_COS(wim[-1]));
        do
        {
            z += 2;
            wre += 2;
            wim -= 2;
            TRANSFORM(z[0],z[o1],z[o2],z[o3],READ_COS(wre[0]),READ_COS(wim[0]));
            TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],READ_COS(wre[1]),READ_COS(wim[-1]));
        }
        while(--n);
    }
#ifdef GCC_OPTIMIZER_BUG
 #pragma GCC optimize ("Os")
#endif

#ifndef CTFFT_MINIMAL
    #undef BUTTERFLIES
    #define BUTTERFLIES BUTTERFLIES_BIG

    static void passBig(Complex<T> *z, const T* wre, unsigned int n)
    {
        T t1, t2, t3, t4, t5, t6;
        int o1 = 2*n;
        int o2 = 4*n;
        int o3 = 6*n;
        const T* wim = wre+o1;

        n--;
        TRANSFORM_ZERO(z[0],z[o1],z[o2],z[o3]);
        TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);
        do
        {
            z += 2;
            wre += 2;
            wim -= 2;
            TRANSFORM(z[0],z[o1],z[o2],z[o3],wre[0],wim[0]);
            TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);
        }
        while(--n);
    }
#endif

    static void fft(Complex<T>* z, unsigned n)
    {
        unsigned n4 = (1<<(n-2));
        dispatch(n-1)(z, n-1);
        dispatch(n-2)(z+n4*2, n-2);
        dispatch(n-2)(z+n4*3, n-2);
    #ifndef CTFFT_MINIMAL
        if (n >= 10)
            passBig(z, CT::get(n), n4/2);
        else
    #endif
            pass(z, CT::get(n), n4/2);
    }

    static FFT dispatch(unsigned n)
    {
        static FFT opt[] = {
            fft4,
            fft8,
            fft16
        };
        return (n-2 < sizeof(opt)/sizeof(opt[0])) ? opt[n-2] : fft;
    }

    #undef BF
    #undef CMUL
    #undef BUTTERFLIES
    #undef BUTTERFLIES_BIG
    #undef TRANSFORM
    #undef TRANSFORM_ZERO
};

template<typename T, unsigned SIZE>
class RealDiscrete
{
public:
    enum TransformType
    {
        kR2C,
        kC2R,
        kInverseR2C,
        kInverseC2R
    };

    RealDiscrete(TransformType transform = kR2C) :
        inverse(transform == kC2R || transform == kC2R),
        negative(transform == kC2R || transform == kR2C),
        sign_convention((transform == kInverseR2C || transform == kC2R) ? 1 : -1),
        fft(inverse)
    {
    }

    void calculate(T* data)
    {
        Complex<T> ev, od, odsum;
        const int n = 1 << nbits;
        const float k1 = 0.5;
        const float k2 = 0.5 - inverse;
        const T* tcos = this->tcos;
        const T* tsin = this->tsin;

        if (!inverse)
        {
            fft.permute((Complex<T>*)data);
            fft.calculate((Complex<T>*)data);
        }

        /* i=0 is a special case because of packing, the DC term is real, so we
           are going to throw the N/2 term (also real) in with it. */
        ev.real = data[0];
        data[0] = ev.real + data[1];
        data[1] = ev.real - data[1];

        int i;
    #define RDFT_UNMANGLE(sign0, sign1)                                              \
        for (i = 1; i < (n>>2); i++)                                                 \
        {                                                                            \
            int i1 = 2*i;                                                            \
            int i2 = n-i1;                                                           \
            /* Separate even and odd FFTs */                                         \
            ev.real =  k1*(data[i1  ]+data[i2  ]);                                   \
            od.img  =  k2*(data[i2  ]-data[i1  ]);                                   \
            ev.img  =  k1*(data[i1+1]-data[i2+1]);                                   \
            od.real =  k2*(data[i1+1]+data[i2+1]);                                   \
            /* Apply twiddle factors to the odd FFT and add to the even FFT */       \
            odsum.real = od.real* READ_COS(tcos[i]) sign0 od.img * READ_COS(tsin[i]);\
            odsum.img  = od.img * READ_COS(tcos[i]) sign1 od.real* READ_COS(tsin[i]);\
            data[i1  ] =  ev.real + odsum.real;                                      \
            data[i1+1] =  ev.img  + odsum.img;                                       \
            data[i2  ] =  ev.real - odsum.real;                                      \
            data[i2+1] =  odsum.img - ev.img;                                        \
        }

        if (negative)
        {
            RDFT_UNMANGLE(+,-)
        }
        else
        {
            RDFT_UNMANGLE(-,+)
        }
    #undef RDFT_UNMANGLE

        data[2*i+1] = sign_convention * data[2*i+1];
        if (inverse)
        {
            data[0] *= k1;
            data[1] *= k1;
            fft.permute((Complex<T>*)data);
            fft.calculate((Complex<T>*)data);
        }
    }

    static constexpr unsigned nbits = log2(SIZE);
    using costable = CosTable<T, nbits>;
    bool inverse;
    bool negative;
    int sign_convention;
    FourierTransform<T, 1<<(nbits-1), costable> fft;
    const T* tcos = costable::get(nbits);
    const T* tsin = tcos + (SIZE >> 2);
};

}
