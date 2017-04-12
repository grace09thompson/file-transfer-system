# file-transfer-system

## Description 
This is a simple file transfer system between a client and a server. ftserver starts on a user-specified port. The server waits on that port for a client request. The client program makes a request in the form 'ftclient <SERVER_HOST> <SERVER-_PORT> <OPTIONS> <DATA_PORT>'. To get a list of the files in the ftserver directory, input '-l' as the OPTIONS, to request a file by name, input '-g <FILENAME>' as the OPTIONS field. If the server receives an incorrect command, an error message is returned via the control socket. If valid command specified, server either sends a directory list of files or the file requested by the client via a newly opened second TCP connection. If specified file is not found, server returns an message that file was not found through the control socket. Server will run until a SIGINT is received, the client closes once the transfer is complete. 

If client requests a list of the files in the current directory, the server sends back a list of ONLY the files in the directory, folders/directories are not included. If client requests a file and already has a file in its current directory by that name, the contents of that file will be overwritten.

## Compile Instructions
To compile FTSERVER:
  javac ftserver.java
  
To compile FTCLIENT using Makefile:
  make
      
## Execution Instructions
To run FTSERVER:
  java ftserver <SERVER_PORT>
    
To run FTCLIENT:
  ./ftclient <SERVER_HOST> <SERVER_PORT> <COMMAND> <FILENAME> <DATA_PORT>
     
## FTCLIENT Execution Options:
To request current server directory files:
  COMMAND: -l 
  
  Example: ./ftclient flip1 33444 -l 55434
    In this example, client attempts to establish a connection with server 'flip1' on server port 33444, and requests the directory files via data port 55434.
  
To request a file by name:
  COMMAND: -g 
  
  Example: ./ftclient flip1 33444 -g tempfile.txt 55434
    In this example, client attempts to establish a connection with server 'flip1' on server port 33444, and requests a file named "tempfile.txt" via port 55434.
