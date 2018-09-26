/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file plugin_CASTALIA_manager.c
 * \brief CASTALIA plugin source.
 *
 * Copyright (C) 2011 Signove Tecnologia Corporation.
 * All rights reserved.
 * Contact: Signove Tecnologia Corporation (contact@signove.com)
 *
 * $LICENSE_TEXT:BEGIN$
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and appearing
 * in the file LICENSE included in the packaging of this file; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * $LICENSE_TEXT:END$
 *
 * IEEE 11073 Communication Model - Finite State Machine implementation
 *
 * \author Elvis Pfutzenreuter
 * \author Adrian Guedes
 * \author Fabricio Silva Epaminondas
 * \date Jun 28, 2011
 */

/**
 * @addtogroup ManagerCASTALIAPlugin
 * @{
 */
extern "C"{
#include "util/strbuff.h"
#include "communication/communication.h"
#include "communication/service.h"
#include "util/log.h"
#include "util/ioutil.h"
#include "manager.h"
}
#include "CastaliaModule.h"
#include "VirtualApplication.h"
#include "MyPacket_m.h"
#include "m_plugin_castalia.h"
#include "m_global.h"
#include "Manager.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>

// #define TEST_FRAGMENTATION 1

// #define TEST_FRAGMENTATION 1

Define_Module(Manager);

unsigned int m_plugin_id = 2;

static const int CASTALIA_ERROR = NETWORK_ERROR;
static const int CASTALIA_ERROR_NONE = NETWORK_ERROR_NONE;
static const int BACKLOG = 1;

static int sk = -1;
static intu8 *buffer = NULL;
static int buffer_size = 0;
static int buffer_retry = -1;

using namespace std;

/**
 * Initialize network layer.
 * Initialize network layer, in this case opens and initializes the CASTALIA socket.
 *
 * @return 1 if operation succeeds and 0 otherwise
 */
static int m_init_socket()
{
	if ((sk = open("castalia", O_RDWR | O_CREAT, 0644)) < 0) return 0;
	ContextId cid = {m_plugin_id, m_port};
	communication_transport_connect_indication(cid, "castalia");
	return 1;
}

/**
 * Initialize network layer, in this case opens and initializes
 *  the file descriptors
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int m_network_init(unsigned int plugin_label)
{
	m_plugin_id = plugin_label;

	if (m_init_socket()) {
		return CASTALIA_ERROR_NONE;
	}

	return CASTALIA_ERROR;
}

/**
 * Blocks to wait data to be available from the file descriptor
 *
 * @param ctx current connection context.
 * @return CASTALIA_ERROR_NONE if data is available or CASTALIA_ERROR if error.
 */
static int m_network_castalia_wait_for_data(Context *ctx)
{
	DEBUG("network castalia (manager): network_wait_for_data");

	if (sk < 0) {
		DEBUG("network castalia: network_wait_for_data error");
		return CASTALIA_ERROR;
	}

	if (buffer_retry != 0) {
		// there may be another APDU in buffer already
		if (buffer_retry == -1){
		buffer_retry = buffer_retry + 1;
		return CASTALIA_ERROR_NONE;
		}else {
		return CASTALIA_ERROR_NONE;
		}
	}
	else{
		//ctx->connection_loop_active = 0;
		buffer_retry = -1;
		return CASTALIA_ERROR;
	}

	//fd_set fds;

	//int ret_value;

	//while (1) {
		//if (sk < 0) {
			//return CASTALIA_ERROR;
		//}

		//FD_ZERO(&fds);
		//FD_SET(sk, &fds);

		//ret_value = select(sk + 1, &fds, NULL, NULL, NULL);
		//if (ret_value < 0) {
			//if (errno == EINTR) {
				//DEBUG(" network:fd Select failed with EINTR");
				//continue;
			//}
			//DEBUG(" network:fd Select failed");
			//return CASTALIA_ERROR;
		//} else if (ret_value == 0) {
			//DEBUG(" network:fd Select timeout");
			//return CASTALIA_ERROR;
		//}

		//break;
	//}

	return CASTALIA_ERROR_NONE;
}

/**
 * Reads an APDU from the file descriptor
 * @param ctx
 * @return a byteStream with the read APDU or NULL if error.
 */
static ByteStreamReader *m_network_get_apdu_stream(Context *ctx)
{
	ContextId cid = {m_plugin_id, m_port};

	if (sk < 0) {
		ERROR("network CASTALIA: network_get_apdu_stream cannot found a valid sokcet");
		communication_transport_disconnect_indication(cid, "castalia");
		return NULL;
	}

	if (buffer_retry) {
		// handling letover data in buffer
		buffer_retry = 0;
	} else {
		int i;
		intu8 localbuf[65535];
		for (i = 0; i < m_st_msg.tam_buff; i++)
		{
			localbuf[i] = m_st_msg.buff_msg[i];
		}
		m_st_msg.tam_buff = 0;
		int bytes_read = i;

		if (bytes_read < 0) {
			close(sk);
			free(buffer);
			buffer = 0;
			buffer_size = 0;
			communication_transport_disconnect_indication(cid, "castalia");
			DEBUG(" network:CASTALIA error");
			sk = -1;
			return NULL;
		} else if (bytes_read == 0) {
			close(sk);
			free(buffer);
			buffer = 0;
			buffer_size = 0;
			communication_transport_disconnect_indication(cid, "castalia");
			DEBUG(" network:CASTALIA closed");
			sk = -1;
			return NULL;
		}

		buffer = (intu8*) realloc(buffer, buffer_size + bytes_read);
		memcpy(buffer + buffer_size, localbuf, bytes_read);
		buffer_size += bytes_read;
	}

	if (buffer_size < 4) {
		DEBUG(" network:CASTALIA incomplete APDU (received %d)", buffer_size);
		return NULL;
	}

	int apdu_size = (buffer[2] << 8 | buffer[3]) + 4;

	if (buffer_size < apdu_size) {
		DEBUG(" network:CASTALIA incomplete APDU (expected %d received %d",
		      					apdu_size, buffer_size);
		return NULL;
	}

	// Create bytestream
	ByteStreamReader *stream = byte_stream_reader_instance(buffer, apdu_size);

	if (stream == NULL) {
		DEBUG(" network:CASTALIA Error creating bytelib");
		free(buffer);
		buffer = NULL;
		buffer_size = 0;
		return NULL;
	}

	buffer = 0;
	buffer_size -= apdu_size;
	if (buffer_size > 0) {
		// leave next APDU in buffer
		buffer_retry = 1;
		buffer = (intu8*) malloc(buffer_size);
		memcpy(buffer, stream->buffer_cur + apdu_size, buffer_size);
	}
	
	DEBUG(" network:CASTALIA APDU received ");
	ioutil_print_buffer(stream->buffer_cur, apdu_size);

	return stream;
}

/**
 * Sends an encoded apdu
 *
 * @param ctx
 * @param stream the apdu to be sent
 * @return CASTALIA_ERROR_NONE if data sent successfully and CASTALIA_ERROR otherwise
 */
static int m_network_send_apdu_stream(Context *ctx, ByteStreamWriter *stream)
{
	unsigned int written = 0;

	while (written < stream->size) {
		int to_send = stream->size - written;
#ifdef TEST_FRAGMENTATION
		to_send = to_send > 50 ? 50 : to_send;
#endif
		int ret = write(sk, stream->buffer + written, to_send);

		DEBUG(" network:CASTALIA sent %d bytes", to_send);

		if (ret <= 0) {
			DEBUG(" network:CASTALIA Error sending APDU.");
			return CASTALIA_ERROR;
		}

		written += ret;
	}
	if ((stream->size) > 0){
	char * str = new char[stream->size*4];

	unsigned int i;
	
	for (i = 0; i < stream->size; i++) {
		m_st_msg.buff_msg[i+m_st_msg.tam_buff] = stream->buffer[i];
		sprintf(str, "%s%.2X ", str, stream->buffer[i]);
	}
	m_st_msg.tam_buff += stream->size;
	m_st_msg.send_str = str;
	//DEBUG("%s", str);
	//fflush(stdout);
	delete[] str;
	//str = NULL;
	}
	DEBUG(" network:CASTALIA APDU sent ");
	ioutil_print_buffer(stream->buffer, stream->size);
	
	return CASTALIA_ERROR_NONE;
}

/**
 * Network disconnect
 *
 * @param ctx
 * @return CASTALIA_ERROR_NONE
 */
static int m_network_disconnect(Context *ctx)
{
	DEBUG("taking the initiative of disconnection");
	close(sk);
	sk = -1;
	free(buffer);
	buffer = 0;
	buffer_size = 0;
	buffer_retry = 0;

	return CASTALIA_ERROR_NONE;
}

/**
 * Finalizes network layer and deallocated data
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int m_network_finalize()
{

	close(sk);
	sk = -1;

	free(buffer);
	buffer = 0;
	buffer_size = 0;

	return CASTALIA_ERROR_NONE;
}

//static int create_socket(int pport)
//{
	//DEBUG("network castalia: creating socket configuration");
	//return CASTALIA_ERROR_NONE;
//}

/**
 * Initiate a CommunicationPlugin struct to use CASTALIA connection.
 *
 * @param plugin CommunicationPlugin pointer
 * @param pport Port of the socket
 *
 * @return CASTALIA_ERROR if error
 */
int m_plugin_network_castalia_manager_setup(CommunicationPlugin *plugin, int pport)
{
	DEBUG("network:castalia Initializing manager socket");
	plugin->network_init = m_network_init;
	plugin->network_wait_for_data = m_network_castalia_wait_for_data;
	plugin->network_get_apdu_stream = m_network_get_apdu_stream;
	plugin->network_send_apdu_stream = m_network_send_apdu_stream;
	plugin->network_disconnect = m_network_disconnect;
	plugin->network_finalize = m_network_finalize;

	return CASTALIA_ERROR_NONE;
}

/** @} */
