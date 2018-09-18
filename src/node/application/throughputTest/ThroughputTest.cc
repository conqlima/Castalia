/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "ThroughputTest.h"

Define_Module(ThroughputTest);

void ThroughputTest::startup()
{
	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();//endereço em string
	recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	dataSN = 0;// o que é isso??
	
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	packetsSent.clear(); //zera as posições do map
	packetsReceived.clear();
	bytesReceived.clear();

	if (packet_spacing > 0 && recipientAddress.compare(SELF_NETWORK_ADDRESS) != 0)//sou um nó transmissor?  Existe packet rate??
		setTimer(SEND_PACKET, packet_spacing + startupDelay);// define o timer para começar enviar pacotes
	else
		trace() << "Not sending packets";

	declareOutput("Packets received per node");
}

void ThroughputTest::fromNetworkLayer(ApplicationPacket * rcvPacket,
		const char *source, double rssi, double lqi)
{
	int sequenceNumber = rcvPacket->getSequenceNumber();
	int sourceId = atoi(source);//numero do nó não é o mesmo que endereço do nó

	// This node is the final recipient for the packet
	if (recipientAddress.compare(SELF_NETWORK_ADDRESS) == 0) {
		//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
		if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 
			trace() << "Received packet #" << sequenceNumber << " from node " << source;
			collectOutput("Packets received per node", sourceId);//Adds one to the value of output name with index 3.
			packetsReceived[sourceId]++;
			bytesReceived[sourceId] += rcvPacket->getByteLength();
		} else {
			trace() << "Packet #" << sequenceNumber << " from node " << source <<
				" exceeded delay limit of " << delayLimit << "s";
		}
	// Packet has to be forwarded to the next hop recipient
	} else {
		ApplicationPacket* fwdPacket = rcvPacket->dup();
		// Reset the size of the packet, otherwise the app overhead will keep adding on
		fwdPacket->setByteLength(0);
		toNetworkLayer(fwdPacket, recipientAddress.c_str());
	}
}

void ThroughputTest::timerFiredCallback(int index)
{
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN;
			toNetworkLayer(createGenericDataPacket(0, dataSN), recipientAddress.c_str());
			packetsSent[recipientId]++;
			dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
			break;
		}
	}
}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void ThroughputTest::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void ThroughputTest::finishSpecific() {
	declareOutput("Packets reception rate");
	declareOutput("Packets loss rate");

	cTopology *topo;	// temp variable to access packets received by other nodes
	topo = new cTopology("topo");// instancia um objeto da classe cTopology e envia como parametro a palavra topo
	//extrai os modulos (nós) que iremos trabalhar
	topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());//Extracts model topology by the fully qualified NED type name of the modules. 

	long bytesDelivered = 0;
	for (int i = 0; i < numNodes; i++) {
		//tenta converter a variável topo (que é do tipo cTopology para uma variável do tipo ThroughputTeste em tempo de execução)
		ThroughputTest *appModule = dynamic_cast<ThroughputTest*>
			(topo->getNode(i)->getModule()->getSubmodule("Application"));
		if (appModule) {
			int packetsSent = appModule->getPacketsSent(self);
			if (packetsSent > 0) { // this node sent us some packets
				float rate = (float)packetsReceived[i]/packetsSent;
				collectOutput("Packets reception rate", i, "total", rate);
				collectOutput("Packets loss rate", i, "total", 1-rate);
			}

			bytesDelivered += appModule->getBytesReceived(self);
		}
	}
	delete(topo);

	if (packet_rate > 0 && bytesDelivered > 0) {
		double energy = (resMgrModule->getSpentEnergy() * 1000000000)/(bytesDelivered * 8);	//in nanojoules/bit
		declareOutput("Energy nJ/bit");
		collectOutput("Energy nJ/bit","",energy);
	}
}
