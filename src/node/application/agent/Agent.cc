#include "Agent.h"

Define_Module(Agent);

//const int tam = getNumberOfNodes();

Tmsg* st_msg = NULL;
CommunicationPlugin* comm_plugin = NULL;
int SETTIMER = 0;

void Agent::startup()
{
	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();//endereço em string
	recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	application_name = par("application_type").stringValue();
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	dataSN = 0;// sequence number of data
	reading_rate = par("reading_rate");// var de controle para enviar dados
	//int time_in_sec = simTime().inUnit(SIMTIME_S);
	//alarmt = (int) alarmt*time_in_sec;
	j = 0;
	RC = 3;
	SETTIMER = 0;
	time_limit = par("time_limit").stringValue();
	simtime_t t = STR_SIMTIME(time_limit.c_str());
	total_sec = t.inUnit(SIMTIME_S);
	alarmt = (int) (total_sec*reading_rate);
	data_spacing = reading_rate > 0 ? 1.0 / reading_rate : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	last_packet = -1;

	numNodes = getParentModule()->getParentModule()->par("numNodes");
	if (st_msg == NULL)
	st_msg = new Tmsg[numNodes];
	
	/*codes from sample_agent.c*/
	//unsigned int opt;
	if (!(strcmp(application_name.c_str(),"pulseoximeter"))) {
			opt = 1;
	} else if (!(strcmp(application_name.c_str(), "bloodpressure"))) {
			opt = 2;
	} else if (!(strcmp(application_name.c_str(), "weightscale"))) {
			opt = 3;
	} else if (!(strcmp(application_name.c_str(), "glucometer"))) {
			opt = 4;
	} else {
		throw cRuntimeError("Invalid application name in node %s", SELF_NETWORK_ADDRESS);
	}
	
	nodeNumber = atoi(SELF_NETWORK_ADDRESS);
	
	if (comm_plugin == NULL) {
	comm_plugin = new CommunicationPlugin[numNodes];
	 comm_plugin[0] = COMMUNICATION_PLUGIN_NULL;
	 comm_plugin[1] = COMMUNICATION_PLUGIN_NULL;
	 comm_plugin[2] = COMMUNICATION_PLUGIN_NULL;
	 comm_plugin[3] = COMMUNICATION_PLUGIN_NULL;
	 comm_plugin[4] = COMMUNICATION_PLUGIN_NULL;
	 comm_plugin[5] = COMMUNICATION_PLUGIN_NULL;
	}
	
	comm_plugin[nodeNumber] = communication_plugin();
	castalia_mode(nodeNumber);
	comm_plugin[nodeNumber].timer_count_timeout = timer_count_timeout;
	comm_plugin[nodeNumber].timer_reset_timeout = timer_reset_timeout;

	CommunicationPlugin *plugins[] = {&comm_plugin[nodeNumber], 0};
	//void* (*event_report_cb)();
	//void *event_report_cb;
	//int specialization;
	
	if (opt == 2) { /* Blood Pressure */
		fprintf(stderr, "Starting Blood Pressure Agent\n");
		event_report_cb = blood_pressure_event_report_cb;
		specialization = 0x02BC;
	} else if (opt == 3) { /* Weight Scale */
		fprintf(stderr, "Starting Weight Scale Agent\n");
		event_report_cb = weightscale_event_report_cb;
		specialization = 0x05DC;
	} else if (opt == 4) { /* Glucometer */
		fprintf(stderr, "Starting Glucometer Agent\n");
		event_report_cb = glucometer_event_report_cb;
		specialization = 0x06A4;
	} else { /* Default Pulse Oximeter */
		fprintf(stderr, "Starting Pulse Oximeter Agent\n");
		event_report_cb = oximeter_event_report_cb;
		// change to 0x0191 if you want timestamps
		specialization = 0x0190;
	}
	
	/*Defina aqui o agente*/
	//event_report_cb = oximeter_event_report_cb;
	// change to 0x0191 if you want timestamps
	//specialization = 0x0190;
	
	
	
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
	setTimer(SEND_PACKET, 0 + startupDelay);// define o timer para começar enviar pacotes
	
	/*Variáveis utilizadas na função FinishEspefic()*/
	packetsSent.clear(); //zera as posições do map
	packetsReceived.clear();
	bytesReceived.clear();
	
	declareOutput("Packets received per node");
}

void Agent::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*Converte o pacote recebido do formato ApplicationPacket
	 * para MyPacket*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	
	/*Pega o número de sequencia do pacote recebido*/
	int sequenceNumber = rcvPacket->getSequenceNumber();
	
	int sourceId = atoi(source);

	if (sequenceNumber != last_packet) {
		
		Context *ctx;
		CONTEXT_ID = {my_plugin_number, 0};
		ctx = context_get_and_lock(CONTEXT_ID);
		fsm_states c = ctx->fsm->state;
		switch(c) {
				case fsm_state_associating:{
					if (getTimer(TO_ASSOC) != 0)
					cancelTimer(TO_ASSOC);
					break;
				}
				
				case fsm_state_operating:{
					if (getTimer(TO_OPERA) != 0)
					cancelTimer(TO_OPERA);
					break;
				}
				default: break;
		}
		context_unlock(ctx);
		
		/*Limpa a variável st_msg antes de fazer qualquer coisa*/
		for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
		{
			st_msg[nodeNumber].buff_msg[i] = '\0';
		}
	
		st_msg[nodeNumber].tam_buff = 0;
		
		//cancelTimer(SEND_DATA);
		/*Verifica se é um pacote duplicado*/
		last_packet = sequenceNumber;
		
		/*Recebe o pacote numa estrutura temporária*/
		Tmsg tmp = rcvPacket->getExtraData();
		st_msg[nodeNumber].tam_buff = tmp.tam_buff;
		
		/*Copia informação do pacote recebido para st_msg*/
		for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
		{
			st_msg[nodeNumber].buff_msg[i] = tmp.buff_msg[i];
		}
		
		st_msg[nodeNumber].send_str = tmp.send_str;
		
		if ((strcmp(source,SELF_NETWORK_ADDRESS)) != 0) {
		
			//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
			if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 

				trace() << "Received packet #" << sequenceNumber << " from node " << source << " type: " << st_msg[nodeNumber].send_str.c_str();
				collectOutput("Packets received per node", sourceId);//Adds one to the value of output name with index 3.
				packetsReceived[sourceId]++;
				bytesReceived[sourceId] += rcvPacket->getByteLength();

				//Context *ctx;
				CONTEXT_ID = {my_plugin_number, 0};
				ctx = context_get_and_lock(CONTEXT_ID);
				
				/*verifica se no pacote recebido existe dados*/
				if (st_msg[nodeNumber].tam_buff > 0) {
					/*ler todos os dados do buffer*/
					while((communication_wait_for_data_input(ctx)) == (NETWORK_ERROR_NONE)) {
						
						communication_read_input_stream(ctx->id);
						trace() << "type: " << st_msg[nodeNumber].send_str.c_str();
					}
				}
				
				/*Verifica se o agente já pode enviar leituras*/
				if (ctx->fsm->state == fsm_state_operating && alarmt > 0) {
					agent_send_data(CONTEXT_ID);

					dataSN++;
					j = 0;
					if (alarmt == (int)((total_sec*reading_rate)))
					setTimer(SEND_PACKET, 0);
					else
					setTimer(SEND_PACKET, data_spacing);
					--alarmt;
				}else if (alarmt == 0) {
						agent_request_association_release(CONTEXT_ID);
						--alarmt;
						dataSN++;
						j = 0;
						setTimer(SEND_PACKET, 0);
				}else if (alarmt == -1) {
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

//void Agent::timerFiredCallback(int index)
//{
	//switch (index) {
		//case SEND_PACKET:{
			////trace() << "Sending packet #" << dataSN;//sequence number
			//toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			//packetsSent[recipientId]++;
			//setTimer(SEND_DATA, 0);
			//break;
		//}
		
		//case SEND_DATA:{
			////trace() << "Sending packet #" << dataSN;//sequence number
			////toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			////packetsSent[recipientId]++;
			
			////for(; j < 6; j++){
				//if (j < 4){
				//setTimer(SEND_PACKET, packet_spacing);
				//j++;
				//}
			////}
			
			//break;
		//}
	//}

//}


void Agent::timerFiredCallback(int index)
{
	Context *ctx;
	CONTEXT_ID = {my_plugin_number, 0};
	ctx = context_get_and_lock(CONTEXT_ID);
	
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN << " to node " << recipientAddress.c_str();//sequence number
			toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			packetsSent[recipientId]++;
			fsm_states c = ctx->fsm->state;
			switch(c) {
				case fsm_state_associating:{
					if (RC)
					setTimer(TO_ASSOC, 10);
					break;
				}
				case fsm_state_operating: {
					if (SETTIMER) {
						setTimer(TO_OPERA, 3);
						SETTIMER = 0;
					}
					break;
				}
				default: break;
			}

			break;
		}
		
		case TO_ASSOC:{
			if (RC == 0) {
				trace() << "response not received for packet #" << dataSN << " in association mode aborting...";
				agent_request_association_abort(CONTEXT_ID);
				toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			}else {
				trace() << "resending packet #" << dataSN;//sequence number
				toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
				packetsSent[recipientId]++;
				setTimer(TO_ASSOC, 10);
				RC--;
			}
			break;
		}
		
		case TO_OPERA:{
				trace() << "response not received for packet #" << dataSN << " in operating mode, aborting...";//sequence number
				agent_request_association_abort(CONTEXT_ID);
				toNetworkLayer(createGenericDataPackett(dataSN), recipientAddress.c_str());
			break;
		}
	}
	context_unlock(ctx);
}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void Agent::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
                        break;
	}
}

void Agent::finishSpecific() {
	declareOutput("Packets reception rate");
	declareOutput("Packets loss rate");

	cTopology *topo;	// temp variable to access packets received by other nodes
	topo = new cTopology("topo");// instancia um objeto da classe cTopology e envia como parametro a palavra topo
	//extrai os modulos (nós) que iremos trabalhar
	topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());//Extracts model topology by the fully qualified NED type name of the modules. 

	long bytesDelivered = 0;
	for (int i = 0; i < numNodes; i++) {
		//tenta converter a variável topo (que é do tipo cTopology para uma variável do tipo Agent em tempo de execução)
		Agent *appModule = dynamic_cast<Agent*>
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
	if (st_msg != NULL) {
	delete[] st_msg;
	st_msg = NULL;
	}
	
	if (comm_plugin != NULL) {
	delete[] comm_plugin;
	comm_plugin = NULL;
	}
}

MyPacket* Agent::createGenericDataPackett(int seqNum)
{
	MyPacket *pktt = new MyPacket("mypacket", APPLICATION_PACKET);
	pktt->setExtraData(st_msg[nodeNumber]);
	pktt->setSequenceNumber(seqNum);
	return pktt;
}



