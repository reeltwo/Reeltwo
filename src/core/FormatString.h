#ifndef FormatString_h
#define FormatString_h

#pragma once

#define FL_LEFTADJUST 0x0001U
#define FL_SIGN 0x0002U
#define FL_ZEROPAD 0x0004U
#define FL_ALTERNATE 0x0008U
#define FL_SPACE 0x0010U
#define FL_SHORT 0x0020U
#define FL_LONG 0x0040U
#define FL_LONGDOUBLE 0x0080U
#define FL_POINTER 0x0100U

#define MAX_WIDTH ((SHRT_MAX - 9) / 10)

#ifndef DBL_DIG
#define DBL_DIG 15
#endif
#define BUFFER_LEN 1000

#define PUTSTR(szPutStr, iPutLen)                                               \
    {                                                                           \
        if (iPutLen > 0)                                                        \
            if (iOutFoo(pvOutData, (const char*)szPutStr, (size_t)iPutLen) < 0) \
                return -1;                                                      \
        iOutCount += iPutLen;                                                   \
    }

#define PAD(szPadStr, iPadLen)                                                   \
    {                                                                            \
        if (iPadLen > 0)                                                         \
        {                                                                        \
            iTmp = iPadLen;                                                      \
            while (iTmp >= 32)                                                   \
            {                                                                    \
                if (iOutFoo(pvOutData, (const char*)szPadStr, 32) < 0)           \
                    return -1;                                                   \
                iOutCount += 32;                                                 \
                iTmp -= 32;                                                      \
            }                                                                    \
            if (iTmp)                                                            \
            {                                                                    \
                if (iOutFoo(pvOutData, (const char*)szPadStr, (size_t)iTmp) < 0) \
                    return -1;                                                   \
                iOutCount += iTmp;                                               \
            }                                                                    \
        }                                                                        \
    }

#define PADSPACES(iPadWidth) PAD(szSpaces, iPadWidth)
#define PADZEROS(iPadWidth) PAD(szZeros, iPadWidth)

static const char szSpaces[] = "                                ";
static const char szZeros[]  = "00000000000000000000000000000000";

/// \private
struct SSnprintf
{
    char* cpBuf;
    size_t tRoomFor;
    char* cpBuffer;
    size_t tBufSize;
    bool tGrow;
};

#ifndef OUTFMT_NO_FLOAT
/// \private
union UIEEE_754
{
    unsigned long aul[2];
    double dVal;
};

static const int iEndianChk = 1;

#define iIsBigEndian() (*(char*)&iEndianChk == 0)

#define iIsNanOrInf(puVal)                                                \
    (/*lint -save -e506 */ /* Constant boolean value */                   \
                sizeof((*puVal).dVal) == sizeof((*puVal).aul)             \
            ? /* We assume 64-bits double in IEEE 754 format */           \
            iIsBigEndian()                                                \
                ? /* Big endian */                                        \
                (((*puVal).aul[0] & 0x7ff00000UL) == 0x7ff00000UL)        \
                    ? /* Inf or Nan */                                    \
                    (((*puVal).aul[0] & 0x000fffffUL) || (*puVal).aul[1]) \
                        ? /* Nan */                                       \
                        ((*puVal).aul[0] & 0x80000000UL)                  \
                            ? 1 /* -Nan  */                               \
                            : 2 /*  Nan  */                               \
                        : /* Inf */                                       \
                        ((*puVal).aul[0] & 0x80000000UL)                  \
                            ? 3 /* -Inf  */                               \
                            : 4 /*  Inf  */                               \
                    : 0                                                   \
                : /* Little endian */                                     \
                (((*puVal).aul[1] & 0x7ff00000UL) == 0x7ff00000UL)        \
                    ? /* Inf or Nan */                                    \
                    (((*puVal).aul[1] & 0x000fffffUL) || (*puVal).aul[0]) \
                        ? /* Nan */                                       \
                        ((*puVal).aul[1] & 0x80000000UL)                  \
                            ? 1 /* -Nan  */                               \
                            : 2 /*  Nan  */                               \
                        : /* Inf */                                       \
                        ((*puVal).aul[1] & 0x80000000UL)                  \
                            ? 3 /* -Inf  */                               \
                            : 4 /*  Inf  */                               \
                    : 0                                                   \
            : 0 /*lint -restore */                                        \
        )
#endif

/// \private
static int _vsOutFmt(int (*iOutFoo)(void* pvOutData, const char*, size_t),
    void* pvOutData,
    const char* szFmt,
    va_list tArg)
{
    int iOutCount = 0;
    while (*szFmt)
    {
        unsigned int uiFlags;
        int iWidth, iPrec, iPrefixLen, iLeadingZeros, iTmp;
        int iStrLen;
        const char* szStr;
        const char* szPrefix;
        unsigned char aucBuffer[BUFFER_LEN];

        if (*szFmt != '%')
        {
            szStr = szFmt++;

            while (*szFmt != '\0' && *szFmt != '%')
                ++szFmt;
            iStrLen = (int)(szFmt - szStr);
            PUTSTR(szStr, iStrLen);
            if (*szFmt == '\0')
                break;
        }
        szPrefix = "";
        iStrLen = iPrefixLen = 0;
        uiFlags = 0;
        ++szFmt;
        while ((*szFmt == '-') || (*szFmt == '+') || (*szFmt == '0') || (*szFmt == '#') || (*szFmt == ' '))
        {
            switch (*szFmt++)
            {
                case '-':
                    uiFlags |= FL_LEFTADJUST;
                    break;
                case '+':
                    uiFlags |= FL_SIGN;
                    break;
                case '0':
                    uiFlags |= FL_ZEROPAD;
                    break;
                case '#':
                    uiFlags |= FL_ALTERNATE;
                    break;
                case ' ':
                    uiFlags |= FL_SPACE;
                    break;
                default:
                    break;
            }
        }

        /* Get width */
        if (*szFmt == '*')
        {
            ++szFmt;
            iWidth = va_arg(tArg, int);
            if (iWidth < 0)
            {
                iWidth = -iWidth;
                uiFlags |= FL_LEFTADJUST;
            }
        }
        else
        {
            for (iWidth = 0; (*szFmt >= '0') && (*szFmt <= '9'); szFmt++)
            {
                if (iWidth < MAX_WIDTH)
                    iWidth = iWidth * 10 + (*szFmt - '0');
            }
        }

        /* Get precision */
        if (*szFmt == '.')
        {
            if (*++szFmt == '*')
            {
                ++szFmt;
                iPrec = va_arg(tArg, int);
                if (iPrec < 0)
                    iPrec = 0;
            }
            else
            {
                for (iPrec = 0; (*szFmt >= '0') && (*szFmt <= '9'); szFmt++)
                {
                    iPrec = iPrec * 10 + (*szFmt - '0');
                }
            }
        }
        else
            iPrec = -1;

        /* Get size */
        switch (*szFmt)
        {
            case 'h':
                uiFlags |= FL_SHORT;
                ++szFmt;
                break;
            case 'l':
                uiFlags |= FL_LONG;
                ++szFmt;
                break;
            case 'L':
                uiFlags |= FL_LONGDOUBLE;
                ++szFmt;
                break;
            default:
                ;
        }

        switch (*szFmt)
        {
            case 'p':
                uiFlags |= FL_POINTER;
                /* Default %p formatting to 0x%08X on 32-bit platforms and 0x%016X on 64-bit platforms */
                uiFlags |= FL_ZEROPAD;
                iWidth = sizeof(void*) * 2;
            /*FALLTHROUGH*/
            case 'x':
            case 'X': /*lint !e616 */
            case 'o':
            case 'u':
            case 'd':
            case 'i':
            {
                unsigned long ulValue;
                char* pcChar = (char*)&aucBuffer[(sizeof aucBuffer) - 1];

                *pcChar = '\0';
                if (uiFlags & FL_POINTER)
                    ulValue = (unsigned long)va_arg(tArg, void*);
                else if (uiFlags & FL_LONG)
                    ulValue = (unsigned long)va_arg(tArg, long);
                else
                    ulValue = (unsigned long)va_arg(tArg, int);

                /* Don't print anything? */
                if (iPrec == 0 && ulValue == 0)
                {
                    szFmt++;
                    continue;
                }

                if (*szFmt == 'd' || *szFmt == 'i')
                {
                    if ((long)ulValue < 0)
                    {
                        ulValue = -(long)ulValue; /*lint !e732 */
                        szPrefix = "-";
                        iPrefixLen = 1;
                    }
                    else if (uiFlags & FL_SIGN)
                    {
                        szPrefix = "+";
                        iPrefixLen = 1;
                    }
                    else if (uiFlags & FL_SPACE)
                    {
                        szPrefix = " ";
                        iPrefixLen = 1;
                    }
                    do
                    {
                        int iChar = ulValue % 10U;
                        ulValue /= 10UL;
                        *--pcChar = (char)('0' + iChar);
                        iStrLen++;
                    } while (ulValue);
                }
                else
                {
                    const unsigned uiRadix = (*szFmt == 'u')
                        ? 10U
                        : (*szFmt == 'o') ? 8U : 16U;
                    const char* acDigits = (*szFmt == 'X')
                        ? "0123456789ABCDEF"
                        : "0123456789abcdef";

                    if (uiFlags & FL_SHORT)
                        ulValue &= USHRT_MAX;

                    do
                    {
                        *--pcChar = acDigits[ulValue % uiRadix];
                        ulValue /= uiRadix;
                        iStrLen++;
                    } while (ulValue);
                    if ((uiFlags & FL_ALTERNATE) && *pcChar != '0')
                    {
                        if (*szFmt == 'o')
                        {
                            iStrLen++;
                            *--pcChar = '0';
                        }
                        else if (*szFmt == 'X')
                        {
                            szPrefix = "0X";
                            iPrefixLen = 2;
                        }
                        else if (*szFmt == 'x')
                        {
                            szPrefix = "0x";
                            iPrefixLen = 2;
                        }
                    }
                }
                szStr = pcChar;
            }

                /* Calc number of leading zeros */
                iLeadingZeros = iStrLen < iPrec ? iPrec - iStrLen : 0;

                if (iPrec < 0 && ((uiFlags & (FL_ZEROPAD | FL_LEFTADJUST)) == FL_ZEROPAD))
                {
                    iTmp = iWidth - iPrefixLen - iLeadingZeros - iStrLen;
                    if (iTmp > 0)
                        iLeadingZeros += iTmp;
                }

                iWidth -= iPrefixLen + iLeadingZeros + iStrLen;

                if (!(uiFlags & FL_LEFTADJUST))
                    PADSPACES(iWidth);
                PUTSTR(szPrefix, iPrefixLen);
                PADZEROS(iLeadingZeros);
                PUTSTR(szStr, iStrLen);
                if (uiFlags & FL_LEFTADJUST)
                    PADSPACES(iWidth);
                ++szFmt;
                continue;

            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
            {
#ifdef OUTFMT_NO_FLOAT
                szStr = "(no float support)";
                goto OutStr;
#else
#if 0
            double dValue = (uiFlags & FL_LONGDOUBLE)
               ? (double)va_arg(tArg, long double)
               : va_arg(tArg, double);
#else
                /* long double support */
                double dValue = va_arg(tArg, double);
#endif
                int iDecZeros, iExpZeros, iDecPoint, iFracDigs;
                int iTrailZeros, iExpLen, iExp, iSavedPrec;
                char *szFrac, *szExp;
                char cFmt = *szFmt;
                static const char* szNanInf[] = { "-NaN", "NaN",
                    "-Inf", "Inf" };

                iDecZeros = iExpZeros = iFracDigs = 0;
                iTrailZeros = iExpLen = iExp = 0;
                iSavedPrec = iPrec;
                szExp = (char*)"";

                iTmp = iIsNanOrInf((union UIEEE_754*)&dValue);
                if (iTmp)
                {
                    szStr = szNanInf[iTmp - 1];
                    goto OutStr;
                }

                if (dValue < 0.0)
                {
                    dValue = -dValue;
                    szPrefix = "-";
                    iPrefixLen = 1;
                }
                else if (uiFlags & FL_SIGN)
                {
                    szPrefix = "+";
                    iPrefixLen = 1;
                }
                else if (uiFlags & FL_SPACE)
                {
                    szPrefix = " ";
                    iPrefixLen = 1;
                }

                if (dValue >= 10.0)
                {
                    if (dValue >= 1e16)
                    {
                        while (dValue >= 1e64)
                        {
                            dValue /= 1e64;
                            iExp += 64;
                        }
                        while (dValue >= 1e32)
                        {
                            dValue /= 1e32;
                            iExp += 32;
                        }
                        while (dValue >= 1e16)
                        {
                            dValue /= 1e16;
                            iExp += 16;
                        }
                    }
                    while (dValue >= 1e08)
                    {
                        dValue /= 1e08;
                        iExp += 8;
                    }
                    while (dValue >= 1e04)
                    {
                        dValue /= 1e04;
                        iExp += 4;
                    }
                    while (dValue >= 1e01)
                    {
                        dValue /= 1e01;
                        iExp += 1;
                    }
                }
                else if (dValue < 1.0 && dValue > 0.0)
                {
                    if (dValue < 1e-15)
                    {
                        while (dValue < 1e-63)
                        {
                            dValue *= 1e64;
                            iExp -= 64;
                        }
                        while (dValue < 1e-31)
                        {
                            dValue *= 1e32;
                            iExp -= 32;
                        }
                        while (dValue < 1e-15)
                        {
                            dValue *= 1e16;
                            iExp -= 16;
                        }
                    }
                    while (dValue < 1e-7)
                    {
                        dValue *= 1e8;
                        iExp -= 8;
                    }
                    while (dValue < 1e-3)
                    {
                        dValue *= 1e4;
                        iExp -= 4;
                    }
                    while (dValue < 1e-0)
                    {
                        dValue *= 1e1;
                        iExp -= 1;
                    }
                    if (dValue >= 10.0)
                    {
                        dValue /= 1e1;
                        iExp += 1;
                    }
                }

                if (iPrec < 0)
                    iPrec = iSavedPrec = 6;

                if (cFmt == 'g' || cFmt == 'G')
                {
                    if (iExp < -4 || iExp >= iPrec)
                    {
                        cFmt = (char)(cFmt == 'g' ? 'e' : 'E');
                        --iPrec;
                    }
                    else
                    {
                        cFmt = 'f';
                        if (iPrec == 0)
                            iPrec = 1;
                        if (iExp >= 0)
                        {
                            iPrec -= iExp + 1;
                        }
                        else
                        {
                            iPrec += -iExp - 1;
                        }
                    }
                    if (iPrec < 0)
                        iPrec = 0;
                }
                szStr = (char*)aucBuffer;
                iStrLen = 0;
                if (cFmt == 'e' || cFmt == 'E')
                {
                    iTmp = (int)dValue;
                    aucBuffer[iStrLen++] = (unsigned char)(iTmp + '0');
                    dValue -= iTmp;
                    dValue *= 10.;
                }
                else if (iExp < 0)
                {
                    aucBuffer[iStrLen++] = '0';
                }
                else
                {
                    if (iExp > DBL_DIG + 1)
                    {
                        iExpZeros = iExp - (DBL_DIG + 1);
                        iExp = DBL_DIG + 1;
                    }

                    do
                    {
                        iTmp = (int)dValue;
                        aucBuffer[iStrLen++] = (unsigned char)(iTmp + '0');
                        dValue -= iTmp;
                        dValue *= 10.;
                    } while (iExp--);
                }

                szFrac = (char*)&aucBuffer[iStrLen];
                if (cFmt == 'f')
                {
                    while (iDecZeros < iPrec && iExp < -1)
                    {
                        ++iDecZeros;
                        ++iExp;
                    }
                }

                if (!iExpZeros)
                {
                    while (iFracDigs + iDecZeros < iPrec && iStrLen + iFracDigs < DBL_DIG + 1)
                    {
                        iTmp = (int)dValue;
                        szFrac[iFracDigs++] = (char)(iTmp + '0');
                        dValue -= iTmp;
                        dValue *= 10.;
                        if (cFmt == 'f')
                            --iExp;
                    }
                    if (!iFracDigs && iDecZeros)
                    {
                        szFrac[iFracDigs++] = (char)'0';
                        --iDecZeros;
                        --iExp;
                    }
                    if (dValue >= 5.0 && (iFracDigs || !iPrec))
                    {
                        iTmp = iStrLen + iFracDigs - 1;
                        while (aucBuffer[iTmp] == '9' && iTmp >= 0)
                            aucBuffer[iTmp--] = '0';
                        if (iTmp == 0 && iDecZeros)
                        {
                            for (iTmp = iStrLen + iFracDigs - 1;
                                 iTmp > 0;
                                 iTmp--)
                            {
                                aucBuffer[iTmp] = aucBuffer[iTmp - 1];
                            }
                            aucBuffer[1]++;
                            iDecZeros--;
                        }
                        else if (iTmp >= 0)
                        {
                            if (cFmt == 'f')
                            {
                                if (iPrec + 1 + iExp >= 0)
                                {
                                    aucBuffer[iTmp]++;
                                }
                            }
                            else if (iPrec == 0 || iFracDigs == iPrec)
                            {
                                aucBuffer[iTmp]++;
                            }
                        }
                        else
                        {
                            /* iTmp == -1 */
                            aucBuffer[0] = '1';
                            aucBuffer[iStrLen + iFracDigs] = '0';
                            if (cFmt == 'f')
                            {
                                if (iStrLen >= iSavedPrec && (*szFmt == 'g' || *szFmt == 'G'))
                                {
                                    cFmt = (char)(*szFmt == 'g' ? 'e' : 'E');
                                    iExp = iStrLen;
                                    iStrLen = 1;
                                }
                                else
                                    iStrLen++;
                            }
                            else
                                iExp++;
                        }
                    }
                }

                if (*szFmt == 'g' || *szFmt == 'G')
                {
                    if (uiFlags & FL_ALTERNATE)
                    {
                        iDecPoint = 1;
                    }
                    else
                    {
                        iTmp = iStrLen + iFracDigs - 1;
                        while (aucBuffer[iTmp--] == '0' && iFracDigs > 0)
                        {
                            iFracDigs--;
                        }
                        if (!iFracDigs)
                            iDecZeros = 0;
                        iTrailZeros = 0;
                        iDecPoint = iFracDigs ? 1 : 0;
                    }
                    if (iExp == 0)
                        goto SkipExp;
                }
                else
                {
                    iTrailZeros = iPrec - iFracDigs - iDecZeros;
                    if (iTrailZeros < 0)
                        iTrailZeros = 0;
                    iDecPoint = iPrec || (uiFlags & FL_ALTERNATE) ? 1 : 0;
                }

                if (cFmt == 'e' || cFmt == 'E')
                {
                    szExp = &szFrac[iFracDigs];
                    szExp[iExpLen++] = cFmt;
                    szExp[iExpLen++] = (char)((iExp < 0)
                        ? (iExp = -iExp),
                        '-'
                        : '+');
#ifndef OUTFMT_LONG_EXP
                    if (iExp >= 100)
#endif
                    {
                        szExp[iExpLen++] = (char)(iExp / 100 + '0');
                        iExp %= 100;
                    }
                    szExp[iExpLen++] = (char)(iExp / 10 + '0');
                    szExp[iExpLen++] = (char)(iExp % 10 + '0');
                }

            SkipExp:
#ifdef OUTFMT_NO_MINUS_ZERO
                if ((iPrefixLen == 1 && *szPrefix == '-') && (iStrLen == 1 && *szStr == '0') && ((iFracDigs == 1 && *szFrac == '0') || iFracDigs == 0))
                {
                    if (uiFlags & FL_SIGN)
                        szPrefix = " ";
                    else
                        iPrefixLen = 0;
                }
#endif
                if ((uiFlags & FL_ZEROPAD) && !(uiFlags & FL_LEFTADJUST))
                {
                    iLeadingZeros = iWidth - (iPrefixLen + iStrLen + iExpZeros + iDecPoint + iDecZeros + iFracDigs + iTrailZeros + iExpLen);
                    if (iLeadingZeros < 0)
                        iLeadingZeros = 0;
                }
                else
                    iLeadingZeros = 0;

                iWidth -= iPrefixLen + iLeadingZeros + iStrLen + iExpZeros + iDecPoint + iDecZeros + iFracDigs + iTrailZeros + iExpLen;

                if (!(uiFlags & FL_LEFTADJUST))
                    PADSPACES(iWidth);
                PUTSTR(szPrefix, iPrefixLen);
                PADZEROS(iLeadingZeros);
                PUTSTR(szStr, iStrLen);
                PADZEROS(iExpZeros);
                PUTSTR(".", iDecPoint);
                PADZEROS(iDecZeros);
                PUTSTR(szFrac, iFracDigs);
                PADZEROS(iTrailZeros);
                PUTSTR(szExp, iExpLen);
                if (uiFlags & FL_LEFTADJUST)
                    PADSPACES(iWidth);
                ++szFmt;
                continue;
#endif
            }
            case 'c':
                aucBuffer[0] = (unsigned char)va_arg(tArg, int);
                szStr = (char*)aucBuffer;
                iStrLen = 1;
                break;
            case 'S':
            {
                String* str = va_arg(tArg, String*);
                if (str == NULL)
                {
                    szStr = "(null)";
                }
                else
                {
                    szStr = str->c_str();
                }
                goto OutStr;
            }
            case 's':
                szStr = va_arg(tArg, char*);
                if (szStr == NULL)
                    szStr = "(null)";
            OutStr:
                iStrLen = 0;
                while (szStr[iStrLen] != '\0')
                    iStrLen++;
                if (iPrec >= 0 && iPrec < iStrLen)
                    iStrLen = iPrec;
                break;
            case '%':
                szStr = "%";
                iStrLen = 1;
                break;
            case 'n':
                if (uiFlags & FL_SHORT)
                    *va_arg(tArg, short*) = (short)iOutCount;
                else if (uiFlags & FL_LONG)
                    *va_arg(tArg, long*) = (long)iOutCount;
                else
                    *va_arg(tArg, int*) = (int)iOutCount;
                ++szFmt;
                continue;
            default:
                aucBuffer[0] = (unsigned char)*szFmt;
                szStr = (char*)aucBuffer;
                iStrLen = 1;
                break;
        }

        iWidth -= iStrLen;
        if (uiFlags & FL_LEFTADJUST)
        {
            PUTSTR(szStr, iStrLen);
            PADSPACES(iWidth);
        }
        else
        {
            PADSPACES(iWidth);
            PUTSTR(szStr, iStrLen);
        }
        ++szFmt;
    }
    if (iOutFoo(pvOutData, NULL, 0) < 0)
        return -1;
    return iOutCount;
} /* _vsOutFmt */

static bool sFormatString_init;
static bool sFormatString_PSRam;

static int
snprintfOut(void* ps, const char* szStr, size_t tLen)
{
    SSnprintf* psSnprintf = (SSnprintf*)ps;

    if (szStr == NULL && tLen == 0)
    {
        if (psSnprintf->tRoomFor)
        {
            *psSnprintf->cpBuf = '\0';
            return 0;
        }
        return -1;
    }
    else
    {
        if (psSnprintf->tGrow && psSnprintf->tRoomFor <= tLen)
        {
            size_t growsize = (tLen - psSnprintf->tRoomFor) + 64;
            psSnprintf->tBufSize += growsize;
            size_t usedsize = psSnprintf->cpBuf - psSnprintf->cpBuffer;
            psSnprintf->cpBuffer = (sFormatString_PSRam) ?
                (char*)ps_realloc(psSnprintf->cpBuffer, psSnprintf->tBufSize) :
                (char*)realloc(psSnprintf->cpBuffer, psSnprintf->tBufSize);
            psSnprintf->cpBuf = psSnprintf->cpBuffer + usedsize;
            psSnprintf->tRoomFor += growsize;
        }
        while (psSnprintf->tRoomFor > 1 && tLen > 0)
        {
            psSnprintf->tRoomFor--;
            tLen--;
            *psSnprintf->cpBuf++ = *szStr++;
        }
        if (tLen > 0)
        {
            *psSnprintf->cpBuf = '\0';
            return -1;
        }
        return 0;
    }
}

int FormatString(char** acBuf, const char* szFmt, va_list tArg)
{
    int iOutCnt;
    SSnprintf ssnprintf;

    ssnprintf.tBufSize = 64;
    if (!sFormatString_init)
    {
        sFormatString_PSRam = psramInit();
        sFormatString_init = true;
    }
    ssnprintf.cpBuf = ssnprintf.cpBuffer = (sFormatString_PSRam) ?
        (char*)ps_malloc(ssnprintf.tBufSize) : (char*)malloc(ssnprintf.tBufSize);
    ssnprintf.tRoomFor = ssnprintf.tBufSize;
    ssnprintf.tGrow = true;

    iOutCnt = _vsOutFmt(snprintfOut, (void*)&ssnprintf, szFmt, tArg);
    *acBuf = ssnprintf.cpBuffer;

    return iOutCnt;
}
#endif
