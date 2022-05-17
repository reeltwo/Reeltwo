#ifndef MedianSampleBuffer_h
#define MedianSampleBuffer_h

/**
  * The median of N numerical values by:
  *
  *   The median of a list of N values is found by sorting the input array in increasing order, and taking the middle value.
  *   The median of a list of N values has the property that in the list there are as many greater as smaller values than this element.
  *
  * Where N is 3, 5, 6, 7, 9, or 25
  */
template <typename T, uint8_t size>
class MedianSampleBuffer
{
public:
    static_assert(size == 3 || size == 5 ||
                  size == 6 || size == 7 ||
                  size == 9 || size == 25
        , "Only 3, 5, 6, 7, 9, 25 are supported" );

    void append(T val)
    {
        for (uint8_t i = 0; i < size-1; i++)
        {
            V[i] = V[i+1];
            B[i] = V[i];
        }
        V[size-1] = B[size-1] = val;
    }

    T median()
    {
        if (size == 3)
        {
            sort(B[0], B[1]);
            sort(B[1], B[2]);
            sort(B[0], B[1]);
            return B[1];
        }
        else if (size == 5)
        {
            sort(B[0], B[1]); sort(B[3], B[4]); sort(B[0], B[3]);
            sort(B[1], B[4]); sort(B[1], B[2]); sort(B[2], B[3]);
            sort(B[1], B[2]);
            return B[2];
        }
        else if (size == 6)
        {
            sort(B[1], B[2]); sort(B[3],B[4]);
            sort(B[0], B[1]); sort(B[2],B[3]); sort(B[4],B[5]);
            sort(B[1], B[2]); sort(B[3],B[4]);
            sort(B[0], B[1]); sort(B[2],B[3]); sort(B[4],B[5]);
            sort(B[1], B[2]); sort(B[3],B[4]);
            return (B[2] + B[3]) / 2;
        }
        else if (size == 7)
        {
            sort(B[0], B[5]); sort(B[0], B[3]); sort(B[1], B[6]);
            sort(B[2], B[4]); sort(B[0], B[1]); sort(B[3], B[5]);
            sort(B[2], B[6]); sort(B[2], B[3]); sort(B[3], B[6]);
            sort(B[4], B[5]); sort(B[1], B[4]); sort(B[1], B[3]);
            sort(B[3], B[4]);
            return B[3];
        }
        else if (size == 9)
        {
            sort(B[1], B[2]); sort(B[4], B[5]); sort(B[7], B[8]);
            sort(B[0], B[1]); sort(B[3], B[4]); sort(B[6], B[7]);
            sort(B[1], B[2]); sort(B[4], B[5]); sort(B[7], B[8]);
            sort(B[0], B[3]); sort(B[5], B[8]); sort(B[4], B[7]);
            sort(B[3], B[6]); sort(B[1], B[4]); sort(B[2], B[5]);
            sort(B[4], B[7]); sort(B[4], B[2]); sort(B[6], B[4]);
            sort(B[4], B[2]);
            return B[4];
        }
        else if (size == 25)
        {
            sort(B[0], B[1]);   sort(B[3], B[4]);   sort(B[2], B[4]);
            sort(B[2], B[3]);   sort(B[6], B[7]);   sort(B[5], B[7]);
            sort(B[5], B[6]);   sort(B[9], B[10]);  sort(B[8], B[10]);
            sort(B[8], B[9]);   sort(B[12], B[13]); sort(B[11], B[13]);
            sort(B[11], B[12]); sort(B[15], B[16]); sort(B[14], B[16]);
            sort(B[14], B[15]); sort(B[18], B[19]); sort(B[17], B[19]);
            sort(B[17], B[18]); sort(B[21], B[22]); sort(B[20], B[22]);
            sort(B[20], B[21]); sort(B[23], B[24]); sort(B[2], B[5]);
            sort(B[3], B[6]);   sort(B[0], B[6]);   sort(B[0], B[3]);
            sort(B[4], B[7]);   sort(B[1], B[7]);   sort(B[1], B[4]);
            sort(B[11], B[14]); sort(B[8], B[14]);  sort(B[8], B[11]);
            sort(B[12], B[15]); sort(B[9], B[15]);  sort(B[9], B[12]);
            sort(B[13], B[16]); sort(B[10], B[16]); sort(B[10], B[13]);
            sort(B[20], B[23]); sort(B[17], B[23]); sort(B[17], B[20]);
            sort(B[21], B[24]); sort(B[18], B[24]); sort(B[18], B[21]);
            sort(B[19], B[22]); sort(B[8], B[17]);  sort(B[9], B[18]);
            sort(B[0], B[18]);  sort(B[0], B[9]);   sort(B[10], B[19]);
            sort(B[1], B[19]);  sort(B[1], B[10]);  sort(B[11], B[20]);
            sort(B[2], B[20]);  sort(B[2], B[11]);  sort(B[12], B[21]);
            sort(B[3], B[21]);  sort(B[3], B[12]);  sort(B[13], B[22]);
            sort(B[4], B[22]);  sort(B[4], B[13]);  sort(B[14], B[23]);
            sort(B[5], B[23]);  sort(B[5], B[14]);  sort(B[15], B[24]);
            sort(B[6], B[24]);  sort(B[6], B[15]);  sort(B[7], B[16]);
            sort(B[7], B[19]);  sort(B[13], B[21]); sort(B[15], B[23]);
            sort(B[7], B[13]);  sort(B[7], B[15]);  sort(B[1], B[9]);
            sort(B[3], B[11]);  sort(B[5], B[17]);  sort(B[11], B[17]);
            sort(B[9], B[17]);  sort(B[4], B[10]);  sort(B[6], B[12]);
            sort(B[7], B[14]);  sort(B[4], B[6]);   sort(B[4], B[7]);
            sort(B[12], B[14]); sort(B[10], B[14]); sort(B[6], B[7]);
            sort(B[10], B[12]); sort(B[6], B[10]);  sort(B[6], B[17]);
            sort(B[12], B[17]); sort(B[7], B[17]);  sort(B[7], B[10]);
            sort(B[12], B[18]); sort(B[7], B[12]);  sort(B[10], B[18]);
            sort(B[12], B[20]); sort(B[10], B[20]); sort(B[10], B[12]);
            return B[12];
        }
        // Not supported
        return 0;
    }

private:
    T V[size] = { 0 };
    T B[size] = { 0 };

    constexpr void sort(T& a, T& b)
    {
        if ((a)>(b))
        {
            T t = a;
            a = b;
            b = t;
        }
    }
};
#endif
