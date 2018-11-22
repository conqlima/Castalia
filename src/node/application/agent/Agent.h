#ifndef _AGENT_H_
#define _AGENT_H_

#include "VirtualApplication.h"
#include "MyPacket_m.h"

#include "plugin_castalia.h"
#include "global.h"
#include "sample_agent_common.h"

#include <simtime.h>
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>

extern "C" {
#include "communication/plugin/plugin.h"
#include "communication/communication.h"
#include "communication/context_manager.h"
#include "communication/context.h"
#include "specializations/pulse_oximeter.h"
#include "specializations/blood_pressure_monitor.h"
#include "specializations/weighing_scale.h"
#include "specializations/glucometer.h"
#include "specializations/thermometer.h"
#include "ieee11073.h"
#include "agent.h"
}


using namespace std;

#define RC_COUNT 3 //number of retransmissions tries

enum AgentTimers {
	SEND_PACKET = 1,
	TO_ASSOC,
	TO_OPERA
};

class Agent : public VirtualApplication {
	
 private:
	double packet_rate;
	double startupDelay;
	double delayLimit;
	double reading_rate;
	float packet_spacing;
	float data_spacing;
	int dataSN;
	int recipientId;
	int alarmt;
	int last_packet;
	int specialization;
	int total_sec;
	int RC;
	int numNodes;
	bool confirmed_event;
	unsigned int my_plugin_number;
	unsigned int opt;
	unsigned int nodeNumber;
	string recipientAddress;
	string application_name;
	string time_limit;
	void* (*event_report_cb)();
	
	//variables below are used to determine the packet delivery rates.	
	map<long,int> packetsReceived;
	map<long,int> bytesReceived;
	map<long,int> packetsSent;

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleRadioControlMessage(RadioControlMessage *);
	void timerFiredCallback(int);
	void finishSpecific();
	MyPacket* createDataPacket(int seqNum);

 public:
	int getPacketsSent(int addr) { return packetsSent[addr]; }
	int getPacketsReceived(int addr) { return packetsReceived[addr]; }
	int getBytesReceived(int addr) { return bytesReceived[addr]; }
	int getNumberOfNodes() { return getParentModule()->getParentModule()->par("numNodes");}

};

#endif
