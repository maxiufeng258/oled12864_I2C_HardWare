/*
 * oled_12864.c
 *
 *  Created on: Feb 21, 2022
 *      Author: maxiufeng
 */

#include <oled12864_I2C.h>
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"

extern I2C_HandleTypeDef oled_i2c;

#define	oled_timeOut	(10*1000)


/* oled's some flag and variables */
typedef struct {
			// 1 update		0 no-update
	uint8_t bufferUpdateFlag;		// If this flag is SET, the oled_display_buff has changed.
			// 1 on		0 off
	uint8_t clear_GDDRAM_Use_0_1_Flag;	// clear all pixels (GDDRAM) value use 0 or 1
}oled_flag_variable_TypeDef;
static oled_flag_variable_TypeDef	oled = {0, 0};

#if	oled_i2c_dma
static uint8_t	oled_i2c_dma_mem_write_flag = 0;
#endif
/* ------------- end ------------- */


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
#if	oled_i2c_dma
	hal_sta = HAL_I2C_Mem_Write_DMA(&oled_i2c, oled_i2c_addr, ctrl_cmd, I2C_MEMADD_SIZE_8BIT, &cmd, sizeof cmd);

	if (hal_sta == HAL_OK)
	{
		uint32_t startTime = HAL_GetTick();
		while (oled_i2c_dma_mem_write_flag != 1  &&  (HAL_GetTick() - startTime) < oled_timeOut);
		if ((HAL_GetTick() - startTime) >= oled_timeOut)
			return 1;
		oled_i2c_dma_mem_write_flag = 0;
	}
	else {
		return 1;
	}
#else
	hal_sta = HAL_I2C_Mem_Write(&oled_i2c, oled_i2c_addr, ctrl_cmd, I2C_MEMADD_SIZE_8BIT, &cmd, sizeof cmd, oled_timeOut);
	if (hal_sta != HAL_OK)
		return 1;
#endif

	return 0;
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
#if	oled_i2c_dma
	hal_sta = HAL_I2C_Mem_Write_DMA(&oled_i2c, oled_i2c_addr, ctrl_data, I2C_MEMADD_SIZE_8BIT, data, len);

	if (hal_sta == HAL_OK)
	{
		uint32_t startTime = HAL_GetTick();
		while (oled_i2c_dma_mem_write_flag != 1  &&  (HAL_GetTick() - startTime) < oled_timeOut);
		if ((HAL_GetTick() - startTime) >= oled_timeOut)
			return 1;
		oled_i2c_dma_mem_write_flag = 0;
	}
	else {
		return 1;
	}
#else
	hal_sta = HAL_I2C_Mem_Write(&oled_i2c, oled_i2c_addr, ctrl_data, I2C_MEMADD_SIZE_8BIT, data, len, oled_timeOut);
	if (hal_sta != HAL_OK)
		return 1;
#endif

	return 0;
}


/**
 * @brief use indicate color(on/off) to filling oled all pixels
 */
static void oled_Fill_GDDRAM_Buffer(oled_color_t color)
{
	for(uint32_t i = 0; i < sizeof(oled_display_buff); i++)
	{
		oled_display_buff[i] = (color == oled_color_Black)? (0x00): (0xFF);
	}
	/* set buffer updated flag */
	oled.bufferUpdateFlag = 1;
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
	// cmd Set Contrast 0x81
	res = oled_Write_CMD(oled_cmd_display_contrast_1);
	if (res != 0)
		return 1;
	// The segment output current increases as the contrast step value increases
	oled_Write_CMD(ContrastVal) ? (res = 1) : (res = 0);
	return res;
}


/**
 * @brief set oled display following GDDRAM or ignore GDDRAM and Show fixed content(Entire Display ON)
 * @NOTE If A5h command is issued, then by using A4h command, the display will resume to the GDDRAM contents.
 * @param RAM_Output: [in]	follow	ignore
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Follow_RAM_Or_No(RAM_Output_t RAM_Output)
{
	uint8_t res = 0;

	switch (RAM_Output) {
		case RAM_output_follow:
			//	cmd 0xA4  A4h command enable display outputs according to the GDDRAM contents.
			//			  A4h command resumes the display from entire display “ON” stage.
			res = oled_Write_CMD(oled_cmd_display_following);
			break;
		case RAM_output_ignore:
			// cmd 0xA5  A5h command forces the entire display to be “ON”, regardless of the contents of the display data RAM.
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
 * @NOTE In normal display a RAM data of 1 indicates an “ON” pixel
 * 			while in inverse display a RAM data of 0 indicates an “ON” pixel
 * @param display_Way_t: [in]  display_normal	display_invers
 * @retval status 0:ok	1:error
 */
uint8_t oled_Set_Display_Normal_Inverse(display_Way_t display_Way)
{
	uint8_t res = 0;
	switch (display_Way) {
		case display_normal:
			//	cmd 0xA6  normal display
			res = oled_Write_CMD(oled_cmd_display_normal);

			break;
		case display_invers:
			//	cmd 0xA7  inverse display
			res = oled_Write_CMD(oled_cmd_display_inverse);

			break;
		default:
			return 1;
	}
	return res;
}


/**
 * @brief oled display On / Off(sleep)
 * @NOTE  These single byte commands are used to turn the OLED panel display ON or OFF.
 * @param display_Switch_t: [in]  display_on	display_off
 * @retval status 0:ok	1:error
 */
uint8_t oled_Set_Display_ON_OFF(display_Switch_t	display_Switch)
{
	uint8_t res = 0;
	switch (display_Switch) {
		case display_on:
			//	cmd	AFh : Display ON
			res = oled_Write_CMD(oled_cmd_display_on);
			break;
		case display_off:
			//	cmd AEh : Display OFF
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
 * 				|- after the display RAM is read/written, the column address pointer is increased automatically by 1
 * 				|- column address pointer reaches column end address, the column address pointer is reset to column start address
 * 				|- and page address pointer is increased by 1
 * 				|- both column and page address pointers reach the end address, the pointers are reset to column start address and page start address
 * 		       01b,Vertical Addressing Mode;
 * 		       	|- after the display RAM is read/written, the page address pointer is increased automatically by 1
 * 		       	|- page address pointer reaches the page end address, the page address pointer is reset to page start address
 * 		       	|- and column address pointer is increased by 1
 * 		       	|- both column and page address pointers reach the end address, the pointers are reset to column start address and page start address
 * 		       10b,Page Addressing Mode (RESET);
 * 		       	|- after the display RAM is read/written, the column address pointer is increased automatically by 1
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
 * @brief Set the column start and end address of the target display location by command 21h.
 * @NOTE	** This command is only for horizontal or vertical addressing mode
 * 		|- This triple byte command specifies column start address and end address of the DDRAM
 *		|- also sets the column address pointer to column start address
 *		|- If horizontal address mode enabled by command 20h, after finishing read/write one column data, column address is automatically increment by 1.
 *		|- column address reach end column address, auto back to reset start column address, page(row) increment to next page(row)
 * @param columnStartAddr: [in] column start address, Column start address, range : 0-127d, (RESET=0d)
 * @param columnEndAddr  : [in] column end   address, Column end   address, range : 0-127d, (RESET =127d)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Column_Start_End_Addr_HVMode(uint8_t columnStartAddr, uint8_t columnEndAddr)
{
	uint8_t res = 0;

	if (columnStartAddr < 0 || columnStartAddr >= oled_H_Pix || columnEndAddr < 0 || columnEndAddr >= oled_H_Pix)
		return 1;

	// cmd 0x21 Set Column Address range
	res = oled_Write_CMD(oled_cmd_set_col_addr_range_1);
	if (res != 0)
		return 1;

	//	Column start address A[6:0]	(RESET=0d)
	//	Column   end address B[6:0]	(RESET=127d)
	uint8_t startAddr = (columnStartAddr & 0b01111111);	//A[6:0]
	uint8_t   endAddr = (columnEndAddr   & 0b01111111);	//B[6:0]
	res = oled_Write_CMD(startAddr);
	if (res != 0)
		return 1;
	res = oled_Write_CMD(endAddr);
	if (res != 0)
		return 1;

	return res;
}


/**
 * @brief Set the page start and end address of the target display location by command 22h.
 * @NOTE	** This command is only for horizontal or vertical addressing mode
 * 		|- This triple byte command specifies page start address and end address of the DDRAM.
 * 		|- also sets the page address pointer to page start address
 * 		|- If vertical address mode enabled by command 20h, after finishing read/write one page data, page address is automatically increment by 1.
 * 		|- page address reach end page address, auto back to reset start page address, column auto increment to next column
 * @param pageStartAddr: [in] Page start Address, range : 0-7d,  (RESET = 0d)
 * @param pageEndAddr  : [in] Page end   Address, range : 0-7d,  (RESET = 7d)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Page_Start_End_Addr_HVMode(uint8_t pageStartAddr, uint8_t pageEndAddr)
{
	uint8_t res = 0;

	if (pageStartAddr < 0 || pageStartAddr >= (oled_V_Pix/8) || pageEndAddr < 0 || pageEndAddr >= (oled_V_Pix/8))
		return 1;

	//	cmd 0x22	Set Page Address range
	res = oled_Write_CMD(oled_cmd_set_page_addr_range_1);
	if (res != 0)
		return 1;

	//	Page start address A[2:0]	(RESET=0)
	//	Page   end address B[2:0]	(RESET=7)
	uint8_t startAddr = (pageStartAddr & 0b00000111);	// A[2:0]
	uint8_t   endAddr = (pageEndAddr   & 0b00000111);	// B[2:0}
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
 * @brief Set [display RAM] display start line register from 0-63 using X5X3X2X1X0
 * @NOTE	With value equal to 0, DRAM row 0 is mapped to COM0.
 * 			With value equal to 1, DRAM row 1 is mapped to COM0 and so on.
 * @param startLineVal: [in] Set Display Start Line  0 ~ oled_V_Pix-1
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Start_Line(uint8_t startLineVal)
{
	uint8_t res = 0;
	if (startLineVal < 0 || startLineVal >= oled_V_Pix)
		return 1;

	//	cmd 0x40 ~ 0x70	Set Display Start Line
	uint8_t startLine = (startLineVal & 0b00011111) | oled_cmd_set_display_start_line;
	res = oled_Write_CMD(startLine);

	return res;
}


/**
 * @brief Set Segment Re-map, 	A0h,X[0]=0b: column address 0 is mapped to SEG0 (RESET)
 * 								A1h,X[0]=1b: column address 127 is mapped to SEG0
 * @NOTE This command changes the mapping between the display data column address and the segment driver.
 * 		 This command only affects subsequent data input.  Data already stored in GDDRAM will have no changes.
 * @param	segment_Map_t:	[in] keep default or remap
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Segment_Map(segment_Map_t segment_Map)
{
	uint8_t res = 0;

	// cmd 0xA0/1 Set Segment Re-map
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
	//	cmd 0xA8  set multiplex ratio value (15 ~ 63)+1
	res = oled_Write_CMD(oled_cmd_set_multiplex_ratio_1);
	if (res != 0)
		return 1;

	//	cmd 15d~63d	multiplex ratio value
	uint8_t muxRatio = (MUX_Ratio & 0b00111111);
	res = oled_Write_CMD(muxRatio);
	return res;
}


/**
 * @brief Set COM Output Scan Direction, COM remap or not
 * 			C0h, X[3]=0b: normal mode (RESET) Scan from COM0 to COM[N –1]
 * 			C8h, X[3]=1b: remapped mode. Scan from COM[N-1] to COM0
 * 			[Where N is the Multiplex ratio   oled_Set_MUX_Ratio() ]
 * @NOTE  --------------------------------------------------------
 * 		------>	*---------------------*  com0 <-----
 * 		↑		*	    oled panel	  *            ↑
 * 		↑	-->	*---------------------*	 com63<--  ↑
 * 		↑	↑									↑  ↑
 * 		<---↑------	*------------* row63  ------>  ↑
 * remap	↑		*   GDDRAM   * normol mapping  ↑	// Normal mapping
 * 			<------ *------------* row0  ---------->
 * 	// remapping
 * @param com_Map: [in] com scan dirction (remap or not)
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Com_Map_Output_Scan_Dirct(com_Map_t com_Map)
{
	uint8_t res = 0;

	switch (com_Map) {
		case com_normal_mapping:
			// cmd C0h	DDRAM row-0  ->  COM0
			res = oled_Write_CMD(oled_cmd_set_com_scan_dir_increase);
			break;
		case com_remapping:
			// cmd C8h	DDRAM row-63 ->  COM0
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
 * @NOTE	both RAM and oled-row offset [together move]
 * @NOTE	this is a double byte command
 * @param offsetVal: [in] vertical offset value
 * @retval status	0:ok	1:error
 */
uint8_t oled_Set_Display_Offset_Vertical(uint8_t offsetVal)
{
	uint8_t res = 0;

	if (offsetVal < 0 || offsetVal >= oled_V_Pix)
		return 1;

	//	cmd 0xD3 set display offset (DDRAM oled_row indicate line start)
	res = oled_Write_CMD(oled_cmd_set_display_offset_1);
	if (res != 0)
		return 1;

	// cmd set offset line value from 0 to 63
	uint8_t offsetValue = (offsetVal & 0b00111111);
	res = oled_Write_CMD(offsetValue);

	return res;
}


/**
 * @brief Set COM Pins Hardware Configuration
 * @NOTE  This command sets the COM signals pin configuration to match the OLED panel hardware layout.
 * @param common_Hardware_Config: [in] common config parameters
 * @retval staus	0:ok	1:error
 */
uint8_t oled_Set_Com_Pins_Hardware_Config(common_Hardware_Config_t common_Hardware_Config)
{
	uint8_t res = 0;

	// cmd Set COM Pins Hardware Configuration (DAh)
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
	// cmd 0xD5	Set Display Clock Divide Ratio/ Oscillator Frequency
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

// ############################################################################
/**
 * @brief	Charge Pump Setting
 * @NOTE	The Charge Pump must be enabled by the following command:
 * 			  |- 8Dh -> Charge Pump Setting
 * 			  |- 14h -> Enable Charge Pump
 * 			  |- AFh -> Display ON
 * @param	charge_pumt_control: [in] charge pump enable or disable
 * @retval	status	0:ok	>0:error
 */
uint8_t oled_Set_Charge_Pump(charge_pump_control_t	charge_pumt_control)
{
	uint8_t res = 0;

	//	cmd 0x8D	Charge Pump Setting
	res = oled_Write_CMD(oled_cmd_set_charge_pump_1);
	if (res != 0)
		return 1;

	switch (charge_pumt_control) {
		case charge_pump_disable:
			res = oled_Write_CMD(0x10);
			break;
		case charge_pump_enable:
			res = oled_Write_CMD(0x14);
			break;
		default:
			res = 1;
			break;
	}

	return res;
}

// ############################################################################

/**
  * @brief oled 12864 init
  * @retval status 0:init ok    1:init error
  */
uint8_t oled_i2c_Init(void)
{
	// status flag
	int status = 0;

	// Wait for the screen to boot
	HAL_Delay(500);

	// oled initial process
	/* 0.Display Off */
	status += oled_Set_Display_ON_OFF(display_off);

	/* 1.Memory address mode : horizontal mode */
	status += oled_Set_Memory_Addr_Mode(addr_mode_Horizontal);
	status += oled_Set_Page_Start_End_Addr_HVMode(0, 7);
	status += oled_Set_Column_Start_End_Addr_HVMode(0, 127);

	/* 2.Set MUX Ratio */
	status += oled_Set_MUX_Ratio(oled_V_Pix);

	/* 3.Set Display Offset */
	status += oled_Set_Display_Offset_Vertical(0);

	/* 4.Set Display Start Line */
	status += oled_Set_Display_Start_Line(0);

	/* 5.Set Segment Re-map */
	status += oled_Set_Segment_Map(segment_remapping);

	/* 6.Set COM Output Scan Direction (map) */
	status += oled_Set_Com_Map_Output_Scan_Dirct(com_remapping);

	/* 7. Set Pre-charge Period value phase1(1~15) phase2(1~15) */
	status += oled_Set_PreCharge_Period(2, 2);

	/* 8.Set COM Pins hardware configuration */
	status += oled_Set_Com_Pins_Hardware_Config(common_alternative_config|common_disable_left_right_remap);

	/* 9.Set Contrast Control */
	status += oled_Set_Contrast(0x7F);

	/* 10.Disable Entire Display On */
	status += oled_Set_Display_Follow_RAM_Or_No(RAM_output_follow);

	/* 11.Set Normal Display */
	status += oled_Set_Display_Normal_Inverse(display_normal);

	/* 12.Set Osc Frequency */
	status += oled_Set_Display_Clock_Parameter(0x0F, 0);

	/* 13.Enable charge pump regulator */
	status += oled_Set_Charge_Pump(charge_pump_enable);

	/* 14.clear whole screen write 0 to GDDRAM */
	status += oled_Fill_Screen_Color(oled_color_Black);

	/* 15.Display On */
	status += oled_Set_Display_ON_OFF(display_on);

//	oled_Clear_Screen();

	if (status != 0)
		return status;

	return 0;
}



/**
 * @brief write oled_cmd_display_contrast_1 into oled GDDRAM
 * @retval status 0:write ok	1:write error
 */
uint8_t oled_Update_Screen(void)
{
	if (oled.bufferUpdateFlag == 0)
		return 0;	// don't need updata screen

	uint8_t res = 0;
	res = oled_Write_Data(oled_display_buff, sizeof(oled_display_buff));

    oled.bufferUpdateFlag = 0;
    return res;
}


/**
 * @brief	set oled all pixels -> '1'
 * 		  include oled screen update
 * @param	oled_color: [in] oled_color_black	oled_color_white
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Fill_Screen_Color(oled_color_t	oled_color)
{
	uint8_t res = 0;

	//	set oled_buffer value
	switch (oled_color) {
		case oled_color_Black:
			oled_Fill_GDDRAM_Buffer(oled_color_Black);
			oled.clear_GDDRAM_Use_0_1_Flag = 0;
			break;
		case oled_color_White:
			oled_Fill_GDDRAM_Buffer(oled_color_White);
			oled.clear_GDDRAM_Use_0_1_Flag = 1;
			break;
		default:
			return 1;
			break;
	}


	//	update screen, write oled_buffer to GDDRAM
	res = oled_Update_Screen();

	return res;
}


/**
 * @brief 	Draw one pixel at the specified (x,y) position.
 * @param	px: [in] x value (0 ~ 127) column	[< oled_H_pix]
 * @param	py: [in] y value (0 ~ 63)  row		[< oled_V_pix]
 * @param	pixel_control:	[in] pixel_control_ON	pixel_control_OFF
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_Pixel(uint8_t px, uint8_t py, pixel_control_t pixel_control)
{
	if (px < 0 || px >= oled_H_Pix || py < 0 || py >= oled_V_Pix)
		return 1;
	// Set the (x,y) pixel value to the opposite of the background value
	if (pixel_control == pixel_control_ON)
	{
		//	GDDRAM buffer -> oled display	1:on	0:off(background)
		if (oled.clear_GDDRAM_Use_0_1_Flag == 0)
		{
			oled_display_buff[px + (py/8)*oled_H_Pix] |=  (0x01 << (py%8));
		}
		//	GDDRAM buffer -> oled display	0:on	1:off(background)
		else if (oled.clear_GDDRAM_Use_0_1_Flag == 1)
		{
			oled_display_buff[px + (py/8)*oled_H_Pix] &= ~(0x01 << (py%8));
		}
		else {
			return 1;
		}
		// set buffer updata flag value
		oled.bufferUpdateFlag = 1;
	}
	// set (x,y) pixel value equal background value
	else if (pixel_control == pixel_control_OFF)
	{
		//	GDDRAM buffer -> oled display	1:on	0:off(background)
		if (oled.clear_GDDRAM_Use_0_1_Flag == 0)
		{
			oled_display_buff[px + (py/8)*oled_H_Pix] &=  ~(0x01 << (py%8));
		}
		//	GDDRAM buffer -> oled display	0:on	1:off(background)
		else if (oled.clear_GDDRAM_Use_0_1_Flag == 1)
		{
			oled_display_buff[px + (py/8)*oled_H_Pix] |=  (0x01 << (py%8));
		}
		else {
			return 1;
		}
		// set buffer updata flag value
		oled.bufferUpdateFlag = 1;
	}
	else
	{
		return 1;
	}

	return 0;
}


/**
 * @brief	Draw Line from Ps(x0,y0)     to      Pe(x1,y1)
 * 			point to point	start point         End point
 * @param	x0:	[in] start point x value
 * @param	y0:	[in] start point y value
 * @param	x1:	[in]   end point x value
 * @param	y1: [in]   end point y value
 * @param	line_width: [in] line width (n pixel)
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_Line (
		uint8_t x0, uint8_t y0,
		uint8_t x1, uint8_t y1,
		line_width_t line_width)
{
	if (x0 == x1 && y0 == y1)
		return 1;

	if (x0 > x1)
	{
		uint8_t m_x = x1, m_y = y1;
		x1 = x0;	y1 = y0;
		x0 = m_x;	y0 = m_y;
	}

	float px = 0, py = 0;
	float tempx0 = x0, tempy0 = y0, tempx1 = x1, tempy1 = y1;

	if (x0 > 127 || x1 > 127 || y0 > 64 || y1 > 64)
		return 1;

	// vertical direction
	if (tempx0 == tempx1)
	{
		if (y0 > y1)
		{
			uint8_t im_x = x1, im_y = y1;
			x1 = x0;	y1 = y0;
			x0 = im_x;	y0 = im_y;
		}

		tempx0 = x0; tempy0 = y0; tempx1 = x1; tempy1 = y1;

		// draw slim width line (1 pixel)
		for (py = tempy0; py <= tempy1; py++)
		{
			px = tempx0;
			oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
		}

		// draw medium width line (2 pixel)
		if (line_width == line_width_medium)
		{
			tempx0 = x0+1, tempy0 = y0, tempx1 = x1+1, tempy1 = y1;
			for (py = tempy0; py <= tempy1; py++)
			{
				px = tempx0;
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}
		}

		if (line_width == line_width_bold)
		{
			tempx0 = x0+1, tempy0 = y0, tempx1 = x1+1, tempy1 = y1;
			for (py = tempy0; py <= tempy1; py++)
			{
				px = tempx0;
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}

			tempx0 = x0-1, tempy0 = y0, tempx1 = x1-1, tempy1 = y1;
			for (py = tempy0; py <= tempy1; py++)
			{
				px = tempx0;
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}
		}
	}
	/* other direction */
	else
	{

		// draw slim width line (1 pixel)
		for (px = tempx0; px <= tempx1; px++)
		{
			py = (((tempy1 - tempy0)/(tempx1 - tempx0)) * (px - tempx1) + tempy1);
//			printf("1_> (%d, %d)\r\n", (uint8_t)px, (uint8_t)py);
			oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
		}

		// draw medium width line (2 pixel)
		if (line_width == line_width_medium)
		{
			tempx0 = x0, tempy0 = y0+1, tempx1 = x1, tempy1 = y1+1;
			for (px = tempx0; px <= tempx1; px++)
			{
				py = (((tempy1 - tempy0)/(tempx1 - tempx0)) * (px - tempx1) + tempy1);
//				printf("2_> (%d, %d)\r\n", (uint8_t)px, (uint8_t)py);
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}
		}

		// draw bold width line (3 pixel)
		if (line_width == line_width_bold)
		{
			tempx0 = x0, tempy0 = y0+1, tempx1 = x1, tempy1 = y1+1;
			for (px = tempx0; px <= tempx1; px++)
			{
				py = (((tempy1 - tempy0)/(tempx1 - tempx0)) * (px - tempx1) + tempy1);
//				printf("3_> (%d, %d)\r\n", (uint8_t)px, (uint8_t)py);
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}

			tempx0 = x0, tempy0 = y0-1, tempx1 = x1, tempy1 = y1-1;
			for (px = tempx0; px <= tempx1; px++)
			{
				py = (((tempy1 - tempy0)/(tempx1 - tempx0)) * (px - tempx1) + tempy1);
//				printf("3_> (%d, %d)\r\n", (uint8_t)px, (uint8_t)py);
				oled_Draw_Pixel((uint8_t)(px), (uint8_t)(py), pixel_control_ON);
			}
		}
	}

	return 0;
}


/**
 * @brief	Draw a rectangle given two points left-top and right-bottom
 * 	1	*-------------------*	3
 * 		|		fill		|
 * 		|		fill		|
 * 	2	*-------------------*	4
 *
 * @param	x0:	[in] start point x value
 * @param	y0:	[in] start point y value
 * @param	x1:	[in]   end point x value
 * @param	y1: [in]   end point y value
 * @param	line_width: [in] line width (n pixel)
 * @param	praphic_fill_effect: [in] graphic internal fill effect [solid or hollow]
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_rectangle (
		uint8_t x0, uint8_t y0,
		uint8_t x1, uint8_t y1,
		line_width_t line_width,
		graphic_fill_effect_t graphic_fill_effect)
{
//	if (x0 > x1 || y0 > y1)
//		return 1;

	if (x0 == x1 || y0 == y1)
		return 1;

	// Whether the rectangle needs to fill the flag
	//	1:need	0:don't need
	uint8_t rectNeedFillFlag = 1;

	if (x0 > x1)
	{
		uint8_t temp_x_0 = x0;
		x0 = x1;	x1 = temp_x_0;
	}
	if (y0 > y1)
	{
		uint8_t temp_y_0 = y0;
		y0 = y1;	y1 = temp_y_0;
	}

	// left-top point
	uint8_t p1_x = x0, p1_y = y0;
	// left-bottom point
	uint8_t p2_x = x0, p2_y = y1;
	// right-top point
	uint8_t p3_x = x1, p3_y = y0;
	// right-bottom point
	uint8_t p4_x = x1, p4_y = y1;

	// draw rectangle frame
	oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
	oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
	oled_Draw_Line(p2_x, p2_y, p4_x, p4_y, line_width_slim);
	oled_Draw_Line(p3_x, p3_y, p4_x, p4_y, line_width_slim);

	switch (line_width) {
		case line_width_slim:
		break;
		case line_width_medium:
			p1_x = p1_x+1; p1_y = p1_y+1;	p3_x = p3_x-1; p3_y = p3_y+1;
			p2_x = p2_x+1; p2_y = p2_y-1;	p4_x = p4_x-1; p4_y = p4_y-1;

			if (p1_x == p3_x && p1_y == p2_y)
			{
				oled_Draw_Pixel(p1_x, p1_y, pixel_control_ON);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_x == p3_x)
			{
				oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_y == p2_y)
			{
				oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}

			oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
			oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
			oled_Draw_Line(p2_x, p2_y, p4_x, p4_y, line_width_slim);
			oled_Draw_Line(p3_x, p3_y, p4_x, p4_y, line_width_slim);

			break;
		case line_width_bold:
			p1_x = p1_x+1; p1_y = p1_y+1;	p3_x = p3_x-1; p3_y = p3_y+1;
			p2_x = p2_x+1; p2_y = p2_y-1;	p4_x = p4_x-1; p4_y = p4_y-1;

			if (p1_x == p3_x && p1_y == p2_y)
			{
				oled_Draw_Pixel(p1_x, p1_y, pixel_control_ON);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_x == p3_x)
			{
				oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_y == p2_y)
			{
				oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}

			oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
			oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
			oled_Draw_Line(p2_x, p2_y, p4_x, p4_y, line_width_slim);
			oled_Draw_Line(p3_x, p3_y, p4_x, p4_y, line_width_slim);

			// -------------------------------------------------------
			p1_x = p1_x+1; p1_y = p1_y+1;	p3_x = p3_x-1; p3_y = p3_y+1;
			p2_x = p2_x+1; p2_y = p2_y-1;	p4_x = p4_x-1; p4_y = p4_y-1;

			if (p1_x == p3_x && p1_y == p2_y)
			{
				oled_Draw_Pixel(p1_x, p1_y, pixel_control_ON);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_x == p3_x)
			{
				oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}
			if (p1_y == p2_y)
			{
				oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
				rectNeedFillFlag = 0;
				break;
			}

			oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
			oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
			oled_Draw_Line(p2_x, p2_y, p4_x, p4_y, line_width_slim);
			oled_Draw_Line(p3_x, p3_y, p4_x, p4_y, line_width_slim);

			break;
	}


	// Fill inside the rectangle
	if ( (graphic_fill_effect == graphic_fill_solid) && (rectNeedFillFlag == 1) )
	{
		p1_x = p1_x+1; p1_y = p1_y+1;	p3_x = p3_x-1; p3_y = p3_y+1;
		p2_x = p2_x+1; p2_y = p2_y-1;	p4_x = p4_x-1; p4_y = p4_y-1;

		if (p1_x == p3_x && p1_y == p2_y)
		{
			oled_Draw_Pixel(p1_x, p1_y, pixel_control_ON);
			return 0;
		}
		if (p1_x == p3_x)
		{
			oled_Draw_Line(p1_x, p1_y, p2_x, p2_y, line_width_slim);
			return 0;
		}
		if (p1_y == p2_y)
		{
			oled_Draw_Line(p1_x, p1_y, p3_x, p3_y, line_width_slim);
			return 0;
		}

		// Fill remaining pixels
		uint8_t rows = p2_y - p1_y + 1;
		uint8_t cols = p3_x - p1_x + 1;
		if (rows <= cols)
		{
			for (uint8_t row = 0; row < rows; row++)
			{
				oled_Draw_Line(p1_x, p1_y + row, p3_x, p1_y + row, line_width_slim);
			}
		}
		// numbers of rows more than cols
		else
		{
			for (uint8_t col = 0; col < cols; col++)
			{
				oled_Draw_Line(p1_x + col, p1_y, p1_x + col, p2_y, line_width_slim);
			}
		}
	}

	return 0;
}




/**
 * @brief	Draw circular arc
 * 			|---------------→ x
 * 			| .		theta
 * 			|    .
 * 			|       .
 * 			|           .
 * 		 y 	↓
 * @param  	   cxo: [in]	center x value
 * @param  	   cyo: [in]	center y value
 * @param	radius: [in]	circular arc radius
 * @param	startAngle: [in]	circular arc start angle [Unit: degree]
 * @param	  endAngle: [in]	circular arc   end angle [Unit: degree]
 * @param	 line_width: [in]	line width (n pixel)
 * @retval	status	0:ok	1:error
 */
uint8_t	oled_Draw_Circular_Arc (
		uint8_t cxo, uint8_t cyo,
		uint8_t radius,
		float startAngle, float endAngle,
		line_width_t line_width)
{
	if (cxo < 0 || cxo > 127 || cyo < 0 || cyo > 63 || radius == 0)
		return 1;

	/* if start angle equal end angle, draw a circle */
	if (startAngle == endAngle)
	{
		oled_Draw_Ellipse(cxo, cyo, radius, radius, line_width, graphic_fill_hollow);
		return 0;
	}

	uint8_t	r = radius;
	float startDeg = startAngle, endDeg = endAngle;
	if (startAngle > endAngle)
	{
		float temp_s_angle = startAngle;
		startDeg = endAngle;
		endDeg = temp_s_angle;
	}
	float curRad = 0.0f;
	float curX = 0.0f, curY = 0.0f;

	// draw arc frame	slim
	for ( float curDeg = startDeg; curDeg <= endDeg; curDeg++ )
	{
		curRad = curDeg * acosf(0) * 1/90.0f;
		curX = r * 1.0f * cosf(curRad) + cxo;
		curY = r * 1.0f * sinf(curRad)	+ cyo;

		oled_Draw_Pixel((uint8_t)(curX+0.5), (uint8_t)(curY+0.5), pixel_control_ON);
	}

	// draw medium and bold line, circle arc
	switch (line_width) {
		case line_width_slim:
			break;
		case line_width_medium:
			r -= 1;
			if (r == 0)
				return 0;
			for ( float curDeg = startDeg; curDeg <= endDeg; curDeg++ )
			{
				curRad = curDeg * acosf(0) * 1/90.0f;
				curX = r * 1.0f * cosf(curRad) +  cxo;
				curY = r * 1.0f * sinf(curRad)	+ cyo;

				oled_Draw_Pixel((uint8_t)(curX+0.5), (uint8_t)(curY+0.5), pixel_control_ON);
			}
			break;
		case line_width_bold:
			r -= 1;
			if (r == 0)
				return 0;
			for ( float curDeg = startDeg; curDeg <= endDeg; curDeg++ )
			{
				curRad = curDeg * acosf(0) * 1/90.0f;
				curX = r * 1.0f * cosf(curRad) +  cxo;
				curY = r * 1.0f * sinf(curRad)	+ cyo;

				oled_Draw_Pixel((uint8_t)(curX+0.5), (uint8_t)(curY+0.5), pixel_control_ON);
			}

			r -= 1;
			if (r == 0)
				return 0;
			for ( float curDeg = startDeg; curDeg <= endDeg; curDeg++ )
			{
				curRad = curDeg * acosf(0) * 1/90.0f;
				curX = r * 1.0f * cosf(curRad) +  cxo;
				curY = r * 1.0f * sinf(curRad)	+ cyo;

				oled_Draw_Pixel((uint8_t)(curX+0.5), (uint8_t)(curY+0.5), pixel_control_ON);
			}

			break;
		default:
			break;
	}

	return 0;
}


/**
 * @brief	Draw round rectangle
 * 	1	*--#-------------#--*	3	Corners are rounded
 * 		#  #	fill	 *  #
 * 		|					|
 * 		#  #	fill	 #  #
 * 	2	*--#-------------#--*	4
 *
 * @param	x0:	[in] start point x value
 * @param	y0:	[in] start point y value
 * @param	x1:	[in]   end point x value
 * @param	y1: [in]   end point y value
 * @param	radius: [in] Corner arc radius
 * @param	line_width: [in] line width (n pixel)
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_Round_Rectangle(
		uint8_t x0, uint8_t y0,
		uint8_t x1, uint8_t y1,
		uint8_t radius,
		line_width_t line_width)
{
	if (x0 == x1 || y0 == y1)
		return 1;

	// Whether the rectangle needs to fill the flag
	//	1:need	0:don't need
	uint8_t rectNeedFillFlag = 1;

	if (x0 > x1)
	{
		uint8_t temp_x_0 = x0;
		x0 = x1;	x1 = temp_x_0;
	}
	if (y0 > y1)
	{
		uint8_t temp_y_0 = y0;
		y0 = y1;	y1 = temp_y_0;
	}

	// rectangle width and height
	uint8_t rect_W	=	x1 - x0+1;
	uint8_t rect_H	=	y1 - y0+1;
	uint8_t minVal = rect_W;
	uint8_t r = radius;
//	uint8_t r_rem = 0;
//	uint8_t store_x0 = x0, store_y0 = y0;
//	uint8_t store_x1 = x1, sotre_y1 = y1;

	if (rect_W > rect_H)	minVal = rect_H;

	// ensure radius value Reasonable
	if (2*r >= minVal)
		r = minVal / 2;


	// left top
	oled_Draw_Circular_Arc(x0+r, y0+r, r, 180, 270, line_width);
	// right top
	oled_Draw_Circular_Arc(x1-r, y0+r, r, -90,   0, line_width);
	// left bottom
	oled_Draw_Circular_Arc(x0+r, y1-r, r,  90, 180, line_width);
	// right bottom
	oled_Draw_Circular_Arc(x1-r, y1-r, r,   0,  90, line_width);

	oled_Draw_Line(x0+r, y0, x1-r, y0, line_width_slim);
	oled_Draw_Line(x0, y0+r, x0, y1-r, line_width_slim);
	oled_Draw_Line(x0+r, y1, x1-r, y1, line_width_slim);
	oled_Draw_Line(x1, y0+r, x1, y1-r, line_width_slim);
	x0 += 1;
	x1 -= 1;
	y0 += 1;
	y1 -= 1;
	if (line_width == line_width_medium)
	{
		oled_Draw_Line(x0+r, y0, x1-r, y0, line_width_slim);
		oled_Draw_Line(x0, y0+r, x0, y1-r, line_width_slim);
		oled_Draw_Line(x0+r, y1, x1-r, y1, line_width_slim);
		oled_Draw_Line(x1, y0+r, x1, y1-r, line_width_slim);
	}
	if (line_width == line_width_bold)
	{
		oled_Draw_Line(x0+r, y0, x1-r, y0, line_width_slim);
		oled_Draw_Line(x0, y0+r, x0, y1-r, line_width_slim);
		oled_Draw_Line(x0+r, y1, x1-r, y1, line_width_slim);
		oled_Draw_Line(x1, y0+r, x1, y1-r, line_width_slim);
		x0 += 1;
		x1 -= 1;
		y0 += 1;
		y1 -= 1;
		oled_Draw_Line(x0+r-1, y0, x1-r+1, y0, line_width_slim);
		oled_Draw_Line(x0, y0+r-1, x0, y1-r+1, line_width_slim);
		oled_Draw_Line(x0+r-1, y1, x1-r+1, y1, line_width_slim);
		oled_Draw_Line(x1, y0+r-1, x1, y1-r+1, line_width_slim);
	}


	// left-top point
//	uint8_t p1_x = x0, p1_y = y0;
//	// left-bottom point
//	uint8_t p2_x = x0, p2_y = y1;
//	// right-top point
//	uint8_t p3_x = x1, p3_y = y0;
//	// right-bottom point
//	uint8_t p4_x = x1, p4_y = y1;
}


/**
 * @brief	Draw a circle or ellipse
 * @param  cxo: [in]	center x value
 * @param  cyo: [in]	center y value
 * @param	 a: [in]	x-axis radius
 * @param	 b: [in]	y-axis radius
 * @param	line_width: [in] line width (n pixel)
 * @param	praphic_fill_effect: [in] graphic internal fill effect [solid or hollow]
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_Ellipse (
		uint8_t cxo, uint8_t cyo,
		uint8_t a_x, uint8_t b_y,
		line_width_t line_width,
		graphic_fill_effect_t graphic_fill_effect)
{
	if (cxo > 127 || cyo > 63)	return 1;

	if (a_x == 0 || a_x > 127 || b_y == 0 || b_y > 63)	return 1;

	// Whether the ellipse needs to fill the flag
	//	1:need	0:don't need
	uint8_t ellipseNeedFillFlag = 1;

	uint8_t tempxo = cxo, tempyo = cyo, tempa = a_x, tempb = b_y;
	float tempx = 0, tempy = 0;

	// Draw the outer circle of the ellipse, the thin line.
	for (float deg = 0; deg < 360; deg +=1) {
		float rad = deg * acosf(0) * (1/90.0f);
		tempx = tempa * cosf(rad) + tempxo;
		tempy = tempb * sinf(rad) + tempyo;

		oled_Draw_Pixel((uint8_t)(tempx + 0.5), (uint8_t)(tempy + 0.5), pixel_control_ON);
	}

	// Different thicknesses of oval lines.
	switch (line_width) {
		case line_width_slim:
			break;
		case line_width_medium:
			tempa = tempa - 1;	tempb = tempb - 1;
			if (tempa == 0 || tempb == 0)
			{
				ellipseNeedFillFlag = 0;
				break;
			}

			for (float deg = 0; deg < 360; deg +=1) {
				float rad = deg * acosf(0) * (1/90.0f);
				tempx = tempa * cosf(rad) + tempxo;
				tempy = tempb * sinf(rad) + tempyo;

				oled_Draw_Pixel((uint8_t)(tempx + 0.5), (uint8_t)(tempy + 0.5), pixel_control_ON);
			}

			break;
		case line_width_bold:
			tempa = tempa - 1;	tempb = tempb - 1;
			if (tempa == 0 || tempb == 0)
				break;

			for (float deg = 0; deg < 360; deg +=1) {
				float rad = deg * acosf(0) * (1/90.0f);
				tempx = tempa * cosf(rad) + tempxo;
				tempy = tempb * sinf(rad) + tempyo;

				oled_Draw_Pixel((uint8_t)(tempx + 0.5), (uint8_t)(tempy + 0.5), pixel_control_ON);
			}

			// -------------------------------------------------------------------
			tempa = tempa - 1;	tempb = tempb - 1;
			if (tempa == 0 || tempb == 0)
			{
				ellipseNeedFillFlag = 0;
				break;
			}

			for (float deg = 0; deg < 360; deg +=1) {
				float rad = deg * acosf(0) * (1/90.0f);
				tempx = tempa * cosf(rad) + tempxo;
				tempy = tempb * sinf(rad) + tempyo;

				oled_Draw_Pixel((uint8_t)(tempx + 0.5), (uint8_t)(tempy + 0.5), pixel_control_ON);
			}

			break;
	}

	// Fill inside the rectangle
	if ( (graphic_fill_effect == graphic_fill_solid) && (ellipseNeedFillFlag == 1) )
	{
		tempa = tempa - 1;	tempb = tempb - 1;

		do {
			if (tempa == 0 && tempb == 0)
			{
				oled_Draw_Pixel(tempxo, tempyo, pixel_control_ON);
				return 0;
			}

			if (tempa == 0)
			{
				oled_Draw_Line(tempxo, tempyo-tempb, tempxo, tempyo+tempb, line_width_slim);
				return 0;
			}

			if (tempb == 0)
			{
				oled_Draw_Line(tempxo-tempa, tempyo, tempxo+tempa, tempyo, line_width_slim);
				return 0;
			}

			for (float deg = 0; deg < 360; deg +=1) {
				float rad = deg * acosf(0) * (1/90.0f);
				tempx = tempa * cosf(rad) + tempxo;
				tempy = tempb * sinf(rad) + tempyo;

				oled_Draw_Pixel((uint8_t)(tempx + 0.5), (uint8_t)(tempy + 0.5), pixel_control_ON);
			}

			tempa = tempa - 1;	tempb = tempb - 1;

		} while (1);


	}


	return 0;
}







// ---------------- character ------------------

/**
 * @brief	Display the character at the given (x,y) point
 * @param	px: [in] x value (0 ~ 127) column	[< oled_H_pix]
 * @param	py: [in] y value (0 ~ 63)  row		[< oled_V_pix]
 * @param	ch:	[in] character to be displayed
 * @param	fontX: [in] using font
 * @retval	status	0:ok	1:error
 */
uint8_t oled_Draw_Character(uint8_t px, uint8_t py, unsigned char ch, oledFont_t fontX)
{
	if (px < 0 || px >= oled_H_Pix || py < 0 || py >= oled_V_Pix)
		return 1;

	if (px + fontX.font_Width > oled_H_Pix)
	{
//		return 1;
//		px = 0;
//		py = py + fontX.font_Height;
	}
	if (py + fontX.font_Height > oled_V_Pix)
		{
//			return 1;
//			py = 0;
//			px = px + fontX.font_Width;
		}

	// write character to GDDRAMBuffer
	uint8_t i, startPx = px, startPy = py;
	uint8_t chVal = ch - ' ';
	uint8_t * ptrFont = malloc(sizeof(uint8_t) * fontX.font_Width * fontX.font_Height / 8);
	memcpy(ptrFont, &(fontX.font_Array[chVal * (fontX.font_Width * fontX.font_Height / 8)]), (fontX.font_Width * fontX.font_Height / 8));
	for (i = 0; i < (fontX.font_Height / 8); i++)	// scan font char height pixel -> n Byte
	{
		px = startPx;

		for (uint8_t j = 0; j < fontX.font_Width; j++)	// scan font char height pixel
		{
			py = startPy + i * 8;

			for (uint8_t k = 0; k < 8; k++)	// Split each pixel data
			{
				if (((*(ptrFont + i * fontX.font_Width + j)) & (0x01 << k)) == (0x01 << k))
				{
					oled_Draw_Pixel(px, py++, pixel_control_ON);
				}
				else {
					oled_Draw_Pixel(px, py++, pixel_control_OFF);
				}
			}
			// next column
			px++;
		}
	}

	// less than one page section
	i = fontX.font_Height % 8;
	if (i != 0)
	{
		px = startPx;

		for (uint8_t j = 0; j < fontX.font_Width; j++)	// scan font char height pixel
		{
			py = startPy + (fontX.font_Height / 8) * 8;

			for (uint8_t k = 0; k < i; k++)	// Split each pixel data
			{
				if (((*(ptrFont + i * fontX.font_Width + j)) & (0x01 << k)) == (0x01 << k))
				{
					oled_Draw_Pixel(px, py++, pixel_control_ON);
				}
				else {
					oled_Draw_Pixel(px, py++, pixel_control_OFF);
				}
			}
			// next column
			px++;
		}
	}


	free(ptrFont);
	return 0;
}



/**
 * @brief	Draw String
 * @param	x:[in] start position x value
 * @param	y:[in] start position y value
 * @param	*pStr:[in] char array pointer
 * @param	strLen:[in] char array length
 * @param	oledFont:[in] indicate used font format
 * @retval	0:ok	1:error
 */
uint8_t oled_Draw_String(uint8_t x, uint8_t y, const unsigned char *pStr, uint8_t strLen, oledFont_t oledFont)
{
	uint8_t cur_x = x, cur_y = y;
	uint8_t font_width = oledFont.font_Width;
	uint8_t font_height= oledFont.font_Height;
	uint8_t str_len = strLen-1;

	if (strLen ==0 || pStr == NULL)
		return 1;

	if (x > oled_H_Pix-1 || y > oled_V_Pix-1)
		return 1;

	for (uint8_t var = 0; var < str_len; ++var) {
		if ( (oled_H_Pix - cur_x) < font_width ) {
			cur_x = 0;
			cur_y += font_height;
			if ((cur_y + font_height) > (oled_V_Pix))
				cur_y = 0;
		}

		if ( (oled_V_Pix - cur_y) < font_height ) {
			cur_y = 0;
			cur_x = 0;
		}

		oled_Draw_Character(cur_x, cur_y, *(pStr + var), oledFont);
		cur_x += font_width;
	}

	return 0;
}



/**
 * @brief	Draw Chinese String		height is nx8
 * @param	x:[in] start position x value
 * @param	y:[in] start position y value
 * @param	chineseChar:[in] indicate used chinese string array info
 * @param	idx:[in] arrary number(index) [<- frome oled_font.c chinese_t]
 * @param	chr_num: [in] number of Chinese characters
 * @retval	0:ok	1:error
 */
uint8_t oled_Draw_Chinese_String(uint8_t x, uint8_t y, chinese_t chineseStr, uint8_t idx, uint8_t chr_num )
{
	if (x > oled_H_Pix-1 || y > oled_V_Pix-1 || chineseStr.array == NULL || chr_num == 0)
		return 1;

	uint8_t cur_x = x, 	cur_y = y;
	uint8_t chr_width  = chineseStr.Width;
	uint8_t chr_height = chineseStr.Height;

	uint16_t tempLen = chr_num * chineseStr.Width * chineseStr.Height / 8;
	uint8_t ptrStr[tempLen];
	uint32_t addr_offset = (chineseStr.Width==16)? (chinese_16x16_strint_MaxLen) : (chinese_24x24_strint_MaxLen);
	memcpy(ptrStr, (uint8_t *)(chineseStr.array +idx*addr_offset), chr_num * chineseStr.Width * chineseStr.Height / 8);


	uint8_t str_len	   = chr_num;

	uint8_t i, startPx = cur_x, startPy = cur_y;

	uint8_t var = 0;
	for (var = 0; var < str_len; ++var) {

		// draw one char *******************************************************************

//		for (uint8_t idx = 0; idx < str_len; idx++)
//		{
			for (i = 0; i < (chr_height / 8); i++)	// scan font char height pixel -> n Byte  rows(page)
			{
				cur_x = startPx + var*chr_width;

				// process a single character
				for (uint8_t j = 0; j < chr_width; j++)	// scan font char height pixel
				{
					cur_y = startPy + i * 8;

					for (uint8_t k = 0; k < 8; k++)	// Split each pixel data
					{
						if (((ptrStr[var * chr_height * chr_width / 8 + i * chr_width + j]) & (0x01 << k)) == (0x01 << k))
						{
							oled_Draw_Pixel(cur_x, cur_y++, pixel_control_ON);
						}
						else {
							oled_Draw_Pixel(cur_x, cur_y++, pixel_control_OFF);
						}
					}
					// next column
					cur_x++;
				}
			}
//		}


		// draw one char end ***************************************************************

//		cur_x += chr_width;
	}

	return 0;
}


/**
 * @brief	Draw Bit_Map	128x64 size
 * @param	bitMap:[in] indicate used bitMap array info
 * @param	idx:[in] arrary number(index) [<- frome oled_font.c BitMap_t]
 * @retval	0:ok	1:error
 */
uint8_t oled_Draw_BitMap(bitMap_t bitMap, uint8_t idx)
{
//	if (x > (oled_H_Pix-1) || y > (oled_V_Pix))
//		return 1;
//	if (oled_H_Pix-x < bitMap.Width || oled_V_Pix < bitMap.Height)
//		return 1;

	uint8_t cur_x = 0, cur_y = 0;
	uint8_t tempy = 0;

	uint8_t ptrStr[bitmap_128x64_MaxLen];
	uint32_t addr_offset = bitmap_128x64_MaxLen;
	memcpy(ptrStr, (uint8_t *)(bitMap.array +idx*addr_offset), addr_offset);


	for (uint8_t i = 0; i < bitMap.Height/8; i++)	// page rows
	{
		cur_x = 0;
		for (uint8_t j = 0; j < bitMap.Width; j++)	// column
		{

			for (uint8_t k = 0; k < 8; k++)	// Split each pixel data
			{
				if (((ptrStr[i * bitMap.Width + j]) & (0x01 << k)) == (0x01 << k))
				{
					oled_Draw_Pixel(cur_x, cur_y++, pixel_control_ON);
				}
				else {
					oled_Draw_Pixel(cur_x, cur_y++, pixel_control_OFF);
				}
			}
			cur_y = tempy;
			cur_x++;
		}
		tempy += 8;
	}
}




//  ############################################################################
/**
 *	Men write transfer complete callback
 */
#if	oled_i2c_dma
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	printf("mem tx cplt call back\r\n");
	if (hi2c->Instance == oled_i2c.Instance)
	{
		oled_i2c_dma_mem_write_flag = 1;
	}
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if (hi2c->Instance == oled_i2c.Instance)
		printf("i2c DMA error...\r\n");
}

#endif
