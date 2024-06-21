// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: Client side of the room reservation system

#include "client.h"

int loggedIn = 0;
string status = "";
Client user;

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

string parseMessage (const string &data) {
    size_t message_pos = data.find("&");
    if (message_pos != string::npos) {
        return data.substr(message_pos + 1);
    }
    return "";
}

string packageMessage (const string &message, const string &type) {
    return "MessageType=" + type + "&" + message;
}

// Offset each Engli halphabet letter and digit by a value n
// n corresponds to the character's position within the username or password from the left
// Encryption is case sensitive
// Encryption cyclically shifts for overflow
string encrypt(const string &text) {
    string encrypted = text;
    for (size_t i = 0; i < text.length(); ++i) {
        if (isalpha(text[i])) {
            if (isupper(text[i])) {
                encrypted[i] = (text[i] - 'A' + i + 1) % 26 + 'A';
            } else {
                encrypted[i] = (text[i] - 'a' + i + 1) % 26 + 'a';  
            }
        } else if (isdigit(text[i])) {
            encrypted[i] = (text[i] - '0' + i + 1) % 10 + '0';
        }
    }
    return encrypted;
}

// Client login to determine if user is member or guest
void clientLogin (int clientSocket) {
    string enc_username, enc_password;

    cout << "Please enter the username: ";
    getline(cin, user.username);

    if (!user.username.empty()) {
        cout << "Please enter the password: ";
        getline(cin, user.password);

        // Encrypt username
        enc_username = encrypt(user.username);

        if (user.password.empty()) {
            string message = enc_username;
            if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
                cerr << "Error sending data" << endl;
                return;
            }
            cout << user.username << " sent an authentication request to the main server." << endl;
        } else {
            // Encrypt password
            enc_password = encrypt(user.password);

            // Send username and password to serverM
            string message = enc_username + " " + enc_password;
            if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
                cerr << "Error sending data" << endl;
                return;
            }
            cout << user.username << " sent an authentication request to the main server." << endl;
        }

        // Receive authentication result from serverM
        char buffer[BUFFER_SIZE] = { 0 };
        if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
            cerr << "Error receiving data" << endl;
            return;
        }

        // Check authentication result
        status = buffer;
        if (status.compare("guest") == 0) {
            cout << "Welcome guest " << user.username << "!" << endl;
            loggedIn = 1;  // Update login status
            return;
        } else if (status.compare("valid") == 0) {
            cout << "Welcome member " << user.username << "!" << endl;
            loggedIn = 1;  // Update login status
            return;
        } else if (status.compare("user_not_found") == 0) {
            cout << "Failed login: Username does not exist." << endl;
            user.username = "";
            user.password = "";
        } else if (status.compare("incorrect_password") == 0) {
            cout << "Failed login: Password does not match." << endl;
            user.username = "";
            user.password = "";
        } else {
            cout << "Unknown status." << endl;
        }
    }
}

void printAvailability (const string &message, const string &day, const string &times) {
    string result = message;
    string delimiter = "\n";

    if (day == "empty" && times == "empty") {
        cout << "The request room is available at the following time slots:" << endl;
    } else if (day != "empty" && times == "empty") {
        cout << "The request room is available at the following time slots on " << day << ":" << endl;
    }

    // Print availability result
    size_t entry_end = result.find(delimiter);
    if (entry_end != string::npos) {
        size_t pos = 0;
        string entry;
        while ((pos = result.find(delimiter)) != string::npos) {
            entry = result.substr(0, pos);
            cout << entry << endl;
            result.erase(0, pos + delimiter.length());
        }
    } else {
        cout << result << endl;
    }

    cout << endl;
    cout << "-----Start a new request-----" << endl;
}

void availabilityRequest (int clientSocket, const string &room, const string &day, const string &times, const string &hour, const string &period, 
                            const string &action, struct sockaddr_in clientAddr) {
    // Send room number, day, time, and action to serverM
    string message = room + " " + day + " " + hour + " " + period + " " + action;

    message = packageMessage(message, "AvailabilityRequest");
    if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
        cerr << "Error sending data" << endl;
        return;
    }
    cout << user.username << " sent an availability request to the main server." << endl;

    // Receive availability result from serverM
    char buffer[BUFFER_SIZE] = { 0 };
    if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
        cerr << "Error receiving data" << endl;
        return;
    }

    // Get client's port number
    int clientPort = ntohs(clientAddr.sin_port);
    cout << "The client received the response from the main server using TCP over port " << clientPort << endl;

    // Print availability result
    string response = buffer;
    response = parseMessage(response);
    printAvailability(response, day, times);
}

void reservationRequest (int clientSocket, const string &room, const string &day, const string &times, const string &hour, const string &period, 
                            const string &action, struct sockaddr_in clientAddr) {
    // Send room number, day, time, and action to serverM
    string message = room + " " + day + " " + hour + " " + period + " " + action;

    message = packageMessage(message, "ReservationRequest");
    if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
        cerr << "Error sending data" << endl;
        return;
    }
    cout << user.username << " sent a reservation request to the main server." << endl;

    // Receive reservation result from serverM
    char buffer[BUFFER_SIZE] = { 0 };
    if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
        cerr << "Error receiving data" << endl;
        return;
    }

    // Get client's port number
    int clientPort = ntohs(clientAddr.sin_port);
    cout << "The client received the response from the main server using TCP over port " << clientPort << endl;

    // Print reservation result
    string response = buffer;
    response = parseMessage(response);
    
    cout << response << endl;
    // Print reservation result
    if (response.compare("success") == 0) {
        cout << "Congratulation! The reservation for Room " << room << " has been made." << endl;
    } else if (response.compare("unavailable") == 0) {
        cout << "Sorry! The requested room is not available." << endl;
    } else if (response.compare("not_found") == 0) {
        cout << "Oops! Not able to find the room." << endl;
    } else {
        cout << "Unknown response." << endl;
    }

    cout << endl;
    cout << "-----Start a new request-----" << endl;
}

void memberClient (int clientSocket, struct sockaddr_in clientAddr) {
    string room, day, times, hour, period, action;

    while (true) {
        if (room == "" && action == "") {
            cout << "Please enter the room number: ";
            getline(cin, room);

            // Error handling
            if (room.empty() || (action == "Reservation" && (day == "empty" || times.empty()))) {
                cout << "Missing Argument" << endl << endl;
                cout << "-----Start a new request-----" << endl;
            }

            cout << "Please enter the day: ";
            getline(cin, day);
            if (day.empty()) {
                day = "empty";
                times = "empty";
                hour = "empty";
                period = "empty";
            } else {
                cout << "Please enter the time: ";
                getline(cin, times);
                stringstream ss(times);
                ss >> hour >> period;
                if (hour.empty() || period.empty()) {
                    hour = "empty";
                    period = "empty";
                }
            }

            cout << "Would you like to search for the availability or make a reservation? ";
            cout << "(Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation): " << endl;
            getline(cin, action);

        } else {
            if (action == "Availability" || action == "availability") {
                availabilityRequest(clientSocket, room, day, times, hour, period, action, clientAddr);

                return;
            } else if (action == "Reservation" || action == "reservation") {
                reservationRequest(clientSocket, room, day, times, hour, period, action, clientAddr);

                return;
            } else {
                cout << "Invalid action." << endl;
                return;
            }
        }
    }
}

void guestClient(int clientSocket, struct sockaddr_in clientAddr) {
    string room, day, times, hour, period, action;

    while (true) {
        if (room == "" && action == "") {
            cout << "Please enter the room number: ";
            getline(cin, room);

            // Error handling
            if (room.empty()) {
                cout << "Missing Argument" << endl << endl;
                cout << "-----Start a new request-----" << endl;
            }

            cout << "Please enter the day: ";
            getline(cin, day);
            if (day.empty()) {
                day = "empty";
                times = "empty";
                hour = "empty";
                period = "empty";
            } else {
                cout << "Please enter the time: ";
                getline(cin, times);
                stringstream ss(times);
                ss >> hour >> period;
                if (hour.empty() || period.empty()) {
                    hour = "empty";
                    period = "empty";
                }
            }

            cout << "Would you like to search for the availability or make a reservation? ";
            cout << "(Enter “Availability” to search for the availability or Enter “Reservation” to make a reservation): " << endl;
            getline(cin, action);

        } else {
            if (action == "Availability" || action == "availability") {
                availabilityRequest(clientSocket, room, day, times, hour, period, action, clientAddr);

                return;
            } else if (action == "Reservation" || action == "reservation") {
                
            } else {
                cout << "Invalid action." << endl;
                return;
            }
        }
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr, clientAddr;

    // Create TCP socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    // Specify serverM address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_M_TCP_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Connect to serverM
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection failed" << endl;
        return -1;
    }

    cout << "Client is up and running." << endl;

    while (true) {
        // Client Login
        if (loggedIn == 0) {
            clientLogin(clientSocket);
        }

        // Accomodate member client
        if (loggedIn == 1 && status.compare("valid") == 0) {
            memberClient(clientSocket, clientAddr);
        } else if (loggedIn == 1 && status.compare("guest") == 0) {
            guestClient(clientSocket, clientAddr);
        }
    }


    // Close socket
    close(clientSocket);

    return 0;
}