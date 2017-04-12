import java.util.*;
import java.net.*;
import java.io.*;

/**
 * ftserver: Server side of File Transfer System
 * Description: The ftserver opens a socket on a user-specified port and waits for a client connection request. Once connected, the client
 * either requests the current directory list of files or a specific file by name, along with a port to establish the data transfer connection
 * on. Server opens and connects to the client on this new data connection to transfer the directory or file, sending an error message via 
 * the control connection in the case of an invalid command or non-existent file. 
 * 
 * @author Grace Thompson 
 * @date-started: 11/19/2016
 * @last-modified: 11/20/2016
 */

public class ftserver
{
    /* function to retrieve messages from client 
     * Parameters: BufferedReader input stream for receiving messages from client 
     * Returns: none
    */
    private static String getMessage(BufferedReader in) {
        String message = null;
        try {
            message = in.readLine();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return message;
    }
    
    /* function to send a short message to client 
     * Parameters: PrintWriter output stream for writing messages to client, String message to send 
     * Returns: none
     */
    private static void sendMessage(PrintWriter out, String message) {
        out.println(message);
        out.flush();
    }
    
    /* function to get directory contents and send to the client via data port 
     * Parameters: PrintWriter output stream for writing directory to client
     * Returns: none
    */
    private static void sendDirectory(PrintWriter out) {
        File currentDir = new File(".");
        File[] list = currentDir.listFiles();
        for(File f : list) {
            if(f.isDirectory()) {
                System.out.println(f.getName());
            }
            if(f.isFile()) {
                System.out.println(f.getName());
                sendMessage(out, f.getName()); //send file names to client 
            }
        }
    }
    
    /* function to create data transfer connection to client
     * Parameters: integer port to establish data connection, String name of file to send
     * Returns: 1 on success, 0 on error (if file not found)
     * 
      FileInputStream REFERENCE: File input stream reference https://www.mkyong.com/java/how-to-read-file-in-java-fileinputstream/ 
    */
    private static int startDataConnection(String clientHost, int port, String filename) {
        if (filename != null) {
                System.out.println("File " + filename + " requested on port " + port);
                System.out.println();
            } else {
                System.out.println("List directory requested on port " + port);
                System.out.println();
            }
        //start new socket on port and accept client connection
        try (
            ServerSocket serverSocket = new ServerSocket(port);
            Socket clientSocket = serverSocket.accept();
            PrintWriter dataOut = 
                new PrintWriter(clientSocket.getOutputStream(), true);
        ) {
            //System.out.println("Connection established on port " + port);
            if (filename != null) {
                //attempt to find specified file in current directory 
                File file = new File(filename);
                int fileSize = 0;
                //check if a file 
                if(file.exists() && !file.isDirectory()) {
                    //send file contents through connection  
                    try (FileInputStream fis = new FileInputStream(file)) {
                        //System.out.println("Total file size to read (in bytes) : " + fis.available());
                        System.out.println("Sending " + filename + " to " + clientHost + ":" + port);
                        System.out.println();
                        fileSize = fis.available();
                        //send filesize to client so they know what to expect 
                        dataOut.println(fileSize);
                        int content; 
                        while ((content = fis.read()) != -1) {
                            //convert to char and display 
                            //System.out.print((char) content);
                            dataOut.print((char) content);
                        }
                        //return 1; //successful data transfer
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    return 1; //successful data transfer
                } else {
                    //send error message to client, filesize 0 as signal of error 
                    //System.out.println("Size of file/error: " + fileSize);
                    dataOut.println(fileSize);
                    return 2; //error
                }
            } else {
                sendDirectory(dataOut);
                return 1;//successful data transfer
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return 1;
    }
    
    /*function to accept incoming connection to receive commands from client, closed from client side
     * Parameters: ServerSocket socket to connect to client with (Connection P)
     * Output: none
    */
    private static void startControlConnection(ServerSocket socket, int port) {
        try (
            //accept incoming request to connect 
            Socket clientSocket = socket.accept();
            //setup input/output streams 
            PrintWriter out =
                new PrintWriter(clientSocket.getOutputStream(), true);
            BufferedReader in = new BufferedReader(
                new InputStreamReader(clientSocket.getInputStream()));
        ) {
            //get hostname of client 
            String clientHost = clientSocket.getInetAddress().getHostName();
            System.out.println("Connection from " + clientHost);
            
            //receive command received from client 
            String command = getMessage(in);
            //System.out.println("Command received: " + command);
            //if valid command, initiate TCP data connection with client on client-specified port 
            int dataPort;
            String requestFile = null;
            if (command.equals("-l")) {
                //get data port number 
                dataPort = Integer.parseInt(getMessage(in));
    
            } else if (command.equals("-g")) {
                requestFile = getMessage(in);
                //System.out.println("File name: " + requestFile);
                //get data port number 
                dataPort = Integer.parseInt(getMessage(in));
 
            } else {
                //invalid command received, send back error message to client
                String message = "Invalid Command Received";
                sendMessage(out, message);
                return;
            }
            //send acknowledgement of valid command 
            sendMessage(out, "1");
            
            //open new socket connection on data port and send requested file/list 
            int success = startDataConnection(clientHost, dataPort, requestFile);
            //if error (2), print file not found 
            if (success == 2) {
                String error = "File Not Found";
                System.out.println("File not found. Sending error message to " + clientHost + ":" + port);
                sendMessage(out, error);
            }
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        //check port number specified from command line as first argument 
        if (args.length < 1) {
            System.out.println("ERROR: Must specify port argument.");
            return;
        }
       // int port = Integer.parseInt(args[0]);
       int port = -1;
        //validate port 
        try {
            port = Integer.parseInt(args[0]);
        } catch (NumberFormatException e) {
            System.out.println("ERROR: Must specify valid port");
        }
        
        //create socket with port number to wait for incoming connections
        try (
            ServerSocket serverSocket = new ServerSocket(port);
        ) {
            //infinite loop until SIGINT is received 
            while (true) {
                System.out.println("Server open on " + port);
                //try to accept incoming connection to client 
                startControlConnection(serverSocket, port);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
