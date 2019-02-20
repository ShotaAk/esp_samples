/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"


#define PIN_NUM_MISO 12
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   15

#define READ_FLAG    0x80


uint8_t mpu9250_read1byte(spi_device_handle_t spi, const uint8_t address){
    // １バイト読み込み

    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       // Zero out the transaction
    t.length = 16;                  // SPI_ADDRESS(8bit) + SPI_DATA(8bit)
    t.flags = SPI_TRANS_USE_RXDATA;

    uint16_t tx_data = (address | READ_FLAG) << 8;
    tx_data = SPI_SWAP_DATA_TX(tx_data , 16);
    t.tx_buffer = &tx_data;

    ret=spi_device_polling_transmit(spi, &t);  // Transmit!
    assert(ret==ESP_OK);            // Should have had no issues.

    uint8_t data = SPI_SWAP_DATA_RX(*(uint16_t*)t.rx_data, 16) & 0x00FF; // FF + Data

    return data;
}


void mpu9250_init(spi_device_handle_t spi){
    // Who AM I
    uint8_t mpu_id = mpu9250_read1byte(spi, 0x75);

    printf("MPU ID: %02X\n", mpu_id);
}


void app_main()
{
    esp_err_t ret;
    spi_device_handle_t spi;
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1, // unused
        .quadhd_io_num=-1, // unused
        .max_transfer_sz=4 // bytes
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=500*1000,   //Clock out at 500 kHz
        .mode=3,                    //SPI mode 3
        .spics_io_num=PIN_NUM_CS,   //CS pin
        .queue_size=7,              //We want to be able to queue 7 transactions at a time
    };
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    //Initialize the LCD
    mpu9250_init(spi);
}
