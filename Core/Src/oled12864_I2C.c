/*
 * oled_12864.c
 *
 *  Created on: Feb 21, 2022
 *      Author: maxiufeng
 */

#include <oled12864_I2C.h>
#include "stdio.h"

extern I2C_HandleTypeDef oled_i2c;

#define	oled_timeOut	(10*1000)


/* some flag  */
static uint8_t bufferUpdateFlag = 0;	// If this flag is SET, the oled_display_buff has changed.
/* --- end --- */


/* oled display data store */
static uint8_t  oled_display_buff[oled_H_Pix * oled_V_Pix / 8];

/* oled I2C function definition */
/**
 * @brief write indicate command to oled Register
 * @param cmd: [in] input command
 * @retval status 0:write cmd ok    1:write cmd error
 */
static uint8_t oled_Write_CMD(uint8_t cmd)
{
	HAL_StatusTypeDef hal_sta;
	hal_sta = HAL_I2C_Mem_Write(&oled_i2c, oled_i2c_addr, ctrl_cmd, I2C_MEMADD_SIZE_8BIT, &cmd, sizeof cmd, oled_timeOut);
	if (hal_sta == HAL_OK)
		return 0;
	return 1;
}


/**
 * @brief write indicate data to oled DRAM
 * @param data[]: [in] input data array
 * @param len:	[in] input data array length
 * @retval status 0:write data ok    1:write data error
 */
static uint8_t oled_Write_Data(uint8_t data[], uint16_t len)
{
	HAL_StatusTypeDef hal_sta;
	hal_sta = HAL_I2C_Mem_Write(&oled_i2c, oled_i2c_addr, ctrl_data, I2C_MEMADD_SIZE_8BIT, data, len, oled_timeOut);
	if (hal_sta == HAL_OK)
		return 0;
	return 1;
}


/**
 * @brief use indicate color to filling oled all pixels
 */
static void oled_Filling(oled_color_t color)
{
	for(uint32_t i = 0; i < sizeof(oled_display_buff); i++)
	{
		oled_display_buff[i] = (color == oled_color_Black)? (0x00): (0x01);
	}

	bufferUpdateFlag = 1;
}


/**
 * @brief scan all online I2C devices
 */
void I2C_Device_Scan(void)
{
	uint8_t num = 1;
	HAL_StatusTypeDef sta = HAL_OK;
	printf("Scan online I2C devices, Addresses are not shifted to the left\r\n");
	uint32_t startTick = HAL_GetTick();
	for (uint8_t i = 1; i < 128; ++i) {
		sta = HAL_I2C_IsDeviceReady(&oled_i2c, (i<<1), 2, 100);
		if (sta != HAL_OK)
			continue;
		else
		{
			printf("  i2c device %d : 0X%0X\r\n", num++, i);
		}
	}
	printf("total time is : %ld ms\r\n", HAL_GetTick() - startTick);
	printf("------------------------- Scan over --------------------------\r\n");
}



/**
 * @brief Double byte command to select 1 out of 256 contrast steps. Contrast increases as the value increases
 * 		[oled_cmd_display_contrast_1(0x81)  +  ContrastVal(default value = 0x7F)]
 * @param ContrastVal: [in] contrast value between 0 t0 255
 * @retval status 0:write command ok    1:write command error
 */
uint8_t oled_Set_Contrast(uint8_t ContrastVal)
{
	uint8_t res = 0;
	oled_Write_CMD(oled_cmd_display_contrast_1);			// Set Contrast first cmd Byte
	oled_Write_CMD(ContrastVal) ? (res = 1) : (res = 0);
	return res;
}


/**
 * @brief oled display normal(on-1,off-0) or inverse(on-0,off-1)
 * @param display_Way_t: [in]  display_normal	display_invers
 * @retval status 0:ok	1:error
 */
uint8_t oled_Set_Display_Normal_Inverse(display_Way_t display_Way)
{
	uint8_t res = 0;
	switch (display_Way) {
		case display_normal:
			res = oled_Write_CMD(oled_cmd_display_normal);
			break;
		case display_invers:
			res = oled_Write_CMD(oled_cmd_display_inverse);
			break;
		default:
			return 1;
	}
	return res;
}


/**
 * @brief oled display On / Off(sleep)
 * @param display_Switch_t: [in]  display_on	display_off
 * @retval status 0:ok	1:error
 */
uint8_t oled_Set_Display_ON_OFF(display_Switch_t	display_Switch)
{
	uint8_t res = 0;
	switch (display_Switch) {
		case display_on:
			res = oled_Write_CMD(oled_cmd_display_on);
			break;
		case display_off:
			res = oled_Write_CMD(oled_cmd_display_off);
			break;
		default:
			return 1;
	}
	return res;
}


/**
 * @brief set memory address mode
 * 		00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
 * @param addr_Mode [in]
 * @retval status 0:ok	1/2:error
 */
uint8_t oled_Set_Memory_Addr_Mode(addr_Mode_t	addr_Mode)
{
	uint8_t res = 0;

	res = oled_Write_CMD(oled_cmd_set_memory_addr_mode_1);
	if (res != 0)
		return 1;
	switch (addr_Mode) {
		case addr_mode_Horizontal:
			res = oled_Write_CMD((uint8_t)addr_Mode);
			break;
		case addr_mode_Vertical:
			res = oled_Write_CMD((uint8_t)addr_Mode);
			break;
		case addr_mode_Page:
			res = oled_Write_CMD((uint8_t)addr_Mode);
			break;
		default:
			res = 1;
			break;
	}

	return res;
}



/**
  * @brief oled 12864 init
  * @retval status 0:init ok    1:init error
  */
uint8_t oled_i2c_Init(void)
{
	// Wait for the screen to boot
	HAL_Delay(100);
	int status = 0;

	// Init LCD
	status += oled_Set_Display_ON_OFF(display_off);   				// Display off

	status += oled_Set_Memory_Addr_Mode(addr_mode_Page);			// Set Memory Addressing Mode

	status += oled_Write_CMD(oled_cmd_set_page_start_addr_0);   	// Set Page Start Address for Page Addressing Mode,0-7
	status += oled_Write_CMD(oled_cmd_set_com_scan_dir_decrease);   // Set COM Output Scan Direction
	status += oled_Write_CMD(oled_cmd_set_l_col_start_addr_0);   	// Set low column address
	status += oled_Write_CMD(oled_cmd_set_h_col_start_addr_0);   	// Set high column address
	status += oled_Write_CMD(oled_cmd_set_display_start_line);   	// Set start line address

	status += oled_Set_Contrast(0x7F);								// set contrast control register

	status += oled_Write_CMD(oled_cmd_set_segment_remap_n);  		// Set segment re-map 0 to 127
	status += oled_Set_Display_Normal_Inverse(display_normal);		// Set normal display


	status += oled_Write_CMD(oled_cmd_set_multiplex_ratio_1);   	// Set multiplex ratio(1 to 64)
	status += oled_Write_CMD(oled_V_Pix - 1);

	status += oled_Write_CMD(oled_cmd_display_following);   		// 0xa4,Output follows RAM content;0xa5,Output ignores RAM content

	status += oled_Write_CMD(oled_cmd_set_display_offset_1);   		// Set display offset
	status += oled_Write_CMD(0x00);   								// No offset

	status += oled_Write_CMD(oled_cmd_set_clk_div_1);   			// Set display clock divide ratio/oscillator frequency
	status += oled_Write_CMD(0xF0);   								// Set divide ratio

	status += oled_Write_CMD(oled_cmd_set_pre_charge_period_1);   	// Set pre-charge period
	status += oled_Write_CMD(0x22);

	status += oled_Write_CMD(oled_cmd_set_com_pins_1);   			// Set com pins hardware configuration
#ifdef oled_com_lr_remap
	status += oled_Write_CMD(0x32);   								// Enable COM left/right remap
#else
	status += oled_Write_CMD(0x12);   								// Do not use COM left/right remap
#endif // SSD1306_COM_LR_REMAP

	status += oled_Write_CMD(oled_cmd_set_Vcomh_deselect_level_1);  // Set vcomh
	status += oled_Write_CMD(0x20);   								// 0x20,0.77xVcc

	status += oled_Write_CMD(oled_cmd_set_charge_pump_1);   		// Set DC-DC enable
	status += oled_Write_CMD(0x14);   								// bit2 = 1, Enable charge pump during display on

	status += oled_Set_Display_ON_OFF(display_on);					// Turn on SSD1306 panel

	if (status != 0)
		return 1;

	oled_Filling(oled_color_White);
	oled_Update_Screen();

	return 0;
}



/**
 * @brief write oled_cmd_display_contrast_1 into oled GDDRAM
 * @retval status 0:write ok	1:write error
 */
uint8_t oled_Update_Screen(void)
{
	if (bufferUpdateFlag == 0)
		return 0;	// don't need updata screen

	uint8_t res = 0;
    for (uint8_t i = 0; i < 8; i++) {
        oled_Write_CMD(oled_cmd_set_page_start_addr_0 + i);	// page value need Increase manually.
        oled_Write_CMD(oled_cmd_set_l_col_start_addr_0);	// column value automatic increase
        oled_Write_CMD(oled_cmd_set_h_col_start_addr_0);

        res = oled_Write_Data(&oled_display_buff[oled_H_Pix * i], sizeof(oled_display_buff)/8);
        if (res != 0)
        {
        	return 1;
        }
    }


    bufferUpdateFlag = 0;
    return res;
}



