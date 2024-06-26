// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for client.cpp

#ifndef CLIENT_H
#define CLIENT_H

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
#include <sstream>

using namespace std;

struct Client {
    string username;
    string password;
    string status;
};

#define HOST "127.0.0.1"
#define SERVER_M_TCP_PORT 35288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string packageMessage (const string &message, const string &type);
string encrypt (const string &text);
void clientLogin (int clientSocket, struct sockaddr_in clientAddr);
void printAvailability (const string &message, const string &day, const string &times);
void availabilityRequest (int clientSocket, const string &room, const string &day, const string &times, const string &hour, const string &period, 
                            const string &action, struct sockaddr_in clientAddr);
void reservationRequest (int clientSocket, const string &room, const string &day, const string &times, const string &hour, const string &period, 
                            const string &action, struct sockaddr_in clientAddr);
void memberClient (int clientSocket, struct sockaddr_in clientAddr);
void guestClient(int clientSocket, struct sockaddr_in clientAddr);

#endif // CLIENT_H
                            