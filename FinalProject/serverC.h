// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Header file for serverC.cpp

#ifndef SERVERC_H
#define SERVERC_H

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

using namespace std;

#define HOST "127.0.0.1"
#define SERVER_C_PORT 31288
#define CLIENT_M_PORT 34288
#define BUFFER_SIZE 1024

// Function declarations
string parseMessageType (const string &data);
string parseRequestID (const string &data);
string parseMessage (const string &data);
string packageMessage (const string &message, const string &type, const string &requestID);
string authenticate (const string &username, const string &password);

#endif // SERVERC_H