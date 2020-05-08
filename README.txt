Deployment instruction:

IMPORTANT: Do not user port 7778 because the upper case server runs on this port

1. Start C servers
	1.1. In order to compile the C servers go to the server/ directory and type: 
		make
	1.2. Deploy the storage server: 
		./storageserver
	1.3. Deploy the intermediary server:
		./server -p <port for listening> -s <ip address of the storage server>
2. Start the upper case server
	2.1. In order to compile the upper case server go to the web_service_upper_case/ directory and
	type: 
		javac Publisher.java
	2.2. Deply the server: 
		java Publisher
3. Start a client
	3.1. Go to client/src and type: 
		javac client.java
	3.2. Run the client: 
		java -cp . client -p <intermediary server port> -s <intermediary server ip>
