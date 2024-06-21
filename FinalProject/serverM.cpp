// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Main server program that interfaces with clients and coordinates interactions with the backend servers.

#include "serverM.h"

string status = "";
Client user;

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

void clientRequest (int clientSocket, int udpSocketRTH, int udpSocketEEB, string &request, string &requestType, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
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

    // Debugging
    if (requestType == "AvailabilityRequest" && (action != "Availability" || action != "availability")) {
        cout << "Transmission error: availability request and message type are inconsistent." << endl;
    }

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

    if (room.substr(0, 3) == "RTH") {
        // Send request to serverRTH
        request = packageMessage(request, requestType);
        if (sendto(udpSocketRTH, request.c_str(), request.size(), 0, (struct sockaddr *)&serverRTHAddr, sizeof(serverRTHAddr)) < 0) {
            cerr << "Send failed" << endl;
            return;
        }
        cout << "The main server sent a request to Server <RTH>." << endl;
    } else if (room.substr(0, 3) == "EEB") {
            // Send request to serverEEB
        request = packageMessage(request, requestType);
        if (sendto(udpSocketEEB, request.c_str(), request.size(), 0, (struct sockaddr *)&serverEEBAddr, sizeof(serverEEBAddr)) < 0) {
            cerr << "Send failed" << endl;
            return;
        }
        cout << "The main server sent a request to Server <EEB>." << endl;
    }

}

void handleClient (int clientSocket, int udpSocketC, struct sockaddr_in serverCAddr, int udpSocketRTH, struct sockaddr_in serverRTHAddr, int udpSocketEEB, struct sockaddr_in serverEEBAddr) {
    // Receive data from client
    char buffer[BUFFER_SIZE] = {0};
    if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
        cerr << "Receive failed" << endl;
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
            cout << "The main server received the guest request for " << user.username << " using TCP over port " << SERVER_M_UDP_PORT << "." << endl;
            cout << "The main server accepts " << user.username << " as a guest." << endl;
            if (send(clientSocket, status.c_str(), status.length(), 0) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
        } else {
            user.username = credentials.substr(0, pos);
            user.password = credentials.substr(pos + 1);
            cout << "The main server received the authentication for " << user.username << " using TCP over port " << SERVER_M_UDP_PORT << "." << endl;
            if (sendto(udpSocketC, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverCAddr, sizeof(serverCAddr)) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
            cout << "The main server forwarded the authentication for ";
            cout << user.username << " using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
        }
        return;
    }

    // Handle Client request
    string requestType = parseMessageType(buffer);
    if ((status == "valid" || status == "guest") && !user.username.empty()) {
        string request = buffer;
        request = parseMessage(request);
        clientRequest(clientSocket, udpSocketRTH, udpSocketEEB, request, requestType, serverRTHAddr, serverEEBAddr);
        return;
    }

}

void handleServerC (int udpSocketC, int tcpClientSocket) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from serverC
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketC, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return;
    }

    string message = buffer;

    // Server C boot check
    if (message == "boot") {
        cout << "The main server has received the notification from Server <C> ";
        cout << "using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
    } else {
        cout << "The main server received the authentication result for ";
        cout << user.username << " using UDP over port " << SERVER_M_UDP_PORT << "." << endl;    
        if (message == "incorrect_password" || message == "user_not_found") {
            user.username = "";
            user.password = "";
        } else {
            status = message;
            cout << "Client status: " << status << endl;
        }
        // Send serverC authentication result to client
        if (send(tcpClientSocket, message.c_str(), message.length(), 0) < 0) {
            cerr << "Send to client failed due to " << strerror(errno) << endl;
            return;
        }
        cout << "The main server sent the authentication result to the client." << endl;
    }

}

void handleServerRTH (int udpSocketRTH, int tcpClientSocket) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from serverRTH
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketRTH, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return;
    }

    string message = buffer;

    // Server RTH boot check
    if (strcmp(buffer, "boot") == 0) {
        cout << "The main server has received the notification from Server <RTH> ";
        cout << "using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
    } else {
        cout << "The main server received the response from Server <RTH> using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
        string responseType = parseMessageType(message);
        // Send serverRTH response to client
        if (responseType == "AvailabilityResponse") {
            if (send(tcpClientSocket, message.c_str(), message.length(), 0) < 0) {
                cerr << "Send to client failed due to " << strerror(errno) << endl;
                return;
            }
            cout << "The main server sent the availability information to the client." << endl;
        } else if (responseType == "ReservationResponse") {
            if (send(tcpClientSocket, message.c_str(), message.length(), 0) < 0) {
                cerr << "Send to client failed due to " << strerror(errno) << endl;
                return;
            }
            cout << "The main server sent the reservation result to the client." << endl;
        }
    }

    return;
}

void handleServerEEB (int udpSocketEEB, int tcpClientSocket) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from serverRTH
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocketEEB, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return;
    }

    string message = buffer;

    // Server RTH boot check
    if (strcmp(buffer, "boot") == 0) {
        cout << "The main server has received the notification from Server <EEB> ";
        cout << "using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
    } else {
        cout << "The main server received the response from Server <EEB> using UDP over port " << SERVER_M_UDP_PORT << "." << endl;
        string responseType = parseMessageType(message);
        // Send serverRTH response to client
        if (responseType == "AvailabilityResponse") {
            if (send(tcpClientSocket, message.c_str(), message.length(), 0) < 0) {
                cerr << "Send to client failed due to " << strerror(errno) << endl;
                return;
            }
            cout << "The main server sent the availability information to the client." << endl;
        } else if (responseType == "ReservationResponse") {
            if (send(tcpClientSocket, message.c_str(), message.length(), 0) < 0) {
                cerr << "Send to client failed due to " << strerror(errno) << endl;
                return;
            }
            cout << "The main server sent the reservation result to the client." << endl;
        }
    }

    return;
}

int main() {
    int tcpSocket, udpSocketC, udpSocketRTH, udpSocketEEB;
    int sd, max_sd;
    struct sockaddr_in serverCAddr, serverRTHAddr, serverEEBAddr, tcpAddr;
    int tcpClientSocket = -1;
    vector<int> clientSockets(MAX_CLIENTS, 0);

    // Create TCP socket
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket < 0) {
        cerr << "TCP socket creation error" << endl;
        return -1;
    }

    // Create UDP socket for serverC
    udpSocketC = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocketC < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }
    
    // Create UDP socket for serverRTH
    udpSocketRTH = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocketRTH < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }
    
    // Create UDP socket for serverEEB
    udpSocketEEB = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocketEEB < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }

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

    // Listen on TCP socket
    if (listen(tcpSocket, 5) < 0) {
        cerr << "Listen failed" << endl;
        return -1;
    }

    cout << "The main server is up and running using UDP on port " << SERVER_M_UDP_PORT << " and TCP on port " << SERVER_M_TCP_PORT << "." << std::endl;

    // Declare a set of file descriptors to monitor for readability
    fd_set readfds;

    while (true) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add TCP socket to set
        FD_SET(tcpSocket, &readfds);
        // Add serverC UDP socket to set
        FD_SET(udpSocketC, &readfds);

        max_sd = (tcpSocket > udpSocketC) ? tcpSocket : udpSocketC;

        for (size_t i = 0; i < clientSockets.size(); ++i) {
            sd = clientSockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }
        
        // Add serverRTH UDP socket to set
        FD_SET(udpSocketRTH, &readfds);
        if (udpSocketRTH > max_sd) {
            max_sd = udpSocketRTH;
        }
        
        // Add serverEEB UDP socket to set
        FD_SET(udpSocketEEB, &readfds);
        if (udpSocketEEB > max_sd) {
            max_sd = udpSocketEEB;
        }

        // Wait for activity on one of the sockets
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            cerr << "Select error" << endl;
            return -1;
        }

        // Handle TCP server
        if (FD_ISSET(tcpSocket, &readfds)) {
            // create a new socket for the client
            int clientSocket;
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            clientSocket = accept(tcpSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
            if (clientSocket < 0) {
                cerr << "Accept failed" << endl;
                return -1;
            }

            // Add new socket to array of sockets
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clientSockets[i] == 0) {
                    clientSockets[i] = clientSocket;
                    break;
                }
            }

            tcpClientSocket = clientSocket; // Save client socket for later use
        }

        // Handle UDP serverC
        if (FD_ISSET(udpSocketC, &readfds)) {
            handleServerC(udpSocketC, tcpClientSocket);
        }

        
        // Handle UDP serverRTH
        if (FD_ISSET(udpSocketRTH, &readfds)) {
            handleServerRTH(udpSocketRTH, tcpClientSocket);
        }
        
        // Handle UDP serverEEB
        if (FD_ISSET(udpSocketEEB, &readfds)) {
            handleServerEEB(udpSocketEEB, tcpClientSocket);
        }
        
        // Handle client
        for (size_t i = 0; i < clientSockets.size(); ++i) {
            sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds)) {
                handleClient(sd, udpSocketC, serverCAddr, udpSocketRTH, serverRTHAddr, udpSocketEEB, serverEEBAddr);
            }
        }
    }

    // Close sockets
    close(tcpClientSocket);
    close(tcpSocket);
    close(udpSocketC);
    close(udpSocketRTH);
    close(udpSocketEEB);

    return 0;
}
