/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTlwip.h"

char expired(Timer* timer)
{
	return ((int) timer->end_time - (int) chTimeNow()) < 0;
}


void countdown_ms(Timer* timer, unsigned int timeout)
{
	timer->end_time = chTimeNow() + MS2ST(timeout);
}


void countdown(Timer* timer, unsigned int timeout)
{
	timer->end_time = chTimeNow() + S2ST(timeout);
}

/* backport ST2MS from ChibiOS 3.0 */
#ifndef ST2MS
#define ST2MS(n) ((((n) - 1UL) / (CH_FREQUENCY / 1000UL)) + 1UL)
#endif

int left_ms(Timer* timer)
{
	return ST2MS(timer->end_time - chTimeNow());
}


void InitTimer(Timer* timer)
{
	timer->end_time = 0;
}


int mqtt_lwip_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (size_t)(len - bytes), 0);
		if (rc == -1)
		{
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
			bytes += rc;
	}
	return bytes;
}


int mqtt_lwip_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = write(n->my_socket, buffer, len);
	return rc;
}


void mqtt_lwip_disconnect(Network* n)
{
	close(n->my_socket);
}


void NewNetwork(Network* n)
{
	n->my_socket = 0;
	n->mqttread = mqtt_lwip_read;
	n->mqttwrite = mqtt_lwip_write;
	n->disconnect = mqtt_lwip_disconnect;
}

int ConnectNetwork(Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	u8_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
		{
			rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
		}
	}

	return rc;
}
