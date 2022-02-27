## **1. OLED I2C Drive** STM32 HAL Lib

0.96 - inch **based on SSD1306**
	128*64 Pixel -> 128 cols * 64 rows(8 pages)

address 0x78/A -> 0b   ***0111 10 0/1***       **0(R/W bit)** [0 write operation, 1 read operation]
				                     (>>1bit)7bits are OLED Address

**dataSheet:**

https://www.digikey.com/htmldatasheets/production/2047793/0/0/1/ssd1306.html#pf28



## **2. Provided interface functions**

```c
void I2C_Device_Scan(void);

uint8_t oled_i2c_Init(void);
uint8_t oled_Update_Screen(void);
uint8_t oled_Fill_Screen_Color(oled_color_t	oled_color);

uint8_t oled_Draw_Pixel(uint8_t px, uint8_t py, pixel_control_t pixel_control);
uint8_t oled_Draw_Line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, line_width_t line_width);
uint8_t oled_Draw_rectangle (
		uint8_t x0, uint8_t y0,
		uint8_t x1, uint8_t y1,
		line_width_t line_width,
		graphic_fill_effect_t praphic_fill_effect);
uint8_t	oled_Draw_Circular_Arc (
		uint8_t cxo, uint8_t cyo,
		uint8_t radius,
		float startAngle, float endAngle,
		line_width_t line_width);
uint8_t oled_Draw_Ellipse (
		uint8_t cxo, uint8_t cyo,
		uint8_t a_x, uint8_t b_y,
		line_width_t line_width,
		graphic_fill_effect_t graphic_fill_effect);
uint8_t oled_Draw_Round_Rectangle(
		uint8_t x0, uint8_t y0,
		uint8_t x1, uint8_t y1,
		uint8_t radius,
		line_width_t line_width);



uint8_t oled_Draw_Character(uint8_t px, uint8_t py, unsigned char ch, oledFont_t fontX);
uint8_t oled_Draw_String(uint8_t x, uint8_t y, const unsigned char *pStr, uint8_t strLen, oledFont_t oledFont);
uint8_t oled_Draw_Chinese_String(uint8_t x, uint8_t y, chinese_t chineseStr, uint8_t idx, uint8_t chr_num );

uint8_t oled_Draw_BitMap(bitMap_t bitMap, uint8_t idx);
```



## **3. Three fonts of ASCII characters are provided in font.c: 08x16, 12x24, 16x32.**

**Two font formats**

```c
oledFont_t	oled_Font_ASCII_08_16_courierNew = { 8, 16, ascii_08x16_courierNew};	// 8 x 16 ascii characters
oledFont_t	oled_Font_ASCII_12_24_courierNew = {12, 24, ascii_12x24_courierNew};	//12 x 24 ascii characters
oledFont_t	oled_Font_ASCII_16_32_courierNew = {16, 32, ascii_16x32_courierNew};	//16 x 32 ascii characters

oledFont_t	oled_Font_ASCII_08_16_digital7V4 = {  8, 16, ascii_08x16_Digital7V4};	// 8 x 16 ascii characters
oledFont_t	oled_Font_ASCII_12_24_digital7V4 = { 12, 24, ascii_12x24_Digital7V4};	// 12 x 24 ascii characters
oledFont_t	oled_Font_ASCII_16_32_digital7V4 = { 16, 32, ascii_16x32_Digital7V4};	// 16 x 32 ascii characters
```



**The display of Chinese characters and bitmaps is different from the display of ASCII characters. A separate modulo operation is required for what is to be displayed, which is more complicated than displaying ASCII characters.**

```c
/* chinese related define */
chinese_t	oled_chinese_str_16_16_songTi	 = {16, 16, chinese_16x16_string};
chinese_t	oled_chinese_str_24_24_songTi	 = {24, 24, chinese_24x24_string};


/* bit map related */
bitMap_t	oled_bitmap_128_64	=	{128, 64, bitmap_128x64};
```



## **4. There are also drawing functions, **

- ​	**straight line (thin-medium-thick), **

- ​	**ellipse (thin-medium-thick, filled), **

- ​	**rectangle (thin-medium-thick, filled), **

- ​	**rounded rectangle (thin-medium-thick, filled), **

- ​	**arc Line (thin-medium-thick).**

## **5. main.c  demo code**

```c
  HAL_Delay(500);
  oled_Draw_Chinese_String(4, 0, oled_chinese_str_16_16_songTi, 2 , 10);
  oled_Update_Screen();

//  HAL_Delay(100);
//  oled_Draw_Chinese_String(0, 0, oled_chinese_str_24_24_songTi, 0 , 10);
//  oled_Update_Screen();

  HAL_Delay(1000);
  oled_Draw_Chinese_String(0, 32, oled_chinese_str_24_24_songTi, 1 , 4);
  oled_Update_Screen();

//  HAL_Delay(100);
  HAL_Delay(1000);
  oled_Fill_Screen_Color(oled_color_Black);
  unsigned char str1[] = "0.96' oled";
  oled_Draw_String(8, 16, str1, sizeof(str1), oled_Font_ASCII_12_24_courierNew);
  oled_Update_Screen();

//  HAL_Delay(100);
  HAL_Delay(1000);
  oled_Fill_Screen_Color(oled_color_Black);
  oled_Draw_Round_Rectangle(3, 2, 120, 60, 12, line_width_slim);
  oled_Update_Screen();

  HAL_Delay(1000);
  oled_Draw_rectangle(12, 12, 22, 22, line_width_medium, graphic_fill_hollow);
  oled_Draw_rectangle(12, 25, 22, 54, line_width_slim, graphic_fill_solid);
  oled_Update_Screen();

  HAL_Delay(1000);
  oled_Draw_Ellipse(43, 32, 16, 12, line_width_bold, graphic_fill_hollow);
  oled_Update_Screen();

  HAL_Delay(1000);
  oled_Draw_Line(125, 10, 125, 56, line_width_medium);
  oled_Update_Screen();

  HAL_Delay(1000);
  oled_Draw_Circular_Arc(60, 32, 20, -90, 45, line_width_bold);
  oled_Update_Screen();


  HAL_Delay(1000);
  oled_Draw_Round_Rectangle(85, 10, 110, 54, 6, line_width_bold);
  oled_Update_Screen();



  HAL_Delay(2000);
  oled_Draw_BitMap(oled_bitmap_128_64, 0);
  oled_Update_Screen();
```

