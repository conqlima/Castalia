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

#include "Thermometer.h"
#include "plugin_castalia.h"
#include "global.h"
#include "VirtualApplication.h"
#include "MyPacket_m.h"

extern "C"{
#include <ieee11073.h>
#include "agent.h"
#include "sample_agent_common.h"
}

Define_Module(Thermometer);

/**
 * Port used by agent to send network data
 */
ContextId CONTEXT_ID = {1, 1};

/**
 * PLugin definition
 */
CommunicationPlugin comm_plugin = COMMUNICATION_PLUGIN_NULL;

int alarms = 4;

int timer_count_timeout(Context *ctx)
{
	return 1;
}

void timer_reset_timeout(Context *ctx)
{
}

void device_associated(Context *ctx)
{
	fprintf(stderr, " main: Associated\n");
	alarm(3);
}

void device_unavailable(Context *ctx)
{
	fprintf(stderr, " main: Disasociated\n");
	alarms = 0;
}

void device_connected(Context *ctx, const char *addr)
{
	fprintf(stderr, "main: Connected\n");

	// ok, make it proceed with association
	// (agent has the initiative)
	agent_associate(ctx->id);
}

void sigalrm(int dummy)
{
	// This is not incredibly safe, because signal may interrupt
	// processing, and is not a technique for a production agent,
	// but suffices for this quick 'n dirty sample
	
	if (alarms > 1) {
		agent_send_data(CONTEXT_ID);
		alarm(3);
	} else if (alarms == 1) {
		agent_request_association_release(CONTEXT_ID);
		alarm(3);
	} else {
		agent_disconnect(CONTEXT_ID);
	}

	--alarms;
}

/**
 * Configure application to use castalia plugin
 */
void castalia_mode()
{
	int port = 0;
	// NOTE we know that plugin id = 1 here, but
	// might not be the case!
	CONTEXT_ID.plugin = 1;
	CONTEXT_ID.connid = port;
	plugin_network_castalia_agent_setup(&comm_plugin, port);
}


