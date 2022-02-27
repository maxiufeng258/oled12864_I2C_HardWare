/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "oled12864_I2C.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c2_tx;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 100);
	return len;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  uint8_t res = oled_i2c_Init();

  if (res != 0)
	  HAL_GPIO_TogglePin(LD1_Red_GPIO_Port, LD1_Red_Pin);
  printf("%d\r\n", res);

//  oled_Fill_Screen_Color(oled_color_White);

//  HAL_Delay(100);
//  oled_Draw_Round_Rectangle(0, 0, 127, 63, 10, line_width_bold);
//  oled_Update_Screen();


//  HAL_Delay(100);
//  oled_Draw_Character(12, 12, '7', oled_Font_ASCII_16_32_digital7V4);
//  oled_Update_Screen();

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



//  HAL_Delay(100);
//  oled_Draw_Circular_Arc(64, 32, 30, 90, 0, line_width_medium);
//  oled_Update_Screen();
//
//  HAL_Delay(100);
//  oled_Draw_Circular_Arc(64, 32, 30, -90, 0, line_width_slim);
//  oled_Update_Screen();
//
//  HAL_Delay(100);
//  oled_Draw_Circular_Arc(64, 32, 30, 180, 270, line_width_bold);
//  oled_Update_Screen();
//
//  HAL_Delay(100);
//  oled_Draw_rectangle(74, 22, 54, 42, line_width_medium, graphic_fill_hollow);
//  oled_Update_Screen();

  //
//  HAL_Delay(200);
//  oled_Draw_Ellipse(50, 32, 14, 14, line_width_medium, graphic_fill_solid);
//  oled_Update_Screen();
//
//
//  HAL_Delay(100);
//  oled_Draw_rectangle(16, 8, 28, 32, line_width_bold, graphic_fill_hollow);
//  oled_Update_Screen();
//
//
//  HAL_Delay(200);
////  oled_Set_Display_Normal_Inverse(display_invers);
//
//
//  HAL_Delay(200);
//  printf("############################################\r\n");

//
//  HAL_Delay(200);
//  oled_Draw_Line(60, 60, 120, 0, line_width_bold);
//  oled_Update_Screen();
//
//  HAL_Delay(200);
//  oled_Draw_Line(60, 60, 120, 16, line_width_bold);
//  oled_Update_Screen();
//
//  HAL_Delay(200);
//  oled_Draw_Line(60, 60, 120, 32, line_width_bold);
//  oled_Update_Screen();



//  for (uint8_t i = 0; i < 16; i++)
//    {
//  	  unsigned char ch = '0' + i;
//  	  HAL_Delay(200);
//  	  oled_Draw_Character(i*oled_Font_ASCII_08_16_courierNew.font_Width, 0, ch, oled_Font_ASCII_08_16_courierNew);
//  	  oled_Update_Screen();
//    }
//
//  for (uint8_t i = 0; i < 16; i++)
//    {
//  	  unsigned char ch = '0' + i;
//  	  HAL_Delay(200);
//  	  oled_Draw_Character(i*oled_Font_ASCII_08_16_digital7V4.font_Width, 16, ch, oled_Font_ASCII_08_16_digital7V4);
//  	  oled_Update_Screen();
//    }
//
//  for (uint8_t i = 0; i < 9; i++)
//  {
//	  unsigned char ch = '0' + i;
//	  HAL_Delay(200);
//	  oled_Draw_Character(i*oled_Font_ASCII_16_32_courierNew.font_Width, 32, ch, oled_Font_ASCII_16_32_courierNew);
//	  oled_Update_Screen();
//  }


  HAL_Delay(2000);
  oled_Draw_BitMap(oled_bitmap_128_64, 0);
  oled_Update_Screen();


  printf("init succeed...\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_GPIO_TogglePin(LD2_Green_GPIO_Port, LD2_Green_Pin);
	  HAL_Delay(100);
	  HAL_GPIO_TogglePin(LD2_Green_GPIO_Port, LD2_Green_Pin);
	  HAL_Delay(3000);


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, LD1_Red_Pin|LD2_Green_Pin|LD3_Blue_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : LD1_Red_Pin LD2_Green_Pin LD3_Blue_Pin */
  GPIO_InitStruct.Pin = LD1_Red_Pin|LD2_Green_Pin|LD3_Blue_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

