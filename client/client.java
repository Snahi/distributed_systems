import java.io.*;
import java.net.*;
import gnu.getopt.Getopt;
import java.lang.*;


class client {

	static final int OK = 0;
	static final int ERROR_USER = 1;
	static final int ERROR = 2;

	/* Cna we use enum for returning codes??
	private static enum CODE {
        OK,
        ERR,
        ERR2
	};
	*/
	
	/******************* ATTRIBUTES *******************/
	
	private static String _server   = null;
	private static int _port = -1;
		
	//Socket of the server
    private static ServerSocket server_Socket;
    //Variable used to check if the user is connected or not
    private static Boolean connect = false; 	
    //Store the name of the current user
    private static String username = "";  
    //Variable used to control the execution of the thread
    private static Boolean operating_thread = true; 
	

	/******************* CONSTRUCTOR (For the thread) *******************/

    public client(ServerSocket sc) {
        this.server_Socket = sc;
    }


	/********************* METHODS ********************/
	

	
	public static String readbytes(DataInputStream a) throws IOException {
        String bytes = "";
        byte lastcharacter = 'l';
        byte current = 'r';
        while (lastcharacter != '\0') {
            current = a.readByte();
            bytes = bytes + (char) current;
            lastcharacter = current;
        }
        return bytes;
	}
	


/*
	public static String readbytes(BufferedReader a) throws IOException {
        String bytes = "";
        byte lastcharacter = 'l';
        byte current = 'r';
        while (lastcharacter != '\0') {
			a.readLine()
            current = a.readByte();
            bytes = bytes + (char) current;
            lastcharacter = current;
        }
        return bytes;
    }
*/

	// This method will keep executing during the execution of the program so that if the client receives some message, the operation will be done
    public void run() {
		//timeout_time ASK IF NECESSARY
		try {
            server_Socket.setSoTimeout(1000); 
        }
		catch(Exception e){}
		
        while(operating_thread == true) {
            try {
                Socket client_Socket = server_Socket.accept(); 
		
            }
            catch(SocketTimeoutException e){}
            catch(IOException ie) {
                System.out.println("c> Message receiving thread finished execution. Connect again to restore.");
                ie.printStackTrace();
            }
            catch(Exception e){}
        }
    }




	/**
	 * @param user - User name to register in the system
	 * 
	 * @return ERROR CODE
	 */
	static int register(String user)
	{
		int rc=0;
		try {
			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message REGISTER and the username
			outToServer.writeBytes("REGISTER\0");
			outToServer.writeBytes(user+"\0");

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			byte response = inFromServer.readByte();

			//Switch for the different returning messages from the server
			switch (response) {
				case 0:
				rc=0;
				System.out.println("c> REGISTER OK");
				break;
				
				case 1:
				rc=1;
				System.out.println("c> USERNAME IN USE");
				break;
				
				case 2:
				rc=2;
				System.out.println("c> REGISTER FAIL");
				break;
			}
			//After checkhing the response, we close the socket
			client_Socket.close();

		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
	}
	
	/**
	 * @param user - User name to unregister from the system
	 * 
	 * @return ERROR CODE
	 */
	static int unregister(String user) 
	{
		int rc=0;
		try {
            Socket client_Socket = new Socket(_server, _port);
            DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message UNREGISTER and the username
            outToServer.writeBytes("UNREGISTER\0");
            outToServer.writeBytes(user+"\0");

            //Receive the message from the server and read it
            DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
            byte response = inFromServer.readByte();

            //Switch for the different returning messages from the server
            switch (response){
                case 0:
				System.out.println("c> UNREGISTER OK");
				rc=0;
				break;
				
				case 1:
				rc=1;
                System.out.println("c> USER DOES NOT EXIST");
                break;
				
				case 2:
				rc=2;
                System.out.println("c> UNREGISTER FAIL");
                break;
            }

			//After checkhing the response, we close the socket
            client_Socket.close();

		}
		catch(ConnectException s){
			System.out.println("c> UNREGISTER FAIL");
		}
        catch(Exception e) {
            System.out.println("Exception: " + e);
            e.printStackTrace();
            return ERROR;
		}
        return rc;
	}
	
    	/**
	 * @param user - User name to connect to the system
	 * 
	 * @return ERROR CODE
	 */
	static int connect(String user) 
	{
		int rc=0;
		if(connect == false){ 
            try {
                Socket client_Socket = new Socket(_server, _port);
                server_Socket = new ServerSocket(0);
                DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

                //Send to the server the message CONNECT and the username and port of the client
				outToServer.writeBytes("CONNECT\0");
				outToServer.flush();
				outToServer.writeBytes(user+"\0");
				outToServer.flush();
				outToServer.writeBytes(String.valueOf(server_Socket.getLocalPort())+"\0");
				outToServer.flush();

                //Receive the message from the server and read it
                DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
                byte [] response = new byte[1];
                inFromServer.read(response);

                //Switch for the different returning messages from the server
                switch (response[0]) {
					case 0:
					rc=0;
                    System.out.println("c> CONNECT OK");
		    		//Set the variable of the thread operating to true
                    operating_thread = true; 
                    //THREAD ??
                    connect = true; 
                    username = user; 
                    break;
					
					case 1:
					rc=1;
					System.out.println("c> CONNECT FAIL, USER DOES NOT EXIST");
                    server_Socket.close();
                    break;
					
					case 2:
					rc=2;
                    System.out.println("c> USER ALREADY CONNECTED");
                    server_Socket.close();
                    break;
					
					case 3:
					rc=3;
                    System.out.println("c> CONNECT FAIL");
                    server_Socket.close();
                    break;
                }

                //We close the socket
                client_Socket.close();

            }
            catch (Exception e) {
                System.out.println("Exception: " + e);
                e.printStackTrace();
                return ERROR;
            }
        }
        else { 
			rc=2;
             System.out.println("c> USER ALREADY CONNECTED");
        }
        return rc;
	}
	
	 /**
	 * @param user - User name to disconnect from the system
	 * 
	 * @return ERROR CODE
	 */
	static int disconnect(String user) 
	{
		int rc=0;
		try {
			//Create the socket
            Socket client_Socket = new Socket(_server, _port);
            DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

            //Send to the server the message DISCONNECT and the username
			outToServer.writeBytes("DISCONNECT\0");
			outToServer.flush();
			outToServer.writeBytes(user+"\0");
			outToServer.flush();

            //Receive the message from the server and read it
            DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
            byte response = inFromServer.readByte();

            //Switch for the different returning messages from the server
            switch (response) {
                case 0:
				rc=0;
				System.out.println("c> DISCONNECT OK");
                connect = false;
                username = ""; 
				//Set the variable of the thread operating to false.
                operating_thread=false; 
				break;
				
				case 1:
				rc=1;
                System.out.println("c> DISCONNECT FAIL / USER DOES NOT EXIST");
				break;
				
				case 2:
				rc=2;
                System.out.println("c> DISCONNECT FAIL / USER NOT CONNECTED");
				break;
				
				case 3:
				rc=3;
                System.out.println("c> DISCONNECT FAIL");
                break;
            }

            //We close the socket
            client_Socket.close();
        }catch(Exception e) {
            System.out.println("Exception: " + e);
            e.printStackTrace();
            return ERROR;
        }
        return rc;
	}


	 /**
	 * @param file_name    - file name
	 * @param description - descrition
	 * 
	 * @return ERROR CODE
	 */
	static int publish(String file_name, String description) 
	{
		int rc=0;
		try {
			
			/*
			if (file_name.isBlank()){
			}
			if (file_name.length()>256){
			}
			if (description.length()>256){
			}
			*/

			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message PUBLISH, the username, the file name and its description
			outToServer.writeBytes("PUBLISH\0");
			outToServer.flush();
			outToServer.writeBytes(username+"\0");
			outToServer.flush();
			outToServer.writeBytes(file_name+"\0");
			outToServer.flush();
			outToServer.writeBytes(description+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			byte response = inFromServer.readByte();

			//Switch for the different returning messages from the server
			switch (response) {
				case 0: //SUCCESS
				rc=0;
				System.out.println("c> PUBLISH OK");
				break;
				
				case 1: //USER DOES NOT EXIST
				rc=1;
				System.out.println("c> PUBLISH FAIL, USER DOES NOT EXIST");
				break;
				
				case 2: //USER IS NOT CONNECTED
				rc=2;
				System.out.println("c> PUBLISH FAIL, USER NOT CONNECTED");
				break;

				case 3: //FILE ALREADY PUBLISH
				rc=3;
				System.out.println("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED");
				break;

				case 4: //ANY OTHER CASE OR ERROR
				rc=4;
				System.out.println("c> PUBLISH FAIL");
				break;
			}
			//After checkhing the response, we close the socket
			client_Socket.close();

		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
		/*
		// Write your code here
		System.out.println("PUBLISH " + file_name + " " + description);
		return 0;
		*/
	}

	 /**
	 * @param file_name    - file name
	 * 
	 * @return ERROR CODE
	 */
	static int delete(String file_name)
	{
		int rc=0;
		try {
			
			/*
			if (file_name.isBlank()){
			}
			if (file_name.length()>256){
			}
			*/

			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message DELETE, the username, the file name and its description
			outToServer.writeBytes("DELETE\0");
			outToServer.flush();
			outToServer.writeBytes(username+"\0");
			outToServer.flush();
			outToServer.writeBytes(file_name+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			byte response = inFromServer.readByte();

			//Switch for the different returning messages from the server
			switch (response) {
				case 0: //SUCCESS
				rc=0;
				System.out.println("c> DELETE OK");
				break;
				
				case 1: //USER DOES NOT EXIST
				rc=1;
				System.out.println("c> DELETE FAIL, USER DOES NOT EXIST");
				break;
				
				case 2: //USER IS NOT CONNECTED
				rc=2;
				System.out.println("c> DELETE FAIL, USER NOT CONNECTED");
				break;

				case 3: //FILE ALREADY PUBLISH
				rc=3;
				System.out.println("c> DELETE FAIL, CONTENT ALREADY PUBLISHED");
				break;

				case 4: //ANY OTHER CASE OR ERROR
				rc=4;
				System.out.println("c> DELETE FAIL");
				break;
			}
			//After checkhing the response, we close the socket
			client_Socket.close();

		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
		/*
		// Write your code here
		System.out.println("DELETE " + file_name);
		return 0;
		*/
	}

	 /**
	 * @return ERROR CODE
	 */
	static int list_users()
	{
		int rc=0;
		try {


			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message PUBLISH, the username, the file name and its description
			outToServer.writeBytes("LIST_USERS\0");
			outToServer.flush();
			//JUST FOR NOW WE SET MANUALLY THE USERNAME
			username="alex";
			outToServer.writeBytes(username+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			byte response = inFromServer.readByte();

			//Switch for the different returning messages from the server
			switch (response) {
				case 0: //SUCCESS
				rc=0;
				//TEST
				//BufferedReader f = new BufferedReader(new InputStreamReader(client_Socket.getInputStream()));
				//String susers = f.readLine();
				
				//String susers = inFromServer.readLine();
				
				//String susers = readbytes(f);

				String susers = readbytes(inFromServer);
				int nusers = Integer.parseInt(susers);
				System.out.println("c> LIST_USERS OK");
				while (nusers!=0){
					String aux = readbytes(inFromServer);
					System.out.print("\t"+aux);
					aux = readbytes(inFromServer);
					System.out.print("\t"+aux);
					aux = readbytes(inFromServer);
					System.out.println("\t"+aux);
					nusers--;
				}
				break;
				
				case 1: //USER DOES NOT EXIST
				rc=1;
				System.out.println("c> LIST_USERS FAIL , USER DOES NOT EXIST");
				break;
				
				case 2: //USER IS NOT CONNECTED
				rc=2;
				System.out.println("c> LIST_USERS FAIL , USER NOT CONNECTED");
				break;

				case 3: //ANY OTHER CASE OR ERROR
				rc=3;
				System.out.println("c>LIST_USERS FAIL");
				break;

			}
			//After checkhing the response, we close the socket
			client_Socket.close();

		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
		/*
		// Write your code here
		System.out.println("LIST_USERS " );
		return 0;
		*/
	}


	 /**
	 * @param user_name    - user name
	 * 
	 * @return ERROR CODE
	 */
	static int list_content(String user_name)
	{
		int rc=0;
		try {
			
			/*
			if (file_name.isBlank()){
			}
			if (file_name.length()>256){
			}
			*/

			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message PUBLISH, the username, the file name and its description
			outToServer.writeBytes("LIST_CONTENT\0");
			outToServer.flush();
			outToServer.writeBytes(username+"\0");
			outToServer.flush();
			outToServer.writeBytes(user_name+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			byte response = inFromServer.readByte();

			//Switch for the different returning messages from the server
			switch (response) {
				case 0: //SUCCESS
				rc=0;
				//String sfiles = inFromServer.readLine();
				String sfiles = readbytes(inFromServer);
				int nfiles = Integer.parseInt(sfiles);
				System.out.println("c> LIST_CONTENT OK");
				while (nfiles!=0){
					String aux = readbytes(inFromServer);
					System.out.print("\t"+aux);
					aux = readbytes(inFromServer);
					System.out.print("\t"+aux);
					aux = readbytes(inFromServer);
					System.out.println("\t"+aux);
					nfiles--;
				}
				break;
				
				case 1: //USER DOES NOT EXIST
				rc=1;
				System.out.println("c> LIST_CONTENT FAIL , USER DOES NOT EXIST");
				break;
				
				case 2: //USER IS NOT CONNECTED
				rc=2;
				System.out.println("c> LIST_CONTENT FAIL , USER NOT CONNECTED");
				break;

				case 3: //ANY OTHER CASE OR ERROR
				rc=3;
				System.out.println("c>LIST_CONTENT FAIL, REMOTE USER DOES NOT EXITS");
				break;

				case 4: //ANY OTHER CASE OR ERROR
				rc=4;
				System.out.println("c>LIST_CONTENT FAIL");
				break;

			}
			//After checkhing the response, we close the socket
			client_Socket.close();

		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
		/*
		// Write your code here
		System.out.println("LIST_CONTENT " + user_name);
		return 0;
		*/
	}

	 /**
	 * @param user_name    - user name
	 * @param remote_file_name    - remote file name
	 * @param local_file_name  - local file name
	 * 
	 * @return ERROR CODE
	 */
	static int get_file(String user_name, String remote_file_name, String local_file_name)
	{
		// Write your code here
		System.out.println("GET_FILE " + user_name + " "  + remote_file_name + " " + local_file_name);
		return 0;
	}

	
	/**
	 * @brief Command interpreter for the client. It calls the protocol functions.
	 */
	static void shell() 
	{
		boolean exit = false;
		String input;
		String [] line;
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

		while (!exit) {
			try {
				System.out.print("c> ");
				input = in.readLine();
				line = input.split("\\s");

				if (line.length > 0) {
					/*********** REGISTER *************/
					if (line[0].equals("REGISTER")) {
						if  (line.length == 2) {
							register(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: REGISTER <userName>");
						}
					} 
					
					/********** UNREGISTER ************/
					else if (line[0].equals("UNREGISTER")) {
						if  (line.length == 2) {
							unregister(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: UNREGISTER <userName>");
						}
                    			} 
                    			
                    			/************ CONNECT *************/
                    			else if (line[0].equals("CONNECT")) {
						if  (line.length == 2) {
							connect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: CONNECT <userName>");
                    				}
                    			} 
                    
                    			/********** DISCONNECT ************/
                    			else if (line[0].equals("DISCONNECT")) {
						if  (line.length == 2) {
							disconnect(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: DISCONNECT <userName>");
                    				}
                    			} 
                    
                    			/************** PUBLISH **************/
                    			else if (line[0].equals("PUBLISH")) {
						if  (line.length >= 3) {
							// Remove first two words
							//String description = input.substring(input.indexOf(' ')+1).substring(input.indexOf(' ')+1);
							String description = input.substring(input.indexOf(' ')+1);
							description = description.substring(description.indexOf(' ')+1);
							publish(line[1], description); // file_name = line[1]
						} else {
							System.out.println("Syntax error. Usage: PUBLISH <file_name> <description>");
                    				}
                    			} 

                    			/************ DELETE *************/
                    			else if (line[0].equals("DELETE")) {
						if  (line.length == 2) {
							delete(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: DELETE <file name>");
                    				}
                    			} 
                    
                    			/************** LIST_USERS **************/
                    			else if (line[0].equals("LIST_USERS")) {
						if  (line.length == 1) {
							// Remove first two words
							list_users(); 
						} else {
							System.out.println("Syntax error. Usage: LIST_USERS ");
                    				}
                    			} 
                    
                    			/************ LIST_CONTENT *************/
                    			else if (line[0].equals("LIST_CONTENT")) {
						if  (line.length == 2) {
							list_content(line[1]); // userName = line[1]
						} else {
							System.out.println("Syntax error. Usage: LIST_CONTENT <user name>");
                    				}
                    			} 
                    
                    			/************** GET_FILE **************/
                    			else if (line[0].equals("GET_FILE")) {
						if  (line.length == 4) {
							get_file(line[1], line[2], line[3]); 
						} else {
							System.out.println("Syntax error. Usage: GET_FILE <user> <remote_file_name> <local_file_name>");
                    				}
                    			} 

                    
                    			/************** QUIT **************/
                    			else if (line[0].equals("QUIT")){
						if (line.length == 1) {
							exit = true;
						} else {
							System.out.println("Syntax error. Use: QUIT");
						}
					} 
					
					/************* UNKNOWN ************/
					else {						
						System.out.println("Error: command '" + line[0] + "' not valid.");
					}
				}				
			} catch (java.io.IOException e) {
				System.out.println("Exception: " + e);
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * @brief Prints program usage
	 */
	static void usage() 
	{
		System.out.println("Usage: java -cp . client -s <server> -p <port>");
	}
	
	/**
	 * @brief Parses program execution arguments 
	 */ 
	static boolean parseArguments(String [] argv) 
	{
		Getopt g = new Getopt("client", argv, "ds:p:");

		int c;
		String arg;

		while ((c = g.getopt()) != -1) {
			switch(c) {
				//case 'd':
				//	_debug = true;
				//	break;
				case 's':
					_server = g.getOptarg();
					break;
				case 'p':
					arg = g.getOptarg();
					_port = Integer.parseInt(arg);
					break;
				case '?':
					System.out.print("getopt() returned " + c + "\n");
					break; // getopt() already printed an error
				default:
					System.out.print("getopt() returned " + c + "\n");
			}
		}
		
		if (_server == null)
			return false;
		
		if ((_port < 1024) || (_port > 65535)) {
			System.out.println("Error: Port must be in the range 1024 <= port <= 65535");
			return false;
		}

		return true;
	}
	
	
	
	/********************* MAIN **********************/
	
	public static void main(String[] argv) 
	{
		if(!parseArguments(argv)) {
			usage();
			return;
		}
		
		// Write code here
		
		shell();
		System.out.println("+++ FINISHED +++");
	}
}
