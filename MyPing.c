#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define BILLION 1000000000L
#define DEFAULT_PORT 100
#define DEFAULT_DATA_LENGTH 56
#define IPV4_HEADER_LEN 20

unsigned short addOneComplement16Bit(unsigned short x, unsigned short y){
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
        unsigned char* tempPointer = (unsigned char*) pointer;
        unsigned short tempValue = 0x0;
        tempValue += (*tempPointer) << 8;
        checkSum = addOneComplement16Bit(checkSum, tempValue);
    }
    return ~checkSum;
}

struct icmphdr* createICMPMessage(int sequenceNumber, char* dataAddress, int dataLength)
{
    int messageLength = dataLength + sizeof(struct icmphdr); // in bytes
    struct icmphdr* icmpMessage = (struct icmphdr*) dataAddress;
    memset(icmpMessage, 0x00, messageLength);
    // We are sending a ICMP_ECHO ICMP packet
    icmpMessage->type = ICMP_ECHO;
    icmpMessage->code = 0;
    icmpMessage->checksum = 0;
    icmpMessage->un.echo.sequence = htons(sequenceNumber);
    // We don't need set the identifier becasue IPPROTO_ICMP socket will do it
    // compute ICMP checksum here
    icmpMessage->checksum = calculateCheckSum(icmpMessage, messageLength);
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
    // take arg[1] as Ipv4 address
    if (0 == inet_aton(argv[1], &destinationAddress.sin_addr))
    {
        printf("Wrong IPv4 address, please put IPv4 as argument 1\n");
        exit(1);
    }

    if ( 3 == argc)
    {
        numberOfPing = atoi(argv[2]);
    }
    else
    {
        numberOfPing = -1; 
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
    waitTime.tv_sec = 1;
    waitTime.tv_usec = 0;
    if (-1 == setsockopt(pingSocket, SOL_SOCKET, SO_RCVTIMEO, (char*) &waitTime, sizeof(struct timeval)))
    {
        printf("Error at set socket option");
	    exit(1);
    }

    // Init some buffer and variable for send mesasge and revice
    int dataLength = DEFAULT_DATA_LENGTH;
    int messageLength = dataLength + sizeof(struct icmphdr);
    unsigned int sequenceNumber = 0;
    char* dataAddress = (char*) calloc(messageLength, sizeof(char));
    struct icmphdr* imcpMessage = (struct icmphdr*) dataAddress; 
    struct msghdr messageHeader;
    char addressBuffer[128];
    struct iovec iov;
    unsigned char* repliedMessage = (unsigned char*)malloc(messageLength + IPV4_HEADER_LEN);
    struct icmphdr *icmpRepliedMessage = (struct icmphdr *)(repliedMessage + IPV4_HEADER_LEN);

    iov.iov_base = (char *) repliedMessage;
    iov.iov_len = messageLength;

    memset(&messageHeader, 0, sizeof(messageHeader));
    messageHeader.msg_name = addressBuffer;
    messageHeader.msg_namelen = sizeof(addressBuffer);
    messageHeader.msg_iov = &iov;
    messageHeader.msg_iovlen = 1;

    int i;
    long pingIdx = 0;
    for (;;)
    {
        pingIdx++;
        createICMPMessage(pingIdx, dataAddress, dataLength);
        gettimeofday(&begin, NULL);
        // Send ECHO Message
        i = sendto(pingSocket, imcpMessage, (messageLength), 0, (struct sockaddr*)&destinationAddress, sizeof(destinationAddress));
        printf("Sent ICMP_ECHO Message\n");
        
        // Wait to receive message
        i = recvmsg(pingSocket, &messageHeader, 0);
        gettimeofday(&end, NULL);

        // Check results
        if (0 > i)
        {
            printf("Error in recvfrom\n\n");
            goto labelEnd;
        };
        if (! 0 == calculateCheckSum(icmpRepliedMessage, dataLength))
        {
            printf("Checksum status : BAD\n\n");
            goto labelEnd;
        }
        if (icmpRepliedMessage->type == ICMP_ECHOREPLY) 
        {
            timeSpent =   (double)(end.tv_usec - begin.tv_usec) / 1000 
                        + (double)(end.tv_sec - begin.tv_sec) * 1000;  // uint in ms
            printf("Reiveied ICMP_ECHOREPLY sequence number = %d\n", ntohs(icmpRepliedMessage->un.echo.sequence));
            printf("Round trip time = %lf ms \n\n", timeSpent);
        } 
        else 
        {
            printf("%ld\n", pingIdx);
            printf("Not received message ICMP_ECHOREPLY\n\n");
        }

labelEnd: sleep(1);
        
        if ( -1 == numberOfPing)
        {
            continue;
        }
        else
        {
            if (pingIdx == numberOfPing)
            {
                break;
            }
        }
    }
    
    // Release dynamic memory
    free(repliedMessage);
    free(imcpMessage);
    return 0;
}
