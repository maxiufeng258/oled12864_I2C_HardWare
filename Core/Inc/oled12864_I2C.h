/*
 * oled_12864.h
 *
 *  Created on: Feb 21, 2022
 *      Author: maxiufeng
 */

#ifndef INC_OLED12864_I2C_H_
#define INC_OLED12864_I2C_H_

#include "stm32f4xx_hal.h"

/* oled connect I2C port, Change according to the actual setting [hi2c1, hi2c2 or hi2cx] */
#define oled_i2c	hi2c2

/* oled i2c address
 * 	0x78	->	0b  0 1 1 1   1 0  0     0
 * 				bit	7 6 5 4   3 2  1     0
 * 								  SA0   W/R
 */
#define oled_i2c_addr	0x78
/* oled panel horizontal pixels */
#define oled_H_Pix		128
/* oled panel vertical pixels */
#define oled_V_Pix		64

/**
 * oled can dispaly color [on/off]
 */
typedef enum {
	oled_color_Black = 0x00,	// pixel off, black
	oled_color_White = 0x01,	// pixel on , white/blue/yellow etc....
}oled_Color_TypeDef;
typedef oled_Color_TypeDef	oled_color_t;


/* include Co bit and D/C bit control Byte */
typedef enum {
	ctrl_cmd  = 0x80,
	ctrl_data = 0x40,
}control_Byte_TypeDef;
typedef control_Byte_TypeDef	ctrl_Byte_t;


/* display normal or invers */
typedef enum {
	display_normal = 0,
	display_invers = 1,
}display_Way_TypeDef;
typedef display_Way_TypeDef		display_Way_t;


/* display On or Off */
typedef enum {
	display_on	=	0,
	display_off	=	1,
}display_Switch_TypeDef;
typedef	display_Switch_TypeDef	display_Switch_t;


/* memory address mode  */
typedef enum {
	addr_mode_Horizontal = 0b00000000,
	addr_mode_Vertical	 = 0b00000001,
	addr_mode_Page		 = 0b00000010,
}addr_Mode_TypeDef;
typedef addr_Mode_TypeDef	addr_Mode_t;


/**
 * define oled Commands
 */
#define	oled_cmd_set_charge_pump_1			0x8D	// Charge Pump Setting	[_2 A[2] = 0b, Disable charge pump(RESET) A[2] = 1b, Enable charge pump during display on]

#define oled_cmd_display_contrast_1			0x81	// oled contrast [0 - 255] 		[_2 default 0x7F]
#define oled_cmd_display_following 			0xA4	// Output follows RAM content	[default]
#define oled_cmd_display_ignore 			0xA5	// Output ignores RAM content
#define oled_cmd_display_normal				0xA6	// Normal display				[default]
#define oled_cmd_display_inverse			0xA7	// Inverse display
#define oled_cmd_display_off	 			0xAE	// Display OFF (sleep mode) 	[default]
#define oled_cmd_display_on	 				0xAF	// Display ON

// 0x00 - 0x0F LSB (bit3~0)	This command is only for page addressing mode
#define oled_cmd_set_l_col_start_addr_0		0x00	//Set Lower Column Start Address for Page Addressing Mode
// 0x10 - 0x1F MSB (bit7~4)	This command is only for page addressing mode
#define oled_cmd_set_h_col_start_addr_0		0x10	// Set Higher Column Start Address for Page Addressing Mode
// 0xB0 - 0xB7 This command is only for page addressing mode
#define oled_cmd_set_page_start_addr_0		0xB0// Set Page Start Address for Page Addressing Mode

#define oled_cmd_set_memory_addr_mode_1		0x20	// Set Memory Addressing Mode [_2 default 0x02->page addr mode]

// This command is only for horizontal or vertical addressing mode.
#define oled_cmd_set_col_addr_range_1		0x21	// Set Column Address	[_2 default 0	_3 default 127]
#define oled_cmd_set_page_addr_range_1		0x22	// Set Page Address		[_2 default 0	_3 default 7]

// 0x40 - 0x7F
#define oled_cmd_set_display_start_line		0x40	// Set display RAM display start line register from 0-63

#define oled_cmd_set_segment_remap_p		0xA0	// Set Segment Re-map, column address 0 is mapped to SEG0 [default]
#define oled_cmd_set_segment_remap_n		0xA1	// Set Segment Re-map, column address 127 is mapped to SEG0
#define oled_cmd_set_multiplex_ratio_1		0xA8	// Set Multiplex Ratio			[_2 default 63 -> 64MUX]
#define oled_cmd_set_com_scan_dir_increase	0xC0	// Set COM Output Scan Direction  0 -> com-1
#define oled_cmd_set_com_scan_dir_decrease	0xC8	// Set COM Output Scan Direction  com-1 -> 0
#define oled_cmd_set_display_offset_1		0xD3	// Set vertical shift by COM from 0d~63d [_2 default 0x00]
#define oled_cmd_set_com_pins_1				0xDA	// Set COM Pins
#define oled_cmd_set_clk_div_1				0xD5	// Set Display Clock Divide Ratio/Oscillator Frequency [_2 default 0x80]
#define oled_cmd_set_pre_charge_period_1	0xD9	// Set Pre-charge Period		[_2 default 0x22]
#define oled_cmd_set_Vcomh_deselect_level_1	0xDB	// Set VCOMH Deselect Level		[_2 default 0x20 0.77Vcc 0x00 0.65Vcc 0x30 0.85Vcc]
#define	oled_cmd_nop						0xE3	// Command for no operation


void I2C_Device_Scan(void);

uint8_t oled_Set_Contrast(uint8_t ContrastVal);
uint8_t oled_Display_Normal_Inverse(display_Way_t display_Way);
uint8_t oled_Set_Display_ON_OFF(display_Switch_t	display_Switch);
uint8_t oled_Set_Memory_Addr_Mode(addr_Mode_t	addr_Mode);
uint8_t oled_i2c_Init(void);
uint8_t oled_Update_Screen(void);






#endif /* INC_OLED12864_I2C_H_ */
