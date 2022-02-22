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
 * @brief use indicate color(on/off) to filling oled all pixels
 */
static void oled_Filling(oled_color_t color)
{
	for(uint32_t i = 0; i < sizeof(oled_display_buff); i++)
	{
		oled_display_buff[i] = (color == oled_color_Black)? (0x00): (0x0F);
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
	res = oled_Write_CMD(oled_cmd_display_contrast_1);			// Set Contrast first cmd Byte
	if (res != 0)
		return 1;
	oled_Write_CMD(ContrastVal) ? (res = 1) : (res = 0);
	return res;
}


/**
 * @brief set oled display following GDDRAM or ignore GDDRAM and Show fixed content(Entire Display ON)
 * @param RAM_Output: [in]	follow	ignore
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Follow_RAM_Or_No(RAM_Output_t RAM_Output)
{
	uint8_t res = 0;

	switch (RAM_Output) {
		case RAM_output_follow:
			res = oled_Write_CMD(oled_cmd_display_following);
			break;
		case RAM_output_ignore:
			res = oled_Write_CMD(oled_cmd_display_ignore);
			break;
		default:
			res = 1;
			break;
	}
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


// ############################################################################


/**
 * @brief set column start address (lower + higher address byte)
 * @NOTE	** This command is only for page addressing mode
 * 				The column address will be incremented by each data access.
 * @param columnVal: [in] 0~(oled_H_Pix-1)
 */
uint8_t oled_Set_Column_Start_Addr_PageMode(uint8_t columnVal)
{
	uint8_t res = 0;
	if (columnVal < 0 || columnVal >= oled_H_Pix)
		return 1;
	// cmd Set the lower start column address of pointer by command 00h~0Fh.
	uint8_t lowerAddr  = ((columnVal >> 0) & 0x0F) | oled_cmd_set_l_col_start_addr_0;
	// cmd Set the upper start column address of pointer by command 10h~1Fh.
	uint8_t higherAddr = ((columnVal >> 4) & 0x0F) | oled_cmd_set_h_col_start_addr_0;

	// lower nibble of 8bit column address
	res = oled_Write_CMD(lowerAddr);
	if (res != 0)
		return 1;
	// higher nibble of 8bit column address
	res = oled_Write_CMD(higherAddr);
	if (res != 0)
		return 1;

	return res;
}



/**
 * @brief set memory address mode
 * 		[A1:0] 00b,Horizontal Addressing Mode;
 * 		       01b,Vertical Addressing Mode;
 * 		       10b,Page Addressing Mode (RESET);
 * 		       	|- display RAM is read/written, the column address pointer is increased automatically by 1
 * 		       	|- column address pointer reaches column end address, the column address pointer is reset to column start address
 * 		       	|- page address pointer is not changed (Users have to set the new page and column addresses)
 * 		       11b,Invalid
 * @param addr_Mode: [in] Memory Addressing Mode
 * @retval status 0:ok	1/2:error
 */
uint8_t oled_Set_Memory_Addr_Mode(addr_Mode_t	addr_Mode)
{
	uint8_t res = 0;

	// cmd Set Memory Addressing Mode (20h)
	res = oled_Write_CMD(oled_cmd_set_memory_addr_mode_1);
	if (res != 0)
		return 1;
	switch (addr_Mode) {
		case addr_mode_Horizontal:
			res = oled_Write_CMD((uint8_t)addr_mode_Horizontal);
			break;
		case addr_mode_Vertical:
			res = oled_Write_CMD((uint8_t)addr_mode_Vertical);
			break;
		case addr_mode_Page:
			res = oled_Write_CMD((uint8_t)addr_mode_Page);
			break;
		default:
			res = 1;
			break;
	}

	return res;
}


/**
 * @brief Setup column start and end address
 * @NOTE	** This command is only for horizontal or vertical addressing mode
 * @param columnStartAddr: [in] column start address, Column start address, range : 0-127d, (RESET=0d)
 * @param columnEndAddr  : [in] column end   address, Column end   address, range : 0-127d, (RESET =127d)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Column_Start_End_Addr_HVMode(uint8_t columnStartAddr, uint8_t columnEndAddr)
{
	uint8_t res = 0;

	if (columnStartAddr < 0 || columnStartAddr >= oled_H_Pix || columnEndAddr < 0 || columnEndAddr >= oled_H_Pix)
		return 1;

	res = oled_Write_CMD(oled_cmd_set_col_addr_range_1);
	if (res != 0)
		return 1;

	uint8_t startAddr = (columnStartAddr & 0b01111111);
	uint8_t   endAddr = (columnEndAddr   & 0b01111111);
	res = oled_Write_CMD(startAddr);
	if (res != 0)
		return 1;
	res = oled_Write_CMD(endAddr);
	if (res != 0)
		return 1;

	return res;
}


/**
 * @brief Setup page start and end address
 * @NOTE	** This command is only for horizontal or vertical addressing mode
 * @param pageStartAddr: [in] Page start Address, range : 0-7d,  (RESET = 0d)
 * @param pageEndAddr  : [in] Page end   Address, range : 0-7d,  (RESET = 7d)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Page_Start_End_Addr_HVMode(uint8_t pageStartAddr, uint8_t pageEndAddr)
{
	uint8_t res = 0;

	if (pageStartAddr < 0 || pageStartAddr >= (oled_V_Pix/8) || pageEndAddr < 0 || pageEndAddr >= (oled_V_Pix/8))
		return 1;

	res = oled_Write_CMD(oled_cmd_set_page_addr_range_1);
	if (res != 0)
		return 1;

	uint8_t startAddr = (pageStartAddr & 0b00000111);
	uint8_t   endAddr = (pageEndAddr   & 0b00000111);
	res = oled_Write_CMD(startAddr);
	if (res != 0)
		return 1;
	res = oled_Write_CMD(endAddr);
	if (res != 0)
		return 1;

	return res;
}


/**
 * @brief Set GDDRAM Page Start Address (PAGE0~PAGE7) for Page Addressing Mode using X[2:0]
 * @NOTE	** This command is only for page addressing mode	0 ~ (oled_V_Pix/8)-1
 * @param pageVal: [in] page start addr value
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Page_Start_Addr_PageMode(uint8_t pageVal)
{
	uint8_t res = 0;

	if (pageVal < 0 || pageVal >= (oled_V_Pix/8))
		return 1;

	// cmd Set the page start address of the target display location by command B0h to B7h.
	uint8_t pageStartAddr = (pageVal & 0b00000111) | oled_cmd_set_page_start_addr_0;
	res = oled_Write_CMD(pageStartAddr);

	return res;
}

// ############################################################################


/**
 * @brief Set display RAM display start line register from 0-63 using X5X3X2X1X0
 * @param startLineVal: [in] Set Display Start Line  0 ~ oled_V_Pix-1
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Start_Line(uint8_t startLineVal)
{
	uint8_t res = 0;
	if (startLineVal < 0 || startLineVal >= oled_V_Pix)
		return 1;

	uint8_t startLine = (startLineVal & 0b00011111) | oled_cmd_set_display_start_line;
	res = oled_Write_CMD(startLine);

	return res;
}


/**
 * @brief Set Segment Re-map, 	A0h,X[0]=0b: column address 0 is mapped to SEG0 (RESET)
 * 								A1h,X[0]=1b: column address 127 is mapped to SEG0
 * @param	segment_Map_t:	[in] keep default or remap
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Segment_Map(segment_Map_t segment_Map)
{
	uint8_t res = 0;

	uint8_t segmentMap = (segment_Map & 0x01) | oled_cmd_set_segment_remap_n;
	res = oled_Write_CMD(segmentMap);

	return res;
}


/**
 * @brief Set MUX ratio to N+1 MUX N=A[5:0]:
 * 			from 16MUX to 64MUX, RESET= 111111b (i.e. 63d, 64MUX) A[5:0] from 0 to 14 are invalid entry
 * @param	MUX_Ratio	from 16 to 64
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_MUX_Ratio(uint8_t MUX_Ratio)
{
	uint8_t res = 0;
	MUX_Ratio -= 1;
	if (MUX_Ratio < 15 || MUX_Ratio >= 64)
		return 1;
	res = oled_Write_CMD(oled_cmd_set_multiplex_ratio_1);
	if (res != 0)
		return 1;

	uint8_t muxRatio = (MUX_Ratio & 0b00111111);
	res = oled_Write_CMD(muxRatio);
	return res;
}


/**
 * @brief Set COM Output Scan Direction, COM remap or not
 * 			C0h, X[3]=0b: normal mode (RESET) Scan from COM0 to COM[N –1]
 * 			C8h, X[3]=1b: remapped mode. Scan from COM[N-1] to COM0
 * 			[Where N is the Multiplex ratio   oled_Set_MUX_Ratio() ]
 * @param com_Map: [in] com scan dirction (remap or not)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Com_Map_Output_Scan_Dirct(com_Map_t com_Map)
{
	uint8_t res = 0;

	switch (com_Map) {
		case common_normal_mapping:
			res = oled_Write_CMD(oled_cmd_set_com_scan_dir_increase);
			break;
		case common_remapping:
			res = oled_Write_CMD(oled_cmd_set_com_scan_dir_decrease);
			break;
		default:
			res = 1;
			break;
	}

	return res;
}


/**
 * @brief set display vertical offset value 0~63
 * @param offsetVal: [in] vertical offset value
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Offset_Vertical(uint8_t offsetVal)
{
	uint8_t res = 0;

	if (offsetVal < 0 || offsetVal >= oled_V_Pix)
		return 1;

	res = oled_Write_CMD(oled_cmd_set_display_offset_1);
	if (res != 0)
		return 1;

	uint8_t offsetValue = (offsetVal & 0b00111111);
	res = oled_Write_CMD(offsetValue);

	return res;
}


/**
 * @brief Set COM Pins Hardware Configuration
 * @param common_Hardware_Config: [in] common config parameters
 * @retval staus	0:ok	1:error
 */
uint8_t oled_Set_Com_Pins_Hardware_Config(common_Hardware_Config_t common_Hardware_Config)
{
	uint8_t res = 0;

	res = oled_Write_CMD(oled_cmd_set_com_pins_1);
	if (res != 0)
		return 1;

	res = oled_Write_CMD(common_Hardware_Config);

	return res;
}

// ############################################################################

/**
 * @brief Set Display Clock Divide Ratio/Oscillator Frequency
 * 			DCLK = Fosc / D					display clock frequency
 * 			Ffrm = Fosc / (D*K*No.of MUX)	frame frequency of display
 * 	A[3:0] : Define the divide ratio (D) of the display clocks (DCLK):
 * 		Divide ratio= A[3:0] + 1, RESET is 0000b (divide ratio = 1)
 * 	A[7:4] : Set the Oscillator Frequency, FOSC.
 * 		Oscillator Frequency increases with the value of A[7:4] and vice versa.
 * 		RESET is 1000b Range:0000b~1111b   Frequency increases as setting value increases.
 * @param Fosc: [in] D5h A[7:4] The higher the register setting results in higher frequency
 * @param factor_D: [in] division factor D    DCLK=FOSC/D (D -> D5h A[3:0]bit)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Clock_Parameter(uint8_t Fosc, uint8_t factor_D)
{
	uint8_t res = 0;

	if (factor_D < 0 || factor_D > 15 || Fosc < 0 || Fosc > 15)
		return 1;

	uint8_t clockVal = (Fosc << 4) |factor_D;
	res = oled_Write_CMD(oled_cmd_set_clk_div_1);
	if (res != 0)
		return 1;
	res = oled_Write_CMD(clockVal);

	return res;
}


/**
 * @brief Set Pre-charge Period
 * @param phase_1_period: [in] Phase 1 period of up to 15 DCLK clocks 0 is invalid entry   (RESET=2h)
 * 			1 ~ 15 A[3:0]
 * @param phase_2_period: [in] Phase 2 period of up to 15 DCLK clocks 0 is invalid entry   (RESET=2h)
 * 			1 ~ 15 A[7:4]
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_PreCharge_Period(uint8_t phase_1_period, uint8_t phase_2_period)
{
	uint8_t res = 0;

	if (phase_1_period < 1 || phase_1_period > 15 || phase_2_period < 1 || phase_2_period > 15)
		return 1;

	uint8_t phasePeriod = (phase_2_period << 4) | phase_1_period;
	res = oled_Write_CMD(oled_cmd_set_pre_charge_period_1);
	if (res != 0)
		return 1;
	res = oled_Write_CMD(phasePeriod);

	return res;
}


/**
 * @brief Set VCOMH Deselect Level
 * @param Vcomh_Level: [in] VCOMH Deselect Leve
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Vcomh_DeSelect_Level(Vcomh_Level_t Vcomh_Level)
{
	uint8_t res = 0;

	res = oled_Write_CMD(oled_cmd_set_Vcomh_deselect_level_1);
	if (res != 0)
		return 1;

	res = oled_Write_CMD(Vcomh_Level);

	return res;
}

// ############################################################################
/**
 * @brief Continuous Horizontal Scroll Setup [cmd 0x26/7]
 * 			Horizontal scroll by 1 column
 * @param horizontal_scroll_dir: [in] horizontal scroll direction
 * @param horizontal_scroll_start_page: [in] start page address
 * @param horizontal_scroll_time_interval: [in] frames interval time
 * @param horizontal_scroll_end_page: [in] end page address
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Horizontal_Scroll(
		horizontal_scroll_direction_t		horizontal_scroll_dir,
		horizontal_scroll_start_page_addr_t	horizontal_scroll_start_page,
		horizontal_scroll_time_interval_t	horizontal_scroll_time_interval,
		horizontal_scroll_end_page_addr_t	horizontal_scroll_end_page)
{
	uint8_t res = 0;

	// cmd 0x26/ 0x27
	switch (horizontal_scroll_dir)
	{
	case horizontal_scroll_right:
		res = oled_Write_CMD(oled_cmd_set_horizontal_right_scroll);
		break;
	case horizontal_scroll_left:
		res = oled_Write_CMD(oled_cmd_set_horizontal_left_scroll);
		break;
	}
	if (res != 0)
		return 1;
	// cmd dummy	0x00	[A]
	res = oled_Write_CMD(0x00);
	if (res != 0)
		return 1;

	// horizontal scroll start page addr	[B]
	res = oled_Write_CMD(horizontal_scroll_start_page);
	if (res != 0)
		return 1;

	// set time interval each scroll step	[C]
	res = oled_Write_CMD(horizontal_scroll_time_interval);
	if (res != 0)
		return 1;

	// end page addr	[D]
	res = oled_Write_CMD(horizontal_scroll_end_page);
	if (res != 0)
		return 1;

	// cmd dummy	0x00	[E]
	res = oled_Write_CMD(0x00);
	if (res != 0)
		return 1;

	// cmd dummy	0xFF	[F]
	res = oled_Write_CMD(0xFF);
	if (res != 0)
		return 1;

	return res;
}


/**
 * @brief Continuous Vertical Horizontal Scroll Setup [cmd 0x29/A]
 * @NOTE	** No continuous vertical scrolling is available
 * @param vertical_horizontalDir_scroll: [in] vertical horizontalX scroll direction
 * @param vertical_horizontal_scroll_start_page: [in] start page address
 * @param vertical_horizontal_scroll_time_interval: [in] frames interval time
 * @param vertical_horizontal_scroll_end_page: [in] end page address
 * @param vertical_scroll_offset: [in] vertical scrolling offset, Scroll delta. [1 line to 64 line]
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Vertical_Horizontal_Scroll(
		vertical_horizontal_scroll_direction_t 			vertical_horizontalDir_scroll,
		vertical_horizontal_scroll_start_page_addr_t	vertical_horizontal_scroll_start_page,
		vertical_horizontal_scroll_time_interval_t		vertical_horizontal_scroll_time_interval,
		vertical_horizontal_scroll_end_page_addr_t		vertical_horizontal_scroll_end_page,
		uint8_t	vertical_scroll_offset)
{
	uint8_t res = 0;

	// cmd 0x29/A
	switch (vertical_horizontalDir_scroll) {
		case vertical_horizontalRight_scroll:
			res = oled_Write_CMD(oled_cmd_set_vertical_and_right_horizontal_scroll);
			break;
		case vertical_horizontalLeft_scroll:
			res = oled_Write_CMD(oled_cmd_set_vertical_and_left_horizontal_scroll);
			break;
		default:
			res = 1;
			break;
	}
	if (res != 0)
		return 1;

	// cmd dummy 0x00	[A]
	res = oled_Write_CMD(0x00);
	if (res != 0)
		return 1;

	// cmd start page address	[B]
	res = oled_Write_CMD(vertical_horizontal_scroll_start_page);
	if (res != 0)
		return 1;

	// cmd set time interval	[C]
	res = oled_Write_CMD(vertical_horizontal_scroll_time_interval);
	if (res != 0)
		return 1;

	// cmd end page address	[D]
	res = oled_Write_CMD(vertical_horizontal_scroll_end_page);
	if (res != 0)
		return 1;

	// cmd vertical scrolling offset	[E]
	res = oled_Write_CMD(vertical_scroll_offset & 0b00111111);

	return res;
}


/**
 * @brief	Stop scrolling that is configured by command 26h/27h/29h/2Ah
 * @NOTE	**	After sending 2Eh command to deactivate the scrolling action,
 * 				the ram data needs to be rewritten
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Set_Scroll_Deactivate(void)
{
	uint8_t res = 0;

	// cmd deactive scroll 0x2E
	res = oled_Write_CMD(oled_cmd_set_deactivate_scroll);

	return res;
}


/**
 * @brief	Start scrolling that is configured by the scrolling setup commands:
 * 			26h/27h/29h/2Ah with the following valid sequences:
 * 				Valid command sequence 1: 26h ;2Fh.
 * 				Valid command sequence 2: 27h ;2Fh.
 * 				Valid command sequence 3: 29h ;2Fh.
 * 				Valid command sequence 4: 2Ah ;2Fh.
 * 			For example,  if “26H; 2AH; 2FH.” commands are issued, the setting in the last scrolling setup command,
 * 			i.e. 2AH in this case, will be executed.
 * 			In other words, setting in the last scrolling setup command overwrites the setting in the previous scrolling setup commands.
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Set_Scroll_Active(void)
{
	uint8_t res = 0;

	// cmd active scroll 0x2F
	res = oled_Write_CMD(oled_cmd_set_activate_scroll);

	return res;
}


/**
 * @brief	Set Vertical Scroll Area
 *	A[5:0] : Set No. of rows in top fixed area.
 *	  The No. of rows in top fixed area is referenced to the top of the GDDRAM (i.e. row 0).[RESET = 0]
 *	B[6:0] : Set No. of rows in scroll area.
 *	  This is the number of rows to be used for vertical scrolling. The scroll area starts in the first row below the top fixed area. [RESET = 64]
 * @NOTE	** (1) A[5:0]+B[6:0] <= MUX ratio
 * 			   (2)  B[6:0] <= MUX ratio
 * 			   (3a) Vertical scrolling offset value (E[5:0] in 29h/2Ah) < B[6:0]
 * 			   (3b) Set Display Start Line (X5X4X3X2X1X0 of 40h~7Fh) < B[6:0]
 * 			   (4) The last row of the scroll area shifts to the first row of the scroll area.
 * 			   (5) For 64d MUX  display
 * 			   		A[5:0] = 0, B[6:0]=64 : whole area scrolls
 * 			   		A[5:0]= 0, B[6:0] < 64 : top area scrolls
 * 			   		A[5:0]  + B[6:0] < 64 : central area scrolls
 * 			   		A[5:0]  + B[6:0] = 64 : bottom area scrolls
 * @param	vertical_scroll_area_top_row: [in] Start row, above this row is fixed and won't scroll	[default 0]
 * @param	vertical_scroll_area_num_rows: [in] The number of rows in the given vertical scroll area[default 64]
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Set_Vertical_Scroll_Area(uint8_t vertical_scroll_area_start_row, uint8_t vertical_scroll_area_num_rows)
{
	uint8_t res = 0;

	// cmd 0xA3
	res = oled_Write_CMD(oled_cmd_set_vertical_scroll_area);
	if (res != 0)
		return 1;

	res = oled_Write_CMD(vertical_scroll_area_start_row & 0b00111111);
	if (res != 0)
		return 1;

	res = oled_Write_CMD(vertical_scroll_area_num_rows & 0b01111111);

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

	status += oled_Set_Page_Start_Addr_PageMode(0x00);				// Set Page Start Address for Page Addressing Mode,0-7

	status += oled_Write_CMD(oled_cmd_set_com_scan_dir_decrease);   // Set COM Output Scan Direction

	status += oled_Set_Column_Start_Addr_PageMode(0x00);			// Set column start address[page mode]

	status += oled_Set_Display_Start_Line(0x00);					// Set start line address

	status += oled_Set_Contrast(0x00);								// set contrast control register 0x00-0xFF

	status += oled_Set_Segment_Map(segment_default_mapping);

	status += oled_Set_Display_Normal_Inverse(display_normal);		// Set normal display

	status += oled_Set_MUX_Ratio(oled_V_Pix);						// Set multiplex ratio(1 to 64)

	status += oled_Set_Display_Follow_RAM_Or_No(RAM_output_follow); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content

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



