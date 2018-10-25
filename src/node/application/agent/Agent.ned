package node.application.agent;
simple Agent like node.application.iApplication
	{
	parameters:
		string applicationID = default(""); //mandatory parameter
		bool collectTraceInfo = default(false); // mandatory parameter
		int priority = default(8); // mandatory parameter
		int packetHeaderOverhead = default(16);	// in bytes
		int constantDataPayload = default(16);	// in bytes
		
		string application_type = default("pulseoximeter"); // agent types
		double reading_rate = default(4);
		string time_limit = default("");
		
		
		//copied from ThroughputTest.ned
		
		double delayLimit = default (0); 		// application delay limit (0 - no limit)

		string nextRecipient = default ("0");	// Destination for packets received in this node. 
												// These parameters can be used to create an
												// application-level static routing. This way we can
												// have a multi-hop throughput test   

		double packet_rate = default (0);		// packets per second, by default we transmit no packets
		double startupDelay = default (0);		// delay in seconds before the app stars producing packets

		double latencyHistogramMax = default (200);
		int latencyHistogramBuckets = default (10);


	gates: 
	 	output toCommunicationModule;
		output toSensorDeviceManager;
		input fromCommunicationModule;
		input fromSensorDeviceManager;
		input fromResourceManager;
	}