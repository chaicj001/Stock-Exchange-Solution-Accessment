#include <iostream>
#include <windows.h>
#include <string>

using namespace std;
SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);


void listenserver(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        cout << "Received from server: " << buffer << endl;
    }
}
SOCKET connectToServer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return INVALID_SOCKET;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP address
    serverAddress.sin_port = htons(12345); // Server port

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return clientSocket;
}

int main() {
     
    int quantity=10;
    double price=170;
    for (int i=0;10>i;i++){
        SOCKET clientSocket = connectToServer();
        if (clientSocket == INVALID_SOCKET) {
        return 1;
    }
        string message;
        message= "tester:2:ADI:";
        message += to_string(quantity);
        message +=":";
        message += to_string(price);
        send(clientSocket, message.c_str(), message.size(), 0);
        price++;
        quantity+=10;
        cout << message << endl;
    }
    

    listenserver(clientSocket);
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}