#ifndef Warbler_h

#if !defined(ESP32)
#error Only supports ESP32
#endif

#include "ReelTwo.h"
#include "core/SetupEvent.h"
#include "core/AnimatedEvent.h"
#include "audio/AudioFrequencyBitmap.h"
#include "audio/AudioPlayer.h"

//#define MIX_AUDIO_FLOAT

#ifdef USE_WARBLER_DEBUG
#define WARBLER_DEBUG_PRINT(s) DEBUG_PRINT(s)
#define WARBLER_DEBUG_PRINTLN(s) DEBUG_PRINTLN(s)
#define WARBLER_DEBUG_PRINT_HEX(s) DEBUG_PRINT_HEX(s)
#define WARBLER_DEBUG_PRINTLN_HEX(s) DEBUG_PRINTLN_HEX(s)
#else
#define WARBLER_DEBUG_PRINT(s)
#define WARBLER_DEBUG_PRINTLN(s)
#define WARBLER_DEBUG_PRINT_HEX(s)
#define WARBLER_DEBUG_PRINTLN_HEX(s)
#endif

#ifndef WARBLER_NUM_TRACKS
#define WARBLER_NUM_TRACKS 10
#endif

#ifndef WARBLER_QUEUE_DEPTH
#define WARBLER_QUEUE_DEPTH 10
#endif

#ifndef WARBLER_DECODE_BUFFER_SIZE
#define WARBLER_DECODE_BUFFER_SIZE 2048
#endif

#ifndef I2S_DOUT_PIN
#define I2S_DOUT_PIN 25  // I2S audio output
#endif

#ifndef I2S_LRC_PIN
#define I2S_LRC_PIN  26  // I2S audio output
#endif

#ifndef I2S_BCLK_PIN
#define I2S_BCLK_PIN 27  // I2S audio output
#endif

class WarblerAudio: public AnimatedEvent, public SetupEvent, protected AudioPlayer
{
public:
    WarblerAudio(fs::FS &fs, AudioFrequencyBitmap* audioBitmap = nullptr,
    			uint8_t bclk = I2S_BCLK_PIN, uint8_t lrc = I2S_LRC_PIN, uint8_t dout = I2S_DOUT_PIN) :
    	fAudioBitmap(audioBitmap),
    	fFS(fs)
    {
    	*fNextSong = '\0';
    	setPinout(bclk, lrc, dout);
    }

    void setBalance(int8_t bal = 0)
    {
    	AudioPlayer::setBalance(bal);
    }

    void setVolume(uint8_t vol)
    {
       	AudioPlayer::setVolume(vol);
    }

	inline bool isComplete() const
	{
		return fActiveRemaining == 0;
	}

    void stop()
    {
    	fStopStream = true;
    }

	virtual void setup() override
	{
	    xTaskCreatePinnedToCore(
	          audioLoopTask,
	          "WarblerAudio",
	          10000,
	          nullptr,
	          1,
	          &fAudioTask,
	          0);
	    fPOD = esp_partition_find_first((esp_partition_type_t)0xBA, (esp_partition_subtype_t)0xBE, NULL);
		if (initTracks())
		    setSampleFilter(audioFilter);
	}

	virtual void animate() override
	{
		if (fActiveRemaining != 0)
			return;
		if (fNextTrack == -1 && fNextWarble == -1 && fNumQueue > 0)
		{
			uint32_t now = millis();
			if (fQueue->fStartMS == 0)
				fQueue->fStartMS = now;
			if (fQueue->fStartMS + fQueue->fDelayMS <= now)
			{
				fNextTrack = fQueue->fTrack;
				fNextWarble = fQueue->fWarble;
				// printf("Play %d:%d\n", fNextTrack, fNextWarble);
				if (--fNumQueue > 0)
				{
					memmove(&fQueue[0], &fQueue[1], fNumQueue * sizeof(fQueue[0]));
					// printf("Remaining: %d\n", fNumQueue);
				}
			}
		}
		if (fNextTrack != -1 && fNextWarble != -1)
		{
			// DEBUG_PRINT("NEXT WARBLE ["); DEBUG_PRINT(fNextTrack);
			// DEBUG_PRINT(":"); DEBUG_PRINT(fNextWarble); DEBUG_PRINTLN("]");
			if (fPOD != nullptr && isPlaying())
			{
				static uint32_t sPODOffset[110] = {
				    64000, 87040, 131328, 176640, 209152, 303104, 357888, 418560, 439296, 512512, 555776,
				    593408, 643328, 698112, 783872, 813056, 850944, 923136, 975104, 1055744, 1090304, 1150720,
				    1304064, 1555968, 1709568, 1837568, 1912064, 2061056, 2269184, 2422016, 2546688, 2652160, 2775808,
				    2843392, 2921216, 3008768, 3069952, 3136000, 3173120, 3241728, 3321856, 3341312, 3398656, 3410944,
				    3565568, 3630848, 3755264, 3881728, 4005376, 4120320, 4211968, 4301568, 4420608, 4509440, 4586496,
				    4640000, 4764672, 4835072, 4884480, 4943616, 5020928, 5113600, 5208064, 5277184, 5344000, 5417984,
				    5560320, 5673216, 5822464, 5968384, 6058496, 6175744, 6312704, 6471936, 6581248, 6676992, 6781184,
				    6813440, 6871808, 6924544, 6967040, 7025408, 7095808, 7159040, 7203328, 7293440, 7323392, 7346176,
				    7423744, 7536640, 7610880, 7708160, 7787776, 7863040, 8000256, 8076800, 8157952, 8200448, 8332288,
				    8492800, 8616704, 8713728, 8941568, 9273600, 9448704, 9582592, 9729536, 9876224, 10041344, 10285056
				};
				uint32_t warbleOffset = 0;
				uint32_t warbleLength = 0;
				uint32_t activeRemaining = 0;
				const int numWarbles = SizeOfArray(sPODOffset) / SizeOfArray(fTrackCount);
				if (fNextWarble > numWarbles)
					fNextWarble = int(float(fNextWarble) / fTrackCount[fNextTrack] * numWarbles);
				uint32_t warbleTrack = fNextWarble + fNextTrack * numWarbles;
				warbleOffset = (warbleTrack > 0) ? sPODOffset[warbleTrack-1] : 0;
				warbleLength = sPODOffset[warbleTrack];
				activeRemaining = warbleLength - warbleOffset;
				fPODActive = true;
				fPODOffset = warbleOffset;
				if (activeRemaining > 0)
					clearDecodeBuffer();
				fActiveRemaining = activeRemaining;
				if (activeRemaining > 0 && !isPlaying())
					connecttonull();
			}
			else
			{
				fPODActive = false;
				if (fNextTrack != fActiveTrack)
				{
					if (fActiveTrack != -1)
						closeTrack();
					openTrack(fNextTrack);
				}
				// DEBUG_PRINTLN(fActiveTrack);
				if (fActiveTrack != -1)
				{
					uint32_t offsetTable;
					uint32_t trackCount = 0;
					uint32_t warbleOffset = 0;
					uint32_t warbleLength = 0;
					uint32_t activeRemaining = 0;
					// DEBUG_PRINT("TABLE: "); DEBUG_PRINTLN((uint32_t)fTrackOffset[fActiveTrack]);
					if (fTrackOffset[fActiveTrack] != nullptr)
					{
						warbleOffset = (fNextWarble > 0) ? fTrackOffset[fActiveTrack][fNextWarble-1] : 0;
						warbleLength = fTrackOffset[fActiveTrack][fNextWarble];
						activeRemaining = warbleLength - warbleOffset;
						// DEBUG_PRINTLN(warbleOffset);
						if (!fFile.seek(warbleOffset + sizeof(offsetTable), SeekSet))
						{
							DEBUG_PRINTLN("Failed to seek");
							activeRemaining = 0;
						}
						// DEBUG_PRINTLN(activeRemaining);
					}
					else if (fFile.seek(0, SeekSet) &&
							 fFile.read((uint8_t*)&offsetTable, sizeof(offsetTable)) == sizeof(offsetTable) &&
							 fFile.seek(offsetTable, SeekSet) &&
							 fFile.read((uint8_t*)&trackCount, sizeof(trackCount)) == sizeof(trackCount))
					{
						if (fNextWarble > 0)
						{
							if (fFile.seek((fNextWarble-1)*sizeof(warbleLength), SeekCur) &&
								fFile.read((uint8_t*)&warbleOffset, sizeof(warbleOffset)) == sizeof(warbleOffset) &&
								fFile.read((uint8_t*)&warbleLength, sizeof(warbleLength)) == sizeof(warbleLength))
							{
								activeRemaining = warbleLength - warbleOffset;
							}
						}
						else if (fFile.read((uint8_t*)&warbleLength, sizeof(warbleLength)) == sizeof(warbleLength))
						{
							activeRemaining = warbleLength;
						}
						if (!fFile.seek(warbleOffset + sizeof(offsetTable), SeekSet))
						{
							activeRemaining = 0;
						}
					}
					else
					{
						DEBUG_PRINTLN("Failed to locate");
					}
					if (activeRemaining > 0)
						clearDecodeBuffer();
					fActiveRemaining = activeRemaining;
					if (activeRemaining > 0 && !isPlaying())
						connecttonull();
				}
			}
			fNextTrack = fNextWarble = -1;
		}
	}

	bool isPlaying()
	{
		return isRunning() && (isLocalFile() || isWebStream());
	}

	void play(const char* file)
	{
		strncpy(fNextSong, file, sizeof(fNextSong)-1);
	}

	void playNext(int track = -1, int32_t warble = -1)
	{
		if (track == -1)
			track = random(SizeOfArray(fTrackCount));
		track = max(min(track, int(SizeOfArray(fTrackCount))), 0);
		if (warble == -1)
			warble = random(fTrackCount[track]);
		warble = max(min(warble, fTrackCount[track]), 0);
		fNextTrack = track;
		fNextWarble = warble;
	}

	void queue(uint32_t delayMS = 0, int track = -1, int32_t warble = -1)
	{
		if (track == -1)
			track = random(SizeOfArray(fTrackCount));
		track = max(min(track, int(SizeOfArray(fTrackCount))), 0);
		if (warble == -1)
			warble = random(fTrackCount[track]);
		warble = max(min(warble, fTrackCount[track]), 0);
		if (fNumQueue < SizeOfArray(fQueue))
		{
			fQueue[fNumQueue].fTrack = track;
			fQueue[fNumQueue].fWarble = warble;
			fQueue[fNumQueue].fStartMS = 0;
			fQueue[fNumQueue].fDelayMS = delayMS;
			fNumQueue++;
		}
	}

#ifdef MIX_AUDIO_FLOAT
	float read()
#else
	int16_t read()
#endif
	{
		static short sDecode[256] =
		{
		    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
		    -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
		    -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
		    -11900, -11388, -10876, -10364, -9852,  -9340,  -8828,  -8316,
		    -7932,  -7676,  -7420,  -7164,  -6908,  -6652,  -6396,  -6140,
		    -5884,  -5628,  -5372,  -5116,  -4860,  -4604,  -4348,  -4092,
		    -3900,  -3772,  -3644,  -3516,  -3388,  -3260,  -3132,  -3004,
		    -2876,  -2748,  -2620,  -2492,  -2364,  -2236,  -2108,  -1980,
		    -1884,  -1820,  -1756,  -1692,  -1628,  -1564,  -1500,  -1436,
		    -1372,  -1308,  -1244,  -1180,  -1116,  -1052,  -988,   -924,
		    -876,   -844,   -812,   -780,   -748,   -716,   -684,   -652,
		    -620,   -588,   -556,   -524,   -492,   -460,   -428,   -396,
		    -372,   -356,   -340,   -324,   -308,   -292,   -276,   -260,
		    -244,   -228,   -212,   -196,   -180,   -164,   -148,   -132,
		    -120,   -112,   -104,   -96,    -88,    -80,    -72,    -64,
		    -56,    -48,    -40,    -32,    -24,    -16,    -8,     0,

		    32124,  31100,  30076,  29052,  28028,  27004,  25980,  24956,
		    23932,  22908,  21884,  20860,  19836,  18812,  17788,  16764,
		    15996,  15484,  14972,  14460,  13948,  13436,  12924,  12412,
		    11900,  11388,  10876,  10364,  9852,   9340,   8828,   8316,
		    7932,   7676,   7420,   7164,   6908,   6652,   6396,   6140,
		    5884,   5628,   5372,   5116,   4860,   4604,   4348,   4092,
		    3900,   3772,   3644,   3516,   3388,   3260,   3132,   3004,
		    2876,   2748,   2620,   2492,   2364,   2236,   2108,   1980,
		    1884,   1820,   1756,   1692,   1628,   1564,   1500,   1436,
		    1372,   1308,   1244,   1180,   1116,   1052,   988,    924,
		    876,    844,    812,    780,    748,    716,    684,    652,
		    620,    588,    556,    524,    492,    460,    428,    396,
		    372,    356,    340,    324,    308,    292,    276,    260,
		    244,    228,    212,    196,    180,    164,    148,    132,
		    120,    112,    104,    96,     88,     80,     72,     64,
		    56,     48,     40,     32,     24,     16,     8,      0
		};
	#ifdef MIX_AUDIO_FLOAT
		float sample = 0;
	#else
		int16_t sample = 0;
	#endif
		if (fActiveRemaining > 0)
		{
			if (fDecodePtr == &fDecodeBuffer[sizeof(fDecodeBuffer)])
				fillDecodeBuffer();
		#ifdef MIX_AUDIO_FLOAT
			sample = float(sDecode[*fDecodePtr++]) / SHRT_MAX;
		#else
			sample = sDecode[*fDecodePtr++];
		#endif
			fActiveRemaining--;
		}
		return sample;
	}

    virtual void audio_eof_mp3(const char *info) override
    {
    	// connecttonull();
    }

private:
	File fFile;
	TaskHandle_t fAudioTask = nullptr;
	int fNextTrack = -1;
	int fNextWarble = -1;
	unsigned fNumQueue = 0;
	bool fOTAInProgress = false;
	struct {
		int fTrack;
		int fWarble;
		uint32_t fStartMS; 
		uint32_t fDelayMS;
	} fQueue[WARBLER_QUEUE_DEPTH];
	int8_t fActiveTrack = -1;
	uint32_t fActiveRemaining = 0;
	uint8_t fDecodeBuffer[WARBLER_DECODE_BUFFER_SIZE];
	uint8_t* fDecodePtr = &fDecodeBuffer[sizeof(fDecodeBuffer)];
	int32_t fTrackCount[WARBLER_NUM_TRACKS] = {};
	uint32_t* fTrackOffset[WARBLER_NUM_TRACKS] = {};
	AudioFrequencyBitmap* fAudioBitmap = nullptr;
	const esp_partition_t* fPOD = nullptr;
	bool fPODActive = false;
	uint32_t fPODOffset = 0;
	fs::FS &fFS;
	bool fStopStream = false;
	char fNextSong[128];

	void clearDecodeBuffer()
	{
		fDecodePtr = &fDecodeBuffer[sizeof(fDecodeBuffer)];
	}

	void fillDecodeBuffer()
	{
		ssize_t bytesRead;
		fDecodePtr = fDecodeBuffer;
		if (fPODActive)
		{
			bytesRead = min(fActiveRemaining, sizeof(fDecodeBuffer));
			if (esp_partition_read(fPOD, fPODOffset, fDecodeBuffer, bytesRead) != ESP_OK)
			{
				bytesRead = 0;
			}
			fPODOffset += bytesRead;
		}
		else
		{
			// dont care about errors output will be silent
			bytesRead = fFile.readBytes((char*)fDecodeBuffer, sizeof(fDecodeBuffer));
		}
		// should never but if we have a read failure or underrun clear to zero
		if (bytesRead < 0)
			bytesRead = 0;
		if (bytesRead != sizeof(fDecodeBuffer))
		{
			memset(&fDecodeBuffer[bytesRead], '\0', sizeof(fDecodeBuffer) - bytesRead);
		}
	}

	static WarblerAudio*& activeWarbler()
	{
		static WarblerAudio* sWarbler;
		return sWarbler;
	}

	bool openTrack(unsigned i)
	{
		char buf[16];
		snprintf(buf, sizeof(buf), "/sys%d.dat", i);
		fFile = fFS.open(buf);
		fActiveTrack = i;
		return (fFile == true);
	}

	void closeTrack()
	{
		fFile.close();
		fActiveTrack = -1;
	}

	bool initTracks()
	{
	    if (!psramInit())
	    	return false;
		bool success = (activeWarbler() == nullptr);
		for (unsigned i = 0; i < SizeOfArray(fTrackCount); i++)
		{
			if (openTrack(i))
			{
				uint32_t offsetTable;
				int32_t trackCount;
				if (fFile.seek(0, SeekSet) &&
					fFile.read((uint8_t*)&offsetTable, sizeof(offsetTable)) == sizeof(offsetTable) &&
					fFile.seek(offsetTable, SeekSet) &&
					fFile.read((uint8_t*)&trackCount, sizeof(trackCount)) == sizeof(trackCount))
				{
					fTrackCount[i] = trackCount;
					fTrackOffset[i] = nullptr;
					// fTrackOffset[i] = (uint32_t*)ps_calloc(trackCount, sizeof(uint32_t));
					WARBLER_DEBUG_PRINT("Track #"); WARBLER_DEBUG_PRINT(i);
					WARBLER_DEBUG_PRINT(": "); WARBLER_DEBUG_PRINT(trackCount);
					WARBLER_DEBUG_PRINT(" tableSize: "); WARBLER_DEBUG_PRINTLN(trackCount * sizeof(uint32_t));
					// if (fTrackOffset[i] != nullptr)
					// {
					// 	size_t trackOffsetSize = trackCount*sizeof(uint32_t);
					// 	if (fFile.read((uint8_t*)fTrackOffset[i], trackOffsetSize) != trackOffsetSize)
					// 	{
					// 		// Failed to read track offsets disable track
					// 		WARBLER_DEBUG_PRINTLN("Failed to read offset table");
					// 		free(fTrackOffset[i]);
					// 		fTrackOffset[i] = nullptr;
					// 		fTrackCount[i] = 0;
					// 	}
					// 	else
					// 	{
					// 		DEBUG_PRINTLN_HEX((uint32_t)fTrackOffset[i]);
					// 	}
					// }
					// else
					// {
					// 	WARBLER_DEBUG_PRINTLN("Failed to allocate offset table");
					// }
				}
				closeTrack();
			}
			else
			{
				success = false;
			}
		}
		if (success)
			activeWarbler() = this;
		return success;
	}

	static inline float mixSamples(float s1, float s2)
	{
        s1 = (s1 + s2) - (s1 * s2);
        // // clip if necessary
        // if (s1 < -1.0f)
        //     s1 = -1.0f;
        // if (s1 > 1.0f)
        //     s1 = 1.0f;
       	return s1;
	}

	static void audioFilter(unsigned numBits, unsigned numChannels, const int16_t* samples, unsigned sampleCount)
	{
    	WarblerAudio* active = activeWarbler();
    	if (active == nullptr)
    		return;
        if (active->fActiveRemaining == 0)
        {
	    	AudioFrequencyBitmap* bitmap = active->fAudioBitmap;
    		if (!active->isNullStream() && bitmap != nullptr)
        		bitmap->processSamples(numBits, numChannels, samples, sampleCount);
        	return;
        }
    #ifdef MIX_AUDIO_FLOAT
		static float sPlayVolume = 1.0f;
		static float sTrackVolume = 1.0f;
    	if (numBits == 8)
    	{
    		uint8_t* outp = (uint8_t*)samples;
    		if (numChannels == 1)
    		{
		    	for (size_t i = 0; i < sampleCount*2; i++)
		    	{
		    		float s2 = active->read() * sPlayVolume;
		    		float s1 = (float(*outp) / UCHAR_MAX) * sTrackVolume;
		    		*outp++ = uint8_t(mixSamples(s1, s2) * UCHAR_MAX);
	    		}
	    	}
	    	else
	    	{
		    	for (size_t i = 0; i < sampleCount*2; i++)
		    	{
		    		float s2 = active->read() * sPlayVolume;
			    	for (size_t ci = 0; ci < numChannels; ci++)
			    	{
			    		float s1 = (float(*outp) / UCHAR_MAX) * sTrackVolume;
			    		*outp++ = uint8_t(mixSamples(s1, s2) * UCHAR_MAX);
			    	}
	    		}
	    	}
    	}
    	else if (numBits == 16)
    	{
    		int16_t* outp = (int16_t*)samples;
    		if (numChannels == 1)
    		{
		    	for (size_t i = 0; i < sampleCount; i++)
		    	{
		    		float s2 = active->read();
		    		float s1 = (float(*outp) / SHRT_MAX) * sTrackVolume;
		    		*outp++ = int16_t(mixSamples(s1, s2) * SHRT_MAX);
	    		}
	    	}
	    	else
	    	{
		    	for (size_t i = 0; i < sampleCount; i++)
		    	{
		    		float s2 = active->read() * sPlayVolume;
			    	for (size_t ci = 0; ci < numChannels; ci++)
			    	{
			    		float s1 = (float(*outp) / SHRT_MAX) * sTrackVolume;
			    		*outp++ = int16_t(mixSamples(s1, s2) * SHRT_MAX);
			    	}
	    		}
	    	}
	    }
	#else
    	if (numBits == 8)
    	{
    		uint8_t* outp = (uint8_t*)samples;
    		if (numChannels == 1)
    		{
		    	for (size_t i = 0; i < sampleCount*2; i++)
		    	{
		    		int32_t s2 = active->read();
		    		int32_t s1 = (*outp - 0x80) << 8;
		    		*outp++ = uint8_t((float((s1 + s2) - (s1 * s2)) / SHRT_MAX) * UCHAR_MAX);
	    		}
	    	}
	    	else
	    	{
		    	for (size_t i = 0; i < sampleCount*2; i++)
		    	{
		    		int32_t s2 = active->read();
			    	for (size_t ci = 0; ci < numChannels; ci++)
			    	{
			    		int32_t s1 = (*outp - 0x80) << 8;
			    		*outp++ = uint8_t((float((s1 + s2) - (s1 * s2)) / SHRT_MAX) * UCHAR_MAX);
			    	}
	    		}
	    	}
    	}
    	else if (numBits == 16)
    	{
    		int16_t* outp = (int16_t*)samples;
    		if (numChannels == 1)
    		{
		    	for (size_t i = 0; i < sampleCount; i++)
		    	{
		    		int32_t s2 = active->read();
		    		int32_t s1 = *outp;
		    		*outp++ = (s1 + s2);
	    		}
	    	}
	    	else
	    	{
		    	for (size_t i = 0; i < sampleCount; i++)
		    	{
		    		int32_t s2 = active->read();
			    	for (size_t ci = 0; ci < numChannels; ci++)
			    	{
			    		int32_t s1 = *outp;
			    		*outp++ = (s1 + s2);
			    	}
	    		}
	    	}
	    }
	#endif
	}

	void audioLoop()
	{
		if (fStopStream || *fNextSong != 0)
		{
			fStopStream = false;
			AudioPlayer::stopSong();
			if (*fNextSong == '\0')
				connecttonull();
		}
		if (*fNextSong)
		{
			if (!connecttoFS(fFS, fNextSong))
				connecttonull();
			*fNextSong = '\0';
		}

    	AudioPlayer::loop();
    	if (fOTAInProgress)
        	AnimatedEvent::process();
	}

	static void audioLoopTask(void*)
	{
	    for (;;)
	    {
	    	WarblerAudio* active = activeWarbler();
	    	if (active != nullptr)
	    		active->audioLoop();
	        vTaskDelay(1);
	    }
	}
};
#endif
