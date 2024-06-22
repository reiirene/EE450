// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Main server program that interfaces with clients and coordinates interactions with the backend servers.

#include "serverM.h"

string status = "";
Client user;
int tcpSocket, udpSocket;
vector<int> clientSockets(MAX_CLIENTS, 0);

string parseMessageType (const string &data) {
    size_t type_pos = data.find("MessageType=");
    if (type_pos != string::npos) {
        size_t type_end = data.find("&", type_pos);
        if (type_end != string::npos) {
            return data.substr(type_pos + 12, type_end - type_pos - 12);    // 12 is the length of "MessageType="
        }
    }
    return "";
}

string parseMessage (const string &data) {
    size_t message_pos = data.find("&");
    if (message_pos != string::npos) {
        return data.substr(message_pos + 1);
    }
    return "";
}

string packageMessage (const string &message, const string &type) {
    return "MessageType=" + type + "&" + message;
}

void bootCheck (int udpSocket, struct sockaddr_in clientAddr, socklen_t clientAddrLen) {
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Server C boot check
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, &serverAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return;
    }

    // Extract server port number
    int serverPort = ntohs(serverAddr.sin_port);

    string serverName;

    if (serverPort == SERVER_C_PORT) {
        serverName = "C";
    } else if (serverPort == SERVER_RTH_PORT) {
        serverName = "RTH";
    } else if (serverPort == SERVER_EEB_PORT) {
        serverName = "EEB";
    }
    
    string message = buffer;
    if (message == "boot") {
        cout << "The main server has received the notification from Server <" << serverName << "> ";
        cout << "using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;
    }

}

//void clientRequest (int clientSocket, int udpSocketRTH, int udpSocketEEB, string &request, string &requestType, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
void clientRequest (int clientSocket, int udpSocket, string &request, string &requestType, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
    stringstream ss(request);
    string  word;

    vector <string> words;
    while (ss >> word) {
        words.push_back(word);
    }
    
    string room, day, hour, period, action;

    if (words.size() > 0) room = words[0];
    if (words.size() > 1) day = words[1];
    if (words.size() > 2) hour = words[2];
    if (words.size() > 3) period = words[3];
    if (words.size() > 4) action = words[4];

    // Determine type of request
    if (requestType == "AvailabilityRequest") {
        cout << "The main server has received the availability request on Room " << room << " at " << hour << " " << period << " on "; 
        cout << day << " from " << user.username << " using TCP over port " << SERVER_M_TCP_PORT << endl;
    } else if (requestType == "ReservationRequest") {
        cout << "The main server has received the reservation request on Room " << room << " at " << hour << " " << period << " on "; 
        cout << day << " from " << user.username << " using TCP over port " << SERVER_M_TCP_PORT << endl;
        if (status == "guest") {
            cout << "Permission denied. " << user.username << " cannot make a reservation." << endl;
            string response = "Permission denied: Guest cannot make a reservation.";
            response = packageMessage(response, "ReservationResponse");
            if (send(clientSocket, response.c_str(), response.size(), 0) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
            cout << "The main server sent the error message to the client." << endl;
            return;
        }
    } 

    string response;

    if (room.substr(0, 3) == "RTH") {
        // Send request to serverRTH
        request = packageMessage(request, requestType);
        response = handleServerRTH(udpSocket, serverRTHAddr, request, requestType);
        // response = handleServerRTH(udpSocketRTH, serverRTHAddr, request, requestType);

    } else if (room.substr(0, 3) == "EEB") {
        // Send request to serverEEB
        request = packageMessage(request, requestType);
        response = handleServerEEB(udpSocket, serverEEBAddr, request, requestType);
        //response = handleServerEEB(udpSocketEEB, serverEEBAddr, request, requestType);

    }

    // Send availability/reservation response to client
    string responseType = parseMessageType(response);
    if (responseType == "AvailabilityResponse") {
        if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
            cerr << "Send to client failed due to " << strerror(errno) << endl;
            return;
        }
        cout << "The main server sent the availability information to the client." << endl;
    } else if (responseType == "ReservationResponse") {
        if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
            cerr << "Send to client failed due to " << strerror(errno) << endl;
            return;
        }
        cout << "The main server sent the reservation result to the client." << endl;
    }

}


// void handleClient (int clientSocket, int udpSocketC, struct sockaddr_in serverCAddr, int udpSocketRTH, struct sockaddr_in serverRTHAddr, int udpSocketEEB, struct sockaddr_in serverEEBAddr) {
void handleClient (int clientSocket, int udpSocket, struct sockaddr_in serverCAddr, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
    // Receive data from client
    
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead < 0) {
        cerr << "Receive failed" << endl;
        close(clientSocket);
        return;
    } else if (bytesRead == 0) {
        cout << "Client disconnected" << endl;
        close(clientSocket);
        return;
    }
    

    // Determine if Member or Guest
    if (user.password.empty() && status.empty()) {
        // Extract username and password
        string credentials = buffer;
        size_t pos = credentials.find(" ");
        if (pos == string::npos) {
            user.username = credentials;
            status = "guest";
            cout << "The main server received the guest request for " << user.username << " using TCP over port " << CLIENT_M_UDP_PORT << "." << endl;
            cout << "The main server accepts " << user.username << " as a guest." << endl;
            if (send(clientSocket, status.c_str(), status.length(), 0) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
        } else {
            user.username = credentials.substr(0, pos);
            user.password = credentials.substr(pos + 1);
            cout << "The main server received the authentication for " << user.username << " using TCP over port " << CLIENT_M_UDP_PORT << "." << endl;

            // Send authentication request to serverC
            string response = handleServerC(udpSocket, serverCAddr, buffer);
            // string response = handleServerC(udpSocketC, serverCAddr, buffer);
            response = packageMessage(response, "AuthenticationResponse");

            // Send serverC authentication result to client
            if (send(clientSocket, response.c_str(), response.size(), 0) < 0) {
                cerr << "Send to client failed due to " << strerror(errno) << endl;
                return;
            }
            cout << "The main server sent the authentication result to the client." << endl;
        }
        return;
    }

    // Handle Client request
    string requestType = parseMessageType(buffer);
    if ((status == "valid" || status == "guest") && !user.username.empty()) {
        string request = buffer;
        request = parseMessage(request);
        clientRequest(clientSocket, udpSocket, request, requestType, serverRTHAddr, serverEEBAddr);
        // clientRequest(clientSocket, udpSocketRTH, udpSocketEEB, request, requestType, serverRTHAddr, serverEEBAddr);
        return;
    }

    close(clientSocket);

}

string handleServerC (int udpSocketC, const struct sockaddr_in &serverCAddr, const char *request) {
    // Send authentication request to serverC
    cout << "Request: " << request << endl;
    if (sendto(udpSocketC, request, strlen(request), 0, (struct sockaddr *)&serverCAddr, sizeof(serverCAddr)) < 0) {
        cerr << "Send failed" << endl;
        return "";
    }
    cout << "The main server forwarded the authentication for ";
    cout << user.username << " using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;

    // Receive authentication result from serverC
    struct sockaddr_in responseAddrC;
    socklen_t responseAddrCLen = sizeof(responseAddrC);
    char buffer[BUFFER_SIZE] = {0};
    
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketC, buffer, sizeof(buffer), 0, (struct sockaddr *)&responseAddrC, &responseAddrCLen) < 0) {
        cerr << "Receive failed" << endl;
        return "";
    }

    string response = buffer;

    cout << "The main server received the authentication result for ";
    cout << user.username << " using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;
    cout << "Response: " << response << endl;
    if (response == "incorrect_password" || response == "user_not_found") {
        user.username = "";
        user.password = "";
    } else {
        status = response;
        cout << "Client status: " << status << endl;
    }
    
    return response;

}

string handleServerRTH (int udpSocketRTH, const struct sockaddr_in &serverRTHAddr, string &request, string &requestType) {
    // Send availability/reservation request to serverRTH
    if (sendto(udpSocketRTH, request.c_str(), request.size(), 0, (struct sockaddr *)&serverRTHAddr, sizeof(serverRTHAddr)) < 0) {
        cerr << "Send failed" << endl;
        return "";
    }

    cout << "The main server sent a request to Server <RTH>." << endl;

    // Receive response from serverRTH
    struct sockaddr_in responseAddrRTH;
    socklen_t responseAddrRTHLen = sizeof(responseAddrRTH);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from serverRTH
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketRTH, buffer, sizeof(buffer), 0, (struct sockaddr *)&responseAddrRTH, &responseAddrRTHLen) < 0) {
        cerr << "Receive failed" << endl;
        return "";
    }

    string response = buffer;

    cout << "The main server received the response from Server <RTH> using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;

    return response;
}

string handleServerEEB (int udpSocketEEB, const struct sockaddr_in &serverEEBAddr, string &request, string &requestType) {
    // Send availability/reservation request to serverEEB
    if (sendto(udpSocketEEB, request.c_str(), request.size(), 0, (struct sockaddr *)&serverEEBAddr, sizeof(serverEEBAddr)) < 0) {
        cerr << "Send failed" << endl;
        return "";
    }

    cout << "The main server sent a request to Server <EEB>." << endl;

    // Receive response from serverEEB
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from serverEEB
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketEEB, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return "";
    }

    string response = buffer;

    cout << "The main server received the response from Server <EEB> using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;

    return response;
}

int main() {
    int sd, max_sd;
    struct sockaddr_in serverCAddr, serverRTHAddr, serverEEBAddr, tcpAddr, udpAddr;

    signal(SIGINT, (__sighandler_t)([](int signum) {
        cout << "\nInterrupt signal (" << signum << ") received. Shutting down the server..." << endl;
    
        // Close all client sockets
        for (int clientSocket : clientSockets) {
            if (clientSocket > 0) {
                close(clientSocket);
            }
        }
    
        // Close the tcp and udp sockets
        close(udpSocket);
        close(tcpSocket);
        exit(signum);
    }));

    // Create TCP socket
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket < 0) {
        cerr << "TCP socket creation error" << endl;
        return -1;
    }

    // Setsockopt for TCP socket
    int opt = 1;    
    if (setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        cerr << "Setsockopt failed" << endl;
        return -1;
    }

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }

    // Setsockopt for UDP socket
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
        cerr << "Setsockopt failed" << endl;
        return -1;
    }

    // Set up UDP client address
    memset(&udpAddr, 0, sizeof(udpAddr));
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(CLIENT_M_UDP_PORT);
    udpAddr.sin_addr.s_addr = INADDR_ANY;

    // Set up UDP server addresses
    memset(&serverCAddr, 0, sizeof(serverCAddr));   // Server C
    serverCAddr.sin_family = AF_INET;
    serverCAddr.sin_port = htons(SERVER_C_PORT);
    serverCAddr.sin_addr.s_addr = INADDR_ANY;
    
    memset(&serverRTHAddr, 0, sizeof(serverRTHAddr));   // Server RTH
    serverRTHAddr.sin_family = AF_INET;
    serverRTHAddr.sin_port = htons(SERVER_RTH_PORT);
    serverRTHAddr.sin_addr.s_addr = INADDR_ANY;
    
    memset(&serverEEBAddr, 0, sizeof(serverEEBAddr));   // Server EEB
    serverEEBAddr.sin_family = AF_INET;
    serverEEBAddr.sin_port = htons(SERVER_EEB_PORT);
    serverEEBAddr.sin_addr.s_addr = INADDR_ANY;

    // Set up TCP server address
    memset(&tcpAddr, 0, sizeof(tcpAddr));
    tcpAddr.sin_family = AF_INET;
    tcpAddr.sin_port = htons(SERVER_M_TCP_PORT);
    tcpAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind TCP socket
    if (bind(tcpSocket, (struct sockaddr *)&tcpAddr, sizeof(tcpAddr)) < 0) {
        cerr << "TCP bind failed due to " << strerror(errno) << endl;
        return -1;
    }

    // Bind UDP socket
    if (bind(udpSocket, (struct sockaddr *)&udpAddr, sizeof(udpAddr)) < 0) {
        cerr << "UDP bind failed" << endl;
        return -1;
    }

    // Listen on TCP socket
    if (listen(tcpSocket, 5) < 0) {
        cerr << "Listen failed" << endl;
        return -1;
    }

    // Declare a set of file descriptors to monitor for readability
    fd_set readfds;
    int new_socket, activity;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    cout << "The main server is up and running using UDP on port " << CLIENT_M_UDP_PORT << " and TCP on port " << SERVER_M_TCP_PORT << "." << std::endl;

    // Main server loop
    while (true) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add TCP socket to set
        FD_SET(tcpSocket, &readfds);
        max_sd = tcpSocket;

        // Add client sockets to set
        for (int clientSocket : clientSockets) {
            // Socket descriptor
            sd = clientSocket;

            // if valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            // Highest file descriptor number, need it for the select function
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Add UDP socket to set
        FD_SET(udpSocket, &readfds);
        if (udpSocket > max_sd) {
            max_sd = udpSocket;
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            cerr << "Select error" << endl;
            return -1;
        }

        // Handle TCP server connection
        // If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(tcpSocket, &readfds)) {

            // Accept new connection
            if ((new_socket = accept(tcpSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                cerr << "Accept failed" << endl;
                return -1;
            }

            // Add new socket to array of sockets
            clientSockets.push_back(new_socket);
            cout << "New connection, socket fd is " << new_socket << ", ip is " << inet_ntoa(address.sin_addr) << ", port is " << ntohs(address.sin_port) << endl;

        }
        
        // Handle UDP activity
        if (FD_ISSET(udpSocket, &readfds)) {
            bootCheck(udpSocket, address, addrlen);
        }


        // Handle TCP activity
        for (size_t i = 0; i < clientSockets.size(); ) {
            sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds)) {
                handleClient(sd, udpSocket, serverCAddr, serverRTHAddr, serverEEBAddr);
                clientSockets.erase(clientSockets.begin() + i);   // Remove client socket from vector
            } else {
                ++i;
            }
        }
        
    
    }

    return 0;
}
