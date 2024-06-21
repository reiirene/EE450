// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverM.cpp

#ifndef SERVERM_H
#define SERVERM_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

struct Client {
    string username;
    string password;
};

#define SERVER_M_UDP_PORT 34288
#define SERVER_M_TCP_PORT 35288
#define SERVER_C_PORT 31288
#define SERVER_RTH_PORT 32288
#define SERVER_EEB_PORT 33288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string packageMessage (const string &message, const string &type);
void clientRequest (int clientSocket, int udpSocketRTH, int udpSocketEEB, string &request, string &requestType, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr);
void handleClient (int clientSocket, int udpSocketC, struct sockaddr_in serverCAddr, int udpSocketRTH, struct sockaddr_in serverRTHAddr, int udpSocketEEB, struct sockaddr_in serverEEBAddr);
void handleServerC (int udpSocketC, int tcpClientSocket);
void handleServerRTH (int udpSocketRTH, int tcpClientSocket);
void handleServerEEB (int udpSocketEEB, int tcpClientSocket);

#endif // SERVERM_H