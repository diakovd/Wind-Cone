/**
 * Copyright (c) 2015 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "nrf_drv_spi.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "SSD1306.h"
#include "Adafruit_GFX.h"
#include "nrf_drv_twi.h"


#define SPI_INSTANCE  1 /**< SPI instance index. */
#define     SPI1_USE_EASY_DMA 1
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define TEST_STRING "Nordic"
static uint8_t       m_tx_buf[2] = {0xFE,0x40}; //TEST_STRING;           /**< TX buffer. FE41 */
static uint8_t       m_rx_buf[sizeof(TEST_STRING) + 1];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event)
{
    spi_xfer_done = true;
    NRF_LOG_INFO("Transfer completed.\r\n");
    if (m_rx_buf[0] != 0)
    {
        NRF_LOG_INFO(" Received: \r\n");
        NRF_LOG_HEXDUMP_INFO(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}

/* TWI instance ID. */
#define TWI_INSTANCE_ID   0
//#define TWI1_USE_EASY_DMA 1  

/* Number of possible TWI addresses. */
 #define TWI_ADDRESSES      127

/* TWI instance. */
const nrf_drv_twi_t m_twi_master = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


/**
 * @? TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_sensors_config = {
       .scl                = 2,  
       .sda                = 3,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    //err_code = nrf_drv_twi_init(&m_twi_lis2dh12, &twi_lis2dh12_config, twi_handler, NULL);
    err_code = nrf_drv_twi_init(&m_twi_master, &twi_sensors_config, NULL, NULL);        // twi in blocking mode.
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi_master);
}

void spi_init (void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPIM0_SS_PIN;
    spi_config.miso_pin = SPIM0_MISO_PIN;
    spi_config.mosi_pin = SPIM0_MOSI_PIN;
    spi_config.sck_pin  = SPIM0_SCK_PIN;
	  spi_config.frequency = NRF_DRV_SPI_FREQ_1M; 
		spi_config.mode = NRF_DRV_SPI_MODE_1; 
//	  spi_config.irq_priority = APP_IRQ_PRIORITY_LOW;
//    spi_config.irq_priority  = APP_IRQ_PRIORITY_HIGH;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config,spi_event_handler)); //NULL)); //
}

void testdrawcircle(void) {
  for (int16_t i=0; i< Adafruit_GFX_height()/2; i+=2) {
    Adafruit_GFX_drawCircle(Adafruit_GFX_width()/2, Adafruit_GFX_height()/2, i, WHITE);
    SSD1306_display();
    nrf_delay_ms(200);
  }
}

int main(void)
{
    bsp_board_leds_init();

// APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
// NRF_LOG_INFO("SPI example\r\n");

	spi_init();
  twi_init();

////
//Test display
////	
    SSD1306_begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    Adafruit_GFX_init(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, SSD1306_drawPixel);

    SSD1306_clearDisplay();
    Adafruit_GFX_drawBitmap(0, 0,  el_logo, 128, 64, 1);
    SSD1306_display();
    nrf_delay_ms(1000);

    for (;;) {

      SSD1306_clearDisplay();
      SSD1306_display();

      testdrawcircle();

      nrf_delay_ms(500);
    }
////	
	
	
    while (1)
    {
        // Reset rx buffer and transfer done flag
        memset(m_rx_buf, 0, m_length);
        spi_xfer_done = false;

        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length));

        while (!spi_xfer_done)
        {
            __WFE();
        }

      //  NRF_LOG_FLUSH();

        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(200);
    }
}
