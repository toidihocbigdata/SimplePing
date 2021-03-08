Functionality:
A programs do a ping protol to an IPv4 address and print round trip time.

What I did:
- create ICMP ECHO message follow RFC 792
- rewrite checksum function
- calculate roundtrip time
- send and receive ICMP message by socket

Build:
$ gcc -o MyPing MyPing.c

Run ping to any server:

Eg1: ping to Google Server 5 times
$ ./MyPing 8.8.8.8 3

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 23.974000 ms 

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 34.736000 ms 

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 23.771000 ms 

Eg2 : ping to 111.65.250.2 (ip of vnexpress.net) repeatly until recieving terminate signal if there no 2rd arg
$ ./MyPing 111.65.250.2

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 3.311000 ms 

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 2
Round trip time = 9.690000 ms 

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 3
Round trip time = 3.438000 ms 

Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 4
Round trip time = 3.070000 ms 

...

Test Tool:
Using wireshark capture with filter: ip.proto == 1
Wireshark support check status checksum and time response
We aslo read data in ICMP_ECHO and ICMP_ECHOREPLY in wireshark.
See test.png for more information

Ping Protocol:

My PC                                           A Server

        ---------- ICMP_ECHOREPLY --------->

        <---------- ICMP_ECHOREPLY ---------


ICMP header : (refer RFC 792)
    There are many ICMP header for many message
    In Ping protocol, we using ICMP_ ECHO and ICMP_ECHOREPLY messages
    They have heade form like this: ()

        0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Identifier          |        Sequence Number        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                Data ...
   +-+-+-+-+-


    Type field:

    8 : message ICMP_ECHO
    0 : message ICMP_ECHOREPLY

    Code: 

    0

    Checksum:

        The checksum is the 16-bit ones's complement of the one's
    complement sum of the ICMP message starting with the ICMP Type.
    For computing the checksum , the checksum field should be zero.
    If the total length is odd, the received data is padded with one
    octet of zeros for computing the checksum.  This checksum may be
    replaced in the future. 

    Identifier

        If code = 0, an identifier to aid in matching echos and replies,
    may be zero.
    For more explain:
    This field help for when many ping processes run concurrently in 
    the same machine. ICMP_ECHO and ICMP_ECHOREPLY must be have the same 
    Identifier field if they are for the same ping process.

   Sequence Number:
        Using for indexing message ICMP_ECHO - ICMP_ECHOREPLY pair for 
        ping many times. This start from 1 ( not 0).

        My PC                                           A Server

            ---------- ICMP_ECHO (SEQ = 1) -------------->

            <---------- ICMP_ECHOREPLY (SEQ = 1) ---------

            ---------- ICMP_ECHO (SEQ = 2) -------------->

            <---------- ICMP_ECHOREPLY (SEQ = 2) ---------

                                ...

   Data

   this field maybe empty, some ping program using
   this field to store time stamp - such as default ping 
   of Linux, support for calculate round trip time


