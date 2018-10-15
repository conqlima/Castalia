#include "Thermometer.h"

Define_Module(Thermometer);

Tmsg st_msg[11];

void Thermometer::startup()
{
	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();//endereço em string
	recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	dataSN = 0;// sequence number of data
	alarmt = 3;// var de controle para enviar dados
	last_packet = -1;
	
	/*codes from sample_agent.c*/
	comm_plugin = communication_plugin();
	castalia_mode();
	comm_plugin.timer_count_timeout = timer_count_timeout;
	comm_plugin.timer_reset_timeout = timer_reset_timeout;

	CommunicationPlugin *plugins[] = {&comm_plugin, 0};
	void* (*event_report_cb)();
	//void *event_report_cb;
	int specialization;
	
	/*Defina aqui o agente*/
	event_report_cb = oximeter_event_report_cb;
	// change to 0x0191 if you want timestamps
	specialization = 0x0190;
	
	unsigned int nodeNumber = atoi(SELF_NETWORK_ADDRESS);
	
	/*Faz a inicialização do plugin*/
	if (nodeNumber == 1) {
		CONTEXT_ID = {1, 0};
		my_plugin_number = CONTEXT_ID.plugin;
	}else {
		CONTEXT_ID = {(nodeNumber*2)-1, 0};
		my_plugin_number = CONTEXT_ID.plugin;
	}
	agent_init(CONTEXT_ID, plugins, specialization, event_report_cb, mds_data_cb);

	AgentListener listener = AGENT_LISTENER_EMPTY;
	listener.device_connected = &device_connected;
	listener.device_associated = &device_associated;
	listener.device_unavailable = &device_unavailable;

	agent_add_listener(CONTEXT_ID, listener);
	
	agent_start(CONTEXT_ID);
	dataSN++;
	setTimer(SEND_PACKET, packet_spacing + startupDelay);// define o timer para começar enviar pacotes
	
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	
	/*Variáveis utilizadas na função FinishEspefic()*/
	packetsSent.clear(); //zera as posições do map
	packetsReceived.clear();
	bytesReceived.clear();
	
	declareOutput("Packets received per node");
}

void Thermometer::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*Converte o pacote recebido do formato ApplicationPacket
	 * para MyPacket*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	
	/*Pega o número de sequencia do pacote recebido*/
	int sequenceNumber = rcvPacket->getSequenceNumber();
	
	int sourceId = atoi(source);
	
	/*Limpa a variável st_msg antes de fazer qualquer coisa*/
	for (int i = 0; i < st_msg[my_plugin_number].tam_buff; i++)
	{
		st_msg[my_plugin_number].buff_msg[i] = '\0';
	}
	
	st_msg[my_plugin_number].tam_buff = 0;
	
	
	if (sequenceNumber != last_packet) {
		
		/*Verifica se é um pacote duplicado*/
		last_packet = sequenceNumber;
		
		/*Recebe o pacote numa estrutura temporária*/
		Tmsg tmp = rcvPacket->getExtraData();
		st_msg[my_plugin_number].tam_buff = tmp.tam_buff;
		
		/*Copia informação do pacote recebido para m_st_msg*/
		for (int i = 0; i < st_msg[my_plugin_number].tam_buff; i++)
		{
			st_msg[my_plugin_number].buff_msg[i] = tmp.buff_msg[i];
		}
		
		if ((strcmp(source,SELF_NETWORK_ADDRESS)) != 0) {
		
			//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
			if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 

				trace() << "Received packet #" << sequenceNumber << " from node " << source;
				collectOutput("Packets received per node", sourceId);//Adds one to the value of output name with index 3.
				packetsReceived[sourceId]++;
				bytesReceived[sourceId] += rcvPacket->getByteLength();

				Context *ctx;
				CONTEXT_ID = {my_plugin_number, 0};
				ctx = context_get_and_lock(CONTEXT_ID);
				
				/*verifica se no pacote recebido existe dados*/
				if (st_msg[my_plugin_number].tam_buff > 0) {
					/*ler todos os dados do buffer*/
					while((communication_wait_for_data_input(ctx)) == (NETWORK_ERROR_NONE))
						communication_read_input_stream(ctx->id);
				}
				
				/*Verifica se o agente já pode enviar leituras*/
				if (ctx->fsm->state == fsm_state_operating && alarmt > 1) {
					agent_send_data(CONTEXT_ID);
					--alarmt;
					dataSN++;
					setTimer(SEND_PACKET, packet_spacing);
				}else if (alarmt == 1) {
						agent_request_association_release(CONTEXT_ID);
						--alarmt;
						dataSN++;
						setTimer(SEND_PACKET, packet_spacing);
				}else if (alarmt == 0) {
						agent_disconnect(CONTEXT_ID);
						--alarmt;
				}
				
				context_unlock(ctx);
				
			}else {
				trace() << "Packet #" << sequenceNumber << " from node " << source <<
				" exceeded delay limit of " << delayLimit << "s";
			}

		}else {
			ApplicationPacket* fwdPacket = rcvPacket->dup();
			// Reset the size of the packet, otherwise the app overhead will keep adding on
			fwdPacket->setByteLength(0);
			toNetworkLayer(fwdPacket, "0");
			dataSN++;
		}
	}
}

void Thermometer::timerFiredCallback(int index)
{
	switch (index) {
		case SEND_PACKET:{
			//trace() << "Sending packet #" << dataSN;//sequence number
			toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			packetsSent[recipientId]++;
			setTimer(SEND_PACKET, packet_spacing);
			break;
		}
	}

}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void Thermometer::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void Thermometer::finishSpecific() {
	declareOutput("Packets reception rate");
	declareOutput("Packets loss rate");

	cTopology *topo;	// temp variable to access packets received by other nodes
	topo = new cTopology("topo");// instancia um objeto da classe cTopology e envia como parametro a palavra topo
	//extrai os modulos (nós) que iremos trabalhar
	topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());//Extracts model topology by the fully qualified NED type name of the modules. 

	long bytesDelivered = 0;
	for (int i = 0; i < numNodes; i++) {
		//tenta converter a variável topo (que é do tipo cTopology para uma variável do tipo Thermometer em tempo de execução)
		Thermometer *appModule = dynamic_cast<Thermometer*>
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

MyPacket* Thermometer::createGenericDataPackett(int seqNum)
{
	MyPacket *pktt = new MyPacket("mypacket", APPLICATION_PACKET);
	pktt->setExtraData(st_msg[my_plugin_number]);
	pktt->setSequenceNumber(seqNum);
	return pktt;
}

