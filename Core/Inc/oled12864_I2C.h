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

/* wether or not use DMA for oled-I2C */
#define	oled_i2c_dma	1

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


/* display follows RAM or not(ignore) */
typedef enum {
	RAM_output_follow = 0,	// GDDRAM decides what to display
	RAM_output_ignore = 1,	// Entire Display ON (All pixels light up)
}RAM_Output_TypeDef;
typedef	RAM_Output_TypeDef	 RAM_Output_t;


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

// ############################################################################

/* memory address mode  */
typedef enum {
	addr_mode_Horizontal = 0b00000000,
	addr_mode_Vertical	 = 0b00000001,
	addr_mode_Page		 = 0b00000010,
}addr_Mode_TypeDef;
typedef addr_Mode_TypeDef	addr_Mode_t;

// ############################################################################

/* segment reMapping or not, scan direction */
typedef enum {
	segment_default_mapping = 0,
	segment_remapping 		= 1,
}segment_Map_TypeDef;
typedef segment_Map_TypeDef	segment_Map_t;


/* common reMapping or not, scan direction */
typedef enum {
	com_normal_mapping	=	0,
	com_remapping		=	1,
}com_Map_TypeDef;
typedef com_Map_TypeDef	com_Map_t;


/* Set COM Pins Hardware Configuration */
typedef enum {
	common_sequential_config		=	0b00000010,	// bit4 = 0
	common_alternative_config		=	0b00010010,	// bit4 = 1	[default]
	common_disable_left_right_remap	=	0b00000010,	// bit5 = 0 [default]
	common_enable_left_right_remap	=	0b00100010,	// bit5 = 1
}common_Hardware_Config_TypeDef;
typedef	common_Hardware_Config_TypeDef	common_Hardware_Config_t;

// ############################################################################

/* VCOMH Deselect Leve */
typedef enum {
	Vcomh_0_p_65_Vcc	=	0b00000000,	// 0x00
	Vcomh_0_p_77_Vcc	=	0b00100000,	// 0x20
	Vcomh_0_p_83_Vcc	=	0b00110000,	// 0x30
}Vcomh_Level_TypeDef;
typedef	Vcomh_Level_TypeDef	Vcomh_Level_t;

// ############################################################################

//--- continues horizontal scroll related-----------
typedef enum {
	horizontal_scroll_right	=	0,
	horizontal_scroll_left	=	1,
}horizontal_scroll_direction_TypeDef;
typedef horizontal_scroll_direction_TypeDef	horizontal_scroll_direction_t;

/* horizontal scroll start and end page address */
typedef enum {
	horizontal_scroll_page_0	=	0,	// PAGE 0
	horizontal_scroll_page_1	=	1,	// PAGE 1
	horizontal_scroll_page_2	=	2,	// PAGE 2
	horizontal_scroll_page_3	=	3,	// PAGE 3
	horizontal_scroll_page_4	=	4,	// PAGE 4
	horizontal_scroll_page_5	=	5,	// PAGE 5
	horizontal_scroll_page_6	=	6,	// PAGE 6
	horizontal_scroll_page_7	=	7,	// PAGE 7
}horizontal_scroll_page_addr_TypeDef;
typedef horizontal_scroll_page_addr_TypeDef	horizontal_scroll_start_page_addr_t;
typedef horizontal_scroll_page_addr_TypeDef	horizontal_scroll_end_page_addr_t;

/* time interval between each scroll step in  terms of  frame frequency */
typedef enum {
	horizontal_scroll_interval_5_frames		=	0b00000000,
	horizontal_scroll_interval_64_frames	=	0b00000001,
	horizontal_scroll_interval_128_frames	=	0b00000010,
	horizontal_scroll_interval_256_frames	=	0b00000011,
	horizontal_scroll_interval_3_frames		=	0b00000100,
	horizontal_scroll_interval_4_frames		=	0b00000101,
	horizontal_scroll_interval_25_frames	=	0b00000110,
	horizontal_scroll_interval_2_frames		=	0b00000111,
}horizontal_scroll_time_interval_TypeDef;
typedef	horizontal_scroll_time_interval_TypeDef	horizontal_scroll_time_interval_t;
//--------------------------------------------------

//continuous vertical and horizontal scroll related-
typedef enum {
	vertical_horizontalRight_scroll	=	0,
	vertical_horizontalLeft_scroll	=	1,
}vertical_horizontal_scroll_direction_TypeDef;
typedef vertical_horizontal_scroll_direction_TypeDef	vertical_horizontal_scroll_direction_t;

typedef horizontal_scroll_page_addr_TypeDef	vertical_horizontal_scroll_start_page_addr_t;
typedef horizontal_scroll_page_addr_TypeDef	vertical_horizontal_scroll_end_page_addr_t;

typedef	horizontal_scroll_time_interval_TypeDef	vertical_horizontal_scroll_time_interval_t;

//--------------------------------------------------

// ############################################################################
/* Charge Pump Command */
typedef enum {
	charge_pump_disable	=	0,	// Disable charge pump(RESET)
	charge_pump_enable	=	1,	// Enable charge pump during display on
}charge_pump_control_TypeDef;
typedef	charge_pump_control_TypeDef	charge_pump_control_t;

// ############################################################################

/**
 * define oled Commands
 */
#define	oled_cmd_set_charge_pump_1			0x8D	// Charge Pump Setting	[_2 A[2] = 0b, Disable charge pump(RESET) A[2] = 1b, Enable charge pump during display on]

#define oled_cmd_display_contrast_1			0x81	// oled contrast [0 - 255] 		[_2 default 0x7F]
#define oled_cmd_display_following 			0xA4	// Output follows RAM content	[default]
#define oled_cmd_display_ignore 			0xA5	// Output ignores RAM content, all pixel light up
#define oled_cmd_display_normal				0xA6	// Normal display				[default]
#define oled_cmd_display_inverse			0xA7	// Inverse display
#define oled_cmd_display_off	 			0xAE	// Display OFF (sleep mode) 	[default]
#define oled_cmd_display_on	 				0xAF	// Display ON

// 0x00 - 0x0F LSB (bit3~0)	This command is only for page addressing mode
#define oled_cmd_set_l_col_start_addr_0		0x00	//Set Lower Column Start Address for Page Addressing Mode
// 0x10 - 0x1F MSB (bit7~4)	This command is only for page addressing mode
#define oled_cmd_set_h_col_start_addr_0		0x10	// Set Higher Column Start Address for Page Addressing Mode
#define oled_cmd_set_memory_addr_mode_1		0x20	// Set Memory Addressing Mode [_2 default 0x02->page addr mode]
// This command is only for horizontal or vertical addressing mode.
#define oled_cmd_set_col_addr_range_1		0x21	// Set Column Address	[_2 default 0	_3 default 127]
#define oled_cmd_set_page_addr_range_1		0x22	// Set Page Address		[_2 default 0	_3 default 7]
// 0xB0 - 0xB7 This command is only for page addressing mode
#define oled_cmd_set_page_start_addr_0		0xB0// Set Page Start Address for Page Addressing Mode

// 0x40 - 0x7F
#define oled_cmd_set_display_start_line		0x40	// Set display RAM display start line register from 0-63
#define oled_cmd_set_segment_remap_n		0xA0	// Set Segment Re-map, column address 0 is mapped to SEG0 [default]
#define oled_cmd_set_segment_remap_y		0xA1	// Set Segment Re-map, column address 127 is mapped to SEG0
#define oled_cmd_set_multiplex_ratio_1		0xA8	// Set Multiplex Ratio			[_2 default 63 -> 64MUX]
#define oled_cmd_set_com_scan_dir_increase	0xC0	// Set COM Output Scan Direction  0 -> com-1
#define oled_cmd_set_com_scan_dir_decrease	0xC8	// Set COM Output Scan Direction  com-1 -> 0
#define oled_cmd_set_display_offset_1		0xD3	// Set vertical shift by COM from 0d~63d [_2 default 0x00]
#define oled_cmd_set_com_pins_1				0xDA	// Set COM Pins
#define oled_cmd_set_clk_div_1				0xD5	// Set Display Clock Divide Ratio/Oscillator Frequency [_2 default 0x80]
#define oled_cmd_set_pre_charge_period_1	0xD9	// Set Pre-charge Period		[_2 default 0x22]
#define oled_cmd_set_Vcomh_deselect_level_1	0xDB	// Set VCOMH Deselect Level		[_2 default 0x20 0.77Vcc 0x00 0.65Vcc 0x30 0.85Vcc]
#define	oled_cmd_nop						0xE3	// Command for no operation


#define oled_cmd_set_horizontal_right_scroll				0x26	// continues horizontal right scroll
#define oled_cmd_set_horizontal_left_scroll					0x27	// continues horizontal left  scroll (Horizontal scroll by 1 column)
#define oled_cmd_set_vertical_and_right_horizontal_scroll 	0x29	// continues vertical scroll and horizontal right scroll
#define oled_cmd_set_vertical_and_left_horizontal_scroll	0x2A	// continues vertical scroll and horizontal left  scroll (Horizontal scroll by 1 column)

#define oled_cmd_set_deactivate_scroll			0x2E
#define	oled_cmd_set_activate_scroll			0x2F
#define oled_cmd_set_vertical_scroll_area		0xA3



void I2C_Device_Scan(void);

uint8_t oled_Set_Contrast(uint8_t ContrastVal);
uint8_t oled_Set_Display_Follow_RAM_Or_No(RAM_Output_t RAM_Output);
uint8_t oled_Set_Display_Normal_Inverse(display_Way_t display_Way);
uint8_t oled_Set_Display_ON_OFF(display_Switch_t	display_Switch);

uint8_t oled_Set_Column_Start_Addr_PageMode(uint8_t columnVal);
uint8_t oled_Set_Memory_Addr_Mode(addr_Mode_t	addr_Mode);
uint8_t oled_Set_Column_Start_End_Addr_HVMode(uint8_t columnStartAddr, uint8_t columnEndAddr);
uint8_t oled_Set_Page_Start_End_Addr_HVMode(uint8_t pageStartAddr, uint8_t pageEndAddr);
uint8_t oled_Set_Page_Start_Addr_PageMode(uint8_t pageVal);

uint8_t oled_Set_Display_Start_Line(uint8_t startLineVal);
uint8_t oled_Set_Segment_Map(segment_Map_t segment_Map);
uint8_t oled_Set_MUX_Ratio(uint8_t MUX_Ratio);
uint8_t oled_Set_Com_Map_Output_Scan_Dirct(com_Map_t com_Map);
uint8_t oled_Set_Display_Offset_Vertical(uint8_t offsetVal);
uint8_t oled_Set_Com_Pins_Hardware_Config(common_Hardware_Config_t common_Hardware_Config);

uint8_t oled_Set_Display_Clock_Parameter(uint8_t Fosc, uint8_t factor_D);
uint8_t oled_Set_PreCharge_Period(uint8_t phase_1_period, uint8_t phase_2_period);
uint8_t oled_Set_Vcomh_DeSelect_Level(Vcomh_Level_t Vcomh_Level);

uint8_t oled_Set_Horizontal_Scroll(
		horizontal_scroll_direction_t		horizontal_scroll_dir,
		horizontal_scroll_start_page_addr_t	horizontal_scroll_start_page,
		horizontal_scroll_time_interval_t	horizontal_scroll_time_interval,
		horizontal_scroll_end_page_addr_t	horizontal_scroll_end_page);
uint8_t oled_Set_Vertical_Horizontal_Scroll(
		vertical_horizontal_scroll_direction_t 			vertical_horizontalDir_scroll,
		vertical_horizontal_scroll_start_page_addr_t	vertical_horizontal_scroll_start_page,
		vertical_horizontal_scroll_time_interval_t		vertical_horizontal_scroll_time_interval,
		vertical_horizontal_scroll_end_page_addr_t		vertical_horizontal_scroll_end_page,
		uint8_t	vertical_scroll_offset);
uint8_t oled_Set_Scroll_Deactivate(void);
uint8_t oled_Set_Scroll_Active(void);
uint8_t oled_Set_Vertical_Scroll_Area(uint8_t vertical_scroll_area_start_row, uint8_t vertical_scroll_area_num_rows);

uint8_t oled_Set_Charge_Pump(charge_pump_control_t	charge_pumt_control);

uint8_t oled_i2c_Init(void);
uint8_t oled_Update_Screen(void);
uint8_t oled_Fill_Screen_Color(oled_color_t	oled_color);
uint8_t oled_Draw_Pixel(uint8_t px, uint8_t py, oled_color_t oled_color);




#endif /* INC_OLED12864_I2C_H_ */
