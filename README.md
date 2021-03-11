# Descriptons for MyPing program
A program executes Ping process to a IPv4 address (or Domain Name)

## Build command
```bash
gcc -o MyPing MyPing.c
```

## Functions
- Ping loop: 
``` bash
$ sudo ./MyPing 127.0.0.1

Resolving DNS..
127.0.0.1

Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 0.059000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 0.066000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 0.063000 ms 
```

- Ping N times (N = 4), pass 4 as arg 3rd of program:
```bash
$ sudo ./MyPing 8.8.8.8 4

Resolving DNS..
8.8.8.8

Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 25.481000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 26.452000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 26.410000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 4
Round trip time = 27.362000 ms 
```

- Ping by domain name:
```bash
sudo ./MyPing dantri.com.vn

Resolving DNS..
42.113.206.26

Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 6.326000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 3.946000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 4.468000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 4
Round trip time = 3.118000 ms 
```

## Ping Protocol

```
        My PC                                           A Server

            ---------- ICMP_ECHO (SEQ = 1) -------------->

            <---------- ICMP_ECHOREPLY (SEQ = 1) ---------

            ---------- ICMP_ECHO (SEQ = 2) -------------->

            <---------- ICMP_ECHOREPLY (SEQ = 2) ---------

                                ...
```
## ICMP Packet

```
    0               1               2               3
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Identifier          |        Sequence Number        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                Data ...                                       |
``` 

+ There are many ICMP header for many message. In Ping protocol, we using ICMP_ ECHO and ICMP_ECHOREPLY messages. They have heade form like this: 
+ Type field:
    + 8 : message ICMP_ECHO
    + 0 : message ICMP_ECHOREPLY
+ Code: 
    + 0 : In normal cases, Code field should be to zero.
+ Checksum: 
The checksum is the 16-bit ones's complement of the one's
complement sum of the ICMP message starting with the ICMP Type.
For computing the checksum , the checksum field should be zero.
If the total length is odd, the received data is padded with one
octet of zeros for computing the checksum.  This checksum may be
replaced in the future. 

+ Identifier: If code = 0, an identifier to aid in matching echos and replies,
may be zero. This field help for when many ping processes run concurrently in 
the same machine. ICMP_ECHO and ICMP_ECHOREPLY must be have the same 
Identifier field if they are for the same ping process.

+ Sequence Number:
Using for indexing message ICMP_ECHO - ICMP_ECHOREPLY pair for 
ping many times. This start from 1 ( not 0).

+ Data: This field maybe empty, some ping program using
this field to store time stamp - such as default ping 
of Linux, support for calculate round trip time

## Test Tool
- Using wireshark capture with filter: ip.proto == 1
- Wireshark support check status checksum and time response
- We aslo read data in ICMP_ECHO and ICMP_ECHOREPLY in wireshark.
- See test.png for more information.

## Some testcase: 
- Host drops and recovers successfully while ping process running:
```bash
sudo ./MyPing 192.168.1.30
Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 4.678000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 5.113000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 3.193000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 4
Round trip time = 4.342000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 5
Round trip time = 3.478000 ms 

Error in recvfrom

Error in recvfrom

Error in recvfrom

Error in recvfrom

Destination Unreachable

Destination Unreachable

Reiveied ICMP_ECHOREPLY sequence number = 10
Round trip time = 0.070000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 11
Round trip time = 0.067000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 12
Round trip time = 0.068000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 13
Round trip time = 0.072000 ms 

Reiveied ICMP_ECHOREPLY sequence number = 14
Round trip time = 0.069000 ms 
```
- Ping to an unreachale host:
```bash
$ sudo ./MyPing 192.168.1.100
Error in recvfrom

Error in recvfrom

Destination Unreachable

Destination Unreachable
```

## What I did:
- create ICMP ECHO message follow RFC 792
- rewrite checksum function
- calculate roundtrip time
- send and receive ICMP message by socket
