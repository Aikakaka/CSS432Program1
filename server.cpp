//
// Aika Usui
// CSS 432 Program1
// server.cpp
// Accept an argument and accept and connect to the client and read data that is sent by client.
//
// the client-server model where a client process establishes a connection to a server,
// sends data or requests, and closes the connection. The server will accept the connection
// and create a thread to service the request and then wait for another connection on the main thread.
// Servicing the request consists of (1) reading the number of iterations the client will perform,
// (2) reading the data sent by the client, and (3) sending the number of reads which the server performed.
//

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

const int BUFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

int repetition, newSD;
char databuf[BUFSIZE];

/*
 * Method for Servicing thread
 * Read a message from the client and write back to the client.
 * close the connection with the connection.
 */
void *service(void *threaded)
{
    int count = 0;
    struct timeval startTime, endTime;
    int numRead;
    for (int i = 0; i < repetition; i++)
    {
        numRead = 0;
        while (numRead < BUFSIZE)
        {
            int bytesRead = read(newSD, databuf + numRead, BUFSIZE - numRead);
            numRead += bytesRead;
            count++;
        }
    }
    write(newSD, &count, sizeof(int));
    close(newSD);
    return NULL;
}

int main(int argc, char *argv[])
{
    int currentConn = 0;
    // socket consists of a port and a server name
    int ServerPort;
    char *servername;
    int port;

    //varification
    if (argc != 2)
    {
        cerr << "Please enter valid port for " << argv[0] << endl;
    }
    port = stoi(argv[1]);
    if (port < 49152 || port > 65535)
    {
        cerr << "Usage: " << argv[0] << " is not accessing a valid"
                                        " port value."
             << endl;
        return -1;
    }

    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(port);
    //creates socket
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    cout << "Socket #: " << serverSD << endl;

    // accepts upto 5 connection
    while (currentConn <= NUM_CONNECTIONS)
    {
        currentConn++;
        // Bind, Listen, Accept
        int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress,
                      sizeof(acceptSocketAddress));

        // Listen
        listen(serverSD, NUM_CONNECTIONS);

        // Accept
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        newSD = accept(serverSD, (sockaddr *)&newSockAddr, &newSockAddrSize);
        cout << "Accepted Socket #: " << newSD << endl;

        cout << "connect starts" << endl;

        // get repetition from client
        read(newSD, &repetition, sizeof(repetition));

        // up to five connections
        pthread_t serviceThread[NUM_CONNECTIONS];
        cout << "main() : creating thread, " << currentConn << endl;
        rc = pthread_create(&serviceThread[currentConn], NULL, service, NULL);
        if (rc)
        {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
        // wait for thread to teminate
        pthread_join(serviceThread[currentConn], NULL);
        //sleep(10);

        // server finishes the service to the client
        // decrease by one. Ready to accept new.
        currentConn--;
        //cout << currentConn << endl;

        // close client socket
        close(newSD);
    }

    // more than 5 connections, decline the connection.
    if (currentConn > 5)
    {
        cout << "Exceed the max connection." << endl;
    }
    // close client socket
    close(newSD);
    // close accept socket
    close(serverSD);
    return 0;
}