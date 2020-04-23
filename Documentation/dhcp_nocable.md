## DHCP Not assigning IP when starting without ethernet cable

WHen there's an ethernet cable connected to the board on boot, and there is a DHCP server on the other end, an IP is automatically assigned to the MCU. 

The problem occurs when the MCU boots without an ethernet cable. When an ethernet cable is connected after the system is fully running, the DHCP assigns an IP of 0.0.0.0. To fix this, restart the DHCP client process in the `link_callback` function.

### 1. Enable link_callback
```c
#define LWIP_NETIF_LINK_CALLBACK 1
```

### 2. Modify the link callback function

```c
static void ethernet_link_status_updated(struct netif *netif) 
{
	if (netif_is_up(netif))
	{
	/* USER CODE BEGIN 5 */

		// Do we have an IP?
		if(netif->ip_addr.addr != 0)
		{
			return;
		}

		// if not, let's check again after 2 seconds
		osDelay(2000);

		// Do we have an IP?
		if(netif->ip_addr.addr != 0)
		{
			return;
		}
		
		// no IP? Restart DHCP client
		dhcp_release(netif);
		osDelay(1000);
		dhcp_stop(netif);
		osDelay(1000);
		dhcp_start(netif);

	/* USER CODE END 5 */
	}
	else /* netif is down */
	{  
	/* USER CODE BEGIN 6 */

		// ethernet cable disconnected

	/* USER CODE END 6 */
	} 
}
```