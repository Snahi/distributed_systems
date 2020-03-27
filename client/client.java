import java.io.*;
import java.net.*;
import gnu.getopt.Getopt;
import java.lang.Thread;


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
                outToServer.writeBytes(user+"\0");
                outToServer.writeBytes(String.valueOf(server_Socket.getLocalPort())+"\0");

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
            outToServer.writeBytes(user+"\0");

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
		// Write your code here
		System.out.println("PUBLISH " + file_name + " " + description);
		return 0;
	}

	 /**
	 * @param file_name    - file name
	 * 
	 * @return ERROR CODE
	 */
	static int delete(String file_name)
	{
		// Write your code here
		System.out.println("DELETE " + file_name);
		return 0;
	}

	 /**
	 * @return ERROR CODE
	 */
	static int list_users()
	{
		// Write your code here
		System.out.println("LIST_USERS " );
		return 0;
	}


	 /**
	 * @param user_name    - user name
	 * 
	 * @return ERROR CODE
	 */
	static int list_content(String user_name)
	{
		// Write your code here
		System.out.println("LIST_CONTENT " + user_name);
		return 0;
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
