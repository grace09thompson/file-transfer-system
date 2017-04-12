/* 
 * Name: Grace Thompson
 *
 * Program Name: ftclient
 *
 * Description: This program is the client side of a file transfer system. 
 * ftclient starts on a user-specified port with a list of commands for requesting 
 * a file or a directory list. File/Directory transfer occurs on a second data TCP
 * connection.
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

//error message function
void error(char *msg) {
    perror(msg);
    exit(1);
}

//command line options error function
//returns a message concerning the format of a command
void commandError() {
    printf("Enter command as: ftclient <SERVER_HOST> <SERVER_PORT> <OPTIONS> <DATA_PORT> \n");
    printf("To request file, OPTIONS: '-g <FILENAME>'\n");
    printf("To request directory list, OPTIONS: '-l'\n");
    exit(1);
}

/* function to create a socket and connect to a running server with the specified port and hostname
 * Parameters:
    int port number
    string hostname
 * Returns:
    int socket file descriptor for successfully created socket
    returns -1 on error
 */
int connectToPort(int port, const char *host) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    //create new TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR: error opening socket\n");
        return -1;
    }
    //get server hostname information
    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "ERROR: no such host\n");
        return -1;
    }
    //initialize serve_addr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //set address to bind to
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    //set port to use
    serv_addr.sin_port = htons(port);
    
    //attempt connection
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "ERROR: error connecting to server socket\n");
        return -1;
    }
    //return connected socket
    return sockfd;
}

/* function to send message to server
 * Parameters:
    int socket file descriptor
    string message to send
 * Returns:
    number of bytes sent on success, -1 on failure
 */
int sendMessage(int sockfd, const char *msg) {
    int bytes_to_send = strlen(msg);
    int bytes_written;
    int total_bytes = 0;
    char buffer[256]; //get message ready to send to server
    strcpy(buffer, msg);
    
    //send until entire message has been sent
    while (bytes_to_send > 0) {
        bytes_written = write(sockfd, buffer, strlen(buffer));
        if (bytes_written < 0) {
            error("ERROR sending message to socket");
        } else if (bytes_written == 0) {
            break;
        }
        bytes_to_send -= bytes_written;
        total_bytes += bytes_written;
    }
    //clear buffer
    bzero(buffer, 256);
    return total_bytes;
}

/* function to receive a message from the server 
 * Parameters: 
    int socket file descriptor
    char message to put response message into
 * Returns:
    Nothing, message is contained in address of string parameter
 */
void getResponse(int sockfd, char *message) {
    char buffer[512];
    bzero(buffer, 512);
    int n;
    n = read(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        fprintf(stderr, "ERROR reading message from socket\n");
        return;
    }
    strncpy(message, buffer, sizeof(buffer));
}

/* function to receive directory structure from server and display it on console to user
 * Parameters:
    int socket file descriptor
    String name of host server
    String server port number
 * Returns:
    Nothing, prints directory list to console
 */
void getDirectoryList(int sockfd, const char *host, const char *port) {
    char buffer[2048];
    bzero(buffer, 2048); //buffer to receive directory contents
    
    printf("Receiving directory structure from %s:%s\n", host, port);
    
    //get response, put in buffer
    int bytes_read = 1;
    while (bytes_read > 0) {
        bytes_read = read(sockfd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            fprintf(stderr, "ERROR reading directory from socket.\n");
            return;
        }
        printf("%s", buffer);
        bzero(buffer, 2048);
    }
    printf("\n");
}

/* function to receive size of incoming file from server
 * Paramters:
    int socket file descriptor
 * Returns:
    int size of file (in bytes) to receive. If receive 0, there was an error with retrieving requested file.
 */
int getFileSize(int sockfd) {
    char buffer[128];
    int n;
    n = read(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        fprintf(stderr, "ERROR reading filesize from server.\n");
        return -1;
    }
    char *token = strtok(buffer, " \n\t");
    int size = atoi(token);
    //printf("filesize received through data socket: %i\n", size);
    return size;
}

/* function to receive file from server. Opens a new file with specified filename and writes 
 * file contents received from server to open file. 
 * NOTE: If file already exists with that name, contents will be OVERWRITTEN.
 * Parameters:
    int socket file descriptor
    int size of file expected (in bytes)
    String name of file being transferred
 * Returns:
    Nothing, data written to file in current directory.
 */
void recvTransferFile(int sockfd, int size, const char *filename) {
    
    //printf("FILE SIZE TO RECEIVE: %i\n", size);
    
    //try to create a file with filename for writing contents to
    FILE *fd;
    fd = fopen(filename, "w");
    //check that file opened successfully
    if (fd == NULL) {
        fprintf(stderr, "ERROR opening file %s", filename);
        exit(1);
    }
    char buffer[2048];
    bzero(buffer, 2048);
    int bytes_read;
    int count = 0;
    //read data from socket, write directly to file
    while (count < size) {
        bytes_read = read(sockfd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            error("ERROR receiving file from socket.\n");
        }
        //printf("BYTE COUNT: %i\n", bytes_read);
        count += bytes_read;
        //printf("TOTAL COUNT: %i\n", count);
        //write buffer to file
        fprintf(fd, "%s", buffer);
        bzero(buffer, 2048);
    }
    //end with newline character
    fprintf(fd, "\n");
    fclose(fd);
    //printf("File %s received from server!\n", filename);
    printf("File transfer complete.\n");
}

/* function to send command to server through connected socket
 * Parameters: 
    int socket file descriptor
    String command (-l, -g)
    String name of file (if requested, is NULL otherwise)
    String port to start data transfer on
 * Returns:
    None
 */
void sendCommands(int sockfd, const char *command, const char *filename, const char *port) {
    //send command
    char commandString[4];
    //add newline to end of command
    strcpy(commandString, command);
    strcat(commandString, "\n");
    sendMessage(sockfd, commandString);
    bzero(commandString, 4);
    
    //if file requested, send file
    if (filename != NULL) {
        //add newline to end of filename
        char filestring[256];
        strcpy(filestring, filename);
        strcat(filestring, "\n");
        sendMessage(sockfd, filestring);
        bzero(filestring, 256);
    }
    
    //send data port to receive response on
    char portString[8];
    strcpy(portString, port);
    strcat(portString, "\n");
    sendMessage(sockfd, portString);
}

/* function to start data transfer connection and receive response from server
 * Parameters:
    int socket file descriptor of control connection
    String name of host server to connect to
    String port for data socket
    String command
    String name of requested file 
 * Returns:
    1 on successful transfer
    0 on file not found error (filesize is 0)
 */
int beginDataTransferConnection(int sockfd, const char *hostServer, const char *port, const char *command, const char *filename) {
    int dataSockfd;
    int dataPort = atoi(port);
    int fileSize = -1;
    if (dataPort == 0) {
        error("ERROR: invalid data port argument");
    }
    dataSockfd = connectToPort(dataPort, hostServer);
    if (dataSockfd == -1) {
        //connection to data socket failed
        fprintf(stderr, "ERROR: could not contact server on port %i\n", dataPort);
        exit(2);
    }
    
    //once established to data port, receive request response
    //printf("Data transfer connection established\n");
    //if command -l, expect directory list
    if (strcmp(command, "-l") == 0) {
        getDirectoryList(dataSockfd, hostServer, port);
    }
    
    //if command -g, expect file transfer
    if (strcmp(command, "-g") == 0) {
        //check filesize being sent through socket
        fileSize = getFileSize(dataSockfd);
        
        //if no error, create file to read contents into
        if (fileSize > 0) {
            printf("Receiving %s from %s:%s\n\n", filename, hostServer, port);
            recvTransferFile(dataSockfd, fileSize, filename);
        }
        //if error, close data socket and receive error message through control socket
    }
    
    //close data socket
    close(dataSockfd);
    
    //if filesize was 0, error
    if (fileSize == 0) {
        return 0;
    } else {
        return 1;
    }

}













