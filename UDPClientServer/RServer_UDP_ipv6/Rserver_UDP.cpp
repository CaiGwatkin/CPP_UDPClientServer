//************************************************************************/
//RUN WITH: Rserver_UDP 1235 0 0
//RUN WITH: Rserver_UDP 1235 1 0 
//RUN WITH: Rserver_UDP 1235 0 1 
//RUN WITH: Rserver_UDP 1235 1 1  
//************************************************************************/

#include "Rserver_UDP.h"


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

    struct addrinfo *serverAddrInfo = NULL;                                             // The address information of the server.
    error = getThisServersAddressInfo(serverAddrInfo, argv);                            // Get the address information of this server from inputted arguments.
    if (error)                                                                          // Check for error.
    {
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    SOCKET s = INVALID_SOCKET;                                                          // The server's socket.
    error = createServersSocket(s, serverAddrInfo);                                     // Create the server's socket using address information.
    if (error)                                                                          // Check for error.
    {
        freeaddrinfo(serverAddrInfo);                                                   // Free memory.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    error = bindServersSocket(s, serverAddrInfo);                                       // Bind the server's socket to the inputted port.
    if (error)                                                                          // Check for error.
    {
        freeaddrinfo(serverAddrInfo);                                                   // Free memory.
        closesocket(s);                                                                 // Close the socket.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    freeaddrinfo(serverAddrInfo);                                                       // Free memory.

    randominit();                                                                       // Initialise the randomiser.

    std::cout << "=================<< UDP SERVER >>=================" << std::endl;     // Alert user.
    std::cout << "channel can damage packets = " << packets_damagedbit << std::endl;     // Alert user.
    std::cout << "channel can lose packets = " << packets_lostbit << std::endl;          // Alert user.

    error = receiveFile(s);                                                             // Receives the packets from the user and saves them to file.
    if (error)                                                                          // Check for error.
    {
        closesocket(s);                                                                 // Close the socket.
        WSACleanup();                                                                   // Clean up winsock.
        return error;                                                                   // End program with error code.
    }

    closesocket(s);                                                                     // Close the socket.
    WSACleanup();                                                                       // Clean up winsock.

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
    if (argc != 4)                                                                      // Check if user inputted incorrect argument number.
    {
        std::cout << "USAGE: Rserver_UDP localport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)" << std::endl;    // Alert user.
        return 1;                                                                       // Return error code.
    }

    if (packets_damagedbit < 0 || packets_damagedbit > 1 || packets_lostbit < 0 || packets_lostbit > 1) // Check if values of args are not as expected.
    {
        std::cout << "USAGE: Rserver_UDP localport allow_corrupted_bits(0 or 1) allow_packet_loss(0 or 1)" << std::endl;    // Alert user.
        return 2;                                                                       // Return error code.
    }

    packets_damagedbit = atoi(argv[2]);                                                  // Argument determining if bits should be randomly damaged.
    packets_lostbit = atoi(argv[3]);                                                     // Argument determining if bits should be randomly lost.

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

// Gets the address information of this server from inputted arguments; returns error.
int getThisServersAddressInfo(struct addrinfo * &serverAddrInfo, char *argv[])
{
    char portNum[NI_MAXSERV];                                                           // The port number which the client should be on.
    sprintf(portNum, "%s", argv[1]);                                                    // Get the port number from the arguments.

    struct addrinfo hints;                                                              // Stores the hints for the address info structure to be created.
    memset(&hints, 0, sizeof(struct addrinfo));                                         // Ensure blank.

    hints.ai_family = AF_INET6;                                                         // Use IPv6.
    hints.ai_socktype = SOCK_DGRAM;                                                     // Use UDP socket type.
    hints.ai_protocol = IPPROTO_UDP;                                                    // Use UDP.
    hints.ai_flags = AI_PASSIVE;                                                        // For wildcard IP address. 

    int iResult = getaddrinfo(NULL, portNum, &hints, &serverAddrInfo);                  // Get address information.

    if (iResult)                                                                        // Check for error.
    {
        std::cout << "getaddrinfo() failed: " << iResult << std::endl;                  // Alert user.
        return 4;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Creates the server's socket using address information; returns error.
int createServersSocket(SOCKET &s, struct addrinfo *serverAddrInfo)
{
    s = socket(serverAddrInfo->ai_family, serverAddrInfo->ai_socktype, serverAddrInfo->ai_protocol);    // Create the socket based on the server address info structure.

    if (s == INVALID_SOCKET)                                                            // Check for error.
    {
        std::cout << "Socket failed" << std::endl;                                      // Alert user.
        return 5;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Bind the server's socket to the inputted port; returns error.
int bindServersSocket(SOCKET s, struct addrinfo *serverAddrInfo)
{
    int iResult = bind(s, serverAddrInfo->ai_addr, (int)serverAddrInfo->ai_addrlen);    // Bind socket to address.

    if (iResult == SOCKET_ERROR)                                                        // Check if socket error occurred.
    {
        std::cout << "bind failed with error: " << WSAGetLastError() << std::endl;      // Alert user.
        return 6;                                                                       // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Receives the packets from the user and saves them to file; returns error.
int receiveFile(SOCKET s)
{
    FILE *fout = fopen("data_received.txt", "w");                                       // Open the file for writing.

    if (fout == NULL)                                                                   // Check if file could not open successfully.
    {
        std::cout << "Cannot open data_received.txt" << std::endl;                      // Alert user.
        return 7;                                                                       // Return error code.
    }

    std::cout << "data_received.txt is now open for receiving" << std::endl;            // Alert user.

    int numOfPackets = 0;
    bool finishedReceiving = false;                                                     // To end while loop.
    std::vector<char *> receivedData;                                                   // The data structure storing lines yet to be saved to file.
    for (int i = 0; i < MAX_NUMBER_OF_LINES; i++)
    {
        receivedData.push_back(new char[BUFFER_SIZE]());
    }

    while (!finishedReceiving)
    {
        int error = receivePacket(s, fout, receivedData, numOfPackets, finishedReceiving);  // Loop while packets left to receive.
        if (error)                                                                      // Check for error.
        {
            return error;                                                               // Return error code.
        }
    }

    return 0;                                                                           // Return no error.
}

// Receives the packets from the client and saves them to file; returns error.
int receivePacket(SOCKET s, FILE *fout, std::vector<char *> &receivedData, int &numOfPackets, bool &finishedReceiving)
{
    struct sockaddr_storage clientAddress;                                              // IPv4 & IPv6-compliant client address storage.
    int addrlen = sizeof(clientAddress);                                                // Length of client address.

    char receiveBuffer[BUFFER_SIZE];                                                    // The bytes received from the client.
    memset(receiveBuffer, 0, sizeof(receiveBuffer));                                    // Ensure blank.

    int bytes = recvfrom(s, receiveBuffer, SEGMENT_SIZE, 0, (struct sockaddr*) &clientAddress, &addrlen);   // Receive packet from client.
    if (bytes == SOCKET_ERROR)                                                          // Check if socket error while receiving.
    {
        std::cout << "recvfrom() returned error" << std::endl;                          // Alert user.
        return 8;                                                                       // Return error code.
    }
   
    char clientHost[NI_MAXHOST];                                                        // The client's IP address.
    char clientService[NI_MAXSERV];                                                     // The client's port number.
    memset(clientHost, 0, sizeof(clientHost));                                          // Ensure blank.
    memset(clientService, 0, sizeof(clientService));                                    // Ensure blank.

    getnameinfo((struct sockaddr *) &clientAddress, addrlen,
                clientHost, sizeof(clientHost),
                clientService, sizeof(clientService),
                NI_NUMERICHOST);                                                        // Get the client's IP address and port number.

    std::cout << "\n==================================================" << std::endl;   // DEBUG: Alert user.
    std::cout << "Received a packet of size " << bytes;                                 // DEBUG: Alert user.
    std::cout << " bytes from <<<UDP Client>>> with IP address: " << clientHost;        // DEBUG: Alert user.
    std::cout << ", at Port: " << clientService << std::endl;                           // DEBUG: Alert user.
    std::cout << "--->" << receiveBuffer << int(strlen(receiveBuffer)) << " elements" <<"\n" << std::endl;  // DEBUG: Alert user.

    if (bytes > 0)                                                                      // Check if non-empty packet received.
    {
        processPacket(receiveBuffer, bytes);                                            // Process the received request from server.
    }
    else if (bytes == 0)                                                                // Check if packet received was empty.
    {
        std::cout << "Empty packet received." << std::endl;                             // Alert user.
        return 9;                                                                       // Return error code.
    }

    int error = acknowledgePacket(s, receiveBuffer, clientAddress, fout, receivedData, numOfPackets, finishedReceiving);    // Send an acknowledgement to the client if required.
    if (error)                                                                          // Check for error.
    {
        return error;                                                                   // Return error code.
    }

    return 0;                                                                           // Return no error.
}

// Process the packet received from the client.
void processPacket(char *receiveBuffer, int bytes)
{
    int i = 0;                                                                          // The index of the receive buffer.
    while (i < bytes)
    {
        if (receiveBuffer[i] == '\n')                                                   // Check if line-feed character.
        {
            receiveBuffer[i] = '\0';                                                    // Cap string.
            break;                                                                      // End processing.
        }

        else if (receiveBuffer[i] == '\r')                                              // Check if carriage return character.
        {
            receiveBuffer[i] = '\0';                                                    // Cap string.
        }

        i++;                                                                            // Increment index.
    }
}

// Send an acknowledgement to the client if necessary.
int acknowledgePacket(SOCKET s, char *receiveBuffer, struct sockaddr_storage clientAddress, FILE *fout, std::vector<char *> &receivedData, int &numOfPackets, bool &finishedReceiving)
{
    char receiveBufferNoChecksum[BUFFER_SIZE];                                          // Buffer used to check packet checksum.
    removeChecksum(receiveBuffer, receiveBufferNoChecksum);                             // Copy receive buffer so that original receive buffer not modified.
    unsigned int newChecksum = CRCpolynomial(receiveBufferNoChecksum);                  // Create checksum from tempBuffer.

    unsigned int checksum = 0;                                                          // Stores the packet's checksum value.
    int packetNumber = 0;                                                               // Stores the packet number.
    char *command = NULL;                                                               // Stores the command.
    char *data = NULL;                                                                  // Stores the data.
    extractTokens(receiveBuffer, checksum, command, packetNumber, data);                // Extract tokens from receive buffer.

    if (newChecksum != checksum)                                                        // Check that checksum is correct.
    {
        std::cout << "Checksum not equal, no ACK sent." << std::endl;                   // DEBUG: Alert user.
        return 0;                                                                       // Do nothing but return no error.
    }
    else if (isPACKET(command))                                                         // Check if client sent valid packet.
    {
        char sendBuffer[BUFFER_SIZE];                                                   // The buffer to store data for sending.
        createPacket(sendBuffer, packetNumber);                                         // Create acknowledgement packet to send to client.
        
        send_unreliably(s, sendBuffer, (sockaddr*) &clientAddress);                     // Send acknowledgement unrealiably.
        
        saveData(receiveBuffer, receivedData[packetNumber]);                            // Save line to memory.
        numOfPackets = packetNumber + 1;                                                // Update number of packets.
    }
    else if (isCLOSE(command))                                                          // Check if client sent CLOSE.
    {
        char sendBuffer[BUFFER_SIZE];                                                   // The buffer to store data for sending.
        createClosePacket(sendBuffer);                                                  // Create acknowledgement packet to send to client.
        
        send_unreliably(s, sendBuffer, (sockaddr*) &clientAddress);                     // Send acknowledgement unrealiably.

        int error = saveDataToFile(fout, receivedData, numOfPackets);                   // Save data to file.
        if (error)                                                                      // Check for error.
        {
            return error;                                                               // Return error code.
        }

        std::cout << "\n==================================================" << std::endl;   // DEBUG: Alert user.
        std::cout << "Server saved data_received.txt\n" << std::endl;                   // DEBUG: Alert user.

        finishedReceiving = true;                                                       // Finished receiving packets.
    }
    else                                                                                // Something other than expected was received.
    {
        // Do nothing with corrupted packets.
    }

    return 0;                                                                           // Return no error.
}

// Modified code from saveLineWithoutHeader function that was provided in startup code.
// Removes the checksum from the receive buffer.
void removeChecksum(char *receiveBuffer, char *dataExtracted)
{
    char tempBuffer[BUFFER_SIZE];                                                       // A temporary character buffer.
    strcpy(tempBuffer, receiveBuffer);                                                  // Copy receive buffer into temp buffer.

    char sep[3];                                                                        // The seperator character.

    strcpy(sep," ");                                                                    // Separator is the space character.
    char *word = NULL;                                                                  // Stores the word returned by strtok.
    int wordCount = 0;                                                                  // Counts the number of words.

    strcpy(dataExtracted, "\0");                                                        // Ensure blank.

    for (word = strtok(tempBuffer, sep); word; word = strtok(NULL, sep))                // Loop while word is not null, getting each word in the receive buffer.
    {
        wordCount++;                                                                    // Increment word count.

        if (wordCount > 1)                                                              // Check that current word is not part of header.
        {
            if (wordCount > 2)                                                          // Check if second word or greater.
            {
                strcat(dataExtracted, " ");                                             // Append space.
            }
            strcat(dataExtracted, word);                                                // Extract the word and store it as part of the data.
        }
    }
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

// Returns true if buffer begins with "PACKET".
bool isPACKET(char *charBuffer)
{
    return !strncmp(charBuffer, "PACKET", 6);                                           // Return true if buffer is "PACKET".
}

// Adds header fields to character buffer to create a packet ready for sending.
void createPacket(char *sendBuffer, int packetCounter)
{
    sprintf(sendBuffer, "ACK %d", packetCounter);                                       // Create acknowledgement of packet.

    char tempBuffer[BUFFER_SIZE];                                                       // Buffer used to create packet.
    sprintf(tempBuffer, "%d ", CRCpolynomial(sendBuffer));                              // Create CRC.

    strcat(tempBuffer, sendBuffer);                                                     // Append data to packet header.
    strcpy(sendBuffer, tempBuffer);                                                     // Create the complete packet.

    strcat(sendBuffer, "\r\n");                                                         // Add return and line-feed characters.
}

// Saves data to file; returns error.
void saveData(char *receiveBuffer, char *receivedData)
{
    char tempBuffer[BUFFER_SIZE];                                                       // A temporary character buffer.
    strcpy(tempBuffer, receiveBuffer);                                                  // Copy receive buffer into temp buffer.

    char sep[3];                                                                        // The seperator character.

    strcpy(sep," ");                                                                    // Separator is the space character.
    char *word;                                                                         // Stores the word returned by strtok.
    int wordCount = 0;                                                                  // Counts the number of words.
    char dataExtracted[BUFFER_SIZE] = "\0";                                             // Stores the extracted strings.

    for (word = strtok(tempBuffer, sep); word; word = strtok(NULL, sep))                // Loop while word is not null, getting each word in the receive buffer.
    {
        wordCount++;

        if(wordCount > NUMBER_OF_WORDS_IN_THE_HEADER)                                   // Check that current word is not part of header.
        {
            if (wordCount > NUMBER_OF_WORDS_IN_THE_HEADER + 1)                          // Check if second word of data or greater.
            {
                strcat(dataExtracted, " ");                                             // Append space.
            }
            strcat(dataExtracted, word);                                                // Extract the word and store it as part of the data.
        }
    }

    strcpy(receivedData, dataExtracted);                                                // Save data.
}

// Returns true if buffer is "CLOSE".
bool isCLOSE(char *charBuffer)
{
    return !strncmp(charBuffer, "CLOSE", 5);                                            // Return true if buffer is "CLOSE".
}

// Creates a packet containing "CLOSE".
void createClosePacket(char *sendBuffer)
{
    strcpy(sendBuffer, "CLOSE");                                                        // Create acknowledgement of CLOSE packet.

    char tempBuffer[BUFFER_SIZE];                                                       // Buffer used to create packet.
    sprintf(tempBuffer, "%d ", CRCpolynomial(sendBuffer));                              // Create CRC.

    strcat(tempBuffer, sendBuffer);                                                     // Append data to packet header.
    strcpy(sendBuffer, tempBuffer);                                                     // Create the complete packet.

    strcat(sendBuffer, "\r\n");                                                         // Add return and line-feed characters.
}

// Saves data from memory into file; returns error.
int saveDataToFile(FILE *fout, std::vector<char *> receivedData, int numOfPackets)
{
    if (fout != NULL)                                                                   // Check if file pointer is legitimate.
    {
        for (int i = 0; i < numOfPackets; i++)                                          // Loop through all stored packets of data.
        {
            if (i > 0)                                                                  // Check if not first line.
            {
                fprintf(fout, "\n%s", receivedData[i]);                                 // Write line to file.
            }
            else                                                                        // First line.
            {
                fprintf(fout, "%s", receivedData[i]);                                   // Write line to file.
            }
        }

        fclose(fout);                                                                   // Close the file.
    }
    else                                                                                // File pointer illegitimate.
    {
        std::cout << "Error in writing to file..." << std::endl;                        // Alert user.
        return 10;                                                                      // Return error.
    }

    return 0;                                                                           // Return no error.
}

