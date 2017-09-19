#if defined __unix__ || defined __APPLE__
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <iostream>


#elif defined _WIN32
//Ws2_32.lib
#define _WIN32_WINNT 0x501  //to recognise getaddrinfo()

//"For historical reasons, the Windows.h header defaults to including the Winsock.h header file for Windows Sockets 1.1. The declarations in the Winsock.h header file will conflict with the declarations in the Winsock2.h header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro prevents the Winsock.h from being included by the Windows.h header"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <iostream>
#endif

#include "myrandomizer.h"

#define WSVERS MAKEWORD(2,0)
#define BUFFER_SIZE 80                                                                  // Has to be at least big enough to receive a packet
#define SEGMENT_SIZE 78                                                                 // Segment size, i.e., if fgets gets more than this number of bytes it segments the message into smaller parts.
#define GENERATOR 0x8005                                                                // For CRC polynomial generation.


int numOfPacketsDamaged = 0;                                                            // Stores the number of damaged packets; accessed by myrandomizer.h.
int numOfPacketsLost = 0;                                                               // Stores the number of lost packets; accessed by myrandomizer.h.
int numOfPacketsUncorrupted = 0;                                                        // Stores the number of uncorrupted packets; accessed by myrandomizer.h.
int packets_damagedbit = 0;                                                              // Stores the bit determining if damaged packets should occur; accessed by myrandomizer.h.
int packets_lostbit = 0;                                                                 // Stores the bit determining if lost packets should occur; accessed by myrandomizer.h.


int checkUserArgs(int argc, char *argv[]);                                              // Checks that user has inputted expected arguments; returns error.
int startWSA();                                                                         // Starts winsock; returns error.
int getThisClientsAddressInfo(struct addrinfo * &clientAddrInfo, char *argv[]);         // Gets the address information of this client from inputted arguments; returns error.
int createClientsSocket(SOCKET &s, struct addrinfo *clientAddrInfo);                    // Creates the client's socket using address information; returns error.
int setIOModeOfSocket(SOCKET &s);                                                       // Sets the IO mode of the socket so that non-blocking is enabled; returns error.
int sendFile(SOCKET s, struct addrinfo *clientAddrInfo);                                // Simulate sending a file over an unreliable UDP connection; returns error.
int sendToServer(SOCKET s, struct addrinfo *clientAddrInfo, FILE *fin, int &packetCounter, bool &finishedSending);  // Sends packets containig file to server; returns error.
void createPacket(char *sendBuffer, int packetCounter);                                 // Adds header fields to character buffer to create a packet ready for sending.
unsigned int CRCpolynomial(char *buffer);                                               // Returns the CRC of the passed character buffer.
void sendPacket(SOCKET s, struct addrinfo *clientAddrInfo, char *sendBuffer, int &packetCounter);   // Sends buffer to address; returns true if acknowledgement received.
void processPacket(char *receiveBuffer, int bytes);                                     // Process the packet received from the server.
void extractTokens(char *str, unsigned int &CRC, char *&command, int &packetNumber, char *&data);   // Extracts the headers and data from a given string.
bool isChecksumValid(char *charBuffer);                                                 // Returns true if checksum is correct.
unsigned int removeChecksum(char *receiveBuffer, char *dataExtracted);                  // Removes the checksum from the receive buffer; returns the checksum value.
bool isACK(char *charBuffer);                                                           // Returns true if character buffer begins with ACK.
bool receiveReply(SOCKET s, char *receive_buffer, int &bytes);                          // Receive reply from server; returns true if reply was received.
bool isCLOSE(char *charBuffer);                                                         // Returns true if character buffer is "CLOSE".
void sendClose(SOCKET s, struct addrinfo *clientAddrInfo);                              // Closes the connection with the server.

