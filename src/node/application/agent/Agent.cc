//extern "C" {
//#include "communication/plugin/plugin.h"
//}
#include "Agent.h"

Define_Module(Agent);

/*this struct represent all the msg exchanged between nodes
*/
Tmsg* st_msg = NULL;
/*this struct contains all information about an agent
*/
CommunicationPlugin* comm_plugin = NULL;
/*var used by Antidote stack library to set 1 if 
 * timeout is required 0 otherwise. 
*/
int* SETTIMER = NULL;
/*var set by user in .ini file to define the hub node.
 * The hub node has to be always the MANAGER*/
int HUBNODE;

void Agent::startup()
{
	HUBNODE = par("hubNode");
	maxNumOfRetransmition = par("maxNumOfRetransmition");
	confirmed_event = par("confirmed_event");
	packet_rate = par("packet_rate");
	packet_rate = par("packet_rate");
	recipientAddress = par("nextRecipient").stringValue();//endereço em string
	recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	application_name = par("application_type").stringValue();
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	dataSN = 0;
	reading_rate = par("reading_rate");// var de controle para enviar dados
	RC = RC_COUNT;// retry count for timeouts
	last_packet = -1;// control duplicate packets
	numOfRetransmissions = 0; //control var for retransmission of data packet
	
	/*the node number*/
	nodeNumber = atoi(SELF_NETWORK_ADDRESS);
	
	/*take sim-time-limit in string format*/
	//time_limit = par("time_limit").stringValue();
	/*convert sim-time-limit to simtime_t format*/
	//simtime_t t = STR_SIMTIME(time_limit.c_str());
	/*convert sim-time-limit to seconds*/
	//total_sec = t.inUnit(SIMTIME_S);
	
	cConfigOption simTimeConfig("sim-time-limit", true, cConfigOption::Type::CFG_DOUBLE, "s", "300", "");
	maxSimTime = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getAsDouble(&simTimeConfig);
	
	maxSimTime = maxSimTime - startupDelay;//startup delay is not part of simulations
	/*calculate the total of events reports (measurements)*/
	alarmt = (int) (maxSimTime*reading_rate);
	/*calculate the time spacing between measurements*/
	data_spacing = reading_rate > 0 ? 1.0 / reading_rate : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão

	/*total number of nodes*/
	numNodes = getNumberOfNodes();
	
	/*creates a Tmsg for each agent*/
	if (st_msg == NULL)
	st_msg = new Tmsg[numNodes];
	
	/*creates a SETTIMER for each agent*/
	if (SETTIMER == NULL)
	SETTIMER = new int[numNodes];
	
	/*codes from sample_agent.c*/
	if (!(strcmp(application_name.c_str(),"pulseoximeter"))) {
			opt = 1;
	} else if (!(strcmp(application_name.c_str(), "bloodpressure"))) {
			opt = 2;
	} else if (!(strcmp(application_name.c_str(), "weightscale"))) {
			opt = 3;
	} else if (!(strcmp(application_name.c_str(), "glucometer"))) {
			opt = 4;
	}else if (!(strcmp(application_name.c_str(), "thermometer"))) {
			opt = 5;
	}else if (!(strcmp(application_name.c_str(), "basicECG"))) {
			opt = 6;
	}else {
		throw cRuntimeError("Invalid application name in node %s", SELF_NETWORK_ADDRESS);
	}
	

	
	SETTIMER[nodeNumber] = 0;
	
	/*creates a CommunicationPlugin for each agent*/
	if (comm_plugin == NULL) {
	comm_plugin = new CommunicationPlugin[numNodes];
	 for (int i = 0; i < numNodes; i++)
	 {
		 comm_plugin[i] = COMMUNICATION_PLUGIN_NULL;
	 }
	}
	
	/*initialize the comm_plugin*/
	comm_plugin[nodeNumber] = communication_plugin();
	
	castalia_mode(nodeNumber);
	
	/*the timeouts fake implementations*/
	comm_plugin[nodeNumber].timer_count_timeout = timer_count_timeout;
	comm_plugin[nodeNumber].timer_reset_timeout = timer_reset_timeout;

	CommunicationPlugin *plugins[] = {&comm_plugin[nodeNumber], 0};

	if (opt == 2) { /* Blood Pressure */
		//fprintf(stderr, "Starting Blood Pressure Agent\n");
		DEBUG("Starting Blood Pressure Agent\n");
		event_report_cb = blood_pressure_event_report_cb;
		specialization = 0x02BC;
		if (confirmed_event)
		event_conf_or_unconf_blood_pressure = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	} else if (opt == 3) { /* Weight Scale */
		//fprintf(stderr, "Starting Weight Scale Agent\n");
		DEBUG("Starting Weight Scale Agent\n");
		event_report_cb = weightscale_event_report_cb;
		specialization = 0x05DC;
		if (confirmed_event)
		event_conf_or_unconf_weighting_scale = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	} else if (opt == 4) { /* Glucometer */
		//fprintf(stderr, "Starting Glucometer Agent\n");
		DEBUG("Starting Glucometer Agent\n");
		event_report_cb = glucometer_event_report_cb;
		specialization = 0x06A4;
		if (confirmed_event)
		event_conf_or_unconf_glucometer = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	} else if (opt == 5) { /* Thermometer */
		//fprintf(stderr, "Starting Thermometer Agent\n");
		DEBUG("Starting Thermometer Agent\n");
		event_report_cb = thermometer_event_report_cb;
		specialization = 0x0320;
		if (confirmed_event)
		event_conf_or_unconf_thermometer = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	}else if (opt == 6) { /* basic ECG */
		//fprintf(stderr, "Starting Basic ECG Agent\n");
		DEBUG("Starting Basic ECG Agent\n");
		event_report_cb = basic_ECG_event_report_cb;
		//specialization = 0x0258;
		specialization = 0x4000;
		if (confirmed_event)
		event_conf_or_unconf_basic_ecg = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	}else { /* Default Pulse Oximeter */
		//fprintf(stderr, "Starting Pulse Oximeter Agent\n");
		DEBUG("Starting Pulse Oximeter Agent\n");
		event_report_cb = oximeter_event_report_cb;
		// change to 0x0191 if you want timestamps
		specialization = 0x0190;
		if (confirmed_event)
		event_conf_or_unconf_pulse_oximeter = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
	}
	
	/*Initializes each plugin*/
	if (nodeNumber == 1) {
		CONTEXT_ID = {1, 0};
		my_plugin_number = CONTEXT_ID.plugin;
	}else {
		CONTEXT_ID = {(nodeNumber*2)-1, 0};
		my_plugin_number = CONTEXT_ID.plugin;
	}
	
	agent_init(CONTEXT_ID, plugins, specialization, event_report_cb, mds_data_cb);

	/*callbacks functions when the agents changes de finite states machine*/
	AgentListener listener = AGENT_LISTENER_EMPTY;
	listener.device_connected = &device_connected;
	listener.device_associated = &device_associated;
	listener.device_unavailable = &device_unavailable;

	agent_add_listener(CONTEXT_ID, listener);
	
	/*clean up the controlPacket and MeasurementPacket
	* map variables from plugin_castalia.cc file*/
	clearVarMap();

	/*agent takes the iniciative to associate*/
	if (data_spacing != -1){
	agent_start(CONTEXT_ID);
	dataSN++;
	setTimer(SEND_PACKET, 0 + startupDelay);
	
	/*number of associations made*/
	isTheFirstAssociation = 0;
	
	/*variables used in finishSpecific*/
	packetsSent.clear();
	packetsReceived.clear();
	bytesReceived.clear();
	
	declareOutput("Packets received per node");
	}else{
	throw cRuntimeError("Invalid reading_rate value: %d", reading_rate);
	}

}

void Agent::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*Converte the packet from ApplicationPacket
	 * to MyPacket*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	
	int sequenceNumber = rcvPacket->getSequenceNumber();
	
	int sourceId = atoi(source);

	if (sequenceNumber != last_packet) {
		
		/*Checks if there is a activated timeout*/
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
		
		/*clear the Tmsg struct to receive a new packet*/
		st_msg[nodeNumber].tam_buff = 0;
		
		//if(st_msg[nodeNumber].buff_msg){
		//free(st_msg[nodeNumber].buff_msg);
		//st_msg[nodeNumber].buff_msg = NULL;
		//}
		
		st_msg[nodeNumber].buff_msgRep.clear();
		
		while(!st_msg[nodeNumber].msgType.empty())
			st_msg[nodeNumber].msgType.pop();

		last_packet = sequenceNumber;

		st_msg[nodeNumber].tam_buff = rcvPacket->getTam_buff();

		//st_msg[nodeNumber].buff_msg = (streamOfByte) malloc(sizeof(uint8_t)*(st_msg[nodeNumber].tam_buff));

		for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
		{
			st_msg[nodeNumber].buff_msgRep.push_back(rcvPacket->getBuff(i));
			//st_msg[nodeNumber].buff_msg[i] = rcvPacket->getBuff(i);
		}
		
		if ((strcmp(source,SELF_NETWORK_ADDRESS)) != 0) {
		
			if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 

				trace() << "Received packet #" << sequenceNumber << " from node " << source;
				collectOutput("Packets received per node", sourceId);//Adds one to the value of output name with index 3.
				packetsReceived[sourceId]++;
				bytesReceived[sourceId] += rcvPacket->getByteLength();

				CONTEXT_ID = {my_plugin_number, 0};
				ctx = context_get_and_lock(CONTEXT_ID);
				
				st_msg[nodeNumber].buff_msgSed.clear();
				/*verifica se no pacote recebido existe dados*/
				if (st_msg[nodeNumber].tam_buff > 0) {
					/*receive all the messages in received packet*/
					while((communication_wait_for_data_input(ctx)) == (NETWORK_ERROR_NONE)) {
						communication_read_input_stream(ctx->id);
						trace() << "type: " << st_msg[nodeNumber].msgType.front();
						st_msg[nodeNumber].msgType.pop();
					}
				}else{
					trace() << "Packet of size 0";
				}
				
				/*Checks if agent can send measurements*/
				if (ctx->fsm->state == fsm_state_operating && alarmt > 0) {
					agent_send_data(CONTEXT_ID);
					dataSN++;
					
					if (getNumberOfAssociationsTotal(nodeNumber) > isTheFirstAssociation){
					setTimer(SEND_PACKET, 0);// the first measurement send after a association
					isTheFirstAssociation = getNumberOfAssociationsTotal(nodeNumber);
					}else
					setTimer(SEND_PACKET, data_spacing);
					
					--alarmt;
				}else if (alarmt == 0) {
						agent_request_association_release(CONTEXT_ID);
						--alarmt;
						dataSN++;
						setTimer(SEND_PACKET, 0);
				}else if (alarmt == -1) {
						agent_disconnect(CONTEXT_ID);
						--alarmt;
				}else{//associantion abort received
				//cancelTimer(SEND_PACKET);
				//cancelTimer(TO_ASSOC);
				//cancelTimer(TO_OPERA);
				if ((ctx->fsm->state == fsm_state_unassociated || ctx->fsm->state == fsm_state_associating)  && alarmt > 0){
				//if ((ctx->fsm->state == fsm_state_unassociated)  && alarmt > 0){
					if (getTimer(SEND_PACKET) != 0)
					alarmt++;
					
					cancelTimer(SEND_PACKET);
					cancelTimer(TO_ASSOC);
					cancelTimer(TO_OPERA);
					agent_request_association_abort(CONTEXT_ID);
					st_msg[nodeNumber].tam_buff = 0;
					st_msg[nodeNumber].buff_msgSed.clear();
					st_msg[nodeNumber].msgType.pop();
					dataSN++;
					service_init(ctx);
					agent_associate(CONTEXT_ID);
					setTimer(SEND_PACKET, 0);
				}
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

void Agent::timerFiredCallback(int index)
{
	Context *ctx;
	CONTEXT_ID = {my_plugin_number, 0};
	ctx = context_get_and_lock(CONTEXT_ID);
	
	switch (index) {
		case SEND_PACKET:{
			trace() << "Sending packet #" << dataSN << " to node " << recipientAddress.c_str();//sequence number
			while(!st_msg[nodeNumber].msgType.empty()){
			trace() << "type: " << st_msg[nodeNumber].msgType.front();
			st_msg[nodeNumber].msgType.pop();
			}
			toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
			packetsSent[recipientId]++;
			fsm_states c = ctx->fsm->state;
			/*set the timeouts*/
			switch(c) {
				case fsm_state_associating:{
					if (RC)
					setTimer(TO_ASSOC, 10);
					break;
				}
				case fsm_state_operating: {
					/*comfirmed event chosen*/
					if (SETTIMER[nodeNumber]) {
						setTimer(TO_OPERA, 3);
						SETTIMER[nodeNumber] = 0;
					}else {//unconfirmed event chosen
							st_msg[nodeNumber].tam_buff = 0;
							st_msg[nodeNumber].buff_msgSed.clear();
						/*Verifica se o agente já pode enviar leituras*/
						if (ctx->fsm->state == fsm_state_operating && alarmt > 0) {
							agent_send_data(CONTEXT_ID);
							--alarmt;
							dataSN++;
							setTimer(SEND_PACKET, data_spacing);
						}else if (alarmt == 0) {
							agent_request_association_release(CONTEXT_ID);
							--alarmt;
							dataSN++;
							setTimer(SEND_PACKET, 0);
						}else if (alarmt == -1) {
							agent_disconnect(CONTEXT_ID);
							--alarmt;
						}
					}
					break;
				}
				default: break;
			}
			break;
		}
		
		case TO_ASSOC:{
			/*request association 3 times, after the 
			 * tries, abort.
			 * */
			if (RC == 0) {
				st_msg[nodeNumber].tam_buff = 0;
				st_msg[nodeNumber].buff_msgSed.clear();
				trace() << "response not received for packet #" << dataSN << " in association mode aborting...";
				dataSN++;
				agent_request_association_abort(CONTEXT_ID);
				while(!st_msg[nodeNumber].msgType.empty()){
					trace() << "type: " << st_msg[nodeNumber].msgType.front();
					st_msg[nodeNumber].msgType.pop();
				}
				toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
				packetsSent[recipientId]++;
			}else {//resend the packet (max of 3 times)
				//fazer a chamada da função aqui
				//trace() << "resending packet #" << dataSN;//sequence number
				//while(!st_msg[nodeNumber].msgType.empty()){
					//trace() << "type: " << st_msg[nodeNumber].msgType.front();
					//st_msg[nodeNumber].msgType.pop();
				//}
				//toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
				//packetsSent[recipientId]++;
				retransmitPacket();
				RC--;
				setTimer(TO_ASSOC, 10);
			}
			break;
		}
		
		case TO_OPERA:{
				trace() << "response not received for packet #" << dataSN << " in operating mode";
				
				/*retransmission of packets in the 
				 * application layer is not 
				 * discussed in 11073 standard, 
				 * this is an independent modification. 
				 * */
				if(numOfRetransmissions < maxNumOfRetransmition){
				retransmitPacket();
				numOfRetransmissions++;
				setTimer(TO_OPERA, 3);
				}else{//ABORT and try a new association
					tryNewAssociation();
				}
				
				/*Uncomment this function call to
				 * try a new association after an ACK
				 * is not received from manager. Read:
				 * 8.4.3 Timeout variables, pg 75, Optimized exchange protoco 11073
				 * */
				//tryNewAssociation();
				
				//st_msg[nodeNumber].tam_buff = 0;
				//st_msg[nodeNumber].buff_msgSed.clear();
				//dataSN++;
				//agent_request_association_abort(CONTEXT_ID);
				//while(!st_msg[nodeNumber].msgType.empty()){
					//trace() << "type: " << st_msg[nodeNumber].msgType.front();
					//st_msg[nodeNumber].msgType.pop();
				//}
				//toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
				//packetsSent[recipientId]++;
				
				//if (alarmt > 0){
				//cancelTimer(SEND_PACKET);
				//cancelTimer(TO_ASSOC);
				//cancelTimer(TO_OPERA);
				//st_msg[nodeNumber].tam_buff = 0;
				//st_msg[nodeNumber].buff_msgSed.clear();
				//dataSN++;
				//service_init(ctx);
				//agent_associate(CONTEXT_ID);
				//setTimer(SEND_PACKET, 0);
				//}
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
	declareOutput("Control Packets");
	declareOutput("Measurement Packets");
	declareOutput("Total of associations");

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
	collectOutput("Control Packets", HUBNODE, "total", getControlPacketsTotal(nodeNumber));
	collectOutput("Measurement Packets", HUBNODE, "total", getMeasurementPacketsTotal(nodeNumber));
	collectOutput("Total of associations", HUBNODE, "total", getNumberOfAssociationsTotal(nodeNumber));

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
	
	if (SETTIMER != NULL) {
	delete[] SETTIMER;
	SETTIMER = NULL;
	}
	
	CONTEXT_ID = {my_plugin_number, 0};
	agent_finalize(CONTEXT_ID);
}

MyPacket* Agent::createDataPacket(int seqNum)
{
	MyPacket *pkt = new MyPacket("mypacket", APPLICATION_PACKET);
	pkt->setBuffArraySize(st_msg[nodeNumber].tam_buff);
	for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
	{
		pkt->setBuff(i,st_msg[nodeNumber].buff_msgSed[i]);
	}
	pkt->setTam_buff(st_msg[nodeNumber].tam_buff);
	pkt->setSequenceNumber(seqNum);
	pkt->setByteLength(pkt->getBuffArraySize() + sizeof(st_msg[nodeNumber].tam_buff));
	return pkt;
}

void Agent::tryNewAssociation(void)
{
	Context *ctx;
	CONTEXT_ID = {my_plugin_number, 0};
	ctx = context_get_and_lock(CONTEXT_ID);
				trace() << "response not received for packet #" << dataSN << " aborting...";
				dataSN++;
				agent_request_association_abort(CONTEXT_ID);
				while(!st_msg[nodeNumber].msgType.empty()){
					trace() << "type: " << st_msg[nodeNumber].msgType.front();
					st_msg[nodeNumber].msgType.pop();
				}
				toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
				packetsSent[recipientId]++;
				
				if (alarmt > 0){
				cancelTimer(SEND_PACKET);
				cancelTimer(TO_ASSOC);
				cancelTimer(TO_OPERA);
				st_msg[nodeNumber].tam_buff = 0;
				st_msg[nodeNumber].buff_msgSed.clear();
				dataSN++;
				trace() << "trying new association";
				service_init(ctx);
				agent_associate(CONTEXT_ID);
				setTimer(SEND_PACKET, 0);
				}
	context_unlock(ctx);
}

void Agent::retransmitPacket(void)
{
	//Context *ctx;
	//CONTEXT_ID = {my_plugin_number, 0};
	//ctx = context_get_and_lock(CONTEXT_ID);
			//if(numOfRetransmissions < maxNumOfRetransmition){
				trace() << "resending packet #" << dataSN;//sequence number
				//while(!st_msg[nodeNumber].msgType.empty()){
					//trace() << "type: " << st_msg[nodeNumber].msgType.front();
					//st_msg[nodeNumber].msgType.pop();
				//}
				toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
				packetsSent[recipientId]++;
				//numOfRetransmissions++;
				
			//}
	//context_unlock(ctx);
}
