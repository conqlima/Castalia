#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "VirtualApplication.h"
#include "MyPacket_m.h"

#include "plugin_castalia.h"
#include "global.h"

#include <iostream>
#include <string>
#include <csignal>

extern "C" {
#include "communication/plugin/plugin.h"
#include "communication/communication.h"
#include "communication/context_manager.h"
#include "communication/context.h"
#include "ieee11073.h"
#include "manager.h"
}


using namespace std;

enum ManagerTimers {
	SEND_PACKET = 1
};

class Manager : public VirtualApplication {
	
 private:
	double packet_rate;
	double startupDelay;
	double delayLimit;
	float packet_spacing;
	int dataSN;
	int recipientId;
	string recipientAddress;
	
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
	void setTrace(const char* s);
	MyPacket* createGenericDataPackett(MyPacket* data, unsigned int seqNum);
	void print_buffer(intu8 *buffer, int size);
};

#endif
