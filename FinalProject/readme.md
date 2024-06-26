Name: Hsin Li<br>
StudentID: 1623988288

PROJECT DETAILS
-----
Extra Credit:
- The client only provides the room number (client.cpp, serverRTH.cpp, serverEEB.cpp)
- The client provides the room number and the day (client.cpp, serverRTH.cpp, serverEEB.cpp)
-----
Code Description:
- client.cpp, client.h
    - Interface for client login, interacts with member based on whether they're member or guest.
    - Message format: Sends with type of request, delimited by a '&', then data is separated by a whitespaces
        - Authentication e.g. MessageType=AuthenticationRequest&username password
        - Availability/Reservation e.g. MessageType=AvailabilityRequest&room day hour period action
            - time is split up into hour and period
    - client fails when it is disconnected from the serverM, or if Ctrl+C is entered by the user
- serverM.cpp, serverM.h
    - Interfaces between client and serverC, serverRTH, serverEEB.
    - Handling client: when receives data from client, it creates a new client socket. The client sockets are stored in a map, and closed when the client disconnects. If a client is requesting as a guest, the authentication request will not be forwarded to serverC, but handled at serverM.
    - Handling servers: when receives data from udp servers, it identifies the request type, and which client socket to send to, then relays the information to the client
    - Message format to client: sends with the request type, delimited by a '&'.
        - Authentication .e.g. MessageType=AuthenticationResponse&data
    - Message format to servers: sends with the request type, delimited by a '&', the data, then followed with a delimiter '*' and the client socket that sent the request as the requestID
        - serverC e.g. MessageType=AuthenticationRequest&username password*requestID
        - serverRTH/EEB e.g. MessageType=AvailabilityRequest&room day hour period action*requestID
    - serverM fails if Ctrl+C is entered by user
- serverC.cpp, serverC.h
    - Handles client request forwarded from serverM for authentication of username and password.
    - Message format: sends data with the request type, delimited by a '&', the data, then followed with a delimiter * and the requestID
        - e.g. MessageType=AuthenticationResponse&result*requestID
    - serverM fails if Ctrl+C is entered by user
- serverRTH.cpp serverRTH.h
    - Handles client request forwarded from serverM to check the availability or reserve a room in building RTH. List of reserved rooms is stored in a string vector.
    - Message format:sends with the request type, delimited by a '&', the data, then followed with a delimiter '*' and the requestID
        - e.g. MessageType=AvailabilityResponse&result*requestID
    - serverM fails if Ctrl+C is entered by user
- serverEEB.cpp serverEEB.h
    - Handles client request forwarded from serverM to check the availability or reserve a room in building EEB. List of reserved rooms is stored in a string vector.
    - Message format:sends with the request type, delimited by a '&', the data, then followed with a delimiter '*' and the requestID
        - e.g. MessageType=AvailabilityResponse&result*requestID
    - serverM fails if Ctrl+C is entered by user
-----
References:
1. Client authentication code (serverC: 43~64) referenced the following video 
    <br>[How to Program a Simple Login...](https://www.youtube.com/watch?v=fTdkqOS5_ro&ab_channel=LeMasterTech)
2. TCP socket programming (client: 81~93, 162~169, 183~194, 332~375; serverM) referenced the following webpage
    <br>[Socket Programming in C++](https://www.geeksforgeeks.org/socket-programming-in-cpp/)
3. UDP socket programming (severC: 69~144; serverRTH: 149~197, 245~273; serverEEB: 149~198, 247~271) referenced the following webpage.
    <br>[UDP Server-Client implementation in C++](https://www.geeksforgeeks.org/udp-server-client-implementation-c/#)
4. UDP Boot Notification (serverC: 104~109; serverRTH: 184~189; serverEEB: 185~190) referenced the following ChatGPT prompt
    <br>[How does one UDP server inform another UDP server that it as been booted up?](chat.openai.com)
5. serverM handling multiple clients and sockets (serverM: 290~468) referenced the following webpage
    <br>[Handling multiple clients on server without multi threading](https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/)
6. Clearing sockets in a set (serverM: 396~400) referenced the following webpage
    <br>[Why FD_SET/FD_ZERO for select() inside of loop?](https://stackoverflow.com/questions/7637765/why-fd-set-fd-zero-for-select-inside-of-loop)
7. Concepts and coding of File Descriptors (serverM: 290~468) referenced the following webpages
    <br>[Unix I/O, Sockets](https://ycpcs.github.io/cs365-spring2017/lectures/lecture15.html)
    <br>[UDP Client Server using connect | C implementation](https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/)
    <br>[Understanding file descriptors in Linux](https://wiyi.org/linux-file-descriptor.html)
8. Terminating the client without terminating the server (serverM: 293~308) referenced the following webpage and the following ChatGPT prompt
    <br>[Detecting TCP Client Disconnect](https://stackoverflow.com/questions/283375/detecting-tcp-client-disconnect)
    <br>[How do I end the TCP client terminal without ending the serer terminal](chat.openai.com)