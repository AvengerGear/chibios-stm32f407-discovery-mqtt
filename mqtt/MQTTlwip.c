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
	int offset = 0;

	if (timeout_ms <= 0) {
		timeout_ms = 100;
	}
	netconn_set_recvtimeout(n->netconn, timeout_ms);

	int bytes = 0;

	while (bytes < len) {
		if (n->netbuf != NULL) {
			inbuf = n->netbuf;
			offset = n->netbuf_offset;
			rc = ERR_OK;
		} else {
			rc = netconn_recv(n->netconn, &inbuf);
			offset = 0;
		}

		if (rc != ERR_OK) {
			if (rc != ERR_TIMEOUT) {
				bytes = -1;
			}
			break;
		} else {
			int nblen = netbuf_len(inbuf) - offset;
			if ((bytes + nblen) > len) {
				netbuf_copy_partial(inbuf, buffer + bytes,
						len - bytes, offset);
				n->netbuf = inbuf;
				n->netbuf_offset = offset + len - bytes;
				bytes = len;
			} else {
				netbuf_copy_partial(inbuf, buffer + bytes,
						nblen, offset);
				bytes += nblen;
				netbuf_delete(inbuf);
				n->netbuf = NULL;
				n->netbuf_offset = 0;
			}
		}
	}

	return bytes;
}


int mqtt_lwip_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	if (timeout_ms <= 0) {
		timeout_ms = 100;
	}
	netconn_set_sendtimeout(n->netconn, timeout_ms);

	int rc = netconn_write(n->netconn, buffer, len, NETCONN_NOCOPY);
	if (rc != ERR_OK) {
		return -1;
	}
	return len;
}


void mqtt_lwip_disconnect(Network* n)
{
	netconn_close(n->netconn);
	netconn_delete(n->netconn);
	n->netconn = NULL;
}


void NewNetwork(Network* n)
{
	n->netconn = NULL;
	n->netbuf = NULL;
	n->netbuf_offset = 0;
	n->mqttread = mqtt_lwip_read;
	n->mqttwrite = mqtt_lwip_write;
	n->disconnect = mqtt_lwip_disconnect;
}

int ConnectNetwork(Network* n, char* addr, int port)
{
	struct ip_addr ip_addr;
	int rc = -1;
	rc = netconn_gethostbyname(addr, &ip_addr);
	if (rc != ERR_OK) {
		return rc;
	}

	n->netconn = netconn_new(NETCONN_TCP);
	if (n->netconn == NULL) {
		return ERR_MEM;
	}

	rc = netconn_connect(n->netconn, &ip_addr, port);

	return rc;
}
