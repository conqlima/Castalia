/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file plugin_CASTALIA_agent.c
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
 * @addtogroup AgentCASTALIAPlugin
 * @{
 */
extern "C"{
#include "util/strbuff.h"
#include "communication/communication.h"
#include "communication/parser/decoder_ASN1.h"
#include "communication/parser/struct_cleaner.h"
#include "util/log.h"
#include "util/ioutil.h"
#include "asn1/phd_types.h"
#include "dim/nomenclature.h"
#include <arpa/inet.h>
}

#include "CastaliaModule.h"
#include "VirtualApplication.h"
#include "MyPacket_m.h"
#include "plugin_castalia.h"
#include "global.h"
#include "Agent.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <iomanip>

// #define TEST_FRAGMENTATION 1

// #define TEST_FRAGMENTATION 1

Define_Module(Agent);


unsigned int plugin_id = 1;

static const int CASTALIA_ERROR = NETWORK_ERROR;
static const int CASTALIA_ERROR_NONE = NETWORK_ERROR_NONE;
static const int BACKLOG = 1;

static int sk = -1;
static unsigned long long port = 0;
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
static int init_socket()
{
	sk = 3;
	ContextId cid = {plugin_id, port};
	communication_transport_connect_indication(cid, "castalia");
	return 1;
}

/**
 * Initialize network layer, in this case opens and initializes
 *  the file descriptors
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int network_init(unsigned int plugin_label)
{
	plugin_id = plugin_label;

	if (init_socket()) {
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
static int network_castalia_wait_for_data(Context *ctx)
{
	DEBUG("network castalia: network_wait_for_data");

	if (sk < 0) {
		DEBUG("network castalia: network_wait_for_data error");
		return CASTALIA_ERROR;
	}

	if (buffer_retry != 0) {
		// there may be another APDU in buffer already
		if (buffer_retry == -1){
			buffer_retry = buffer_retry + 1;
			//return CASTALIA_ERROR_NONE;
		}//else {
			//return CASTALIA_ERROR_NONE;
		//}
	}else {
		buffer_retry = -1;
		return CASTALIA_ERROR;
	}

	return CASTALIA_ERROR_NONE;
}

//static void message_type_ByteStreamReader(Context* ctx, ByteStreamReader *stream,  APDU *pointer, int *error, int apdu_size)
//{
	//unsigned int nodeId = (ctx->id.plugin+1) / 2;
	//ByteStreamReader *streamTmp = byte_stream_reader_instance(stream->buffer_cur, apdu_size);
	//pointer->choice = read_intu16(streamTmp, error);
	//// APDU_choice choice
	//pointer->length = read_intu16(streamTmp, error);
	//pointer->length = read_intu16(streamTmp, error);
	//// intu16 length

	//switch (pointer->choice) {
	//case AARQ_CHOSEN:
		//st_msg[nodeId].send_str = "Association Request";
		////CHK(decode_aarq_apdu(stream, &pointer->u.aarq, error));
		//break;
	//case AARE_CHOSEN:
		//st_msg[nodeId].send_str = "Association Response";
		////CHK(decode_aare_apdu(stream, &pointer->u.aare, error));
		//break;
	//case RLRQ_CHOSEN:
		//st_msg[nodeId].send_str = "Association Release Request";
		////CHK(decode_rlrq_apdu(stream, &pointer->u.rlrq, error));
		//break;
	//case RLRE_CHOSEN:
		//st_msg[nodeId].send_str = "Association Release Response";
		////CHK(decode_rlre_apdu(stream, &pointer->u.rlre, error));
		//break;
	//case ABRT_CHOSEN:
		//st_msg[nodeId].send_str = "Association Abort";
		////CHK(decode_abrt_apdu(stream, &pointer->u.abrt, error));
		//break;
	//case PRST_CHOSEN:{
		////CHK(decode_prst_apdu(stream, &pointer->u.prst, error));
		//pointer->choice = read_intu16(streamTmp, error);
		//pointer->choice = read_intu16(streamTmp, error);
		//// APDU_choice choice
		//switch (pointer->choice) {
			//case ROIV_CMIP_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "Configuration with no confirmation";
				//else
				//st_msg[nodeId].send_str = "Measurement with no confirmation";
				//break;
			//}
			//case ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "Configuration with confirmation";
				//else
				//st_msg[nodeId].send_str = "Measurement with confirmation";
				//break;
			//}
			//case ROIV_CMIP_GET_CHOSEN:
				//st_msg[nodeId].send_str = "GET configuration with confirmation";
				//break;
			//case ROIV_CMIP_SET_CHOSEN:
				//st_msg[nodeId].send_str = "SET configuration with no confirmation";
				//break;
			//case ROIV_CMIP_CONFIRMED_SET_CHOSEN:
				//st_msg[nodeId].send_str = "SET configuration with confirmation";
				//break;
			//case ROIV_CMIP_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "ACTION with no confirmation";
				//break;
			//case ROIV_CMIP_CONFIRMED_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "ACTION with confirmation";
				//break;
			//case RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//pointer->choice = read_intu16(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "response of a configuration with confirmation";
				//else
				//st_msg[nodeId].send_str = "response of a measurement with confirmation";
				//break;
			//}
			//case RORS_CMIP_GET_CHOSEN:
				//st_msg[nodeId].send_str = "response of a GET configuration";
				//break;
			//case RORS_CMIP_CONFIRMED_SET_CHOSEN:
				//st_msg[nodeId].send_str = "response for a SET configuration with confirmation";
				//break;
			//case RORS_CMIP_CONFIRMED_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "response of a ACTION with confirmation";
				//break;
			//case ROER_CHOSEN:
				//st_msg[nodeId].send_str = "Remote Invoke Error";
				//break;
			//case RORJ_CHOSEN:
				//st_msg[nodeId].send_str = "Remote Invoke Reject";
				//break;
			//default:
				//st_msg[nodeId].send_str = "unknown data apdu choice";
				////ERROR("unknown data apdu choice");
				////goto fail;
				//break;
		//}
			////EPILOGUE(data_apdu_message);
				//break;
	//}
	//default:
		//st_msg[nodeId].send_str = "unknown data apdu choice";
		////ERROR("unknown apdu choice");
		////goto fail;
		//break;
	//}
	////EPILOGUE(apdu);
//}

//static void message_type_ByteStreamWriter(Context* ctx, ByteStreamWriter *stream, APDU *pointer, int *error, int size)
//{
	//unsigned int nodeId = (ctx->id.plugin+1) / 2;
	//ByteStreamWriter *streamTmp = (ByteStreamWriter *) calloc(1, sizeof(ByteStreamWriter));
	//streamTmp->buffer = (intu8 *) calloc(1, size * sizeof(intu8));
	//streamTmp->buffer = stream->buffer;
	//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
	//// APDU_choice choice
	//pointer->length = read_intu16_ByteStreamWriter(streamTmp, error);
	//pointer->length = read_intu16_ByteStreamWriter(streamTmp, error);
	//// intu16 length
	//switch (pointer->choice) {
	//case AARQ_CHOSEN:
		//st_msg[nodeId].send_str = "Association Request";
		////CHK(decode_aarq_apdu(stream, &pointer->u.aarq, error));
		//break;
	//case AARE_CHOSEN:
		//st_msg[nodeId].send_str = "Association Response";
		////CHK(decode_aare_apdu(stream, &pointer->u.aare, error));
		//break;
	//case RLRQ_CHOSEN:
		//st_msg[nodeId].send_str = "Association Release Request";
		////CHK(decode_rlrq_apdu(stream, &pointer->u.rlrq, error));
		//break;
	//case RLRE_CHOSEN:
		//st_msg[nodeId].send_str = "Association Release Response";
		////CHK(decode_rlre_apdu(stream, &pointer->u.rlre, error));
		//break;
	//case ABRT_CHOSEN:
		//st_msg[nodeId].send_str = "Association Abort";
		////CHK(decode_abrt_apdu(stream, &pointer->u.abrt, error));
		//break;
	//case PRST_CHOSEN:{
		////CHK(decode_prst_apdu(stream, &pointer->u.prst, error));
		//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
		//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
		//// APDU_choice choice
		//switch (pointer->choice) {
			//case ROIV_CMIP_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "Configuration with no confirmation";
				//else
				//st_msg[nodeId].send_str = "Measurement with no confirmation";
				//break;
			//}
			//case ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "Configuration with confirmation";
				//else
				//st_msg[nodeId].send_str = "Measurement with confirmation";
				//break;
			//}
			//case ROIV_CMIP_GET_CHOSEN:
				//st_msg[nodeId].send_str = "GET configuration with confirmation";
				//break;
			//case ROIV_CMIP_SET_CHOSEN:
				//st_msg[nodeId].send_str = "SET configuration with no confirmation";
				//break;
			//case ROIV_CMIP_CONFIRMED_SET_CHOSEN:
				//st_msg[nodeId].send_str = "SET configuration with confirmation";
				//break;
			//case ROIV_CMIP_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "ACTION with no confirmation";
				//break;
			//case ROIV_CMIP_CONFIRMED_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "ACTION with confirmation";
				//break;
			//case RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//pointer->choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//int dec;
				//std::stringstream streamc;
				//streamc << pointer->choice;
				//streamc >> std::dec >> dec;
				//if (dec == MDC_NOTI_CONFIG)
				//st_msg[nodeId].send_str = "response of a configuration with confirmation";
				//else
				//st_msg[nodeId].send_str = "response of a measurement with confirmation";
				//break;
			//}
			//case RORS_CMIP_GET_CHOSEN:
				//st_msg[nodeId].send_str = "GET configuration";
				//break;
			//case RORS_CMIP_CONFIRMED_SET_CHOSEN:
				//st_msg[nodeId].send_str = "response for a SET configuration with confirmation";
				//break;
			//case RORS_CMIP_CONFIRMED_ACTION_CHOSEN:
				//st_msg[nodeId].send_str = "response of a ACTION with confirmation";
				//break;
			//case ROER_CHOSEN:
				//st_msg[nodeId].send_str = "Remote Invoke Error";
				//break;
			//case RORJ_CHOSEN:
				//st_msg[nodeId].send_str = "Remote Invoke Reject";
				//break;
			//default:
				//st_msg[nodeId].send_str = "unknown data apdu choice";
				////ERROR("unknown data apdu choice");
				////goto fail;
				//break;
		//}
			////EPILOGUE(data_apdu_message);
				//break;
	//}
	//default:
		//st_msg[nodeId].send_str = "unknown data apdu choice";
		////ERROR("unknown apdu choice");
		////goto fail;
		//break;
	//}
	////EPILOGUE(apdu);
//}

static void message_type(intu8 * buffer, int size, Context* ctx){
	unsigned int nodeId = (ctx->id.plugin+1) / 2;
	intu8 * bufferTmp = (intu8 *) calloc(1, size * sizeof(intu8));
	bufferTmp = buffer;
	
	intu16 choice;
	choice = ntohs(*((uint16_t *) bufferTmp));
	bufferTmp += 8;
	
	switch (choice) {
	case AARQ_CHOSEN:
		st_msg[nodeId].send_str = "Association Request";
		st_msg[nodeId].fila.push("Association Request");
		//CHK(decode_aarq_apdu(stream, &pointer->u.aarq, error));
		break;
	case AARE_CHOSEN:
		st_msg[nodeId].send_str = "Association Response";
		st_msg[nodeId].fila.push("Association Response");
		//CHK(decode_aare_apdu(stream, &pointer->u.aare, error));
		break;
	case RLRQ_CHOSEN:
		st_msg[nodeId].send_str = "Association Release Request";
		st_msg[nodeId].fila.push("Association Release Request");
		//CHK(decode_rlrq_apdu(stream, &pointer->u.rlrq, error));
		break;
	case RLRE_CHOSEN:
		st_msg[nodeId].send_str = "Association Release Response";
		st_msg[nodeId].fila.push("Association Release Response");
		//CHK(decode_rlre_apdu(stream, &pointer->u.rlre, error));
		break;
	case ABRT_CHOSEN:
		st_msg[nodeId].send_str = "Association Abort";
		st_msg[nodeId].fila.push("Association Abort");
		//CHK(decode_abrt_apdu(stream, &pointer->u.abrt, error));
		break;
	case PRST_CHOSEN:{
		choice = ntohs(*((uint16_t *) bufferTmp));
		//CHK(decode_prst_apdu(stream, &pointer->u.prst, error));
		//choice = read_intu16_ByteStreamWriter(streamTmp, error);
		//choice = read_intu16_ByteStreamWriter(streamTmp, error);
		// APDU_choice choice
		switch (choice) {
			case ROIV_CMIP_EVENT_REPORT_CHOSEN:{
				bufferTmp += 10;
				choice = ntohs(*((uint16_t *) bufferTmp));
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				int dec;
				std::stringstream streamc;
				streamc << choice;
				streamc >> std::dec >> dec;
				if (dec == MDC_NOTI_CONFIG){
				st_msg[nodeId].send_str = "Configuration with no confirmation";
				st_msg[nodeId].fila.push("Configuration with no confirmation");
				}else{
				st_msg[nodeId].send_str = "Measurement with no confirmation";
				st_msg[nodeId].fila.push("Measurement with no confirmation");
				}
				break;
			}
			case ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				bufferTmp += 10;
				choice = ntohs(*((uint16_t *) bufferTmp));
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				int dec;
				std::stringstream streamc;
				streamc << choice;
				streamc >> std::dec >> dec;
				if (dec == MDC_NOTI_CONFIG){
				st_msg[nodeId].send_str = "Configuration with confirmation";
				st_msg[nodeId].fila.push("Configuration with confirmation");
				}else{
				st_msg[nodeId].send_str = "Measurement with confirmation";
				st_msg[nodeId].fila.push("Measurement with confirmation");
				}
				break;
			}
			case ROIV_CMIP_GET_CHOSEN:
				st_msg[nodeId].send_str = "GET configuration with confirmation";
				st_msg[nodeId].fila.push("GET configuration with confirmation");
				break;
			case ROIV_CMIP_SET_CHOSEN:
				st_msg[nodeId].send_str = "SET configuration with no confirmation";
				st_msg[nodeId].fila.push("SET configuration with no confirmation");
				break;
			case ROIV_CMIP_CONFIRMED_SET_CHOSEN:
				st_msg[nodeId].send_str = "SET configuration with confirmation";
				st_msg[nodeId].fila.push("SET configuration with confirmation");
				break;
			case ROIV_CMIP_ACTION_CHOSEN:
				st_msg[nodeId].send_str = "ACTION with no confirmation";
				st_msg[nodeId].fila.push("ACTION with no confirmation");
				break;
			case ROIV_CMIP_CONFIRMED_ACTION_CHOSEN:
				st_msg[nodeId].send_str = "ACTION with confirmation";
				st_msg[nodeId].fila.push("ACTION with confirmation");
				break;
			case RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:{
				bufferTmp += 10;
				choice = ntohs(*((uint16_t *) bufferTmp));
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				//choice = read_intu16_ByteStreamWriter(streamTmp, error);
				int dec;
				std::stringstream streamc;
				streamc << choice;
				streamc >> std::dec >> dec;
				if (dec == MDC_NOTI_CONFIG){
				st_msg[nodeId].send_str = "response of a configuration with confirmation";
				st_msg[nodeId].fila.push("response of a configuration with confirmation");
				}else{
				st_msg[nodeId].send_str = "response of a measurement with confirmation";
				st_msg[nodeId].fila.push("response of a measurement with confirmation");
				}
				break;
			}
			case RORS_CMIP_GET_CHOSEN:
				st_msg[nodeId].send_str = "GET configuration";
				st_msg[nodeId].fila.push("response of a GET configuration");
				break;
			case RORS_CMIP_CONFIRMED_SET_CHOSEN:
				st_msg[nodeId].send_str = "response for a SET configuration with confirmation";
				st_msg[nodeId].fila.push("response for a SET configuration with confirmation");
				break;
			case RORS_CMIP_CONFIRMED_ACTION_CHOSEN:
				st_msg[nodeId].send_str = "response of a ACTION with confirmation";
				st_msg[nodeId].fila.push("response of a ACTION with confirmation");
				break;
			case ROER_CHOSEN:
				st_msg[nodeId].send_str = "Remote Invoke Error";
				st_msg[nodeId].fila.push("Remote Invoke Error");
				break;
			case RORJ_CHOSEN:
				st_msg[nodeId].send_str = "Remote Invoke Reject";
				st_msg[nodeId].fila.push("Remote Invoke Reject");
				break;
			default:
				st_msg[nodeId].send_str = "unknown data apdu choice";
				st_msg[nodeId].fila.push("unknown data apdu choice");
				//ERROR("unknown data apdu choice");
				//goto fail;
				break;
		}
			//EPILOGUE(data_apdu_message);
				break;
	}
	default:
		st_msg[nodeId].send_str = "unknown data apdu choice";
		//ERROR("unknown apdu choice");
		//goto fail;
		break;
	}
}

/**
 * Reads an APDU from the file descriptor
 * @param ctx
 * @return a byteStream with the read APDU or NULL if error.
 */
static ByteStreamReader *network_get_apdu_stream(Context *ctx)
{
	ContextId cid = {plugin_id, port};
	unsigned int nodeId = (ctx->id.plugin+1) / 2;
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
		
		/*Transfere dados de st_msg para localbuf*/
		for (i = 0; i < st_msg[nodeId].tam_buff; i++)
		{
			localbuf[i] = st_msg[nodeId].buff_msg[i];
		}
		
		int bytes_read = st_msg[nodeId].tam_buff;
		st_msg[nodeId].tam_buff = 0;

		if (bytes_read < 0) {
			free(buffer);
			buffer = 0;
			buffer_size = 0;
			communication_transport_disconnect_indication(cid, "castalia");
			DEBUG(" network:CASTALIA error");
			sk = -1;
			return NULL;
		} else if (bytes_read == 0) {
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
	//int error = 0;
	//APDU apdu;
	//message_type_ByteStreamReader(ctx, stream, &apdu, &error, apdu_size);
	message_type(stream->buffer, apdu_size, ctx);
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
static int network_send_apdu_stream(Context *ctx, ByteStreamWriter *stream)
{
	unsigned int written = 0;
	//pego o número do nó baseado no número do plugin
	unsigned int nodeId = (ctx->id.plugin + 1)/2;
	while (written < stream->size) {
		int to_send = stream->size - written;
#ifdef TEST_FRAGMENTATION
		to_send = to_send > 50 ? 50 : to_send;
#endif
		//int ret = write(sk, stream->buffer + written, to_send);
		int ret = stream->size;
		DEBUG(" network:CASTALIA sent %d bytes", to_send);

		if (ret <= 0) {
			DEBUG(" network:CASTALIA Error sending APDU.");
			return CASTALIA_ERROR;
		}

		written += ret;
	}
	
	if ((stream->size) > 0){
		unsigned int i;
		
		/*Copia conteúdo de stream->buffer para m_st_msg*/
		for (i = 0; i < stream->size; i++) {
			st_msg[nodeId].buff_msg[i+st_msg[nodeId].tam_buff] = stream->buffer[i];
			//sprintf(str, "%s%.2X ", str, stream->buffer[i]);
		}
		
		/*Caso o pacote tenha duas mensagens, o tamanho do pacote
		 * deve ser o tamanho da msg1 + msg2 + ...*/
		st_msg[nodeId].tam_buff += stream->size;
		
		/*limpa o restanto do pacote*/
		//for (int i = st_msg[ctx->id.plugin].tam_buff; i < 65535; i++) {
		for (int i = st_msg[nodeId].tam_buff; i < 65535; i++) {
			st_msg[nodeId].buff_msg[i] = '\0';
		}

	}
	
	//int error = 0;
	//APDU apdu;
	//message_type_ByteStreamWriter(ctx, stream, &apdu, &error, stream->size);
	message_type(stream->buffer, stream->size, ctx);
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
static int network_disconnect(Context *ctx)
{
	DEBUG("taking the initiative of disconnection");
	free(buffer);
	buffer = 0;
	buffer_size = 0;
	return CASTALIA_ERROR_NONE;
}

/**
 * Finalizes network layer and deallocated data
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int network_finalize()
{
	free(buffer);
	buffer = 0;
	buffer_size = 0;
	return CASTALIA_ERROR_NONE;
}

/**
 * Initiate a CommunicationPlugin struct to use CASTALIA connection.
 *
 * @param plugin CommunicationPlugin pointer
 * @param pport Port of the socket
 *
 * @return CASTALIA_ERROR if error
 */
int plugin_network_castalia_agent_setup(CommunicationPlugin *plugin, int pport)
{
	DEBUG("network:castalia Initializing agent socket");
	plugin->network_init = network_init;
	plugin->network_wait_for_data = network_castalia_wait_for_data;
	plugin->network_get_apdu_stream = network_get_apdu_stream;
	plugin->network_send_apdu_stream = network_send_apdu_stream;
	plugin->network_disconnect = network_disconnect;
	plugin->network_finalize = network_finalize;

	return CASTALIA_ERROR_NONE;
}



/** @} */
