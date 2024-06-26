// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: This program is the server for login credentials authentication.

#include "serverC.h"

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
        }
    }
    return "";
}

string packageMessage (const string &message, const string &type, const string &requestID) {
    return "MessageType=" + type + "&" + message + "*" + requestID;
}

// Referenced (1) login code for lines 43~64
string authenticate (const string &username, const string &password) {
    // Go through members.txt and check if username and password are valid
    ifstream file("member.txt");
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(",");
        if (pos != string::npos) {
            string fileUser = line.substr(0, pos);
            string filePass = line.substr(pos + 2);
            if (fileUser == username) {
                if (filePass == password) {
                    cout << "Successful authentication." << endl;
                    return "valid";
                } else {
                    cout << "Password does not match." << endl;
                    return "incorrect_password";
                }
            }
        }
    }
    cout << "Username does not exist." << endl;
    return "user_not_found";
}

// Referenced (3) UDP server code for lines 69~144
int main() {
    int udpSocket;
    struct sockaddr_in serverCAddr, serverMAddr;
    char buffer[BUFFER_SIZE] = {0};
    socklen_t serverMAddrLen = sizeof(serverMAddr);

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        // cerr << "UDP socket creation error" << endl;
        return -1;
    }

    // Set up serverC address
    memset(&serverCAddr, 0, sizeof(serverCAddr));
    serverCAddr.sin_family = AF_INET;
    serverCAddr.sin_port = htons(SERVER_C_PORT);
    serverCAddr.sin_addr.s_addr = inet_addr(HOST);

    
    // Bind socket to port
    if (bind(udpSocket, (struct sockaddr *)&serverCAddr, sizeof(serverCAddr)) < 0) {
        // cerr << "Bind failed with " << strerror(errno) << endl;
        close(udpSocket);
        return -1;
    }

    cout << "The Server C is up and running using UDP on port " << SERVER_C_PORT << "." << endl;

    // Set up serverM address
    memset(&serverMAddr, 0, sizeof(serverMAddr));
    serverMAddr.sin_family = AF_INET;
    serverMAddr.sin_port = htons(CLIENT_M_PORT);
    serverMAddr.sin_addr.s_addr = inet_addr(HOST);

    // Send boot message to main server, Referenced (4) for lines 104~109
    string bootMsg = "boot";
    if (sendto(udpSocket, bootMsg.c_str(), bootMsg.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
        // cerr << "Send failed" << endl;
        return -1;
    }
    cout << "The Server C has informed the main server." << endl;

    // Receive authentication request from serverM
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // clear buffer
        if (recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverMAddr, &serverMAddrLen) < 0) {
            // cerr << "Receive failed" << endl;
            return -1;
        }
        cout << "The Server C received an authentication request from the main server." << endl;

        // Extract username and password
        string message = buffer;
        string requestID = parseRequestID(message);
        message = parseMessage(message);
        size_t pos = message.find(" ");
        string username = message.substr(0, pos);
        string password = message.substr(pos + 1);

        // Authenticate user
        string response = authenticate(username, password);

        // Send authentication result to main server
        response = packageMessage(response, "AuthenticationResponse", requestID);
        if (sendto(udpSocket, response.c_str(), response.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
            // cerr << "Send failed" << endl;
            return -1;
        }
        cout << "The Server C finished sending the response to the main server." << endl;
    }
    

    // Close socket
    close(udpSocket);

    return 0;
}
