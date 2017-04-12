//
//  ftclient.h
//  Project2Client
//
//  Created by Grace Thompson on 11/20/16.
//  Copyright © 2016 Grace Thompson. All rights reserved.
//

#ifndef ftclient_h
#define ftclient_h

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

//error message functions
void error(char *msg);
void commandError();

int connectToPort(int port, const char *host);

int sendMessage(int sockfd, const char *msg);

void getResponse(int sockfd, char *messge);

void getDirectoryList(int sockfd, const char *host, const char *port);

int getFileSize(int sockfd);

void recvTransferFile(int sockfd, int size, const char *filename);

void sendCommands(int sockfd, const char *command, const char *filename, const char *port);

int beginDataTransferConnection(int sockfd, const char *hostServer, const char *port, const char *command, const char *filename);

#endif /* ftclient_h */
