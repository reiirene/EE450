// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverEEB.cpp

#ifndef SERVEREEB_H
#define SERVEREEB_H

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
#include <vector>

using namespace std;

#define HOST "127.0.0.1"
#define SERVER_EEB_PORT 33288
#define CLIENT_M_PORT 34288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string parseRequestID (const string &data);
string packageMessage (const string &message, const string &type, const string &requestID);
string roomReservation (int udpSocket, const string &room, const string &day, const string &times, vector<string> &reservedRooms);
string searchAvailability (int udpSocket, const string &room, const string &day, const string &times, const vector<string> &reservedRooms);

#endif // SERVEREEB_H