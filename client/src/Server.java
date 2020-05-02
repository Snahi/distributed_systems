import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;

/*
    Server part of the client - for sending the actual files.
    In order to start the server call start(). In order to stop the server call stop(). The server can be reused,
    whenever it is stopped and started it will create a new server socket and thread.
 */
public class Server implements Runnable {

    // CONST ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*
    Path to the folder where the published files are kept (available to other users).
     */
    static final String FILES_FOLDER_PATH = "files/";
    /*
    Because there are no limits for file size, files will be sent in parts. This constant stores the size of a single
    part in bytes.
     */
    static final int FILE_CHUNK_SIZE = 4096;
    /*
    Request sent to the server in order to get a file. It is followed by filename.
     */
    static final String REQ_GET_FILE = "GET_FILE";
    // responses
    static final byte RESP_WILL_TRANSFER_FILE = 0;
    static final byte RESP_ERR_FILE_DOESNT_EXIST = 1;
    static final byte RESP_ERR_OTHER = 2;

    // fields //////////////////////////////////////////////////////////////////////////////////////////////////////////
    private ServerSocket serverSocket;
    private Thread serverThread;
    /*
    If some file is being sent and the user disconnects the this flag will be changed to false and the file transmission
    will be stopped.
     */
    private boolean isRunning;



    Server()
    {
        serverSocket = null;
        serverThread = null;
        isRunning = false;
    }



    boolean start() throws IOException
    {
        if (serverThread != null)
            return false;

        serverSocket = new ServerSocket(0); // start listening on a random available port

        isRunning = true;
        serverThread = new Thread(this);
        serverThread.start();

        return true;
    }



    @Override
    public void run()
    {
        Socket clientSocket;

        while (isRunning)
        {
            try
            {
                clientSocket = serverSocket.accept();
                sendFile(clientSocket);
                clientSocket.close();
            }
            catch (SocketException e) // stop the server, close() was called while server was blocking on accept()
            {
                return;
            }
            catch (Exception e)
            {
                System.err.println("Could not accept a client socket");
                e.printStackTrace();
            }
        }
    }



    private void sendFile(Socket clientSocket)
    {
        String fileName = readFilename(clientSocket);

        // obtain output stream to the client
        DataOutputStream out = getClientOutputStream(clientSocket);
        if (out == null)
            return;

        if (fileName == null)
        {
            sendOtherErrorResponse(out);
            return;
        }

        // all ok, transfer the file
        byte transferRes = transferFile(out, fileName);

        // send response if error occurred (otherwise it has been already sent in transferFile(...)
        if (transferRes != RESP_WILL_TRANSFER_FILE)
        {
            try
            {
                out.write(transferRes);
                out.flush();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }



    private String readFilename(Socket socket)
    {
        String fileName =  null;
        try
        {
            ObjectInputStream in = new ObjectInputStream(socket.getInputStream());
            String request = (String) in.readObject();
            if (request.equals(REQ_GET_FILE))
                fileName = (String) in.readObject();
        }
        catch (IOException | ClassNotFoundException e)
        {
            System.err.println("Can't read file name");
            e.printStackTrace();
        }

        // the input stream will be closed when the socket is closed, automatically
        return fileName;
    }



    private DataOutputStream getClientOutputStream(Socket clientSocket)
    {
        DataOutputStream out = null;
        try
        {
            out = new DataOutputStream(clientSocket.getOutputStream());
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }

        return out;
    }



    private void sendOtherErrorResponse(DataOutputStream out)
    {
        try
        {
            out.write(RESP_ERR_OTHER);
            out.flush();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }



    private byte transferFile(DataOutputStream out, String fileName)
    {
        byte res = RESP_WILL_TRANSFER_FILE;

        String filePath = FILES_FOLDER_PATH + fileName;
        try (BufferedInputStream inFileStream = new BufferedInputStream(new FileInputStream(filePath)))
        {
            out.write(RESP_WILL_TRANSFER_FILE);
            out.flush();

            byte[] buff = new byte[FILE_CHUNK_SIZE];
            int nBytesRead;

            while ((nBytesRead = inFileStream.read(buff)) != -1 && isRunning)
            {
                out.write(buff, 0, nBytesRead);
            }
        }
        catch (FileNotFoundException e) // no file with such name
        {
            res = RESP_ERR_FILE_DOESNT_EXIST;
        }
        catch (IOException e)
        {
            res = RESP_ERR_OTHER;
            e.printStackTrace();
        }

        return res;
    }



    boolean stop()
    {
        if (serverThread == null)
            return false;

        // wait till the thread stops
        try
        {
            isRunning = false;
            serverSocket.close();
            serverSocket = null;

            serverThread.join();
            serverThread = null;
        }
        catch (IOException e)
        {
            System.err.println("Could not close the server socket");
            e.printStackTrace();
            return false;
        }
        catch (InterruptedException e)
        {
            System.err.println("Could not join the server thread");
            e.printStackTrace();
            return false;
        }

        return true;
    }



    // getters && setters //////////////////////////////////////////////////////////////////////////////////////////////



    int getPort()
    {
        return serverSocket.getLocalPort();
    }

}
