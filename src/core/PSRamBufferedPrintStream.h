#ifndef PSRamBufferedPrintStream_h
#define PSRamBufferedPrintStream_h

#pragma once

class RamBufferedPrintStream : public Print
{
public:
    RamBufferedPrintStream(Stream& stream, size_t siz = 4096) :
        fStream(stream),
        fSize(siz)
    {
        fPtr = fBuffer = (uint8_t*)malloc(fSize);
        fEnd = fPtr + fSize;
    }

    void flush()
    {
        if (fPtr - fBuffer > 0)
        {
            fStream.write(fBuffer, fPtr - fBuffer);
            fPtr = fBuffer;
        }
    }

    virtual ~RamBufferedPrintStream()
    {
        flush();
        if (fBuffer != NULL)
            free(fBuffer);
    }

    virtual size_t write(uint8_t ch) override
    {
        return write(&ch, 1);
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        if (fPtr + size >= fEnd)
        {
            flush();
        }
        if (fPtr + size < fEnd)
        {
            memcpy(fPtr, buffer, size);
            fPtr += size;
        }
        else
        {
            fStream.write(buffer, size);
        }
        return size;
    }

private:
    Stream& fStream;
    uint8_t* fBuffer;
    uint8_t* fEnd;
    uint8_t* fPtr;
    size_t fSize;
};

class PSRamBufferedPrintStream : public Print
{
public:
    PSRamBufferedPrintStream(Stream& stream, size_t siz = 64768) :
        fStream(stream),
        fSize(siz)
    {
        fPtr = fBuffer = (uint8_t*)ps_malloc(fSize);
        fEnd = fPtr + fSize;
    }

    void flush()
    {
        if (fPtr - fBuffer > 0)
        {
            fStream.write(fBuffer, fPtr - fBuffer);
            fPtr = fBuffer;
        }
    }

    virtual ~PSRamBufferedPrintStream()
    {
        flush();
        if (fBuffer != NULL)
            free(fBuffer);
    }

    virtual size_t write(uint8_t ch) override
    {
        return write(&ch, 1);
    }

    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        if (fPtr + size >= fEnd)
        {
            flush();
        }
        if (fPtr + size < fEnd)
        {
            memcpy(fPtr, buffer, size);
            fPtr += size;
        }
        else
        {
            fStream.write(buffer, size);
        }
        return size;
    }

private:
    Stream& fStream;
    uint8_t* fBuffer;
    uint8_t* fEnd;
    uint8_t* fPtr;
    size_t fSize;
};

#endif
