/*
 * custom_funcs.c
 *
 *  Created on: Apr 28, 2020
 *      Author: ashri
 */


#include "main.h"
#include "usart.h"


// redirects printf output to UART3

int _write(int file, char *ptr, int len)
{
	//HAL_StatusTypeDef status = HAL_UART_Transmit(&huart3, (uint8_t *)ptr, len, HAL_MAX_DELAY);

//	return (status == HAL_OK ? len : 0);
	return 0;
}
