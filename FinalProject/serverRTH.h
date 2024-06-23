// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverRTH.cpp

#ifndef SERVERRTH_H
#define SERVERRTH_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <vector>

using namespace std;

#define HOST "127.0.0.1"
#define SERVER_RTH_PORT 32288
#define CLIENT_M_PORT 34288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string parseRequestID (const string &data);
string packageMessage (const string &message, const string &type, const string &requestID);
string roomReservation (int udpSocket, const string &room, const string &day, const string &times, vector<string> &reservedRooms);
string searchAvailability (int udpSocket, const string &room, const string &day, const string &times, const vector<string> &reservedRooms);

#endif // SERVERRTH_H