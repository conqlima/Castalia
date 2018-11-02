#include "Manager.h"

Define_Module(Manager);

Tmsg* m_st_msg = NULL;
CommunicationPlugin* m_comm_plugin;
int m_SETTIMER[6] = {0};

void Manager::startup()
{
	packet_rate = par("packet_rate");
	//recipientAddress = par("nextRecipient").stringValue();//endereço em string
	//recipientId = atoi(recipientAddress.c_str());// endereço em inteiro
	startupDelay = par("startupDelay");
	delayLimit = par("delayLimit");
	packet_spacing = packet_rate > 0 ? 1 / float (packet_rate) : -1;//divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão
	numNodes = getParentModule()->getParentModule()->par("numNodes");
	//m_SETTIMER = 0;
	//dataSN = new int[numNodes];
	//last_packet = new int[numNodes];
	//leftToSend = new int[numNodes];
	//reSend = new int[numNodes];

	leftToSend[0] = reSend[0] = 0;
	leftToSend[1] = reSend[1] = 0;
	leftToSend[2] = reSend[2] = 0;
	leftToSend[3] = reSend[3] = 0;
	leftToSend[4] = reSend[4] = 0;
	leftToSend[5] = reSend[5] = 0;
	/*codes from sample_manager.c*/
	//m_comm_plugin = communication_plugin();
	//m_castalia_mode();
	
	fprintf(stderr, "\nIEEE 11073 Sample application\n");

	//m_comm_plugin.timer_count_timeout = m_timer_count_timeout;
	//m_comm_plugin.timer_reset_timeout = m_timer_reset_timeout;

	//CommunicationPlugin *m_comm_plugins[] = {&m_comm_plugin, 0};
	
	/*obtem o número total de nós*/
	if (m_st_msg == NULL)
	m_st_msg = new Tmsg[numNodes];
	
	m_comm_plugin = new CommunicationPlugin[numNodes];
	 m_comm_plugin[0] = COMMUNICATION_PLUGIN_NULL;
	 m_comm_plugin[1] = COMMUNICATION_PLUGIN_NULL;
	 m_comm_plugin[2] = COMMUNICATION_PLUGIN_NULL;
	 m_comm_plugin[3] = COMMUNICATION_PLUGIN_NULL;
	 m_comm_plugin[4] = COMMUNICATION_PLUGIN_NULL;
	 m_comm_plugin[5] = COMMUNICATION_PLUGIN_NULL;

	/*pego o número total de plugins que a aplicação vai ter*/
	numPlugin = (2*(numNodes-1));
	
	ManagerListener listener[6] = {MANAGER_LISTENER_EMPTY};

	/*Faz a inicialização de todos os plugins pares, isto é,
	 * o agente somente utiliza plugins de numeros pares ini-
	 * ciando no número 2.*/
	for (unsigned int i = 2; i <= numPlugin; i += 2){
		
		if ( (i % 2) == 0){
			//pego o número do nó baseado no número do plugin
			unsigned int nodeId = i/2;
			m_comm_plugin[nodeId] = communication_plugin();
			m_castalia_mode(i);
	
			fprintf(stderr, "\nIEEE 11073 Sample application\n");

			m_comm_plugin[nodeId].timer_count_timeout = m_timer_count_timeout;
			m_comm_plugin[nodeId].timer_reset_timeout = m_timer_reset_timeout;

			CommunicationPlugin *m_comm_plugins[] = {&m_comm_plugin[nodeId], 0};

			m_CONTEXT_ID = {i,0};
			manager_init(m_CONTEXT_ID, m_comm_plugins);
			
			//ManagerListener listener = MANAGER_LISTENER_EMPTY;
			listener[nodeId].measurement_data_updated = &new_data_received;
			listener[nodeId].device_available = &device_associated;
			listener[nodeId].device_unavailable = &m_device_unavailable;
		
			manager_add_listener(m_CONTEXT_ID, listener[nodeId]);
			
			manager_start(m_CONTEXT_ID);
		}
	}
	
	/*Variáveis utilizadas na função FinishEspefic()*/
	packetsSent.clear();
	packetsReceived.clear();
	bytesReceived.clear();
	dataSN.clear();
	last_packet.clear();
	RC[0] = 3;
	RC[1] = 3;
	RC[2] = 3;
	RC[3] = 3;
	RC[4] = 3;
	RC[5] = 3;
	declareOutput("Packets received per node");
}

void Manager::fromNetworkLayer(ApplicationPacket * rcvPacketa,
		const char *source, double rssi, double lqi)
{
	/*Converte o pacote recebido do formato ApplicationPacket
	 * para MyPacket*/
	MyPacket * rcvPacket = check_and_cast<MyPacket*>(rcvPacketa);
	
	/*Pega o número de sequencia do pacote recebido*/
	int sequenceNumber = rcvPacket->getSequenceNumber();
	
	/*Pega o número do nó que recebemos o pacote e converte
	 * para inteiro*/
	unsigned int sourceId = atoi(source);//numero do nó não é o mesmo que endereço do nó

	//número do nó no formato string
	recipientAddress = source;
	
	if (sequenceNumber != last_packet[sourceId]) {
		reSend[sourceId] = 0;
		recipientId = sourceId;
		Context *ctx;
		my_plugin_number = sourceId*2;
		m_CONTEXT_ID = {my_plugin_number, 0};
		ctx = context_get_and_lock(m_CONTEXT_ID);
		fsm_states c = ctx->fsm->state;
		switch(c) {
				case fsm_state_operating:{
					if (getTimer(sourceId) != 0) {
						leftToSend[sourceId] = 0;
						//operatingTimers.pop();
						cancelTimer(sourceId);
					}
					break;
				}
				default: break;
		}
		context_unlock(ctx);
		//número do plugin corrente
		//my_plugin_number = sourceId*2;
		
		//cancelTimer(SEND_PACKET);
		/*Verifica se é um pacote duplicado*/
		last_packet[sourceId] = sequenceNumber;
		
		/*Recebe o pacote numa estrutura temporária*/
		//Tmsg tmp = rcvPacket->getExtraData();
		//m_st_msg[sourceId].tam_buff = tmp.tam_buff;
		m_st_msg[sourceId] = rcvPacket->getExtraData();
		///*Copia informação do pacote recebido para m_st_msg*/
		//for (int i = 0; i < m_st_msg[sourceId].tam_buff; i++)
		//{
			//m_st_msg[sourceId].buff_msg[i] = tmp.buff_msg[i];
		//}
		
		//m_st_msg[sourceId].send_str = tmp.send_str;
		//m_st_msg[sourceId].recv_str = tmp.recv_str;
		
		if ((strcmp(source,SELF_NETWORK_ADDRESS)) != 0) {
			
			//delaylimit == 0 (sem limite) ou o tempo atual da simulação menos a hora q o pacote foi criado
			if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit) { 
				
				trace() << "Received packet #" << sequenceNumber << " from node " << source;
				collectOutput("Packets received per node", sourceId);//Adds one to the value of output name with index 3.
				packetsReceived[sourceId]++;
				bytesReceived[sourceId] += rcvPacket->getByteLength();
				
				/*verifica se no pacote recebido existe dados*/
				if (m_st_msg[sourceId].tam_buff > 0) {
				
					m_CONTEXT_ID = {my_plugin_number, 0};
					Context *m_ctx;
					m_ctx = context_get_and_lock(m_CONTEXT_ID);
					
					/*ler todos os dados do buffer*/
					while((communication_wait_for_data_input(m_ctx)) == NETWORK_ERROR_NONE) {
						communication_read_input_stream(m_ctx->id);
						//trace() << "type: " << m_st_msg[sourceId].recv_str.c_str();
						//while(!m_st_msg[nodeNumber].fila.empty()){
							trace() << "type: " << m_st_msg[sourceId].fila.front();
							m_st_msg[sourceId].fila.pop();
						//}
					}
					context_unlock(m_ctx);
					//my_plugin_number = sourceId*2;

					
					leftToSend[sourceId] = 1;
					//i = 1;
					//for(unsigned int i = 1; i < numNodes; i++){
					//if (reSend[i]){
						//leftToSend[i] = 1;
						////setTimer(SEND_PACKET, packet_spacing);
						//}
					//}
					//if (i == 6)
					//i = 1;
					if (m_SETTIMER[sourceId]) {
					setTimer(sourceId, 3);
					m_SETTIMER[sourceId] = 0;
					}
					
					if (m_st_msg[sourceId].tam_buff > 0) {
						dataSN[sourceId]++;
						trace() << "Sending packet #" << dataSN[sourceId] << " to node " << sourceId;//sequence number
						toNetworkLayer(createGenericDataPackett(dataSN[sourceId]), source);
						//trace() << "type: " << m_st_msg[sourceId].send_str.c_str();
						while(!m_st_msg[sourceId].fila.empty()){
							trace() << "type: " << m_st_msg[sourceId].fila.front();
							m_st_msg[sourceId].fila.pop();
						}
						packetsSent[recipientId]++;
					}
				}
			}else {
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
	Context *ctx;
	my_plugin_number = index*2;
	m_CONTEXT_ID = {my_plugin_number, 0};
	ctx = context_get_and_lock(m_CONTEXT_ID);
	leftToSend[index] = 0;
	
	fsm_states c = ctx->fsm->state;
	switch(c){
		case fsm_state_operating:{
			trace() << "response not received for packet #" << dataSN[index] << " from node " << index << " in operating mode ";//sequence number
			break;
		}
		case fsm_state_associating:{
			trace() << "response not received for packet #" << dataSN[index] << " from node " << index << " in associating mode ";//sequence number
			break;
		}
		default: break;
	}
	
	for (int i = 0; i < m_st_msg[index].tam_buff; i++)
	{
		m_st_msg[index].buff_msg[i] = '\0';
	}
	
	m_st_msg[index].tam_buff = 0;
	
	manager_request_association_abort(m_CONTEXT_ID);
	const char* recipient = std::to_string(index).c_str();
	dataSN[index]++;
	trace() << "Sending packet #" << dataSN[index] << " to node " << index;//sequence number
	toNetworkLayer(createGenericDataPackett(dataSN[index]), recipient);
	//trace() << "type: " << m_st_msg[index].send_str.c_str();
	while(!m_st_msg[index].fila.empty()){
		trace() << "type: " << m_st_msg[index].fila.front();
		m_st_msg[index].fila.pop();
	}
	packetsSent[recipientId]++;
	context_unlock(ctx);
	//switch (index) {
		//case SEND_PACKET:{
			//Context *ctx;
		//for (unsigned int i = 1; i < numNodes; i++) {
			//if (reSend[i]) {
					//my_plugin_number = i*2;
					//m_CONTEXT_ID = {my_plugin_number, 0};
					//ctx = context_get_and_lock(m_CONTEXT_ID);
					//trace() << "Sending packet #" << dataSN[i];//sequence number
					//const char* recipient = std::to_string(i).c_str();
					//toNetworkLayer(createGenericDataPackett(dataSN[i]), recipient);
					//reSend[i] = 0;
					//packetsSent[recipientId]++;
					//fsm_states c = ctx->fsm->state;
					//switch(c) {
						//case fsm_state_operating:{
							////node has timeout
							//leftToSend[i] = 1;
							////operatingTimers.push_back(i);
							////node id
							//setTimer(i, 3);
							//break;
						//}
						//default: break;
					//}
			//}
		//}
			//context_unlock(ctx);
			//break;
		//}
		
		////case TO_ASSOC:{
			//////opção inválida, concertar
			////if (RC[0] == 0)
				////manager_request_association_abort(m_CONTEXT_ID);
			////else {
				////trace() << "resending packet #" << dataSN[i];//sequence number
				////const char* recipient = std::to_string(i).c_str();
				////toNetworkLayer(createGenericDataPackett(dataSN[i]), recipient);
				////packetsSent[recipientId]++;
				////setTimer(TO_ASSOC, 10);
				////RC[i]--;
			////}
			////break;
		////}
		
		////case TO_OPERA:{
				////int node = operatingTimers.front();
				////leftToSend[node] = 0;
				////trace() << "response not received for packet #" << dataSN[i] << " from node " << i << " aborting...";//sequence number
				////m_CONTEXT_ID = {node*2, 0};
				////manager_request_association_abort(m_CONTEXT_ID);
				////const char* recipient = std::to_string(node).c_str();
				////toNetworkLayer(createGenericDataPackett(dataSN[node]), recipient);
				////operatingTimers.pop_front();
			////break;
		////}
		//default:{
			//leftToSend[index] = 0;
			//trace() << "response not received for packet #" << dataSN[index] << " from node " << index << " aborting...";//sequence number
			//manager_request_association_abort(m_CONTEXT_ID);
			////const char* recipient = std::to_string(index).c_str();
			////toNetworkLayer(createGenericDataPackett(dataSN[index]), recipient);
			//break;
		//}
	//}
	
}

//void Manager::timerFiredCallback(int index)
//{
	////static int i = 0;
	//switch (index) {
		//case SEND_AUX:{
				///*Como em toda a aplicação, este laço considera que
				//*o nó "hub" ou "sink" sempre será o nó com o endereço zero*/
				//for(unsigned int i = 1; i < numNodes; i++){
						//if (leftToSend[i] == 1){
						//my_plugin_number = i;
						//const char* c = std::to_string(i).c_str();
						////toNetworkLayer(createGenericDataPackett(dataSN[i]), c);
						//packetsSent[recipientId]++;
						////setTimer(SEND_PACKET, 0);
						////break;
						//}
				//}
		//break;
		//}
		
		//case SEND_PACKET:{
				////trace() << "Sending packet #" << dataSN[my_plugin_number];
				///*Como em toda a aplicação, este laço considera que
				//*o nó "hub" ou "sink" sempre será o nó com o endereço zero*/
				//for(unsigned int i = 1; i < numNodes; i++){
					////if (leftToSend[i] > 0){
						////my_plugin_number = i*2;
					//while(leftToSend[i] > 1){
						////my_plugin_number = i*2;
						////const char* c = std::to_string(i).c_str();
						////toNetworkLayer(createGenericDataPackett(dataSN[my_plugin_number]), c);
						////if ( i != 6 && leftToSend[i] > 1){
						////usleep(2);
						//setTimer(SEND_AUX, packet_spacing);
						//leftToSend[i]--;
						////}else if (i < 6) {
						////i++;
						////setTimer(SEND_PACKET, packet_spacing);
						////}
						
					//}
					////}
				////setTimer(SEND_PACKET, packet_spacing);
				//}
			//break;
		//}
	//}

//}


// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void Manager::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
	switch (radioMsg->getRadioControlMessageKind()) {
		case CARRIER_SENSE_INTERRUPT:
			trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
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
	for (unsigned int i = 0; i < numNodes; i++) {
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
	manager_finalize(m_CONTEXT_ID);
	if (m_st_msg != NULL) {
	delete[] m_st_msg;
	m_st_msg = NULL;
	}
	
	if (m_comm_plugin != NULL) {
	delete[] m_comm_plugin;
	m_comm_plugin = NULL;
	}
}

MyPacket* Manager::createGenericDataPackett(int seqNum)
{
	MyPacket *pktt = new MyPacket("mypacket", APPLICATION_PACKET);
	//pktt->setExtraData(m_st_msg[recipientId]);
	pktt->setExtraData(m_st_msg[my_plugin_number/2]);
	pktt->setSequenceNumber(seqNum);
	//pktt->setByteLength(m_st_msg[recipientId].tam_buff);
	pktt->setByteLength(m_st_msg[my_plugin_number/2].tam_buff);
	return pktt;
}
