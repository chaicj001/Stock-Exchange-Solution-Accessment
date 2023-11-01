#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    serverAddress.sin_port = htons(12345); // Server port

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "into whlie loop!" << endl;    
    while (true) {
        string message=" ";
        cout << "Enter a message to send to the server (type 'quit' to exit): ";

        if (!getline(cin, message)) {
            cerr << "Failed to read input." << endl;
            break;
        }
        cout << ("your message was sent") << endl;
        if (message == "quit") {
            break;
        }

        // Remove the newline character if present
        if (!message.empty() && message[message.size() - 1] == '\n') {
            message.pop_back();
        }

        send(clientSocket, message.c_str(), message.size(), 0);

        char buffer[1024];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            cout << "Received from server: " << buffer << endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}