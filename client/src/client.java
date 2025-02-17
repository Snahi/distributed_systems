import java.io.*;
import java.net.*;
import java.util.ArrayList;
import upper_case_client.UpperCaseService;
import upper_case_client.UpperCaseServiceService;

import gnu.getopt.Getopt;
//import java.lang.*;


class client {

	private static final int ERROR = 2;
	private static final String DOWNLOADS_PATH = "downloads/";

	
	/******************* ATTRIBUTES *******************/
	
	private static String _server   = null;
	private static int _port = -1;

    //Variable used to check if the user is connected or not
    private static Boolean connect = false; 	
    //Store the name of the current user
    private static String username = "";
	// server part of client - to send the real files
	private static Server server = new Server();


	/********************* METHODS ********************/


	
	private static String readbytes(DataInputStream a) throws IOException {
        StringBuilder bytes = new StringBuilder();
        byte lastcharacter = 'l';
        byte current = 'r';
        while (lastcharacter != '\0') {
            current = a.readByte();
			if (current != '\0')
				bytes.append((char) current);
            lastcharacter = current;
        }
        return bytes.toString();
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
		catch (ConnectException e)
		{
			System.out.println("c> REGISTER FAIL");
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
			byte response = 0;

			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message UNREGISTER and the username
			outToServer.writeBytes("UNREGISTER\0");
			outToServer.flush();
			outToServer.writeBytes(user+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			response = inFromServer.readByte();

			//After checkhing the response, we close the socket
			client_Socket.close();

            //Switch for the different returning messages from the server
            switch (response){
				case 0:
				System.out.println("c> UNREGISTER OK");
				if (user.equals(username))
				{
					username = "";
					connect = false;
					server.stop();
				}
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

				if (!server.start())
				{
					System.out.println("c> CONNECT FAIL");
					return 3;
				}

                //Send to the server the message CONNECT and the username and port of the client
				Socket client_Socket = new Socket(_server, _port);
				DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

				outToServer.writeBytes("CONNECT\0");
				outToServer.flush();
				outToServer.writeBytes(user+"\0");
				outToServer.flush();
				outToServer.writeBytes(server.getPort() + "\0");
				outToServer.flush();

                //Receive the message from the server and read it
                DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
                byte [] responseArr = new byte[1];
                int readLen = inFromServer.read(responseArr);

				//We close the socket
				client_Socket.close();

				rc = responseArr[0];

				if (rc == 0 && readLen > 0)
				{
					System.out.println("c> CONNECT OK");
					connect = true;
					username = user;
				}
				else
				{
					switch (rc) {
						case 1:
							System.out.println("c> CONNECT FAIL, USER DOES NOT EXIST");
							break;

						case 2:
							System.out.println("c> USER ALREADY CONNECTED");
							break;

						default:
							System.out.println("c> CONNECT FAIL");
							break;
					}

					// failed -> stop the server, no message printed because it was already printed before, and the
					// only way it fails is when some serious error occurs, so the exception will be printed anyway
					server.stop();
				}
			}
			catch (ConnectException e)
			{
				System.out.println("c> CONNECT FAIL");
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
		int rc=disconnect_bare(user);
		//Switch for the different returning messages from the server
		switch (rc) {
			case 0:
				System.out.println("c> DISCONNECT OK");
				break;

			case 1:
				System.out.println("c> DISCONNECT FAIL / USER DOES NOT EXIST");
				break;

			case 2:
				System.out.println("c> DISCONNECT FAIL / USER NOT CONNECTED");
				break;

			case 3:
				System.out.println("c> DISCONNECT FAIL");
				break;
		}
        return rc;
	}



	/*
	Disconnects the specified user, but doesn't print any error. This is a method for disconnect which will be
	called in other methods which don't require printing the output. The base disconnect method also uses it.
	 */
	private static byte disconnect_bare(String user)
	{
		byte res;

		try (Socket client_Socket = new Socket(_server, _port))
		{
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message DISCONNECT and the username
			outToServer.writeBytes("DISCONNECT\0");
			outToServer.flush();
			outToServer.writeBytes(user+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());

			res = inFromServer.readByte();
		}
		catch (IOException e)
		{
			res = 3;
		}

		// stop the server no matter what
		if (server.isRunning())
		{
			if (!server.stop())
				res = 3;
		}
		
		// disconnect locally in any case
		connect = false;
		username = "";

		return res;
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
		if (username == null || username.length() == 0)
		{
			System.out.println("c> PUBLISH FAIL, USER NOT CONNECTED");
			return 2;
		}

		try {
			// transform description to uppercase
			UpperCaseServiceService upperCaseService = new UpperCaseServiceService();
			UpperCaseService upperCaser = upperCaseService.getUpperCaseServicePort();
			description = upperCaser.upperCase(description);

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
		catch(ConnectException e)
		{
			rc=4;
			System.out.println("c> PUBLISH FAIL");
		}
		catch(Exception e) {
			System.out.println("Exception: " + e);
			e.printStackTrace();
			return ERROR;
		}
		return rc;
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
				System.out.println("c> DELETE FAIL, CONTENT NOT PUBLISHED");
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
	}

	 /**
	 * @return ERROR CODE
	 */
	static int list_users()
	{
		ArrayList<Usuario> users = new ArrayList<>();
		byte response = getConnectedUsersList(users);

		//Switch for the different returning messages from the server
		switch (response) {
			case 0: //SUCCESS
			System.out.println("c> LIST_USERS OK");
			for (Usuario u : users)
			{
				System.out.print("\t" + u.getNombre());
				System.out.print("\t" + u.getIp());
				System.out.println("\t" + u.getPort());
			}
			break;
			
			case 1: //USER DOES NOT EXIST
			System.out.println("c> LIST_USERS FAIL , USER DOES NOT EXIST");
			break;
			
			case 2: //USER IS NOT CONNECTED
			System.out.println("c> LIST_USERS FAIL , USER NOT CONNECTED");
			break;

			case 3: //ANY OTHER CASE OR ERROR
			System.out.println("c>LIST_USERS FAIL");
			break;

		}

		return response;
	}



	private static byte getConnectedUsersList(ArrayList<Usuario> users)
	{
		byte response = 0;

		try {
			//Create the socket
			Socket client_Socket = new Socket(_server, _port);
			DataOutputStream outToServer = new DataOutputStream(client_Socket.getOutputStream());

			//Send to the server the message PUBLISH, the username, the file name and its description
			outToServer.writeBytes("LIST_USERS\0");
			outToServer.flush();
			outToServer.writeBytes(username+"\0");
			outToServer.flush();

			//Receive the message from the server and read it
			DataInputStream inFromServer = new DataInputStream(client_Socket.getInputStream());
			response = inFromServer.readByte();

			if (response == 0)
			{
				// get number of users
				String strNumOfUsers = readbytes(inFromServer);
				int numOfUsers = -1;
				numOfUsers = Integer.parseInt(strNumOfUsers);

				Usuario tmpUser = null;
				for (int i = 0; i < numOfUsers; i++)
				{
					String name = readbytes(inFromServer);
					String ip = readbytes(inFromServer);
					String port = readbytes(inFromServer);

					tmpUser = new Usuario();

					tmpUser.setNombre(name);
					tmpUser.setIp(ip);
					tmpUser.setPort(Integer.parseInt(port));

					users.add(tmpUser);
				}
			}

			client_Socket.close();
		}
		catch (Exception e)
		{
			response = 3;
			e.printStackTrace();
		}
		
		return response;
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
		catch (ConnectException e)
		{
			System.out.println("c> LIST_CONTENT FAIL");
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
		int res = Server.RESP_ERR_OTHER;
		Usuario serverUsr = getConnectedUserByUsername(user_name);

		if (serverUsr != null)
		{
			String serverIp = serverUsr.getIp();
			int serverPort = serverUsr.getPort();

			res = downloadFile(remote_file_name, local_file_name, serverIp, serverPort);
		}

		switch (res)
		{
			case Server.RESP_WILL_TRANSFER_FILE :
				System.out.println("GET_FILE OK");
				break;
			case Server.RESP_ERR_FILE_DOESNT_EXIST :
				System.out.println("GET_FILE_FAIL / FILE NOT EXIST");
				break;
			default :
				System.out.println("GET_FILE FAIL");
		}

		return res;
	}



	private static Usuario getConnectedUserByUsername(String username)
	{
		ArrayList<Usuario> users = new ArrayList<>();
		int res = getConnectedUsersList(users);

		if (res != 0)
			return null;

		for (Usuario u : users)
		{
			if (u.getNombre().equals(username))
				return u;
		}

		return null;
	}



	private static byte downloadFile(String remoteFilename, String localFilename, String ip, int port)
	{
		byte res;

		try (Socket clientSocket = new Socket(ip, port))
		{
			ObjectOutputStream outToServer = new ObjectOutputStream(clientSocket.getOutputStream());

			outToServer.writeObject(Server.REQ_GET_FILE);
			outToServer.flush();
			outToServer.writeObject(remoteFilename);
			outToServer.flush();

			DataInputStream in = new DataInputStream(clientSocket.getInputStream());
			// receive result
			res = in.readByte();

			if (res == Server.RESP_WILL_TRANSFER_FILE) // success, file will be transferred
				createDownloadedFileLocally(in, localFilename);
		}
		catch (Exception e)
		{
			res = Server.RESP_ERR_OTHER;
		}

		return res;
	}



	private static void createDownloadedFileLocally(DataInputStream in, String localFileName) throws IOException
	{
		// out stream for the local file
		String filePath = DOWNLOADS_PATH + localFileName;
		FileOutputStream outFile = new FileOutputStream(filePath);

		// buffer (file is sent in chunks)
		byte[] receivedData = new byte[Server.FILE_CHUNK_SIZE];
		int bytesRead;

		while ((bytesRead = in.read(receivedData)) != -1)
		{
			outFile.write(receivedData, 0, bytesRead);
		}

		outFile.close();
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
							if (username != null && username.length() > 0)
								disconnect_bare(username);
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
		
		// create folders for shared files and downloads if they dont exist
        File filesDir = new File(Server.FILES_FOLDER_PATH);
		if (!filesDir.exists())
        {
            if (!filesDir.mkdir())
            {
                System.out.println("Could not create files directory");
                return;
            }
        }

		File downloadsDir = new File(DOWNLOADS_PATH);
		if (!downloadsDir.exists())
		{
		    if (!downloadsDir.mkdir())
            {
                System.out.println("Could not create downloads directory");
                return;
            }
        }
		
		shell();
		System.out.println("+++ FINISHED +++");
	}
}
