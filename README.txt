Functionality:
A programs do a ping protol to an IPv4 address (1 time) and print round trip time.

Build:
$ gcc -o MyPing MyPing.c

Run ping to Google DNS:
$ ./MyPing 8.8.8.8  

Results:
Sent ICMP_ECHO Message
Reiveied ICMP_ECHOREPLY sequence number = 1
Round trip time = 25.895000 ms 

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

            ---------- ICMP_ECHOREPLY (SEQ = 1)--------->

            <---------- ICMP_ECHOREPLY (SEQ = 1) --------

            ---------- ICMP_ECHOREPLY (SEQ = 2)--------->

            <---------- ICMP_ECHOREPLY (SEQ = 2) --------

                                ...

   Data

   this field maybe empty, some ping program using
   this field to store time stamp - such as default ping 
   of Linux, support for calculate round trip time


