/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
/* USER CODE END 0 */

extern UART_HandleTypeDef huart2;


/* USER CODE BEGIN 1 */
#ifdef __CC_ARM
  #pragma import(__use_no_semihosting)
#endif

struct __FILE   
{   
int handle;   
/* Whatever you require here. If the only file you are using is */   
/* standard output using printf() for debugging, no file handling */   
/* is required. */   
};   
/* FILE is typedef? d in stdio.h. */   
FILE __stdout;  

int _sys_exit(int x)   
{   
	x = x;   
	return x;
}  

int fputc(int ch, FILE *f)  
{  
		HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;  
}  

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
