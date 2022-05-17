#if defined(ESP32)

// IRAM_ATTR code in a header file causes compiler errors

#include "PWMDecoder.h"
#include "driver/rmt.h"

#define RMT_TICK_PER_US 8
// determines how many clock cycles one "tick" is
// [1..255], source is generally 80MHz APB clk
#define RMT_RX_CLK_DIV (80000000/RMT_TICK_PER_US/1000000)
// time before receiver goes idle
#define RMT_RX_MAX_US 3500

PWMDecoder* PWMDecoder::sActive;

void IRAM_ATTR
PWMDecoder::rmt_isr_handler(void* arg)
{
	// with reference to https://www.esp32.com/viewtopic.php?t=7116#p32383
	// but modified so that this ISR only checks chX_rx_end
	uint32_t intr_st = RMT.int_st.val;

	// see declaration of RMT.int_st:
	// takes the form of 
	// bit 0: ch0_tx_end
	// bit 1: ch0_rx_end
	// bit 2: ch0_err
	// bit 3: ch1_tx_end
	// bit 4: ch1_rx_end
	// ...
	// thus, check whether bit (channel*3 + 1) is set to identify
	// whether that channel has changed

	for (unsigned i = 0; i < sActive->fNumChannels; i++)
	{
		uint8_t channel = sActive->fChannels[i];
		uint32_t channel_mask = BIT(channel*3+1);

		if (!(intr_st & channel_mask))
			continue;

		RMT.conf_ch[channel].conf1.rx_en = 0;
		RMT.conf_ch[channel].conf1.mem_owner = RMT_MEM_OWNER_TX;
		volatile rmt_item32_t* item = RMTMEM.chan[channel].data32;
		if (item)
		{
			sActive->fRawPulse[i] = item->duration0;
		}

		RMT.conf_ch[channel].conf1.mem_wr_rst = 1;
		RMT.conf_ch[channel].conf1.mem_owner = RMT_MEM_OWNER_RX;
		RMT.conf_ch[channel].conf1.rx_en = 1;

		//clear RMT interrupt status.
		RMT.int_clr.val = channel_mask;
	}
}

void
PWMDecoder::begin(void)
{
	sActive = this;
	rmt_config_t rmt_channels[fMaxChannels] = {};
	for (unsigned i = 0; i < fNumChannels; i++)
	{
		fPulse[i] = RECEIVER_CH_CENTER;
		fRawPulse[i] = RECEIVER_CH_CENTER;

		rmt_channels[i].channel = (rmt_channel_t)fChannels[i];
		rmt_channels[i].gpio_num = (gpio_num_t)fGPIO[i];
		rmt_channels[i].clk_div = RMT_RX_CLK_DIV;
		rmt_channels[i].mem_block_num = 1;
		rmt_channels[i].rmt_mode = RMT_MODE_RX;
		rmt_channels[i].rx_config.filter_en = true;
		rmt_channels[i].rx_config.filter_ticks_thresh = 100;
		rmt_channels[i].rx_config.idle_threshold = RMT_RX_MAX_US * RMT_TICK_PER_US;

		rmt_config(&rmt_channels[i]);
		rmt_set_rx_intr_en(rmt_channels[i].channel, true);
		rmt_rx_start(rmt_channels[i].channel, 1);
	}

	rmt_isr_register(rmt_isr_handler, NULL, 0, NULL);
	ESP_LOGI(TAG, "Init ISR on %d", xPortGetCoreID());
}

#endif
