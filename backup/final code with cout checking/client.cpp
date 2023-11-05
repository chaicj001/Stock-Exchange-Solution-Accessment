#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <unordered_map>


using namespace std;

struct Order {
    int orderid;
    string symbol;
    string action;
    double price;
    int quantity;
};

vector <Order> holding;
SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

void listenserver(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';

        istringstream iss(buffer);
        string firstToken;
        string secondToken;
        getline(iss, firstToken, ':'); // Read the first part of the reply
        getline(iss, secondToken, '|');

        if (firstToken == "SELL" || firstToken == "BUY") {
            // The reply follows the expected format
            string token;
            Order order;

            // Parse the order information
            
            getline(iss, token, '|'); // Read orderid 
            order.orderid = stoi(token);
            

            getline(iss, token, '|'); // Read symbol
            order.symbol = token;

            getline(iss, token, '|'); // Read price
            order.price = stod(token);

            getline(iss, token, '|'); // Read quantity
            order.quantity = stoi(token);
            order.action = firstToken;

            // Push the order into the vector of orders
            holding.push_back(order);

        }
        cout << "Received from server: " << firstToken <<" " << secondToken << endl;
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
    string username;
    cout << "Enter your username:";
    cin >> username;
    while (true){

        string message= username+":";
        string action;
        string symbol;
        string price;
        string quantity;

        SOCKET clientSocket = connectToServer();
        if (clientSocket == INVALID_SOCKET) {
            return 1;
        }
        cout << "Menu Option" << endl;
        cout << "1. View Stock" << endl;
        cout << "2. Sell Stock" << endl;
        cout << "3. Buy Stock" << endl;
        cout << "4. View All Pending Stock" << endl;
        cout << "5. View Holding " << endl;
        cout << "quit - Exit program" << endl;
        cout << "Enter your action:";
        cin >> action;
        message += action +":";
        //quit function first 
        if (action == "quit") {
            send(clientSocket, message.c_str(), message.size(), 0);
            //quit condition hear from server before quit 
            listenserver(clientSocket);
            closesocket(clientSocket);
            WSACleanup();
            break;
        }


       
        //switch case for action but it not possible to done in c++
        //hence, using traditional way with if statement to check the action and give respond to user
        if (action=="2"){
            cout << "Sell Stock"<< endl;
            cout << "Enter your stock you want to sell (Symbol):";
            cin >> symbol;
            message += symbol +":";

            cout << "Enter your quantity:";
            cin >> quantity;
            message += quantity +":";

            cout << "Enter your price:";
            cin >> price;
            message += price +":";
        }
        else if (action=="3"){
            cout << "Buy Stock"<< endl;
            cout << "Enter your stock you want to buy (Symbol):";
            cin >> symbol;
            message += symbol +":";

            cout << "Enter your quantity:";
            cin >> quantity;
            message += quantity +":";

            cout << "Enter your price:";
            cin >> price;
            message += price +":";     

        }
        else if (action=="4"){
            cout << "View Pending Stock"<< endl;
        }
        else if (action=="5"){
            cout << "Your Holdings"<< endl;
            for (Order& order : holding) {
        cout << "Order ID: " << order.orderid <<", "<< order.action << ", Symbol: " << order.symbol
             << ", Price: " << order.price << ", Quantity: " << order.quantity << endl;
        }
        }        
        else{
            cout <<"Action:" << action << endl;
        }

        send(clientSocket, message.c_str(), message.size(), 0);


        listenserver(clientSocket);      
        closesocket(clientSocket);
        WSACleanup();
    }
    return 0;
}