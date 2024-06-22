// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Main server program that interfaces with clients and coordinates interactions with the backend servers.

#include "serverM.h"

int tcpSocket, udpSocket;
vector<int> clientSockets(MAX_CLIENTS, 0);  // Array of client sockets
map <int, Client> clients;  // Map of client sockets to client information

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

string parseRequestID (const string &data) {
    size_t requestID_pos = data.find_last_of("*");
    if (requestID_pos != string::npos) {
        return data.substr(requestID_pos + 1);
    }
    return "";
}

string parseMessage (const string &data) {
    size_t message_pos = data.find("&");
    if (message_pos != string::npos) {
        size_t requestID_pos = data.find_last_of("*");
        if (requestID_pos != string::npos) {
            return data.substr(message_pos + 1, requestID_pos - message_pos - 1);
        } else {
            return data.substr(message_pos + 1);
        }
    }
    return "";
}

string packageMessage (const string &message, const string &type, const string &requestID, const string &receiver) {
    if (receiver == "client") {
        return "MessageType=" + type + "&" + message;
    } else if (receiver == "server") {
        return "MessageType=" + type + "&" + message + "*" + requestID;
    }
    return "";
}

void clientRequest (int clientSocket, int udpSocket, string &request, string &requestType, string &requestID, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
    stringstream ss(request);
    string word;
    Client user;

    user = clients[clientSocket];
    // Debugging: shows information about the user who sent the request
    // cout << "Requesting user: " << user.username << " " << user.password << " " << user.status << endl;

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
        if (user.status == "guest") {
            cout << "Permission denied. " << user.username << " cannot make a reservation." << endl;
            string response = "Permission denied: Guest cannot make a reservation.";
            response = packageMessage(response, "ReservationResponse", requestID, "client");
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
        request = packageMessage(request, requestType, requestID, "server");

        if (sendto(udpSocket, request.c_str(), request.size(), 0, (struct sockaddr *)&serverRTHAddr, sizeof(serverRTHAddr)) < 0) {
            cerr << "Send failed" << endl;
            return;
        }

        cout << "The main server sent a request to Server <RTH>." << endl;
    } else if (room.substr(0, 3) == "EEB") {
        // Send request to serverEEB
        request = packageMessage(request, requestType, requestID, "server");

        if (sendto(udpSocket, request.c_str(), request.size(), 0, (struct sockaddr *)&serverEEBAddr, sizeof(serverEEBAddr)) < 0) {
            cerr << "Send failed" << endl;
            return;
        }

    }

}

void handleClient (int clientSocket, int udpSocket, struct sockaddr_in serverCAddr, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
    Client user;
    string request, requestType, requestID;

    // Receive data from client
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead < 0) {
        cerr << "Receive failed" << endl;
        close(clientSocket);
        return;
    } else if (bytesRead == 0) {
        // Debugging: shows information about the client who disconnected
        // cout << "User " << user.username << " has disconnected on client socket " << clientSocket << endl;
        close(clientSocket);
        return;
    }

    // Generate UDP request ID
    requestID = to_string(clientSocket);
    string message = buffer;
    request = parseMessage(message);
    requestType = parseMessageType(message);

    // Check if the client is new or existing
    if (clients.find(clientSocket) == clients.end()) {
        // New client
    } else {
        // Existing client
        user = clients[clientSocket];

        // Debugging: shows information about the existing client
        // cout << "Existing client: " << user.username << " " << user.password << " " << user.status << endl;
    }

    // Handle Client authentication
    if (requestType == "AuthenticationRequest") {
        // Extract username and password
        size_t pos = request.find(" ");
        if (pos == string::npos) {
            user.username = request;
            user.password = "";
            user.status = "guest";
            cout << "The main server received the guest request for " << user.username << " using TCP over port " << CLIENT_M_UDP_PORT << "." << endl;
            cout << "The main server accepts " << user.username << " as a guest." << endl;
            string response = packageMessage(user.status, "AuthenticationResponse", requestID, "client");
            if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
        } else {
            user.username = request.substr(0, pos);
            user.password = request.substr(pos + 1);
            cout << "The main server received the authentication for " << user.username << " using TCP over port " << CLIENT_M_UDP_PORT << "." << endl;            

            request = packageMessage(request, "AuthenticationRequest", requestID, "server");

            // Send authentication request to serverC
            if (sendto(udpSocket, request.c_str(), request.size(), 0, (struct sockaddr *)&serverCAddr, sizeof(serverCAddr)) < 0) {
                cerr << "Send failed" << endl;
                return;
            }
            cout << "The main server forwarded the authentication for ";
            cout << user.username << " using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;
        }

        // Add guest to clients map
        clients[clientSocket] = user;

        return;
    }

    // Handle Client request
    if (requestType == "AvailabilityRequest" || requestType == "ReservationRequest") {

        clientRequest(clientSocket, udpSocket, request, requestType, requestID, serverRTHAddr, serverEEBAddr);

        return;
    }

}

void handleServer (int udpSocket, struct sockaddr_in serverCAddr, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr) {
    Client user;
    string message, response, responseType, requestID, serverName;
    // Receive data from server
    struct sockaddr_in responseAddr;
    socklen_t responseAddrLen = sizeof(responseAddr);
    char buffer[BUFFER_SIZE] = {0};

    // Receive data from server
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&responseAddr, &responseAddrLen) < 0) {
        cerr << "Receive failed" << endl;
        return;
    }

    // Determine which server the response is from
    int serverPort = ntohs(responseAddr.sin_port);

    if (serverPort == SERVER_C_PORT) {
        serverName = "C";
    } else if (serverPort == SERVER_RTH_PORT) {
        serverName = "RTH";
    } else if (serverPort == SERVER_EEB_PORT) {
        serverName = "EEB";
    }

    message = buffer;

    // Check if the response is a boot message
    if (message == "boot") {
        cout << "The main server has received the notification from Server <" << serverName << "> ";
        cout << "using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;
        return;
    }
    
    response = parseMessage(message);
    responseType = parseMessageType(message);
    requestID = parseRequestID(message);

    user = clients[atoi(requestID.c_str())];

    // get the client socket
    for (size_t i = 0; i < clientSockets.size(); ++i) {
        int clientSocket = clientSockets[i];
        if (clientSocket == atoi(requestID.c_str())) {
            if (responseType == "AuthenticationResponse") {

                cout << "The main server received the authentication result for ";
                cout << user.username << " using UDP over port " << CLIENT_M_UDP_PORT << "." << endl;

                if (response == "incorrect_password" || response == "user_not_found") {
                    user.username = "";
                    user.password = "";
                } else {
                    user.status = response;
                }

                // Send authentication result to client
                response = packageMessage(response, responseType, requestID, "client");
                if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
                    cerr << "Send to client failed due to " << strerror(errno) << endl;
                    return;
                }
                cout << "The main server sent the authentication result to the client." << endl;
            }  else if (responseType == "AvailabilityResponse") {
                // Send availability information to client
                response = packageMessage(response, responseType, requestID, "client");
                if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
                    cerr << "Send to client failed due to " << strerror(errno) << endl;
                    return;
                }
                cout << "The main server sent the availability information to the client." << endl;
            } else if (responseType == "ReservationResponse") {
                // Send reservation result to client
                response = packageMessage(response, responseType, requestID, "client");
                if (send(clientSocket, response.c_str(), response.length(), 0) < 0) {
                    cerr << "Send to client failed due to " << strerror(errno) << endl;
                    return;
                }
                cout << "The main server sent the reservation result to the client." << endl;
            }
        }
    }
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

        // Create new socket for new incoming TCP connection
        if (FD_ISSET(tcpSocket, &readfds)) {

            // Accept new connection
            if ((new_socket = accept(tcpSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                cerr << "Accept failed" << endl;
                return -1;
            }

            // Add new socket to array of sockets
            clientSockets.push_back(new_socket);

            // Debugging: shows information about the new connection
            // cout << "New connection, socket fd is " << new_socket << ", ip is " << inet_ntoa(address.sin_addr) << ", port is " << ntohs(address.sin_port) << endl;
        }
        
        // Handle UDP activity
        if (FD_ISSET(udpSocket, &readfds)) {
            handleServer(udpSocket, serverCAddr, serverRTHAddr, serverEEBAddr);
        }


        // Handle TCP activity
        for (size_t i = 0; i < clientSockets.size(); ++i) {
            sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds)) {
                handleClient(sd, udpSocket, serverCAddr, serverRTHAddr, serverEEBAddr);
                if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
                    clientSockets.erase(clientSockets.begin() + i);
                }
            }
        }
        
    
    }

    return 0;
}
