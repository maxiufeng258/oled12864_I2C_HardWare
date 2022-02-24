/*
 * oled_font.h
 *
 *  Created on: Feb 24, 2022
 *      Author: maxiufeng
 */

#ifndef INC_OLED_FONT_H_
#define INC_OLED_FONT_H_

/* oled font info structure */
typedef struct {
	const unsigned char	font_Width;		// font width  pixel
	const unsigned char	font_Height;	// font height pixel
	const unsigned char	*font_Array;	// font array pointer
}oledFont_TypeDef;
typedef oledFont_TypeDef	oledFont_t;

/* Define font structure variables */
extern oledFont_t	oled_Font_ASCII_08_16_courierNew ;	// 8 x 16 ascii characters courierNew format
extern oledFont_t	oled_Font_ASCII_12_24_courierNew ;	//12 x 24 ascii characters
extern oledFont_t	oled_Font_ASCII_16_32_courierNew ;	//16 x 32 ascii characters

#endif /* INC_OLED_FONT_H_ */
