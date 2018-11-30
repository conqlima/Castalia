/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file sample_agent_common.c
 * \brief Main application implementation.
 *
 * Copyright (C) 2012 Signove Tecnologia Corporation.
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
 * \author Elvis Pfutzenreuter
 * \date Apr 17, 2012
 */
extern "C" {
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <ieee11073.h>
#include "specializations/pulse_oximeter.h"
#include "specializations/blood_pressure_monitor.h"
#include "specializations/weighing_scale.h"
#include "specializations/glucometer.h"
#include "specializations/thermometer.h"
#include "specializations/basic_ECG.h"
#include "agent.h"
}
#include "sample_agent_common.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <netinet/in.h>

float ECG_samples[] = {
-0.060,
-0.065,
-0.060,
-0.075,
-0.065,
-0.070,
-0.070,
-0.090,
-0.080,
-0.095,
-0.080,
-0.095,
-0.080,
-0.095,
-0.085,
-0.090,
-0.090,
-0.100,
-0.085,
-0.105,
-0.090,
-0.045,
 0.005,
 0.015,
 0.045,
 0.155,
 0.140,
 0.045,
 0.005,
-0.040,
-0.085,
-0.200,
-0.195,
-0.200,
-0.200,
-0.240,
-0.130,
 0.340,
 1.155,
 1.470,
-0.155,
-0.825,
-0.590,
-0.350,
-0.155,
-0.170,
-0.140,
-0.155,
-0.115,
-0.125,
-0.090,
-0.095,
-0.065,
-0.055,
-0.015,
-0.005,
 0.035,
 0.045,
 0.090,
 0.110,
 0.150,
 0.180,
 0.205,
 0.225,
 0.230,
 0.220,
 0.235,
 0.230,
 0.200,
 0.170,
 0.120,
 0.075,
 0.040,
 0.020,
 0.005,
-0.005,
-0.005,
-0.010,
-0.015,
-0.010};

intu8 AGENT_SYSTEM_ID_VALUE[] = { 0x11, 0x33, 0x55, 0x77, 0x99,
					0xbb, 0xdd, 0xff};

//int alarms = 4;
//ContextId CONTEXT_ID = {1, 1};
//CommunicationPlugin comm_plugin = COMMUNICATION_PLUGIN_NULL;


/**
 * Generate data for oximeter event report
 */
void *oximeter_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct oximeter_event_report_data* data =
		(oximeter_event_report_data*) malloc(sizeof(struct oximeter_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->beats = 60.5 + random() % 20;
	data->oximetry = 90.5 + random() % 10;
	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for blood pressure event report
 */
void *blood_pressure_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct blood_pressure_event_report_data* data =
		(blood_pressure_event_report_data*) malloc(sizeof(struct blood_pressure_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->systolic = 110 + random() % 30;
	data->diastolic = 70 + random() % 20;
	data->mean = 90 + random() % 10;
	data->pulse_rate = 60 + random() % 30;

	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for weight scale event report
 */
void *weightscale_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct weightscale_event_report_data* data =
		(weightscale_event_report_data*) malloc(sizeof(struct weightscale_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->weight = 70.2 + random() % 20;
	data->bmi = 20.3 + random() % 10;

	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for Glucometer event report
 */
void *glucometer_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct glucometer_event_report_data* data =
		(glucometer_event_report_data*) malloc(sizeof(struct glucometer_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->capillary_whole_blood = 10.2 + random() % 20;

	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for Thermometer event report
 */
void *thermometer_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct thermometer_event_report_data* data =
		(thermometer_event_report_data*) malloc(sizeof(struct thermometer_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->temperature = 36.5 + random() % 5;

	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for Basic ECG event report
 */
void *basic_ECG_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct basic_ECG_event_report_data* data =
		(basic_ECG_event_report_data*) malloc(sizeof(struct basic_ECG_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);
	//std::ifstream filein("mv");
	//if (!filein.is_open())
	//perror("error while opening file");
	//for (std::string line; std::getline(filein, line); )
	float R;
	intu16 X[20];
	int j = 0;
	data->mV = (intu8*) calloc(sizeof(intu8), 40);
	for (static int i = 0; i < 80; i++)
	{
		//std::istringstream iss(line);
		//if (!(iss >> R)){ // error
	      //break;
	    //}
	    R = ECG_samples[i];
	    float M = (2.0-(-2.0))/(800.0-0.0);//0,005
	    float B = 2.0-(M*800.0);//-2

	    if (j > 19){ 
			j = 0;
			break;
		}
		X[j] = (intu16)((R - B) / M);
		j++;
	}
	for (int i = 0; i < 20; i++)
	{
		*((uint16_t *) (data->mV + (i*2))) = htons(X[i]);
	}
	
	//memcpy ( data->mV , X , 40 );
	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

/**
 * Generate data for MDS
 */
struct mds_system_data *mds_data_cb()
{
	struct mds_system_data* data = (mds_system_data*) malloc(sizeof(struct mds_system_data));
	memcpy(&data->system_id, AGENT_SYSTEM_ID_VALUE, 8);
	return data;
}

//int timer_count_timeout(Context *ctx)
//{
	//return 1;
//}

//void timer_reset_timeout(Context *ctx)
//{
//}

//void device_associated(Context *ctx)
//{
	//fprintf(stderr, " main: Associated\n");
	//alarm(3);
//}

//void device_unavailable(Context *ctx)
//{
	//fprintf(stderr, " main: Disasociated\n");
	//alarms = 0;
//}

//void device_connected(Context *ctx, const char *addr)
//{
	//fprintf(stderr, "main: Connected\n");

	//// ok, make it proceed with association
	//// (agent has the initiative)
	//agent_associate(ctx->id);
//}

//void sigalrm(int dummy)
//{
	//// This is not incredibly safe, because signal may interrupt
	//// processing, and is not a technique for a production agent,
	//// but suffices for this quick 'n dirty sample
	
	//if (alarms > 1) {
		//agent_send_data(CONTEXT_ID);
		//alarm(3);
	//} else if (alarms == 1) {
		//agent_request_association_release(CONTEXT_ID);
		//alarm(3);
	//} else {
		//agent_disconnect(CONTEXT_ID);
	//}

	//--alarms;
//}

///**
 //* Configure application to use tcp plugin
 //*/
//void tcp_mode()
//{
	//int port = 6024;
	//// NOTE we know that plugin id = 1 here, but
	//// might not be the case!
	//CONTEXT_ID.plugin = 1;
	//CONTEXT_ID.connid = port;
	//plugin_network_tcp_agent_setup(&comm_plugin, port);
//}


