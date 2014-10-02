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
	int rc;
	struct netbuf *inbuf;

	if (timeout_ms <= 0) {
		timeout_ms = 100;
	}

	netconn_set_recvtimeout(n->netconn, timeout_ms);
	if (rc != ERR_OK) {
		return -1;
	}

	int bytes = 0;
	while (bytes < len) {
		rc = netconn_recv(n->netconn, &inbuf);
		if (rc != ERR_OK) {
			if (errno != ENOTCONN && errno != ECONNRESET) {
				bytes = -1;
				break;
			}
		} else {
			bytes += netbuf_len(inbuf);
		}
	}

	netbuf_copy(inbuf, buffer, bytes);
	return bytes;
}


int mqtt_lwip_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	if (timeout_ms <= 0) {
		timeout_ms = 100;
	}

	/* netconn_set_recvtimeout(n->netconn, timeout_ms); */
	int rc = netconn_write(n->netconn, buffer, len, NETCONN_COPY);
	return rc;
}


void mqtt_lwip_disconnect(Network* n)
{
	netconn_close(n->netconn);
	netconn_delete(n->netconn);
	n->netconn = NULL;
	netbuf_delete(n->netbuf);
	n->netbuf = NULL;
}


void NewNetwork(Network* n)
{
	n->netconn = NULL;
	n->mqttread = mqtt_lwip_read;
	n->mqttwrite = mqtt_lwip_write;
	n->disconnect = mqtt_lwip_disconnect;
}

int ConnectNetwork(Network* n, char* addr, int port)
{
	struct ip_addr ip_addr;
	int rc = -1;

	if (rc = netconn_gethostbyname(addr, &ip_addr) != ERR_OK) {
		return rc;
	}

	n->netconn = netconn_new(NETCONN_TCP);
	if (n->netconn == NULL) {
		return ERR_MEM;
	}

	rc = netconn_connect(n->netconn, &ip_addr, port);

	return rc;
}
