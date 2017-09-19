//************************************************************************/
// RUN WITH: Rclient_UDP localhost 1235 0 0 
// RUN WITH: Rclient_UDP localhost 1235 0 1 
// RUN WITH: Rclient_UDP localhost 1235 1 0 
// RUN WITH: Rclient_UDP localhost 1235 1 1 
//************************************************************************/
#include "Rclient_UDP.h"

// Main function, handles everything.
int main(int argc, char *argv[])
{
    int error = checkUserArgs(argc, argv);                                              // Check that user has inputted expected arguments.
    if (error)                                                                          // Check for error.
    {
        return error;                                                                   // End program with error code.
    }

    error = startWSA();                                                                 // Start winsock.
    if (error)                                                                          // Check for error.
    {
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    struct addrinfo *clientAddrInfo = NULL;                                             // The address information of the client.
    error = getThisClientsAddressInfo(clientAddrInfo, argv);                            // Get the address information of this client from inputted arguments.
    if (error)                                                                          // Check for error.
    {
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    SOCKET s = INVALID_SOCKET;                                                          // The client's socket.
    error = createClientsSocket(s, clientAddrInfo);                                     // Create the client's socket using address information.
    if (error)                                                                          // Check for error.
    {
        freeaddrinfo(clientAddrInfo);                                                   // Free memory.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    error = setIOModeOfSocket(s);                                                       // Set the IO mode of the socket so that non-blocking is enabled.
    if (error)                                                                          // Check for error.
    {
        freeaddrinfo(clientAddrInfo);                                                   // Free memory.
        closesocket(s);                                                                 // Close the socket.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    randominit();                                                                       // Initialise the randomiser.

    std::cout << "=================<< UDP CLIENT >>=================" << std::endl;     // Alert user.
    std::cout << "Channel can damage packets = " << packets_damagedbit << std::endl;     // Alert user.
    std::cout << "Channel can lose packets = " << packets_lostbit << std::endl;          // Alert user.

    error = sendFile(s, clientAddrInfo);                                                // Simulate sending a file over an unreliable UDP connection.
    if (error)                                                                          // Check for error.
    {
        freeaddrinfo(clientAddrInfo);                                                   // Free memory.
        closesocket(s);                                                                 // Close the socket.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    freeaddrinfo(clientAddrInfo);                                                       // Free memory.
    closesocket(s);                                                                     // Close the socket.
    WSACleanup();                                                                       // Clean up winsock.

    std::cout << "\nClosing the socket connection and Exiting...\n" << std::endl;       // Alert user.
    std::cout << "=================<< STATISTICS >>=================" << std::endl;     // Alert user.
    std::cout << "numOfPacketsDamaged = " << numOfPacketsDamaged << std::endl;          // Alert user.
    std::cout << "numOfPacketsLost = " << numOfPacketsLost << std::endl;                // Alert user.
    std::cout << "numOfPacketsUncorrupted = " << numOfPacketsUncorrupted << std::endl;  // Alert user.
    std::cout << "==================================================" << std::endl;     // Alert user.

    return 0;                                                                           // Return program finished without error.
}

// Checks that user has inputted expected arguments; returns error.
int checkUserArgs(int argc, char *argv[])
{
    if (argc != 5)                                                                      // Check if user inputted incorrect argument number.
    {
        std::cout << "USAGE: Rclient_UDP remote_IP-address remoteport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)" << std::endl; // Alert user.
        return 1;                                                                       // Return error code.
    }

    if (packets_damagedbit < 0 || packets_damagedbit > 1 || packets_lostbit < 0 || packets_lostbit > 1) // Check if values of args are not as expected.
    {
        std::cout << "USAGE: Rclient_UDP remote_IP-address remoteport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)" << std::endl; // Alert user.
        return 2;                                                                       // Return error code.
    }

    packets_damagedbit = atoi(argv[3]);                                                  // Argument determining if bits should be randomly damaged.
    packets_lostbit = atoi(argv[4]);                                                     // Argument determining if bits should be randomly lost.

    return 0;                                                                           // Return no error.
}

// Starts winsock; returns error.
int startWSA()
{
    WSADATA wsadata;                                                                    // To store data about winsock.

    if (WSAStartup(WSVERS, &wsadata) != 0)                                              // Check for error.
    {
        std::cout << "WSAStartup failed" << std::endl;                                  // Alert user.
        return 3;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Gets the address information of this client from inputted arguments; returns error.
int getThisClientsAddressInfo(struct addrinfo * &clientAddrInfo, char *argv[])
{
    char portNum[NI_MAXSERV];                                                           // The port number which the client should be on.
    sprintf(portNum, "%s", argv[2]);                                                    // Get the port number from the arguments.

    struct addrinfo hints;                                                              // Stores the hints for the address info structure to be created.
    memset(&hints, 0, sizeof(struct addrinfo));                                         // Ensure blank.

    hints.ai_family = AF_INET6;                                                         // Use IPv6.
    hints.ai_socktype = SOCK_DGRAM;                                                     // Use UDP socket type.
    hints.ai_protocol = IPPROTO_UDP;                                                    // Use UDP.

    int iResult = getaddrinfo(argv[1], portNum, &hints, &clientAddrInfo);               // Get address information.

    //freeaddrinfo(&hints);                                                               // Free memory.

    if (iResult)                                                                        // Check for error.
    {
        std::cout << "getaddrinfo() failed: " << iResult << std::endl;                  // Alert user.
        return 4;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Creates the client's socket using address information; returns error.
int createClientsSocket(SOCKET &s, struct addrinfo *clientAddrInfo)
{
    s = socket(clientAddrInfo->ai_family, clientAddrInfo->ai_socktype, clientAddrInfo->ai_protocol);    // Create the socket based on the client address info structure.

    if (s == INVALID_SOCKET)                                                            // Check for error.
    {
        std::cout << "Socket failed" << std::endl;                                      // Alert user.
        return 5;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Sets the IO mode of the socket so that non-blocking is enabled; returns error.
int setIOModeOfSocket(SOCKET &s)
{
    u_long iMode = 1;                                                                   // Set so that non-blocking mode is enabled.

    int iResult = ioctlsocket(s, FIONBIO, &iMode);                                      // Set the IO mode of the socket.

    if (iResult != NO_ERROR)                                                            // Check for error.
    {
        std::cout << "ioctlsocket failed with error: " << iResult << std::endl;         // Alert user.
        return 6;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Simulate sending a file over an unreliable UDP connection; returns error.
int sendFile(SOCKET s, struct addrinfo *clientAddrInfo)
{
    FILE *fin = fopen("data_for_transmission.txt", "rb");                               // Open file for transmission.

    if (fin == NULL)                                                                    // Check if file could not open successfully.
    {
        std::cout << "Cannot open data_for_transmission.txt" << std::endl;              // Alert user.
        return 7;                                                                       // Return error code.
    }

    std::cout << "data_for_transmission.txt is now open for sending" << std::endl;      // Alert user.

    int packetCounter = 0;                                                              // Counts the number of packets sent.
    bool finishedSending = false;                                                       // To end while loop.

    while (!finishedSending)                                                            // Loop while packets left to send.
    {
        int error = sendToServer(s, clientAddrInfo, fin, packetCounter, finishedSending);   // Sends packets containig file to server.
        if (error)                                                                      // Check for error.
        {
            return error;                                                               // Return error code.
        }
    }

    return 0;                                                                           // Return no error.
}

// Sends packets containig file to server; returns error.
int sendToServer(SOCKET s, struct addrinfo *clientAddrInfo, FILE *fin, int &packetCounter, bool &finishedSending)
{
    char sendBuffer[BUFFER_SIZE];                                                       // Buffer containing packet to be sent.
    memset(sendBuffer, 0, sizeof(sendBuffer));                                          // Ensure blank.

    if (!feof(fin))                                                                     // Check that end of file not reached.
    {
        fgets(sendBuffer, SEGMENT_SIZE, fin);                                           // Get one line of data from the file.

        createPacket(sendBuffer, packetCounter);                                        // Add header to send buffer to create packet.
        sendPacket(s, clientAddrInfo, sendBuffer, packetCounter);                       // Send packet to server.
    }
    else                                                                                // End of file.
    {
        fclose(fin);                                                                    // Close file.

        std::cout << "\n==================================================" << std::endl;   // DEBUG: Alert user.
        std::cout << "End-of-File reached." << std::endl;                               // DEBUG: Alert user.
        std::cout << "\n==================================================" << std::endl;   // DEBUG: Alert user.

        sendClose(s, clientAddrInfo);                                                   // Close the connection with the server.

        finishedSending = true;                                                         // Finished sending packets.
    }

    return 0;
}

// Adds header fields to character buffer to create a packet ready for sending.
void createPacket(char *sendBuffer, int packetCounter)
{
    char tempBuffer[BUFFER_SIZE];                                                       // Buffer used to create packet.
    sprintf(tempBuffer, "PACKET %d ", packetCounter);                                   // Create packet header with sequence number.

    strcat(tempBuffer, sendBuffer);                                                     // Append data to packet header.
    strcpy(sendBuffer, tempBuffer);                                                     // Create the complete packet.

    processPacket(sendBuffer, strlen(sendBuffer));                                      // Remove any return/line-feed characters.

    sprintf(tempBuffer, "%d ", CRCpolynomial(sendBuffer));                              // Create CRC.

    strcat(tempBuffer, sendBuffer);                                                     // Append data to packet header.
    strcpy(sendBuffer, tempBuffer);                                                     // Create the complete packet.

    strcat(sendBuffer, "\r\n");                                                         // Add return and line-feed characters.
}

// From support utilities provided by N. H. Reyes in 159.334.
// Returns the CRC of the passed character buffer.
unsigned int CRCpolynomial(char *buffer)
{
    unsigned int rem = 0x0000;
    unsigned int bufferSize = strlen(buffer);
    
    while (bufferSize-- != 0)
    {
        for (unsigned char i = 0x80; i != 0; i /= 2)
        {
            if ((rem & 0x8000) != 0)
            {
                rem = rem << 1;
                rem ^= GENERATOR;
            }
            else
            {
                rem = rem << 1;
            }
            if ((*buffer & i) != 0)
            {
               rem ^= GENERATOR;
            }
        }
        buffer++;
    }

    rem = rem & 0xffff;
    return rem;
}

// Sends buffer to address; returns true if acknowledgement received.
void sendPacket(SOCKET s, struct addrinfo *clientAddrInfo, char *sendBuffer, int &packetCounter)
{
    std::cout << "\n==================================================" << std::endl;   // DEBUG: Alert user.
    std::cout << "calling send_unreliably, to deliver data of size " << strlen(sendBuffer) << std::endl;    // DEBUG: Alert user.

    bool ackReceived = false;                                                           // True when acknowledgement received.

    while (!ackReceived)                                                                // Loop while no acknowledgement received.
    {
        send_unreliably(s, sendBuffer, (clientAddrInfo->ai_addr));                      // Send the packet over the unreliable data channel.
        Sleep(10);                                                                      // Sleep for 10 milliseconds.

        char receiveBuffer[BUFFER_SIZE];                                                // Buffer to receive data in.
        memset(receiveBuffer, 0, sizeof(receiveBuffer));                                // Ensure blank.

        int bytes = 0;                                                                  // Stores the number of bytes in the received packet.
        bool replyExpected = true;                                                      // True when a reply is expected.

        while (replyExpected)
        {
            replyExpected = false;                                                      // Don't expect another reply, resend packet.

            if (receiveReply(s, receiveBuffer, bytes))                                  // Receive reply from server.
            {
                std::cout << "--->"<< receiveBuffer << int(strlen(receiveBuffer)) << " elements" << std::endl;  // DEBUG: Alert user.

                if (bytes > 0)                                                          // Check if non-empty packet received.
                {
                    processPacket(receiveBuffer, bytes);                                // Remove trailing carriage return and end-line from packet.

                    unsigned int checksum = 0;                                          // Stores the packet's checksum value.
                    int packetNumber = 0;                                               // Stores the packet number.
                    char *command = NULL;                                               // Stores the command.
                    char *data = NULL;                                                  // Stores the data.
                    extractTokens(receiveBuffer, checksum, command, packetNumber, data);    // Extract tokens from receive buffer.

                    if (isChecksumValid(receiveBuffer) && isACK(command))               // Check if received packet is ACK.
                    {
                        if (packetNumber == packetCounter)                              // Check if ACK number is last sent packet.
                        {
                            packetCounter++;                                            // Increment packet counter.
                            ackReceived = true;                                         // Expected acknowledgement was received.
                        }
                        else if (packetNumber == packetCounter - 1)                     // Check if ACK number is duplicate.
                        {
                            replyExpected = true;                                       // Check if another reply is waiting in transport layer.
                        }
                    }
                }
            }
        }
    }
}

// Process the packet received from the server.
void processPacket(char *charBuffer, int bytes)
{
    int i = 0;                                                                          // The index of the receive buffer.
    while (i < bytes)
    {
        if (charBuffer[i] == '\n')                                                      // Check if line-feed character.
        {
            charBuffer[i] = '\0';                                                       // Cap string.
            break;                                                                      // End processing.
        }

        else if (charBuffer[i] == '\r')                                                 // Check if carriage return character.
        {
            charBuffer[i] = '\0';                                                       // Cap string.
        }

        i++;                                                                            // Increment index.
    }
}

// From support utilities provided by N. H. Reyes in 159.334.
// Extracts the headers and data from a given string.
void extractTokens(char *str, unsigned int &CRC, char *&command, int &packetNumber, char *&data)
{
    char tempBuffer[BUFFER_SIZE];                                                       // A temporary character buffer.
    strcpy(tempBuffer, str);                                                            // Copy str into temp buffer.

    char * pch;
    int tokenCounter = 0;

    while (1)
    {
        if (tokenCounter == 0)
        { 
            pch = strtok(tempBuffer, " ");
        }
        else
        {
            pch = strtok(NULL, " ");
        }
        if (pch == NULL)
        {
            break;
        }
     
        switch(tokenCounter)
        {
            case 0:
                CRC = atoi(pch);
                break;
            case 1:
                command = new char[strlen(pch)];
                strcpy(command, pch);
                break;            
            case 2:
                packetNumber = atoi(pch);
                break;
            case 3:
                data = new char[strlen(pch)];
                strcpy(data, pch);
                break;            
        }   
      
        tokenCounter++;
    }
}
/*
// Returns true if checksum is correct.
bool isChecksumValid(char *charBuffer, unsigned int checksum)
{
    char charBufferNoChecksum[BUFFER_SIZE];                                             // Buffer used to check packet checksum.
    removeChecksum(charBuffer, charBufferNoChecksum);                                   // Remove checksum from char buffer.
    unsigned int newChecksum = CRCpolynomial(charBufferNoChecksum);                     // Create new checksum from tempBuffer.

    return checksum == newChecksum;                                                     // Return true if checksums match.
}*/

// Returns true if checksum is correct.
bool isChecksumValid(char *charBuffer)
{
    char charBufferNoChecksum[BUFFER_SIZE];                                             // Buffer used to check packet checksum.
    unsigned int checksum = removeChecksum(charBuffer, charBufferNoChecksum);           // Remove checksum from char buffer and save value.
    unsigned int newChecksum = CRCpolynomial(charBufferNoChecksum);                     // Create new checksum from tempBuffer.

    return checksum == newChecksum;                                                     // Return true if checksums match.
}

// Modified code from saveLineWithoutHeader function that was provided in startup code.
// Removes the checksum from the receive buffer; returns the checksum value.
unsigned int removeChecksum(char *receiveBuffer, char *dataExtracted)
{
    char tempBuffer[BUFFER_SIZE];                                                       // A temporary character buffer.
    strcpy(tempBuffer, receiveBuffer);                                                  // Copy receive buffer into temp buffer.

    char sep[3];                                                                        // The seperator character.

    strcpy(sep, " ");                                                                   // Separator is the space character.
    char *word = NULL;                                                                  // Stores the word returned by strtok.
    char checksum[BUFFER_SIZE];                                                         // Stores the word returned by strtok.
    int wordCount = 0;                                                                  // Counts the number of words.

    strcpy(dataExtracted, "\0");                                                        // Ensure blank.

    for (word = strtok(tempBuffer, sep); word; word = strtok(NULL, sep))                // Loop while word is not null, getting each word in the receive buffer.
    {
        wordCount++;                                                                    // Increment word count.

        if (wordCount == 1)                                                             // Check if first word.
        {
            strcpy(checksum, word);                                                     // Copy the into checksum.
        }
        else                                                                            // Current word is not checksum.
        {
            if (wordCount > 2)                                                          // Check if second word or greater.
            {
                strcat(dataExtracted, " ");                                             // Append space.
            }
            strcat(dataExtracted, word);                                                // Extract the word and store it as part of the data.
        }
    }

    return atoi(checksum);                                                              // Return the checksum.
}

// Returns true if character buffer begins with ACK.
bool isACK(char *charBuffer)
{
    return !strncmp(charBuffer, "ACK", 3);                                              // Return true if buffer begins "ACK ".
}

// Receive reply from server; returns true if reply was received.
bool receiveReply(SOCKET s, char *receiveBuffer, int &bytes)
{
    struct sockaddr_storage serverAddress;                                              // Stores the address of the server.
    memset(&serverAddress, 0, sizeof(serverAddress));                                   // Ensure blank.

    int addrlen = sizeof(serverAddress);                                            // IPv4 & IPv6-compliant length of address.

    bytes = recvfrom(s, receiveBuffer, 78, 0, (struct sockaddr*) &serverAddress, &addrlen); // Receive data.
    if (bytes == SOCKET_ERROR)                                                          // Check if socket error while receiving.
    {
        return false;                                                                   // No reply received.
    }

    char serverHost[NI_MAXHOST];                                                        // Stores the server's IP address.
    char serverService[NI_MAXSERV];                                                     // Stores the server's port number.
    memset(serverHost, 0, sizeof(serverHost));                                          // Ensure blank.
    memset(serverService, 0, sizeof(serverService));                                    // Ensure blank.

    getnameinfo((struct sockaddr *) &serverAddress, addrlen,
                serverHost, sizeof(serverHost),
                serverService, sizeof(serverService),
                NI_NUMERICHOST);                                                        // Get the server's IP address and port number.

    std::cout << "\nReceived a packet of size " << bytes;                               // DEBUG: Alert user.
    std::cout << " bytes from <<<UDP Server>>> with IP address: " << serverHost;        // DEBUG: Alert user.
    std::cout << ", at Port: " << serverService << std::endl;                           // DEBUG: Alert user.

    return true;                                                                        // Return that reply was received.
}

// Closes the connection with the server.
void sendClose(SOCKET s, struct addrinfo *clientAddrInfo)
{
    char sendBuffer[BUFFER_SIZE];                                                       // Buffer containing packet to be sent.
    memset(sendBuffer, 0, sizeof(sendBuffer));                                          // Empty send buffer.

    sprintf(sendBuffer, "CLOSE");                                                       // Create a CLOSE command to send to the server.

    char tempBuffer[BUFFER_SIZE];                                                       // Buffer used to create packet.
    sprintf(tempBuffer, "%d ", CRCpolynomial(sendBuffer));                              // Create CRC.

    strcat(tempBuffer, sendBuffer);                                                     // Append data to packet header.
    strcpy(sendBuffer, tempBuffer);                                                     // Create the complete packet.
    strcat(sendBuffer, "\r\n");                                                         // Add return and line-feed characters.

    bool ackReceived = false;                                                           // True when acknowledgement received.

    while (!ackReceived)                                                                // Loop while no acknowledgement received.
    {
        send_unreliably(s, sendBuffer, (clientAddrInfo->ai_addr));                      // Send the packet over the unreliable data channel.
        Sleep(10);                                                                      // Sleep for 10 milliseconds.

        char receiveBuffer[BUFFER_SIZE];                                                // Buffer to receive data in.
        memset(receiveBuffer, 0, sizeof(receiveBuffer));                                // Ensure blank.

        int bytes = 0;                                                                  // Stores the number of bytes in the received packet.
        bool replyExpected = true;                                                      // True when a reply is expected.

        while (replyExpected)
        {
            replyExpected = false;                                                      // Don't expect another reply, resend packet.

            if (receiveReply(s, receiveBuffer, bytes))                                  // Receive reply from server.
            {
                std::cout << "--->"<< receiveBuffer << int(strlen(receiveBuffer)) << " elements" << std::endl;  // DEBUG: Alert user.

                if (bytes > 0)                                                          // Check if non-empty packet received.
                {
                    processPacket(receiveBuffer, bytes);                                // Remove trailing carriage return and end-line from packet.
                    char command[BUFFER_SIZE];
                    removeChecksum(receiveBuffer, command);

                    if (isChecksumValid(receiveBuffer) && isCLOSE(command))             // Check if received packet is CLOSE.
                    {
                        std::cout << "Acknowledgement of CLOSE command was received." << std::endl; // DEBUG: Alert user.
                        ackReceived = true;                                             // Expected acknowledgement was received.
                    }
                    else if (!isACK(command))                                           // Check if any reply was received that isn't an ACK.
                    {
                        std::cout << "Acknowledgement of CLOSE command was damaged, but sending will end anyway." << std::endl; // DEBUG: Alert user.
                        ackReceived = true;                                             // The CLOSE reply was probably damaged, but the reply was received so end connection.
                    }
                }
            }
        }
    }
}

// Returns true if character buffer is "CLOSE".
bool isCLOSE(char *charBuffer)
{
    return !strncmp(charBuffer, "CLOSE", 5);                                            // Return true if buffer is "CLOSE".
}

