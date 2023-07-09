# ReliableUDP
Socket Programming Realiable UDP and FTP


Basics of TCP and UDP

Type of service:

● TCP is a connection-oriented protocol. Connection-orientation means that the communicating
device should establish a connection before transmitting data and should close the connection
after transmitting the data.

● UDP is the connectionless protocol. This is because there is no procedure for opening a
connection, maintaining a connection and terminating a connection.


Reliability:

● TCP is reliable as it guarantees the delivery of data to the destination. It provides flow control and
acknowledgment of data.

● The delivery of data to the destination cannot be guaranteed in UDP.
Error checking mechanism:

● TCP provides extensive error-checking mechanisms.

● UDP has only the basic error checking mechanism using checksums.

Acknowledgement:

● In TCP an acknowledgement segment is present.

● In UDP no acknowledgement segment.


Sequence:

● Sequencing of data is a feature of transmission control protocol(TCP). This will make sure that
each and every segment has received.

● There is no sequencing of data in UDP. If the order is required, it has to be managed by the
application layer.

Speed:

● TCP is comparatively slower than UDP.

● UDP is faster, simpler and more efficient than TCP.

Retransmission:

● Retransmission of lost packets is possible in TCP, but not in UDP.

● There is no retransmission of lost packets in the UDP.

Handshaking Techniques:

● Uses handshakes such as SYN, ACK, SYN-ACK.

● UDP is a connectionless protocol.


# FTP Protocol

FTP (File Transfer Protocol) is a network protocol for transmitting files between computers
over Transmission Control Protocol/Internet Protocol (TCP/IP) connections. Within the
TCP/IP suite, FTP is considered an application layer protocol.


In an FTP transaction, the end user's computer is typically called the local host. The second
computer involved in FTP is a remote host, which is usually a server. Both computers need to
be connected via a network and configured properly to transfer files via FTP. Servers must be
set up to run FTP services, and the client must have FTP software installed to access these
services.

FTP is a client-server protocol that relies on two communications channels between the client
and server: a command channel for controlling the conversation and a data channel for
transmitting file content.

Types of connection:

1.Control Connection:

For sending control information like user identification, password, commands to change
the remote directory, commands to retrieve and store files, etc., FTP makes use of a
control connection.

2.Data Connection:

For sending the actual file, FTP makes use of a data connection. A data connection is
initiated on port number 20.
