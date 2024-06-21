// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverC.cpp

#ifndef SERVERC_H
#define SERVERC_H

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

#define SERVER_C_PORT 31288
#define SERVER_M_PORT 34288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseMessage (const string &data);
string packageMessage (const string &message, const string &type);
string authenticate (const string &username, const string &password);

#endif // SERVERC_H