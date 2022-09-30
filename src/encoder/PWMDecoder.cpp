#if defined(ESP32)

#include "PWMDecoder.h"

// Check if we are compiling against a more recent hal layer
#ifndef RMT_RX_MODE
// Nope. arduino-esp32 1.0.6 will crash so we configure the RMT device manually instead
#include "driver/rmt.h"
#include "soc/rmt_reg.h"

void IRAM_ATTR
PWMDecoder::rmt_isr_handler(void* arg)
{
	PWMDecoder* decoder = (PWMDecoder*)arg;
	uint32_t intr_st = RMT.int_st.val;

	uint8_t numChannels = decoder->fNumChannels;
	for (unsigned i = 0; i < numChannels; i++)
	{
		uint32_t channel_mask = BIT(i*3+1);
		if (!(intr_st & channel_mask))
			continue;

		RMT.conf_ch[i].conf1.rx_en = 0;
		RMT.conf_ch[i].conf1.mem_owner = RMT_MEM_OWNER_TX;
		volatile rmt_item32_t* item = RMTMEM.chan[i].data32;
		if (item)
		{
			decoder->fChannel[i].fRawPulse = item->duration0;
		}

		RMT.conf_ch[i].conf1.mem_wr_rst = 1;
		RMT.conf_ch[i].conf1.mem_owner = RMT_MEM_OWNER_RX;
		RMT.conf_ch[i].conf1.rx_en = 1;

		//clear RMT interrupt status.
		RMT.int_clr.val = channel_mask;
	}
}

void
PWMDecoder::checkActive(unsigned i)
{
    uint32_t status = 0;
    if (i < fNumChannels)
    {
	    rmt_get_status(rmt_channel_t(i), &status);
	    uint8_t state = (status & RMT_STATE_CH0) >> RMT_STATE_CH0_S;
	    if (state == 3)
	    {
			fChannel[i].fLastActive = millis();
	    }
	}
}

void
PWMDecoder::begin(void)
{
	if (fISRHandle != nullptr)
		return;
	rmt_config_t rmt_channels[fMaxChannels] = {};
	for (unsigned i = 0; i < fNumChannels; i++)
	{
		fChannel[i].fRawPulse = fChannel[i].fPulse = 1500;

		rmt_channels[i].channel = rmt_channel_t(i);
		rmt_channels[i].gpio_num = gpio_num_t(fChannel[i].fGPIO);
		rmt_channels[i].clk_div = 80;
		rmt_channels[i].mem_block_num = 4;
		rmt_channels[i].rmt_mode = RMT_MODE_RX;
		rmt_channels[i].rx_config.filter_en = true;
		rmt_channels[i].rx_config.filter_ticks_thresh = 100;
		rmt_channels[i].rx_config.idle_threshold = 5000;

		rmt_config(&rmt_channels[i]);
		rmt_set_rx_intr_en(rmt_channels[i].channel, true);
		rmt_rx_start(rmt_channels[i].channel, 1);
	}

	rmt_isr_register(rmt_isr_handler, this, 0, &fISRHandle);
	ESP_LOGI(TAG, "Init ISR on %d", xPortGetCoreID());
}

void
PWMDecoder::end(void)
{
	if (fISRHandle == nullptr)
		return;
	for (unsigned i = 0; i < fNumChannels; i++)
	{
		rmt_tx_stop(rmt_channel_t(i));
	}
	if (fISRHandle != nullptr)
	{
		rmt_isr_deregister(fISRHandle);
		fISRHandle = nullptr;
	}
}
#endif
#endif