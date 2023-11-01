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

    while (true) {
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

        string message = "hi server spam here";
        send(clientSocket, message.c_str(), message.size(), 0);

        // Close the connection for this message
        closesocket(clientSocket);

        // Optional: Sleep or add a delay before sending the next message
        // Sleep(1000); // Sleep for 1 second
    }

    WSACleanup();
    return 0;
}