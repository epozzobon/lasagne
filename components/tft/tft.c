#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_heap_caps.h"

#include "tft.h"


#define CHUNK_SIZE 1024


static const char *TAG = "ili9341";


/*
   Some info about the ILI9341: It has an C/D line, which is connected to a GPIO here. It expects this
   line to be low for a command and high for data. We use a pre-transmit callback here to control that
   line: every transaction has as the user-definable argument the needed state of the D/C line and just
   before the transaction is sent, the callback will set this line to the correct state.
 */

/*
   The ILI9341 needs a bunch of command/argument values to be initialized. They are stored in this struct.
 */
typedef struct {
	uint8_t cmd;
	uint8_t data[16];
	uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} ili_init_cmd_t;

static const ili_init_cmd_t ili_init_cmds_drom[]={
	{0xCF, {0x00, 0x83, 0X30}, 3},
	{0xED, {0x64, 0x03, 0X12, 0X81}, 4},
	{0xE8, {0x85, 0x01, 0x79}, 3},
	{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
	{0xF7, {0x20}, 1},
	{0xEA, {0x00, 0x00}, 2},
	{0xC0, {0x26}, 1},
	{0xC1, {0x11}, 1},
	{0xC5, {0x35, 0x3E}, 2},
	{0xC7, {0xBE}, 1},
	{0x36, {0xe8}, 1},
	{0x3A, {0x55}, 1},
	{0xB1, {0x00, 0x1B}, 2},
	{0xF2, {0x08}, 1},
	{0x26, {0x01}, 1},
	{0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
	{0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
	{0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
	{0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
	{0x2C, {0}, 0},
	{0xB7, {0x07}, 1},
	{0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
	{0x33, {0x00, 0x00, 0x01, 0x40, 0x00, 0x00}, 6},
	{0x37, {0x00, 0x00}, 2},
	{0x11, {0}, 0x80},
	{0x29, {0}, 0x80},
	{0, {0}, 0xff},
};

typedef struct {
	void *buffer1, *buffer2, *user;
	ili_draw_cb_t finish_cb;
} ili_draw_finish_handle_t;

typedef struct {
	bool dc;
	uint8_t dc_io_num;
	ili_draw_finish_handle_t *finish_handle;
} spi_transaction_user_data_t;

//Send a command to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
void ili_cmd(ili_device_handle_t ili, const uint8_t cmd) {
	esp_err_t ret;
	spi_transaction_t *t = heap_caps_malloc(sizeof(spi_transaction_t), MALLOC_CAP_DMA);
	assert(t);
	spi_transaction_user_data_t u;
	memset(t, 0, sizeof(spi_transaction_t));       //Zero out the transaction
	memset(&u, 0, sizeof(u));       //Zero out the transaction
	t->length=8;                     //Command is 8 bits
	t->tx_buffer=&cmd;               //The data is the cmd itself
	t->user=&u;
	u.dc=false;                         //D/C needs to be set to 0
	u.dc_io_num = ili->dc_io_num;
	ret=spi_device_transmit(ili->spi, t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	free(t);
}

//Send data to the ILI9341. Uses spi_device_transmit, which waits until the transfer is complete.
void ili_data(ili_device_handle_t ili, const uint8_t *data, int len) {
	esp_err_t ret;
	spi_transaction_t *t = heap_caps_malloc(sizeof(spi_transaction_t), MALLOC_CAP_DMA);
	assert(t);
	spi_transaction_user_data_t u;
	if (len==0) return;             //no need to send anything
	memset(t, 0, sizeof(spi_transaction_t));       //Zero out the transaction
	t->length=len*8;                 //Len is in bytes, transaction length is in bits.
	t->tx_buffer=data;               //Data
	t->user=&u;
	u.dc=true;
	u.dc_io_num = ili->dc_io_num;
	ret=spi_device_transmit(ili->spi, t);  //Transmit!
	assert(ret==ESP_OK);            //Should have had no issues.
	free(t);
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void ili_spi_pre_transfer_callback(spi_transaction_t *t) {
	if ( ((spi_transaction_user_data_t*)t->user)->dc )
		gpio_set_level(((spi_transaction_user_data_t*)t->user)->dc_io_num, 1);
	else
		gpio_set_level(((spi_transaction_user_data_t*)t->user)->dc_io_num, 0);
}

esp_err_t ili_bus_add_device(spi_host_device_t host, ili_config_t *dev_config, ili_device_handle_t *handle) {
	esp_err_t ret;
	spi_device_handle_t spi;

	//TODO: check return code
	ili_device_t *dev = malloc(sizeof(ili_device_t));

	dev->dc_io_num = dev_config->dc_io_num;
	dev->bckl_io_num = dev_config->bckl_io_num;

	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 40000000,                   //Clock out at 40 MHz
		.mode = 0,                                    //SPI mode 0
		.spics_io_num = dev_config->spics_io_num,     //CS pin
		.queue_size = 64,
		.pre_cb = ili_spi_pre_transfer_callback,      //Specify pre-transfer callback to handle D/C line
	};

	//Attach the LCD to the SPI bus
	ret = spi_bus_add_device(host, &devcfg, &spi);
	if ( ret != ESP_OK ) {
		free(dev);
		return ret;
	}

	//TODO: check return code
	xTaskCreate(ili_result_task, "ili_result_task", 4096, (void*)dev, 0, NULL);

	dev->spi = spi;
	*handle = dev;
	return ret;
}

//Initialize the display
esp_err_t ili_init(ili_device_handle_t ili) {
	ili_init_cmd_t *ili_init_cmds = heap_caps_malloc(sizeof(ili_init_cmds_drom), MALLOC_CAP_DMA);
	if ( !ili_init_cmds ) return ESP_ERR_NO_MEM;
	memcpy(ili_init_cmds, ili_init_cmds_drom, sizeof(ili_init_cmds_drom));

	int cmd = 0;
	//Initialize non-SPI GPIOs
	gpio_set_direction(ili->dc_io_num, GPIO_MODE_OUTPUT);
	gpio_set_direction(ili->bckl_io_num, GPIO_MODE_OUTPUT);

	//Send all the commands
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ili_cmd(ili, ili_init_cmds[cmd].cmd);
		ili_data(ili, ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes&0x80) {
			vTaskDelay(100 / portTICK_RATE_MS);
		}
		cmd++;
	}

	free(ili_init_cmds);

	///Enable backlight
	gpio_set_level(ili->bckl_io_num, 0);

	return ESP_OK;
}

esp_err_t ili_draw_bitmap(ili_device_handle_t ili, uint16_t x, uint16_t y,
                          uint16_t w, uint16_t h, const uint16_t *bitmap, ili_draw_cb_t finish_cb, void *user) {
	int i;
	esp_err_t ret;

	unsigned int pixel = w * h;
	unsigned int remain = pixel % CHUNK_SIZE;
	unsigned int chunks = pixel / CHUNK_SIZE + ( remain ? 1 : 0 );

	ESP_LOGD(TAG, "bitmap address: 0x%08x", (unsigned int)bitmap);
	ESP_LOGD(TAG, "pixel: %d, chunks: %d, remain: %d", pixel, chunks, remain);

	spi_transaction_t *trans = heap_caps_calloc(chunks + 5, sizeof(spi_transaction_t), MALLOC_CAP_DMA);
	spi_transaction_user_data_t *userdata = calloc(3, sizeof(spi_transaction_user_data_t));
	ili_draw_finish_handle_t *finish_handle = calloc(1, sizeof(ili_draw_finish_handle_t));

	if ( !(trans && userdata && finish_handle) ) {
		free(trans);
		free(userdata);
		free(finish_handle);
		return ESP_ERR_NO_MEM;
	}

	ESP_LOGD(TAG, "calloc'ed trans: 0x%08x", (unsigned int)trans);
	ESP_LOGD(TAG, "calloc'ed userdata: 0x%08x", (unsigned int)userdata);
	ESP_LOGD(TAG, "calloc'ed finish_handle: 0x%08x", (unsigned int)finish_handle);

	memset(trans, 0, (chunks + 5) * sizeof(spi_transaction_t));       //Zero out the transaction

	finish_handle->buffer1 = (void*)trans;
	finish_handle->buffer2 = (void*)userdata;
	finish_handle->user = user;
	finish_handle->finish_cb = finish_cb;

	for (i = 0; i < 3; i++) {
		userdata[i].dc_io_num = ili->dc_io_num;
		if ( i ) userdata[i].dc = true;
	}

	userdata[2].finish_handle = finish_handle;

	for (i = 0; i < 5; i++) {
		if ( !(i & 1) ) {
			trans[i].length = 8;
			trans[i].user = (void*)&userdata[0];
		} else {
			trans[i].length = 8 * 4;
			trans[i].user = (void*)&userdata[1];
		}
		trans[i].flags = SPI_TRANS_USE_TXDATA;
	}

	trans[0].tx_data[0]=0x2A;           //Column Address Set

	trans[1].tx_data[0]= (uint8_t) (x >> 8);           //Start Col High
	trans[1].tx_data[1]= (uint8_t) (x & 0xff);         //Start Col Low
	trans[1].tx_data[2]= (uint8_t) ((x + w - 1) >> 8);     //End Col High
	trans[1].tx_data[3]= (uint8_t) ((x + w - 1) & 0xff);   //End Col Low

	trans[2].tx_data[0]=0x2B;           //Page address set

	trans[3].tx_data[0]= (uint8_t) (y >> 8);           //Start page high
	trans[3].tx_data[1]= (uint8_t) (y & 0xff);         //start page low
	trans[3].tx_data[2]= (uint8_t) ((y + h - 1) >> 8);     //end page high
	trans[3].tx_data[3]= (uint8_t) ((y + h - 1) & 0xff);   //end page low

	trans[4].tx_data[0]=0x2C;           //memory write

	for (i = 0; i < chunks; i++) {
		trans[i + 5].tx_buffer = bitmap + CHUNK_SIZE * i;
		trans[i + 5].length = CHUNK_SIZE * 16;
		trans[i + 5].user = (void*)&userdata[1];
	}

	if ( remain ) trans[chunks + 4].length = remain * 16;

	trans[chunks + 4].user = (void*)&userdata[2];

	for (i = 0; i < chunks + 5; i++) {
		ret = spi_device_queue_trans(ili->spi, &trans[i], portMAX_DELAY);
		if ( ret != ESP_OK ) return ret;
	}
	return ESP_OK;
}


void ili_result_task(void *pvParameter) {
	ili_device_handle_t ili = (ili_device_handle_t)pvParameter;
	spi_transaction_t *rtrans;
	ili_draw_cb_t finish_cb;
	void *user;
	esp_err_t ret;

	for (;;) {
		ret = spi_device_get_trans_result(ili->spi, &rtrans, portMAX_DELAY);
		assert(ret==ESP_OK);

		if ( rtrans->user ) {
			ESP_LOGD(TAG, "spi_device_get_trans_result, user: 0x%08x, finish_handle: 0x%08x",
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user),
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user)->finish_handle);
		} else {
			ESP_LOGD(TAG, "spi_device_get_trans_result, user: 0x%08x",
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user));
		}

		if ( rtrans->user && ((spi_transaction_user_data_t*)rtrans->user)->finish_handle ) {
			finish_cb = ((spi_transaction_user_data_t*)rtrans->user)->finish_handle->finish_cb;
			user = ((spi_transaction_user_data_t*)rtrans->user)->finish_handle->user;

			ESP_LOGD(TAG, "free'ing buffer1: 0x%08x",
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user)->finish_handle->buffer1);
			free(((spi_transaction_user_data_t*)rtrans->user)->finish_handle->buffer1);

			ESP_LOGD(TAG, "free'ing buffer2: 0x%08x",
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user)->finish_handle->buffer2);
			free(((spi_transaction_user_data_t*)rtrans->user)->finish_handle->buffer2);

			ESP_LOGD(TAG, "free'ing finish_handle: 0x%08x",
			         (unsigned int)((spi_transaction_user_data_t*)rtrans->user)->finish_handle);
			free(((spi_transaction_user_data_t*)rtrans->user)->finish_handle);

			if ( finish_cb ) (finish_cb)(user);
		}
	}
}