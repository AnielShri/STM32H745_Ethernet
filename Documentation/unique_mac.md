## Generate (pseudo)unique mac address

STM32 MCUs have a 96-bit unique id (literally called UID). This can be used to generate a unique ethernet MAC. Keep in mind that, since only 3-Bytes (or 25%) of the UID is used. This increases the likelyhood of multiple MCUs having the same MAC address. For most practical applications this should not be an issue.

### Instructions:
Edid `low_level_init` in [`ethernetif.c`](../CM7/LWIP/Target/ethernetif.c) and add the following code before `HAL_ETH_Init`.

```c
/* USER CODE BEGIN MACADDRESS */

uint32_t id0 = HAL_GetUIDw0();

uint8_t mac[6];

// first 3 bytes are ST specific max prefixes
mac[0] = 0x00;
mac[1] = 0x80;
mac[2] = 0xE1;

// last 3 bytes are used to set unique mac based on UID
mac[3] = (id0 >> 16) & 0x000000FF;
mac[4] = (id0 >> 8) & 0x000000FF;
mac[5] = (id0 >> 0) & 0x000000FF;

heth.Init.MACAddr = &mac[0];

/* USER CODE END MACADDRESS */
```
