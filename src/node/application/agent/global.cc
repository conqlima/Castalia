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

#include "Agent.h"
#include "plugin_castalia.h"
#include "global.h"
#include "VirtualApplication.h"
#include "MyPacket_m.h"

extern "C" {
#include <ieee11073.h>
#include "agent.h"
#include "sample_agent_common.h"
}

Define_Module(Agent);

/**
 * Port used by agent to send network data
 */
ContextId CONTEXT_ID = {1, 0};

/**
 * PLugin definition
 */
//CommunicationPlugin comm_plugin[6];
//CommunicationPlugin comm_plugin[6] = {COMMUNICATION_PLUGIN_NULL};
//comm_plugin[0] = COMMUNICATION_PLUGIN_NULL;
//comm_plugin[1] = COMMUNICATION_PLUGIN_NULL;
//comm_plugin[2] = COMMUNICATION_PLUGIN_NULL;
//comm_plugin[3] = COMMUNICATION_PLUGIN_NULL;
//comm_plugin[4] = COMMUNICATION_PLUGIN_NULL;
//comm_plugin[5] = COMMUNICATION_PLUGIN_NULL;

/**
 * Waits 0 milliseconds for timeout.
 *
 * @param ctx current context.
 * @return fake timeout id
 */
int timer_count_timeout(Context *ctx)
{
	unsigned int nodeId = (ctx->id.plugin+1) / 2;
	SETTIMER[nodeId] = 1;
	return 1;
}

/**
 * Fake implementation of the reset timeout function.
 * @param ctx current context.
 */
void timer_reset_timeout(Context *ctx)
{
}

/**
 * Callback function that is called whenever a 
 * Divece is associated with a Manager.
 * 
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_associated(Context *ctx)
{
	fprintf(stderr, " main: Associated\n");
}

/**
 * Callback function that is called whenever a device
 * is unvailable.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_unavailable(Context *ctx)
{
	fprintf(stderr, " main: Disasociated\n");
}

/**
 * Callback function that is called whenever a 
 * Divece is initiated (connected).
 * 
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_connected(Context *ctx, const char *addr)
{
	fprintf(stderr, "main: Connected\n");

	// ok, make it proceed with association
	// (agent has the initiative)
	agent_associate(ctx->id);
}

/**
 * Configure application to use castalia plugin
 */
void castalia_mode(unsigned int nodeNumber)
{
	int port = 0;
	// NOTE we know that plugin id = 1 here, but
	// might not be the case!
	CONTEXT_ID.plugin = 1;
	CONTEXT_ID.connid = port;
	plugin_network_castalia_agent_setup(&comm_plugin[nodeNumber], port);
}
