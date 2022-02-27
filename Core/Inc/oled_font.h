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

extern oledFont_t	oled_Font_ASCII_08_16_digital7V4 ;	// 8 x 16 ascii characters
extern oledFont_t	oled_Font_ASCII_12_24_digital7V4 ;	// 12 x 24 ascii characters
extern oledFont_t	oled_Font_ASCII_16_32_digital7V4 ;	// 16 x 32 ascii characters



#define	chinese_16x16_strint_MaxLen		(10*32)	// most of number 10;
#define chinese_24x24_strint_MaxLen		(10*72)	// most of number 10;
#define	bitmap_128x64_MaxLen			(128*8)	// most of image is 128x64 pixel

typedef struct {
	unsigned char Width;	// width
	unsigned char Height;	// height
	unsigned char *array;		// array pointer
}arrayOriginInfo_TypeDef;
typedef arrayOriginInfo_TypeDef	chinese_t;

extern chinese_t	oled_chinese_str_16_16_songTi;
extern chinese_t	oled_chinese_str_24_24_songTi;

typedef arrayOriginInfo_TypeDef	bitMap_t;
extern bitMap_t	oled_bitmap_128_64;


#endif /* INC_OLED_FONT_H_ */
