#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define DEFAULT_PORT 100
#define DEFAULT_DATA_LENGTH 56
#define IPV4_HEADER_LEN 20
#define RECV_TIMEOUT 1 

unsigned short addOneComplement16Bit(unsigned short x, unsigned short y)
{
    unsigned int sum;
    sum = x + y;
    sum = (sum & 0xFFFF) + (sum >> 16);
    return (sum & 0xFFFF);
}

unsigned short calculateCheckSum(void* headerAddress, int length)
{   
    unsigned short checkSum = 0;
    unsigned short* pointer = (unsigned short*) headerAddress;
    for (int i = 0; i < length/2; i++)
    {
        checkSum = addOneComplement16Bit(checkSum, *pointer);
        pointer++;
    }
    if (1 == length%2)
    {
        // process remain odd byte 
        unsigned char* tempPointer  = (unsigned char*) pointer;
        unsigned short tempValue    = 0x0;
        tempValue         += (*tempPointer) << 8;
        checkSum           = addOneComplement16Bit(checkSum, tempValue);
    }
    return ~checkSum;
}

struct icmphdr* createICMPMessage(int sequenceNumber, char* dataAddress, int dataLength)
{
    int messageLength = dataLength + sizeof(struct icmphdr); // in bytes
    struct icmphdr* icmpMessage = (struct icmphdr*) dataAddress;
    memset(icmpMessage, 0x00, messageLength);
    // We are sending a ICMP_ECHO ICMP packet
    icmpMessage->type               = ICMP_ECHO;
    icmpMessage->code               = 0;
    icmpMessage->checksum           = 0;
    icmpMessage->un.echo.sequence   = htons(sequenceNumber);
    icmpMessage->un.echo.id         = getpid();
    icmpMessage->checksum           = calculateCheckSum(icmpMessage, messageLength);
    return icmpMessage;
}

int main(int argc, char **argv)
{
    // Init socket
    long numberOfPing;
    struct timeval begin, end, waitTime;
    double timeSpent;
    int pingSocket, addressLength;
    struct sockaddr_in sourceAddress;
    struct sockaddr_in destinationAddress;
    memset(&sourceAddress, 0, sizeof(struct sockaddr_in));
    memset(&destinationAddress, 0, sizeof(struct sockaddr_in));
    sourceAddress.sin_family = AF_INET;
    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port = htons(DEFAULT_PORT);
    addressLength = sizeof(destinationAddress);
    // take arg[1] as Ipv4 address
    if (0 == inet_aton(argv[1], &destinationAddress.sin_addr))
    {
        printf("Wrong IPv4 address, please put IPv4 as argument 1\n");
        exit(1);
    }

    if ( 3 == argc)
    {
        numberOfPing = atoi(argv[2]);
        if ( 1 > numberOfPing) 
        {
            printf("number of Ping must be a postive integer\n");
            exit(1);
        }
    }
    
    pingSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (-1 == pingSocket)
    {
        printf("Cannot create socket\n");
        exit(1);
    }
    if (-1 == getsockname(pingSocket, (struct sockaddr*)&sourceAddress, &addressLength))
    {
        printf("Cannot getsockname\n");
        exit(1);
    }
    waitTime.tv_sec = RECV_TIMEOUT;
    waitTime.tv_usec = 0;
    if (-1 == setsockopt(pingSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &waitTime, sizeof(struct timeval)))
    {
        printf("Error at set socket option");
	    exit(1);
    }

    // Init some buffer and variable for send mesasge and revice
    unsigned short sequenceNumber = 0;
    int icmpDataLength = DEFAULT_DATA_LENGTH;
    int icmpMessLength = icmpDataLength + sizeof(struct icmphdr);

    unsigned char* sentBuffer = (unsigned char*) calloc(icmpMessLength, sizeof(char));
    struct icmphdr* imcpHeader = (struct icmphdr*) sentBuffer; 
    
    unsigned char* recvBuffer = (unsigned char*)malloc(icmpMessLength + IPV4_HEADER_LEN);
    struct icmphdr* icmpReplyHeader = (struct icmphdr*)(recvBuffer + IPV4_HEADER_LEN);

    struct iphdr* ipHeader = (struct iphdr*)recvBuffer;

    int i;
    int pingIdx = 0;
    for (;;)
    {
        sleep(1);
        pingIdx++;
        createICMPMessage(pingIdx, sentBuffer, icmpMessLength);
        gettimeofday(&begin, NULL);
        // Send ECHO Message
        i = sendto(pingSocket, sentBuffer, icmpMessLength, 0, (struct sockaddr*)&destinationAddress, sizeof(destinationAddress));
        if ( 0 > i)
        {
            printf("Error in sendto\n\n");
            continue;
        }
        // Receive ECHO REPLY message
        inet_aton(argv[1], &destinationAddress.sin_addr);
        i = recvfrom(pingSocket, recvBuffer, icmpMessLength + IPV4_HEADER_LEN, 0, (struct sockaddr*)&destinationAddress, &addressLength);
        gettimeofday(&end, NULL);;
        if (0 > i)
        {
            printf("Error in recvfrom\n\n");
            continue;
        };

        if (! 0 == calculateCheckSum(icmpReplyHeader, icmpMessLength))
        {
            printf("Checksum status : BAD\n\n");
            continue;
        }

        if ((icmpReplyHeader->type == ICMP_ECHOREPLY) && 
            (((unsigned int)ipHeader->saddr == (unsigned int)destinationAddress.sin_addr.s_addr)))
        {
            timeSpent =   (double)(end.tv_usec - begin.tv_usec) / 1000 
                        + (double)(end.tv_sec - begin.tv_sec) * 1000;  // uint in ms
            printf("Reiveied ICMP_ECHOREPLY sequence number = %d\n", ntohs(icmpReplyHeader->un.echo.sequence));
            printf("Round trip time = %lf ms \n\n", timeSpent);
        } 
        else 
        {   
            if (icmpReplyHeader->type ==ICMP_DEST_UNREACH)
            {
                printf("Destination Unreachable\n");
            }

        }        
        if ( 0 == numberOfPing) continue;
        else
        {
            if (pingIdx == numberOfPing)
            {
                break;
            }
        }
    }
    
    // Release dynamic memory
    free(sentBuffer);
    free(recvBuffer);
    return 0;
}
