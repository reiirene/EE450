// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverM.cpp

#ifndef SERVERM_H
#define SERVERM_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <thread>
#include <signal.h>
#include <map>

using namespace std;

struct Client {
    string username;
    string password;
    string status;
};

#define HOST "127.0.0.1"
#define CLIENT_M_UDP_PORT 34288
#define SERVER_M_TCP_PORT 35288
#define SERVER_C_PORT 31288
#define SERVER_RTH_PORT 32288
#define SERVER_EEB_PORT 33288
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string parseRequestID (const string &data);
string packageMessage (const string &message, const string &type, const string &requestID, const string &receiver);
// void bootCheck (int udpSocket, struct sockaddr_in clientAddr, socklen_t clientAddrLen);
void clientRequest (int clientSocket, int udpSocketRTH, int udpSocketEEB, string &request, string &requestType, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr);
void handleClient (int clientSocket, int udpSocket, struct sockaddr_in serverCAddr, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr);
void handleServer (int udpSocket, struct sockaddr_in serverCAddr, struct sockaddr_in serverRTHAddr, struct sockaddr_in serverEEBAddr);
// string handleServerC (int udpSocketC, const struct sockaddr_in &serverCAddr, const char *request);
// string handleServerRTH (int udpSocketRTH, const struct sockaddr_in &serverRTHAddr, string &request, string &requestType);
// string handleServerEEB (int udpSocketEEB, const struct sockaddr_in &serverEEBAddr, string &request, string &requestType);
#endif // SERVERM_H