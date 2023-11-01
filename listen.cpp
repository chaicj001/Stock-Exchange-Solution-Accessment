#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>

using namespace std;

void HandleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        cout << "Received from client: " << buffer << endl;

        // You can send a response back to the client if needed.
        const char* response = "Hello, client!";
        send(clientSocket, response, strlen(response), 0);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(12345); // Replace with the desired port

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "Bind failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening for incoming connections..." << endl;

    vector<thread> clientThreads;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed" << endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        // Create a new thread to handle the client
        clientThreads.emplace_back(HandleClient, clientSocket);
    }

    // Wait for all client threads to finish before exiting
    for (auto& thread : clientThreads) {
        thread.join();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
