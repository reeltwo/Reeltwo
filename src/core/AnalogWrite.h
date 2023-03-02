#ifndef AnalogWrite_h
#define AnalogWrite_h

#include "ReelTwo.h"

#ifdef ESP32
struct analog_write_channel
{
    int8_t pin;
    uint32_t frequency;
    uint8_t resolution;
};

#define ANALOG_WRITE_DEFAULT_FREQUENCY 1000
#define ANALOG_WRITE_DEFAULT_RESOLUTION 8

analog_write_channel _analog_write_channels[LEDC_CHANNELS];
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13},
//     {-1, 5000, 13}
// };

int analogWriteChannel(uint8_t pin)
{
    int channelNum = -1;

    // Check if pin already attached to a channel
    for (uint8_t i = 0; i < SizeOfArray(_analog_write_channels); i++)
    {
        if (_analog_write_channels[i].pin == pin)
        {
            channelNum = i;
            break;
        }
    }

    // If not, attach it to a free channel
    if (channelNum == -1)
    {
        for (int i = SizeOfArray(_analog_write_channels)-1; i >= 0; i--)
        {
            analog_write_channel& channel = _analog_write_channels[i];
            if (channel.pin == 0)
            {
                channel.pin = pin;
                channelNum = i;
                // Setup default values
                if (channel.frequency == 0 && channel.resolution == 0)
                {
                    channel.frequency = ANALOG_WRITE_DEFAULT_FREQUENCY;
                    channel.resolution = ANALOG_WRITE_DEFAULT_RESOLUTION;
                }
                // printf("ATTACH PIN=%d to CHANNEL=%d frequency=%u bits=%d\n", pin, channelNum, channel.frequency, channel.resolution);
                ledcAttachPin(pin, channelNum);
                ledcSetup(channelNum, ANALOG_WRITE_DEFAULT_FREQUENCY, ANALOG_WRITE_DEFAULT_RESOLUTION);
                if (ledcChangeFrequency(i, channel.frequency, channel.resolution) != channel.frequency)
                {
                    // Failed to change frequency revert to known defaults
                    channel.frequency = ANALOG_WRITE_DEFAULT_FREQUENCY;
                    channel.resolution = ANALOG_WRITE_DEFAULT_RESOLUTION;
                }
                break;
            }
        }
    }
    return channelNum;
}

void analogWriteFrequencyResolution(uint32_t frequency, uint8_t resolution = 8)
{
    for (uint8_t i = 0; i < SizeOfArray(_analog_write_channels); i++)
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[i];
        if (channel.pin != 0)
            updateChannel = (ledcChangeFrequency(i, frequency, resolution) == frequency);
        if (updateChannel)
        {
            channel.frequency = frequency;
            channel.resolution = resolution;
        }
    }
}

bool analogWriteFrequencyResolution(uint8_t pin, uint32_t frequency, uint8_t resolution = 8)
{
    unsigned channelNum = analogWriteChannel(pin);

    // Make sure the pin was attached to a channel, if not do nothing
    if (channelNum < SizeOfArray(_analog_write_channels))
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[channelNum];
        if (channel.pin != 0)
        {
            // uint32_t freq = ledcChangeFrequency(channelNum, frequency, resolution);
            // printf("freq: %u\n", freq);
            updateChannel = (ledcChangeFrequency(channelNum, frequency, resolution) == frequency);
        }
        if (updateChannel)
        {
            channel.frequency = frequency;
            channel.resolution = resolution;
            return true;
        }
    }
    return false;
}

void analogWriteFrequency(uint32_t frequency)
{
    for (uint8_t i = 0; i < SizeOfArray(_analog_write_channels); i++)
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[i];
        if (channel.pin != 0)
            updateChannel = (ledcChangeFrequency(i, frequency, channel.resolution) == frequency);
        if (updateChannel)
            channel.frequency = frequency;
    }
}

void analogWriteFrequency(uint8_t pin, uint32_t frequency)
{
    unsigned channelNum = analogWriteChannel(pin);

    // Make sure the pin was attached to a channel, if not do nothing
    if (channelNum < SizeOfArray(_analog_write_channels))
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[channelNum];
        if (channel.pin != 0)
            updateChannel = (ledcChangeFrequency(channelNum, frequency, channel.resolution) == frequency);
        if (updateChannel)
            channel.frequency = frequency;
    }
}

void analogWriteResolution(uint8_t resolution)
{
    for (uint8_t i = 0; i < SizeOfArray(_analog_write_channels); i++)
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[i];
        if (channel.pin != 0)
            updateChannel = (ledcChangeFrequency(i, channel.frequency, resolution) == channel.frequency);
        if (updateChannel)
            channel.resolution = resolution;
    }
}

void analogWriteResolution(uint8_t pin, uint8_t resolution)
{
    unsigned channelNum = analogWriteChannel(pin);

    // Make sure the pin was attached to a channel, if not do nothing
    if (channelNum < SizeOfArray(_analog_write_channels))
    {
        bool updateChannel = true;
        analog_write_channel& channel = _analog_write_channels[channelNum];
        if (channel.pin != 0)
            updateChannel = (ledcChangeFrequency(channelNum, channel.frequency, resolution) == channel.frequency);
        if (updateChannel)
            channel.resolution = resolution;
    }
}

void analogWrite(uint8_t pin, uint32_t value, uint32_t valueMax)
{
    int channel = analogWriteChannel(pin);

    // Make sure the pin was attached to a channel, if not do nothing
    if (channel != -1 && channel < SizeOfArray(_analog_write_channels))
    {
        uint8_t resolution = _analog_write_channels[channel].resolution;
        uint32_t levels = pow(2, resolution);
        uint32_t duty = ((levels - 1) / valueMax) * min(value, valueMax);

        // write duty to LEDC
        // printf("ledcWrite pin=%d duty=%u levels=%u\n", pin, duty, levels);
        ledcWrite(channel, duty);
    }
}

void analogWrite(uint8_t pin, float value)
{
    value = min(max(0.0f, value), 1.0f);
    int channel = analogWriteChannel(pin);

    // Make sure the pin was attached to a channel, if not do nothing
    if (channel != -1 && channel < SizeOfArray(_analog_write_channels))
    {
        uint8_t resolution = _analog_write_channels[channel].resolution;
        uint32_t levels = pow(2, resolution);
        uint32_t duty = (levels - 1) * value;

        // write duty to LEDC
        // printf("ledcWrite pin=%d duty=%u levels=%u\n", pin, duty, levels);
        ledcWrite(channel, duty);
    }
}
#endif

#endif