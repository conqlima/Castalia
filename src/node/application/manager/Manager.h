#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "VirtualApplication.h"
#include "MyPacket_m.h"

#include "m_plugin_castalia.h"
#include "m_global.h"

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
	SEND_PACKET = 1,
	TO_ASSOC,
	TO_OPERA
};

class Manager : public VirtualApplication {
	
 private:
	double packet_rate;
	double startupDelay;
	double delayLimit;
	float packet_spacing;
	//int dataSN[6];
	//int* dataSN = nullptr;
	int recipientId;
	string recipientAddress;
	//int last_packet[6];
	//int* last_packet = nullptr;
	unsigned int my_plugin_number;
	unsigned int numPlugin;
	int leftToSend[6];
	//int* leftToSend = nullptr;
	int reSend[6];
	//int* reSend = nullptr;
	unsigned int numNodes;
	unsigned int i = 1;


	//variables below are used to determine the packet delivery rates.	
	map<long,int> packetsReceived;
	map<long,int> bytesReceived;
	map<long,int> packetsSent;
	map<long,int> dataSN;
	map<long,int> last_packet;
	map<long,int> RC;


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
	MyPacket* createGenericDataPackett(int seqNum);

};

#endif
