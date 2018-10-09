#include "Manager.h"

Define_Module(Manager);

Tmsg m_st_msg[11];

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

	
	CommunicationPlugin *m_comm_plugins[] = {&m_comm_plugin, 0};
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	unsigned int numPlugin = (2*(numNodes-1));
	for (unsigned int i = 1; i <= numPlugin; i++)
	{
		if ( (i % 2) == 0){
		m_CONTEXT_ID = {i,0};
		manager_init(m_CONTEXT_ID, m_comm_plugins);
		
		ManagerListener listener = MANAGER_LISTENER_EMPTY;
		listener.measurement_data_updated = &new_data_received;
		listener.device_available = &device_associated;
		listener.device_unavailable = &m_device_unavailable;
	
		manager_add_listener(m_CONTEXT_ID, listener);
		
		manager_start(m_CONTEXT_ID);
		}
	}
	packetsSent.clear(); //zera as posições do map
	packetsReceived.clear();
	bytesReceived.clear();
}

void Manager::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*coloco a string recebida na estrutura*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	int sequenceNumber = rcvPacket->getSequenceNumber();
	unsigned int sourceId = atoi(source);//numero do nó não é o mesmo que endereço do nó
	recipientAddress = source;
	recipientId = atoi(recipientAddress.c_str());
	my_plugin_number = recipientId*2;
	
	if (sequenceNumber != last_packet[sourceId]) {
		
	//for (int i = 0; i < m_st_msg.tam_buff; i++)
	//{
		//m_st_msg.buff_msg[i] = '\0';
	//}
	//m_st_msg.tam_buff = 0;
	
	last_packet[sourceId] = sequenceNumber;
	
	Tmsg tmp = rcvPacket->getExtraData();
	m_st_msg[my_plugin_number].tam_buff = tmp.tam_buff;
	//m_st_msg[my_plugin_number].recv_str = tmp.send_str;
	
	for (int i = 0; i < m_st_msg[my_plugin_number].tam_buff; i++)
	{
		m_st_msg[my_plugin_number].buff_msg[i] = tmp.buff_msg[i];
	}
	
	if ((strcmp(source,SELF_NETWORK_ADDRESS)) != 0) {
		//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
		if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 
			//trace() << m_st_msg.recv_str;
			trace() << "Received packet #" << sequenceNumber << " from node " << source;
	
			if (m_st_msg[my_plugin_number].tam_buff > 0){
			
			m_CONTEXT_ID = {sourceId*2, 0};
			Context *m_ctx;
			m_ctx = context_get_and_lock(m_CONTEXT_ID);
			
			while((communication_wait_for_data_input(m_ctx)) == NETWORK_ERROR_NONE)
			communication_read_input_stream(m_ctx->id);
			
			context_unlock(m_ctx);
			dataSN++;
			setTimer(SEND_PACKET, packet_spacing);
			}
			
		} else {
			trace() << "Packet #" << sequenceNumber << " from node " << source <<
				" exceeded delay limit of " << delayLimit << "s";
		}
	}else {
		ApplicationPacket* fwdPacket = rcvPacket->dup();
		// Reset the size of the packet, otherwise the app overhead will keep adding on
		fwdPacket->setByteLength(0);
		toNetworkLayer(fwdPacket, recipientAddress.c_str());
	}
}
}

void Manager::timerFiredCallback(int index)
{
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN;//sequence number
			toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			//dataSN++;
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
	//declareOutput("Packets reception rate");
	//declareOutput("Packets loss rate");

	//cTopology *topo;	// temp variable to access packets received by other nodes
	//topo = new cTopology("topo");// instancia um objeto da classe cTopology e envia como parametro a palavra topo
	////extrai os modulos (nós) que iremos trabalhar
	//topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());//Extracts model topology by the fully qualified NED type name of the modules. 

	//long bytesDelivered = 0;
	//for (int i = 0; i < numNodes; i++) {
		////tenta converter a variável topo (que é do tipo cTopology para uma variável do tipo Manager em tempo de execução)
		//Manager *appModule = dynamic_cast<Manager*>
			//(topo->getNode(i)->getModule()->getSubmodule("Application"));
		//if (appModule) {
			//int packetsSent = appModule->getPacketsSent(self);
			//if (packetsSent > 0) { // this node sent us some packets
				//float rate = (float)packetsReceived[i]/packetsSent;
				//collectOutput("Packets reception rate", i, "total", rate);
				//collectOutput("Packets loss rate", i, "total", 1-rate);
			//}

			//bytesDelivered += appModule->getBytesReceived(self);
		//}
	//}
	//delete(topo);

	//if (packet_rate > 0 && bytesDelivered > 0) {
		//double energy = (resMgrModule->getSpentEnergy() * 1000000000)/(bytesDelivered * 8);	//in nanojoules/bit
		//declareOutput("Energy nJ/bit");
		//collectOutput("Energy nJ/bit","",energy);
	//}
	unsigned int numPlugin = (2*(numNodes-1));
	for (unsigned int i = 1; i <= numPlugin; i++)
	{
		if ( (i % 2) == 0){
		m_CONTEXT_ID = {i,0};
		manager_finalize(m_CONTEXT_ID);
		}
	}
	//m_CONTEXT_ID = {2, 0};
	
}

MyPacket* Manager::createGenericDataPackett(unsigned int seqNum)
{
	MyPacket *pktt = new MyPacket("mypacket", APPLICATION_PACKET);
	pktt->setExtraData(m_st_msg[my_plugin_number]);
	pktt->setSequenceNumber(seqNum);
	//Tmsg tmp = pktt->getExtraData();
	//trace() << tmp.send_str;
	//m_st_msg.tam_buff = 0;
	//for (int i = 0; i < 65535; i++)
	//{
	//m_st_msg.buff_msg[i] = '\0';
	//}
	return pktt;
}

