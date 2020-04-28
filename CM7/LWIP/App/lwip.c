/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"

/* USER CODE BEGIN 0 */

#include <stdio.h>
#include "main.h"
#include "usart.h"
#include "lwip/netifapi.h"

/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
static void ethernet_link_status_updated(struct netif *netif);
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/* USER CODE BEGIN 2 */

void ethernet_status_callback(struct netif *netif)
{
	uint8_t msg[48];
	size_t msg_len;

	if (netif_is_up(netif))
	{
		uint8_t msg_ip[20];
		ipaddr_ntoa_r(&netif->ip_addr, (char* )msg_ip, 20);
		msg_len = sprintf((char*) msg, "STATUS connected @ %s\r\n", (char*) msg_ip);
	}
	else
	{
		 msg_len = sprintf((char *)msg, "STATUS down @ %lu\r\n", HAL_GetTick()/1000);
	}

	HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);
}

uint8_t ethernet_ip_check(struct netif *netif)
{
	  osDelay(2000);
	  uint8_t msg[127];

	  if(netif->ip_addr.addr != 0)
	  {
		  char msg_ip[18], msg_mask[18], msg_gate[18];
		  ipaddr_ntoa_r(&netif->ip_addr, msg_ip, 20);
		  ipaddr_ntoa_r(&netif->netmask, msg_mask, 20);
		  ipaddr_ntoa_r(&netif->gw, msg_gate, 20);
		  size_t msg_len = sprintf((char *)msg, "LINK after 2s @ %s; %s; %s\r\n",  msg_ip, msg_mask, msg_gate);
		  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);
		  return 1;
	  }
	  else
	  {
		  size_t msg_len = sprintf((char *)msg, "No IP after 2 seconds\r\n");
		  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);
		  return 0;
	  }
}

/* USER CODE END 2 */

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
  /* Initilialize the LwIP stack with RTOS */
  tcpip_init( NULL, NULL );

  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  /* Create the Ethernet link handler thread */
/* USER CODE BEGIN H7_OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */
  osThreadDef(EthLink, ethernet_link_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE *2);
  osThreadCreate (osThread(EthLink), &gnetif);
/* USER CODE END H7_OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */

  /* Start DHCP negotiation for a network interface (IPv4) */
  dhcp_start(&gnetif);

/* USER CODE BEGIN 3 */

  // add status callback for future status updates
  netif_set_status_callback(&gnetif, ethernet_status_callback);

  // did the network change?
  ethernet_link_status_updated(&gnetif);

  // add name; just because
//  char host_name[] = "STM32_Infinity";
//  netif_set_hostname(&gnetif, host_name);


/* USER CODE END 3 */
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
#endif

/**
  * @brief  Notify the User about the network interface config status 
  * @param  netif: the network interface
  * @retval None
  */
static void ethernet_link_status_updated(struct netif *netif) 
{
  if (netif_is_up(netif))
  {
/* USER CODE BEGIN 5 */
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

//	  ethernet_ip_check(netif);

	  uint8_t msg[48], msg_ip[18];
	  ipaddr_ntoa_r(&netif->ip_addr, (char *)msg_ip, 20);
	  size_t msg_len = sprintf((char *)msg, "LINK connected @ %s\r\n", (char *)msg_ip);
	  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);

	  if(ethernet_ip_check(netif) == 0)
	  {
		  // no IP assigned
		  // => DHCP failled?
		  // 	=> Let's try again
		  msg_len = sprintf((char *)msg, "Initial DHCP failed, trying again\r\n");
		  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);

		  // TODO: change to netifapi_dhcp_release_and_stop in v2.1+
		  netifapi_dhcp_release(netif);
		  netifapi_dhcp_stop(netif);
		  osDelay(2000);
		  netifapi_dhcp_start(netif);
		  osDelay(2000);

		  // did we get an IP?
		  if(ethernet_ip_check(netif) == 0)
		  {
			  // still no IP?
			  // => use static fall back
			  msg_len = sprintf((char *)msg, "Second DHCP failed, using static fallback\r\n");
			  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);

//			  dhcp_release(netif);
			  // TODO: change to netifapi_dhcp_release_and_stop in v2.1+
			  netifapi_dhcp_release(netif);
			  netifapi_dhcp_stop(netif);
			  osDelay(1000);
			  netifapi_netif_set_down(netif);
			  osDelay(3000);


			  // set static IP address
			  IP4_ADDR(&ipaddr, 192, 168, 1, 3);
			  IP4_ADDR(&netmask, 255, 255, 255, 0);
			  IP4_ADDR(&gw, 192, 168, 1, 1);

			  // set fallback value
			  netifapi_netif_set_addr(&gnetif, &ipaddr, &netmask, &gw);
//			  netifapi_netif_set_up(netif);

			  // did it work?
			  ethernet_ip_check(netif);
		  }
	  }


/* USER CODE END 5 */
  }
  else /* netif is down */
  {  
/* USER CODE BEGIN 6 */
	  uint8_t msg[32];
	  size_t msg_len = sprintf((char *)msg, "LINK down @ %lu\r\n", HAL_GetTick()/1000);
	  HAL_UART_Transmit(&huart3, msg, msg_len, HAL_MAX_DELAY);
	  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);

	  // release IP
//	  netifapi_dhcp_release(netif);
//	  netifapi_dhcp_stop(netif);

/* USER CODE END 6 */
  } 
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */
	
  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */	
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */	
  return recved_bytes;
}
#endif /* MDK ARM Compiler */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
