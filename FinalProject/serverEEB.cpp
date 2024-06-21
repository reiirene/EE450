// Hsin Li
// 2024-06-20
// EE450 Final Project
// Description: This program is the server for the building EEB.
// It receives availability and reservation requests from the main server and sends back responses.

#include "serverEEB.h"

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

string roomReservation (int udpSocket, const string &room, const string &day, const string &times, vector<string> &reservedRooms) {
    string result = "";
    string availability = searchAvailability(udpSocket, room, day, times, reservedRooms);
    
    if (availability == "The requested room is available.") {
        // Add reservation to reservedRooms
        reservedRooms.push_back(room + ", " + day + ", " + times);
        cout << "Successful reservation. The status of Room " << room << " is updated." << endl; 
        result = "success";
    } else if (availability == "The requested room is not available.") {
        cout << "Cannot make a reservation. Room " << room << " is not available." << endl;
        result = "unavailable";
    } else if (availability == "Not able to find the room.") {
        cout << "Cannot make a reservation. Not able to find the room layout" << endl;
        result = "not_found";
    }

    return result;

}

string searchAvailability (int udpSocket, const string &room, const string &day, const string &times, const vector<string> &reservedRooms) {
    string result = "";
    int roomFound = 0;

    // Search for availability
    ifstream file("EEB.txt");
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string fileRoom, fileDay, fileTime;

        // Read items separated by commas
        if (getline(ss, fileRoom, ',') && getline(ss, fileDay, ',') && getline(ss, fileTime, ',')) {
            // Trim leading spaces from day and time
            fileDay = fileDay.substr(fileDay.find_first_not_of(" "));
            fileTime = fileTime.substr(fileTime.find_first_not_of(" "));
        }

        if (day == "empty" && times == "empty"){    // if day and time are empty
            cout << "here" << endl;
            if (fileRoom == room) {
                if (reservedRooms.empty()) {
                    result += fileDay + ", " + fileTime + "\n";
                } else {
                    for (const string &reservedRoom : reservedRooms) {  // check if the room is reserved
                        if (reservedRoom != line) {
                            // Add to result if room is not reserved
                            result += fileDay + ", " + fileTime + "\n";
                        }
                    }
                }
                roomFound = 1;
            }
        } else if (day != "empty" && times == "empty") {    // if day is not empty and time is empty
            if (fileRoom == room && fileDay == day) {
                if (reservedRooms.empty()) {
                    result += fileDay + ", " + fileTime + "\n";
                } else {
                    for (const string &reservedRoom : reservedRooms) {  // check if the room is reserved
                        if (reservedRoom != line) {
                            // Add to result if room is not reserved
                            result += fileDay + ", " + fileTime + "\n";
                        }
                    }
                }
                roomFound = 1;
            }
        } else{   // if day and time are not empty
            if (fileRoom == room && fileDay == day && fileTime == times) {  // if the room, day, and time are the same
                if (reservedRooms.empty()) {
                    result = "The requested room is available.";
                    cout << "Room " << room << " is available at " << times << " on " << day << endl;
                } else {
                    for (const string &reservedRoom : reservedRooms) {  // check if the room is reserved
                        if (reservedRoom == line) {
                            result = "The requested room is not available.";
                            return result;
                        }
                    }
                    result = "The requested room is available.";
                }
                roomFound = 1;
                return result;
            } else if ((fileRoom == room && fileDay != day) || (fileRoom == room && fileTime != times)) {   // if the room is the same but the day or time is different
                roomFound = 1;
            }
        }
    
    }

    if (roomFound == 1) {
         if (day == "empty" && times == "empty") {
            cout << "All the availability of the room " << room << " has been extracted." << endl;
        } else if (day != "empty" && times == "empty") {
            cout << "All the availability of the room " << room << " on " << day << " has been extracted." << endl;
        } else {
            result = "The requested room is not available.";
        }
    } else {
        result = "Not able to find the room.";
    }

    return result;
}

int main() {
    int udpSocket;
    struct sockaddr_in serverEEBAddr, serverMAddr;
    char buffer[BUFFER_SIZE] = {0};
    socklen_t serverMAddrLen = sizeof(serverMAddr);
    vector <string> reservedRooms;

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        cerr << "UDP socket creation error" << endl;
        return -1;
    }

    // Set up serverRTH address
    memset(&serverEEBAddr, 0, sizeof(serverEEBAddr));
    serverEEBAddr.sin_family = AF_INET;
    serverEEBAddr.sin_port = htons(SERVER_EEB_PORT);
    serverEEBAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to port
    if (bind(udpSocket, (struct sockaddr *)&serverEEBAddr, sizeof(serverEEBAddr)) < 0) {
        cerr << "Bind failed" << endl;
        return -1;
    }

    cout << "Server EEB is up and running using UDP on port " << SERVER_EEB_PORT << endl;

    // Set up serverM address
    memset(&serverMAddr, 0, sizeof(serverMAddr));
    serverMAddr.sin_family = AF_INET;
    serverMAddr.sin_port = htons(SERVER_M_PORT);
    serverMAddr.sin_addr.s_addr = INADDR_ANY;

    // Send boot message to main server
    string bootMsg = "boot";
    if (sendto(udpSocket, bootMsg.c_str(), bootMsg.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
        cerr << "Error sending boot message" << endl;
        return -1;
    }
    cout << "Server EEB has informed the main server." << endl;

    // Receive availabiity/reservation request from serverM
    while (true) {
        memset(buffer, 0, sizeof(buffer));  // clear buffer
        if (recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverMAddr, &serverMAddrLen) < 0) {
            cerr << "Receive failed" << endl;
            return -1;
        }
        
        // Extract message to determine request type
        string requestType;
        string request = buffer;
        requestType = parseMessageType(request);
        request = parseMessage(request);

        stringstream message(request);
        string word;

        vector <string> words;
        while (message >> word) {
            words.push_back(word);
        }

        string room, day, hour, period, action;

        if (words.size() > 0) room = words[0];
        if (words.size() > 1) day = words[1];
        if (words.size() > 2) hour = words[2];
        if (words.size() > 3) period = words[3];
        if (words.size() > 4) action = words[4];

        // Combine hour and period
        string times;
        if (hour == "empty" && period == "empty") {
            times = "empty";
        } else {
            times = hour + " " + period;
        }

        // Determine request type
        if (requestType == "AvailabilityRequest") {
            // Search for availability
            cout << "The Server <EEB> received an availability request from the main server." << endl;
            string response = searchAvailability(udpSocket, room, day, times, reservedRooms);

            // print the availability
            if (response == "The requested room is available.") {
                cout << "Room " << room << " is available at " << times << " on " << day << endl;
            } else if (response == "The requested room is not available.") {
                cout << "Room " << room << " is not available at " << times << " on " << day << endl;
            } else if (response == "Not able to find the room.") {
                cout << "Not able to find the room " << room << endl;
            }

            response = packageMessage(response, "AvailabilityResponse");
            if (sendto(udpSocket, response.c_str(), response.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
                cerr << "Error sending availability response" << endl;
                return -1;
            }
            cout << "The Server <EEB> finished sending the response to the main server." << endl;
        } else if (requestType == "ReservationRequest") {
            // Make a reservation
            cout << "The Server <EEB> received a reservation request from the main server." << endl;
            string response = roomReservation(udpSocket, room, day, times, reservedRooms);
            response = packageMessage(response, "ReservationResponse");
            if (sendto(udpSocket, response.c_str(), response.size(), 0, (struct sockaddr *)&serverMAddr, sizeof(serverMAddr)) < 0) {
                cerr << "Error sending reservation response" << endl;
                return -1;
            }
            cout << "The Server <EEB> finished sending the response to the main server." << endl;
        } else {
            cerr << "Invalid action" << endl;
        }

    }

    // Close socket
    close(udpSocket);

    return 0;
}