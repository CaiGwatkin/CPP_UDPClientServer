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
#include <vector>

#include "myrandomizer.h" 

#define BUFFER_SIZE 80  //used by receive_buffer and send_buffer
                        //the BUFFER_SIZE needs to be at least big enough to receive the packet
#define SEGMENT_SIZE 78
#define WSVERS MAKEWORD(2,0)
#define GENERATOR 0x8005                                                                // For CRC polynomial generation.

int numOfPacketsDamaged = 0;
int numOfPacketsLost = 0;
int numOfPacketsUncorrupted = 0;
int packets_damagedbit = 0;
int packets_lostbit = 0;

//*******************************************************************
//Function to save lines and discard the header
//*******************************************************************
//You are allowed to change this. You will need to alter the NUMBER_OF_WORDS_IN_THE_HEADER if you add a CRC
#define NUMBER_OF_WORDS_IN_THE_HEADER 3
#define MAX_NUMBER_OF_LINES 1000


int checkUserArgs(int argc, char *argv[]);                                              // Checks that user has inputted expected arguments; returns error.
int startWSA();                                                                         // Starts winsock; returns error.
int getThisServersAddressInfo(struct addrinfo * &serverAddrInfo, char *argv[]);         // Gets the address information of this server from inputted arguments; returns error.
int createServersSocket(SOCKET &s, struct addrinfo *serverAddrInfo);                    // Creates the server's socket using address information; returns error.
int bindServersSocket(SOCKET s, struct addrinfo *serverAddrInfo);                       // Bind the server's socket to the inputted port; returns error.
int receiveFile(SOCKET s);                                                              // Receives the packets from the client and saves them to file; returns error.
int receivePacket(SOCKET s, FILE *fout, std::vector<char *> &receivedData, int &numOfPackets, bool &finishedReceiving); // Receives the packets from the client and saves them to file; returns error.
void processPacket(char *receiveBuffer, int bytes);                                     // Process the packet received from the client.
int acknowledgePacket(SOCKET s, char *receiveBuffer, struct sockaddr_storage clientAddress, FILE *fout, std::vector<char *> &receivedData, int &numOfPackets, bool &finishedReceiving); // Send an acknowledgement to the client if necessary.
void removeChecksum(char *receiveBuffer, char *dataExtracted);                          // Removes the checksum from the receive buffer.
unsigned int CRCpolynomial(char *buffer);                                               // Returns the CRC of the passed character buffer.
void extractTokens(char *str, unsigned int &CRC, char *&command, int &packetNumber, char *&data);   // Extracts the headers and data from a given string.
bool isPACKET(char *charBuffer);                                                        // Returns true if buffer begins with "PACKET".
void createPacket(char *sendBuffer, int packetCounter);                                 // Adds header fields to character buffer to create a packet ready for sending.
void saveData(char *data, char *receivedData);                                          // Saves data to file; returns error.
bool isCLOSE(char *charBuffer);                                                         // Returns true if buffer is "CLOSE".
void createClosePacket(char *sendBuffer);                                               // Creates a packet containing "CLOSE".6
int saveDataToFile(FILE *fout, std::vector<char *> receivedData, int numOfPackets);     // Saves data from memory into file; returns error.
