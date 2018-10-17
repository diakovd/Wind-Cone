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


#define SPI_INSTANCE  0 /**< SPI instance index. */
//#define     SPI1_USE_EASY_DMA 0
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

#define hRD_System_Configuration_Reg 0x40
#define lRD_System_Configuration_Reg 0xFE

static uint8_t       m_tx_buf[2] = {0xFE,0x40}; //TEST_STRING;           /**< TX buffer. FE41 */
static uint8_t       m_rx_buf[3];    /**< RX buffer. */
static const uint8_t m_length = sizeof(m_tx_buf);        /**< Transfer length. */

#define TEST_STRING2 "WindCone Test"
static uint8_t  displ_string[sizeof(TEST_STRING2) + 1] = TEST_STRING2;    /**< RX buffer. */
static uint8_t  displ_rx[20];// = {0xFE,0x40}; //TEST_STRING;           /**< TX buffer. FE41 */


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
#define TWI_INSTANCE_ID   1
#define TWI1_USE_EASY_DMA 0  

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
       .scl                = 12,  
       .sda                = 10,
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
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL)); //spi_event_handler)); //NULL)); //
}

void testdrawcircle(void) {
  for (int16_t i=0; i< Adafruit_GFX_height()/2; i+=2) {
    Adafruit_GFX_drawCircle(Adafruit_GFX_width()/2, Adafruit_GFX_height()/2, i, WHITE);
    SSD1306_display();
    nrf_delay_ms(200);
  }
}

void printStr(char *str)
{
    for (uint8_t i = 0; i < strlen(str); i++) {
        Adafruit_GFX_write(str[i]);
        if ((i > 0) && (i % 21 == 0))
            Adafruit_GFX_write('\n');
    }
    Adafruit_GFX_write('\n');
    SSD1306_display();
	
}	

int main(void)
{
  bsp_board_leds_init();

	spi_init();
  twi_init();

////
//Test display
////	
    SSD1306_begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    Adafruit_GFX_init(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, SSD1306_drawPixel);

	  SSD1306_clearDisplay();
    Adafruit_GFX_setTextSize(1);
    Adafruit_GFX_setTextColor(1,0);
    Adafruit_GFX_setCursor(0, 0);

		printStr(TEST_STRING2);
	
	
	
    while (1)
    {
        // Reset rx buffer and transfer done flag
        memset(m_rx_buf, 0, m_length);
        spi_xfer_done = false;

				m_tx_buf[0] = lRD_System_Configuration_Reg;
				m_tx_buf[1] = hRD_System_Configuration_Reg;
			
        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, m_length, m_rx_buf, m_length));

/*        while (!spi_xfer_done)
        {
            __WFE();
        }
*/
				sprintf(displ_rx,"Sys_Conf_Reg 0x%X 0x%X", m_rx_buf[0], m_rx_buf[1]);

				printStr(displ_rx);

			
        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(200);
    }
}
