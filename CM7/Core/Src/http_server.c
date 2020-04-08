/*
 * http_server.c
 *
 *  Created on: Apr 6, 2020
 *      Author: ashri
 */

#include <string.h>

#include "http_server.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "cmsis_os.h"
#define WEBSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 4 )

static const char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
static const char http_index_html[] = "<html><head><title>Congrats!</title></head><body><h1>Welcome to our lwIP HTTP server!</h1><p>This is a small test page, served by httpserver-netconn.</body></html>";

static const char http_404_hdr[] = "HTTP/1.1 404 ERROR\r\nContent-type: text/html\r\n\r\n";
static const char http_404_html[] = "<html><head><title>Error</title></head><body><h1>LWIP Error</h1><p>These are not the droids you're looking for</body></html>";


/** Serve one HTTP connection accepted in the http thread */
static void http_server_netconn_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;

	/* Read the data from the port, blocking if nothing yet there.
	 We assume the request (the part we care about) is in one netbuf */
	err = netconn_recv(conn, &inbuf);

	if (err == ERR_OK)
	{
		netbuf_data(inbuf, (void**) &buf, &buflen);

		/* Is this an HTTP GET command? (only check the first 5 chars, since
		 there are other formats for GET, and we're keeping it very simple )*/
		if(strncmp((char const *)buf,"GET /index.html ", 16) == 0)
		{
			/* Send the HTML header
			 * subtract 1 from the size, since we dont send the \0 in the string
			 * NETCONN_NOCOPY: our data is const static, so no need to copy it
			 */
			netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);

			/* Send our HTML page */
			netconn_write(conn, http_index_html, sizeof(http_index_html) - 1, NETCONN_NOCOPY);
		}
		else
		{
			netconn_write(conn, http_404_hdr, sizeof(http_404_hdr) - 1, NETCONN_NOCOPY);
			netconn_write(conn, http_404_html, sizeof(http_404_html) - 1, NETCONN_NOCOPY);
		}
	}
	/* Close the connection (server closes in HTTP) */
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
	 so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}


/** The main function, never returns! */
static void http_server_netconn_thread(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err;
	LWIP_UNUSED_ARG(arg);

	/* Create a new TCP connection handle */
	/* Bind to port 80 (HTTP) with default IP address */
	conn = netconn_new(NETCONN_TCP);
	netconn_bind(conn, IP_ADDR_ANY, 80);
	LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);

	/* Put the connection into LISTEN state */
	netconn_listen(conn);

	do
	{
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK)
		{
			http_server_netconn_serve(newconn);
			netconn_delete(newconn);
		}
	} while (err == ERR_OK);

	LWIP_DEBUGF(HTTPD_DEBUG, ("http_server_netconn_thread: netconn_accept received error %d, shutting down", err));
	netconn_close(conn);
	netconn_delete(conn);
}


/** Initialize the HTTP server (start its thread) */
void http_server_netconn_init(void)
{
  sys_thread_new("http_server_netconn", http_server_netconn_thread, NULL, DEFAULT_THREAD_STACKSIZE, WEBSERVER_THREAD_PRIO);
}
