#include "Manager.h"

Define_Module(Manager);

Tmsg m_st_msg;

void Manager::startup()
{

	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();//endereço em string
	recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	dataSN = 0;// sequence number of data
	
	/*codes from sample_manager.c*/
	m_comm_plugin = communication_plugin();
	m_castalia_mode();
	
	fprintf(stderr, "\nIEEE 11073 Sample application\n");

	m_comm_plugin.timer_count_timeout = m_timer_count_timeout;
	m_comm_plugin.timer_reset_timeout = m_timer_reset_timeout;

	
	CommunicationPlugin *comm_plugins[] = {&m_comm_plugin, 0};
	manager_init(comm_plugins);
	
	ManagerListener listener = MANAGER_LISTENER_EMPTY;
	listener.measurement_data_updated = &new_data_received;
	listener.device_available = &device_associated;

	manager_add_listener(listener);
	
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	packetsSent.clear(); //zera as posições do map
	packetsReceived.clear();
	bytesReceived.clear();
	
	manager_start();

	declareOutput("Packets received per node");
}

void Manager::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*coloco a string recebida na estrutura*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	int sequenceNumber = rcvPacket->getSequenceNumber();
	int sourceId = atoi(source);//numero do nó não é o mesmo que endereço do nó
	
	Tmsg tmp = rcvPacket->getExtraData();
	m_st_msg.recv_str = tmp.send_str;
	
	// This node is the final recipient for the packet
	if (recipientAddress.compare(SELF_NETWORK_ADDRESS) == 0) {
		//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
		if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 
			trace() << m_st_msg.recv_str;
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

void Manager::timerFiredCallback(int index)
{
	MyPacket *pktt = new MyPacket("mypacketrob", APPLICATION_PACKET);
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN;//sequence number
			/*aqui que eu tenho que mudar para chamar as funções*/
			//toNetworkLayer(createGenericDataPacket(0, dataSN), recipientAddress.c_str());

			pktt->setExtraData(m_st_msg);
			pktt->setSequenceNumber(dataSN);
			Tmsg tmp = pktt->getExtraData();
			trace() << tmp.send_str;
			toNetworkLayer(pktt, recipientAddress.c_str());
			packetsSent[recipientId]++;
			dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
			break;
		}
	}

}


// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void Manager::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			//trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void Manager::finishSpecific() {
	declareOutput("Packets reception rate");
	declareOutput("Packets loss rate");

	cTopology *topo;	// temp variable to access packets received by other nodes
	topo = new cTopology("topo");// instancia um objeto da classe cTopology e envia como parametro a palavra topo
	//extrai os modulos (nós) que iremos trabalhar
	topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());//Extracts model topology by the fully qualified NED type name of the modules. 

	long bytesDelivered = 0;
	for (int i = 0; i < numNodes; i++) {
		//tenta converter a variável topo (que é do tipo cTopology para uma variável do tipo Manager em tempo de execução)
		Manager *appModule = dynamic_cast<Manager*>
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
	//manager_finalize();
	//MyPacket *pktt = new MyPacket("mypacketrob", APPLICATION_PACKET);
	//pktt->setExtraData(m_st_msg);
	//pktt->setSequenceNumber(dataSN);
	//Tmsg tmp = pktt->getExtraData();
	//trace() << tmp.send_str;
	//toNetworkLayer(pktt, recipientAddress.c_str());
}

MyPacket* Manager::createGenericDataPackett(MyPacket* data, unsigned int seqNum)
{
	MyPacket *newPacket = new MyPacket("app with arqu packet", APPLICATION_PACKET);
	newPacket->setExtraData(m_st_msg);
	newPacket->setSequenceNumber(seqNum);
	//if (size > 0) newPacket->setByteLength(size);
	return newPacket;
}

void Manager::print_buffer(intu8 *buffer, int size)
{
	char  *str = (char*)calloc(size*4, sizeof(char));

	int i;

	for (i = 0; i < size; i++) {
		sprintf(str, "%s%.2X ", str, buffer[i]);
	}

	//DEBUG("%s", str);
	trace() << str;

	//fflush(stdout);
	free(str);
	//str = NULL;
}

