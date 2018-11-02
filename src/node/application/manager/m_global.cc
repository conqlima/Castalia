/*
 * global.cc
 * 
 * Copyright 2018 Robson Araújo Lima <robson@robson-lenovo>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <csignal>

#include "Manager.h"
#include "m_plugin_castalia.h"
#include "m_global.h"
#include "VirtualApplication.h"
#include "MyPacket_m.h"

extern "C" {
#include "ieee11073.h"
#include "manager.h"
}

Define_Module(Manager);

/**
 * Plugin used by agent to send network data
 */
ContextId m_CONTEXT_ID = {2, 0};

/**
 * PLugin definition
 */
//CommunicationPlugin m_comm_plugin[6] = {COMMUNICATION_PLUGIN_NULL};
//CommunicationPlugin m_comm_plugin[6] = {COMMUNICATION_PLUGIN_NULL};

/**
 *  Variable used by the stack
 */
unsigned long long m_port = 0;


/**
 * Callback function that is called whenever a device
 * is unvailable.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void m_device_unavailable(Context *ctx)
{
	fprintf(stderr, " main: Disasociated\n");
}

/**
 * Callback function that is called whenever a new data
 * has been received.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void new_data_received(Context *ctx, DataList *list)
{
	fprintf(stderr, "Medical Device Data Updated:\n");

	char *data = json_encode_data_list(list);

	if (data != NULL) {
		fprintf(stderr, "%s", data);
		fprintf(stderr, "\n");

		fflush(stderr);
		free(data);
	}

	// uncomment for manager-initiated disassociation testing
	// manager_request_association_release(m_CONTEXT_ID);
}

/**
 * Callback function that is called whenever a new device
 * has been available
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_associated(Context *ctx, DataList *list)
{
	fprintf(stderr, " Medical Device System Associated:\n");

	char *data = json_encode_data_list(list);

	if (data != NULL) {
		fprintf(stderr, "%s", data);
		fprintf(stderr, "\n");

		fflush(stderr);
		free(data);
	}

	device_reqmdsattr();
}

/**
 * Callback function that is called whenever a new device
 * is connected to the Manager.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void print_device_attributes(Context *ctx, Request *r, DATA_apdu *response_apdu)
{
	DataList *list = manager_get_mds_attributes(m_CONTEXT_ID);
	char *data = json_encode_data_list(list);

	fprintf(stderr, "Print device attributes:\n");

	if (data != NULL) {
		fprintf(stderr, "%s", data);
		fprintf(stderr, "\n");

		fflush(stderr);
	}

	data_list_del(list);
	free(data);
}

/**
 * Request all MDS attributes
 *
 */
void device_reqmdsattr()
{
	fprintf(stderr, "device_reqmdsattr\n");
	manager_request_get_all_mds_attributes(m_CONTEXT_ID, print_device_attributes);
}

/**
 * Waits 0 milliseconds for timeout.
 *
 * @param ctx current context.
 * @return fake timeout id
 */
int m_timer_count_timeout(Context *ctx)
{
	unsigned int nodeId = (ctx->id.plugin) / 2;
	m_SETTIMER[nodeId] = 1;
	return 1;
}

/**
 * Fake implementation of the reset timeout function.
 * @param ctx current context.
 */
void m_timer_reset_timeout(Context *ctx)
{
}

/**
 * Configure application to use castalia plugin
 */
void m_castalia_mode(unsigned int my_plugin_number)
{
	m_CONTEXT_ID.plugin = 2;
	m_CONTEXT_ID.connid = m_port;
	m_plugin_network_castalia_manager_setup(&m_comm_plugin[my_plugin_number/2], m_port);
}


