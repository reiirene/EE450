// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: This program is the server for login credentials authentication.

#include "serverC.h"

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

int main() {
    int udpSocket;
    struct sockaddr_in serverCAddr, serverMAddr;
    char buffer[BUFFER_SIZE] = {0};
    socklen_t serverMAddrLen = sizeof(serverMAddr);

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }

    // Set up serverC address
    memset(&serverCAddr, 0, sizeof(serverCAddr));
    serverCAddr.sin_family = AF_INET;
    serverCAddr.sin_port = htons(SERVER_C_PORT);
    serverCAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port
    if (bind(udpSocket, (struct sockaddr *)&serverCAddr, sizeof(serverCAddr)) < 0) {
        cerr << "Bind failed with " << strerror(errno) << endl;
        return -1;
    }

    cout << "The Server C is up and running using UDP on port " << SERVER_C_PORT << "." << endl;

    // Set up serverM address
    memset(&serverMAddr, 0, sizeof(serverMAddr));
    serverMAddr.sin_family = AF_INET;
    serverMAddr.sin_port = htons(SERVER_M_PORT);
    serverMAddr.sin_addr.s_addr = INADDR_ANY;

    // Send boot message to main server
    string bootMsg = "boot";
    if (sendto(udpSocket, bootMsg.c_str(), bootMsg.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
        cerr << "Send failed" << endl;
        return -1;
    }
    cout << "The Server C has informed the main server." << endl;

    // Receive authentication request from serverM
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // clear buffer
        if (recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverMAddr, &serverMAddrLen) < 0) {
            cerr << "Receive failed" << endl;
            return -1;
        }
        cout << "The Server C received an authentication request from the main server." << endl;

        // Extract username and password
        string message = buffer;
        size_t pos = message.find(" ");
        string username = message.substr(0, pos);
        string password = message.substr(pos + 1);

        // Authenticate user
        string response = authenticate(username, password);

        // Send authentication result to main server
        if (sendto(udpSocket, response.c_str(), response.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
            cerr << "Send failed" << endl;
            return -1;
        }
        cout << "The Server C finished sending the response to the main server." << endl;
    }
    

    // Close socket
    close(udpSocket);

    return 0;
}
