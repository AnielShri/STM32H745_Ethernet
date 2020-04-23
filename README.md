# STM32H745_Ethernet

Trying to get Ethernet, LWIP and FreeRTOS working on the STM32H745. Testing on the NUCLEO-H745ZI-Q using FW_1.7 and the STM32CubeIDE.

## Instructions on how to get started:
* [LWIP without RTOS](Documentation/lwip_nortos.md)
* [LWIP with RTOS](Dcumentation/lwip_rtos.md)

## Bugs and improvements
* [SysTick not increasing ticks](Documentation/no_systick.md)
* [No DHCP IP when starting without ethernet cable](Documentation/dhcp_nocable.md)

## Current status: 
* RTOS works (blinky)
* LWIP works (nucleo board gets IP from DHCP on router)
* HTTP test server works (navigate to http://\<nucleo_ip\>/index.html for demo)

---





---

---


