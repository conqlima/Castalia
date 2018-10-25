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

extern "C" {
#include "communication/plugin/plugin.h"
#include "communication/communication.h"
#include "communication/context_manager.h"
#include "communication/context.h"
#include "ieee11073.h"
#include "agent.h"
}


using namespace std;

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
	float packet_spacing;
	int dataSN;
	int recipientId;
	string recipientAddress;
	int alarmt;
	unsigned int my_plugin_number;
	int last_packet;
	string application_name;
	int specialization;
	unsigned int opt;
	void* (*event_report_cb)();
	unsigned int nodeNumber;
	string time_limit;
	int total_sec;
	double reading_rate;
	float data_spacing;
	int j;
	int RC;
	
	//variables below are used to determine the packet delivery rates.	
	int numNodes;
	map<long,int> packetsReceived;
	map<long,int> bytesReceived;
	map<long,int> packetsSent;

 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void handleRadioControlMessage(RadioControlMessage *);
	void timerFiredCallback(int);
	void finishSpecific();

 public:
	int getPacketsSent(int addr) { return packetsSent[addr]; }
	int getPacketsReceived(int addr) { return packetsReceived[addr]; }
	int getBytesReceived(int addr) { return bytesReceived[addr]; }
	int getNumberOfNodes() { return getParentModule()->getParentModule()->par("numNodes");}
	MyPacket* createGenericDataPackett(int seqNum);

};

#endif
