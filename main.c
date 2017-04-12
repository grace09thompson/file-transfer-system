/*
 * Name: Grace Thompson
 *
 * Program Name: ftclient main
 *
 * Description: This program is the client side of a file transfer system. ftclient starts on a user-specified
 * port and specifies a server host to connect to, the server port number, and a port to start a data connection
 * on. The client first makes a connection with the server, and makes a TCP data connection to make the file transfer.
 */

#include "ftclient.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>



int main(int argc, const char * argv[]) {
    //verify number of arguments provided
    if (argc < 5) {
        commandError();
    }
    //hostname of server to connect to
    const char *serverHost = argv[1];
    
    //port number of server to connect to
    int serverPort = atoi(argv[2]);
    //check for valid port
    if (serverPort == 0) {
        error("ERROR: invalid server port argument");
    }
    
    //check command for -l or -g. If -g, check for filename argument
    const char *command = argv[3];
  
    const char *requestFile;
    const char *dataPort;
    
    if (strcmp(command, "-g") == 0) {
        //check that there are enough arguments provided
        if (argc < 6) {
            commandError();
        }
        //check for filename
        //make sure it's a filename, not the data port (which should be the next argument
        if (atoi(argv[4]) != 0) {
            error("ERROR: expected filename");
        }
        requestFile = argv[4];
        //get data port number
        dataPort = argv[5];
        if (atoi(dataPort) == 0) {
            error("ERROR: invalid data port argument");
        }
       
    } else {
        //check for data port number
        dataPort = argv[4];
        if (atoi(dataPort) == 0) {
            error("ERROR: invalid data port argument");
        }
    }
    //printf("DATA PORT: %s\n", dataPort);
    
    //establish connection with specified server
    int controlSockfd;
    controlSockfd = connectToPort(serverPort, serverHost);
    if (controlSockfd == -1) {
        //connection failed
        fprintf(stderr, "ERROR: could not contact server on port %i\n", serverPort);
        exit(2);
    }
    
    //once established, send command, filename (if -g), and data port to server
    //printf("Successfully connected to server.\n");
    
    //send commands
    sendCommands(controlSockfd, command, requestFile, dataPort);
    
    //receive response from server
    char buffer[512];
    getResponse(controlSockfd, buffer);
    
    //if no error, try to connect to server via data transfer port
    if (buffer[0] == '1') {
        int success = beginDataTransferConnection(controlSockfd, serverHost, dataPort, command, requestFile);
        if (success == 0) {
            //retrieve error message and display it
            char error[256];
            getResponse(controlSockfd, error);
            printf("%s:%d says %s\n", serverHost, serverPort, error);
        }
    } else {
        //error message received
        printf("%s\n", buffer);
    }
    
    //close connection
    close(controlSockfd);
    
    return 0;
}
