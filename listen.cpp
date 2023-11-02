#include <iostream>
#include <windows.h>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

class Stock {
public:
    string symbol;
    string name;
    long long marketCap;
    double lastSale;

    Stock(const string& s, const string& n, long long m, double l)
        : symbol(s), name(n), marketCap(m), lastSale(l) {}
};


class ReadStock {
public:
    vector<Stock> stockList;
    void loadcsv() {
        ifstream inputFile("stock_info_full.csv");
        if (!inputFile) {
            cerr << "Failed to open the CSV file." << endl;
            return;
        } else {
            cout << "Opened the CSV file." << endl;
        }
        string line;
        getline(inputFile, line); // Skip the header line
        int counter = 0;
        while (getline(inputFile, line)) {
            counter++;
            vector<string> tokens;
            istringstream ss(line);
            string token;

            string symbol;
            string name;
            string marketCap;
            string lastSale;

            getline(ss, symbol, ',');
            getline(ss, name, ',');
            getline(ss, marketCap, '$'); // Read until the before $ character
            getline(ss, lastSale, ' ');  // Read until the before empty character
            
            //checking code 
            //cout << symbol << " " << name << "marketCap: " << marketCap << "lastSale: " << lastSale << endl;


            // Remove commas and quotes from market cap and dollar sign from last sale using algorithm library > faster and efficient
            marketCap.erase(remove(marketCap.begin(), marketCap.end(), ','), marketCap.end());
            marketCap.erase(remove(marketCap.begin(), marketCap.end(), '"'), marketCap.end());
        
            //checking code
            //cout << symbol << " " << name << " " << marketCap << " " << lastSale << endl;

            stockList.push_back(Stock(symbol, name, stoll(marketCap), stod(lastSale)));
        }
        inputFile.close();

        if (stockList.empty()) {
            cout << "ALERT: No data loaded." << endl;
        }
        else if (stockList.size()!= counter){
            cout << "ALERT: Data not loaded completely." << endl;
        }
        else if (stockList.size()==counter){
            cout << "Data loaded successfully." << endl;
        }
    }

    void printStock() {
        for (int i = 0; i < stockList.size(); i++) {
            cout << stockList[i].symbol << " " << stockList[i].name << " " << stockList[i].marketCap << " " << stockList[i].lastSale << endl;
        }
    }
    vector<Stock> returnStock(){
        return stockList;
    }
};

class StockServer {
public:
    StockServer(int port) : port(port) {}

    void Start() {
        //take in the csv stock data
        ReadStock stockData;
        stockData.loadcsv();
        vector<Stock> maindata = stockData.returnStock();

        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed" << endl;
            return;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Socket creation failed" << endl;
            WSACleanup();
            return;
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
            cerr << "Bind failed" << endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            cerr << "Listen failed" << endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        cout << "StockServer is listening for incoming connections on port " << port << "..." << endl;

        vector<thread> clientThreads;

        while (true) {
            SOCKET clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                cerr << "Accept failed" << endl;
                closesocket(serverSocket);
                WSACleanup();
                return;
            }

            // Create a new thread to handle the client
            clientThreads.emplace_back(&StockServer::HandleClient, this, clientSocket);
        }

        // Wait for all client threads to finish before exiting
        for (auto& thread : clientThreads) {
            thread.join();
        }

        closesocket(serverSocket);
        WSACleanup();
    }

private:
    int port;
    SOCKET serverSocket;
    //how server reply to client
    //algorithm start here
    void HandleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';

        // Check if the received message contains colons
        char* colonPosition = strchr(buffer, ':');
        if (colonPosition != nullptr) {
            *colonPosition = '\0';
            const char* username = buffer;

            // Move to the next component
            char* nextColonPosition = strchr(colonPosition + 1, ':');
            const char* action = colonPosition + 1;
           if (strcmp(action, "quit") == 0) {
            // Send a "bye" message along with the username
            char byeMessage[1024];
            sprintf(byeMessage, "bye %s", username);
            send(clientSocket, byeMessage, strlen(byeMessage), 0);
        } else if (strcmp(action, "1") == 0) {
            // Send the stock data as a response
            const char* stockData = "AAPL:150:250.0, MSFT:100:300.0";
            send(clientSocket, stockData, strlen(stockData), 0);
        } else if (strcmp(action, "2") == 0) {
            // Extract the symbol, quantity, and price
            char* nextColonPosition = strchr(nextColonPosition + 1, ':');
            if (nextColonPosition != nullptr) {
                *nextColonPosition = '\0';
                const char* symbol = nextColonPosition + 1;
                
                char* colonPosition2 = strchr(nextColonPosition + 1, ':');
                if (colonPosition2 != nullptr) {
                    *colonPosition2 = '\0';
                    const char* quantity = colonPosition2 + 1;

                    const char* price = colonPosition2 + 1;

                    // Process the buy stock action
                    cout << "Received from client - Username: " << username << ", Action: " << action << ", Symbol: " << symbol << ", Quantity: " << quantity << ", Price: " << price << endl;

                    // Handle the purchase logic and send a response
                    const char* response = "Stock purchased.";
                    send(clientSocket, response, strlen(response), 0);
                }
            }
    } else {
        // Process other actions as needed
        cout << "Received from client - Username: " << username << ", Action: " << action << endl;

        // Send a response
        const char* response = "Message received.";
        send(clientSocket, response, strlen(response), 0);
    }
        } else {
            // If no colon is found, send an error response
            const char* response = "Invalid message format. Use 'username:action:symbol:quantity:price'.";
            send(clientSocket, response, strlen(response), 0);
        }
    }

    closesocket(clientSocket);
    }


};


int main() {
    // Create a StockServer instance on port 12345
    StockServer server(12345); 
    server.Start();
    return 0;
}
