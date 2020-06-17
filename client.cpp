//
// Aika Usui
// CSS 432 Program1
// client.cpp
// Accepts arguments, create socket, connect to server and print out the transfer time and throughput.
//
// the client-server model where a client process establishes a connection to a server,
// sends data or requests, and closes the connection. The server will accept the connection
// and create a thread to service the request and then wait for another connection on the main thread.
// Servicing the request consists of (1) reading the number of iterations the client will perform,
// (2) reading the data sent by the client, and (3) sending the number of reads which the server performed.
//

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
#include <chrono>

using namespace std;

const int BUFFSIZE = 1500;

int port, repetition, nbufs, bufsize, type;

// argv[1] = serverName
// argv[2] = IP port number used by server (last 5 of student ID)
// argv[3] = Repetition of sending a set of data buffers
// argv[4] = Number of data buffers
// argv[5] = Size of each data buffer (in bytes)
// argv[6] = type of transfer scenario: 1, 2, or 3
int main(int argc, char *argv[])
{
    char *serverName; // argv[1]

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;

    /*
     * Argument Validation
     */
    if (argc != 7)
    {
        cerr << "Usage: " << argv[0] << " does not provide correct number of "
                                        "arguments."
             << endl;
        return -1;
    }

    /*
     * Use getaddrinfo() to get addrinfo structure corresponding to
     * serverName / Port
     * This addrinfo structure has internet address which can be used to
     * create a socket too
     */
    serverName = argv[1];
    port = stoi(argv[2]);
    repetition = stoi(argv[3]);
    nbufs = stoi(argv[4]);
    bufsize = stoi(argv[5]);
    type = stoi(argv[6]);

    if (port < 49152 || port > 65535)
    {
        cerr << "Usage: " << argv[0] << " is not accessing a valid"
                                        " port value."
             << endl;
        return -1;
    }

    if (repetition < 0)
    {
        cerr << "Usage: " << argv[0] << " did not provide the correct value "
                                        "for repetitions"
             << endl;
        return -1;
    }

    // needs to be equal to 1500
    if ((nbufs * bufsize) != BUFFSIZE)
    {
        cerr << "Usage: " << argv[0] << " did not provide proper buffer "
                                        "values"
             << endl;
    }

    if (type < 1 || type > 3)
    {
        cerr << "Usage: " << argv[0] << " did not provide the correct value "
                                        "for transfer type."
             << endl;
        return -1;
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = 0;              // Optional options --0 means no optional arguments used
    hints.ai_protocol = 0;           // Allow any protocol

    int rc = getaddrinfo(serverName, argv[2], &hints, &result);
    if (rc != 0)
    {
        cerr << "ERROR:" << gai_strerror(rc) << endl;
    }

    /*
     * Iterate through addresses and connect
     * This cycles through results to see if any of them are connectable
     */
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

        if (clientSD == -1)
        {
            continue;
        }
        /*
         * A socket has been successfully created
         */
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
        }
        else
        { // success
            break;
        }
    }

    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Client Socket: " << clientSD << endl;
    }

    freeaddrinfo(result);

    /*
     * Write and read data over network
     */
    char databuf[nbufs][bufsize];

    //send repqtition to server
    write(clientSD, &repetition, sizeof(repetition));

    auto start = chrono::steady_clock::now();
    for (int i = 0; i < repetition; i++)
    {
        if (type == 1)
        {
            for (int j = 0; j < nbufs; j++)
            {
                write(clientSD, databuf[j], bufsize);
            }
        }
        if (type == 2)
        {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++)
            {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev(clientSD, vector, nbufs);
        }
        if (type == 3)
        {
            write(clientSD, databuf, (nbufs * bufsize));
        }
    }

    auto end = chrono::steady_clock::now();
    int nreads;

    read(clientSD, &nreads, nbufs);

    //cout << nreads << endl;
    int time = 0;
    time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    int timeS = 0;
    timeS = chrono::duration_cast<chrono::seconds>(end - start).count();
    //cout << "Time in second: " << timeS << endl;
    double throughput = 0.0;

    //for the test
    // cout << nbufs * bufsize << endl;
     //cout << "size in Gb: " << 1500 * 8 * 0.000000001 << endl;
     //cout << "convert time to sec: " << time * 0.000001 << endl;
     //cout << "throughput in bps: " << (repetition * 1500.0 * 8.0) / (time * 0.000001) << endl;
     //cout << time / 1000000 << endl;
    throughput = ((repetition * 1500.0 * 8.0 * 0.000000001) / (time * 0.000001));
    cout << "Test #" << type << ": time = " << time << " µsec, #reads = " << nreads << ", throughput " << throughput << " Gbps" << endl;

    // close client socket
    close(clientSD);
    return 0;
}