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

class Stock {
public:
    string symbol;
    string name;
    long long marketCap;
    double lastSale;

    Stock(const string& s, const string& n, long long m, double l)
        : symbol(s), name(n), marketCap(m), lastSale(l) {}

    // Define the equality operator for Stock
    bool operator==(const Stock& other) const {
        return symbol == other.symbol;
    }
};

//give up for using stock class since order do not need user to key in entire stock name
//for order matching later later
struct OrderKey {
    string symbol;
    double price;
    int quantity;

    bool operator==(const OrderKey& other) const {
        return symbol == other.symbol && price == other.price && quantity == other.quantity;
    }
};

struct OrderKeyHash {
    std::size_t operator()(const OrderKey& key) const {
        size_t h1 = std::hash<string>{}(key.symbol);
        size_t h2 = std::hash<double>{}(key.price);
        size_t h3 = std::hash<int>{}(key.quantity);
        return h1 ^ (h2 << 1) ^ h3;
    }
};

//from option 2 and 3 from this struct after taking input from user
struct Order {
    string username;
    string symbol;
    double price;
    int quantity;
};



class ReadStock {
public:
    vector<Stock> stockList;
    void loadcsv() {
        ifstream inputFile("stock_info.csv");
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
    WSADATA wsaData;
    StockServer(int port) : port(port) {
        stockData.loadcsv();
        maindata = stockData.returnStock();
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
    }

    void Start() {
        //server socket
        if (serverSocket == INVALID_SOCKET) {
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

    ~StockServer() {
        closesocket(serverSocket);
        WSACleanup();
    }

    void placeBuyOrder(const Order& order) {
        OrderKey key = {order.symbol, order.price, order.quantity};
        pendingBuyOrders[key].push_back(order);
    }

    void placeSellOrder(const Order& order) {
        OrderKey key = {order.symbol, order.price, order.quantity};
        pendingSellOrders[key].push_back(order);
    }
string displayPendingOrders() {
    stringstream output;

    output << "\n Pending Buy Orders:" << endl;
    for (const auto& entry : pendingBuyOrders) {
        const OrderKey& key = entry.first;
        const vector<Order>& orders = entry.second;

        for (const Order& order : orders) {
            output <<"Username: " << order.username << ", Symbol: " << key.symbol << ", Price: " << key.price << ", Quantity: " << key.quantity << endl;
        }
    }

    output << "Pending Sell Orders:" << endl;
    for (const auto& entry : pendingSellOrders) {
        const OrderKey& key = entry.first;
        const vector<Order>& orders = entry.second;

        for (const Order& order : orders) {
            output <<"Username: " << order.username << ", Symbol: " << key.symbol << ", Price: " << key.price << ", Quantity: " << key.quantity << endl;
        }
    }

    return output.str();
}

private:    
    unordered_map<OrderKey, vector<Order>, OrderKeyHash> pendingBuyOrders;
    unordered_map<OrderKey, vector<Order>, OrderKeyHash> pendingSellOrders;

    int port;
    SOCKET serverSocket;

    //initial start to read stock data from csv 
    ReadStock stockData;
    vector<Stock> maindata;

    //how server reply to client

    //algorithm start here
    void HandleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        // Handle invalid or empty messages if needed
        const char* response = "ALERT: No message was received!";
        send(clientSocket, response, strlen(response), 0);
        closesocket(clientSocket);
        return;
    }
    //receiver
    buffer[bytesRead] = '\0';

    char* token = strtok(buffer, ":");
    const char* username = token;
    token = strtok(nullptr, ":");
    //if username = null / alert
    if (token == nullptr) {
        // Invalid message format
        const char* response = "ALERT: No username/action was received!";
        send(clientSocket, response, strlen(response), 0);
        closesocket(clientSocket);
        return;
    }

    const char* action = token;

    if (strcmp(action, "quit") == 0) {
        char byeMessage[1024];
        sprintf(byeMessage, "bye %s", username);
        send(clientSocket, byeMessage, strlen(byeMessage), 0);
    } else if (strcmp(action, "1") == 0) {
    // Construct the stock data as a response
    string response;
    response += "\nSymbol :Company Name                               :Market Cap     :Last Price \n";
    for (int i = 0; i < maindata.size(); i++) {
        response += maindata[i].symbol;
        int spaceCount = 7 - maindata[i].symbol.length();
        for (int i = 0; i < spaceCount; i++) {
            response += " ";
        }
        response+= ":"+ maindata[i].name;
        int spaceCount2 = 43 - maindata[i].name.length();
        for (int i = 0; i < spaceCount2; i++) {
            response += " ";
        }
        response+=":" + to_string(maindata[i].marketCap);
        int spaceCount3 = 15 - to_string(maindata[i].marketCap).length();
        for (int i = 0; i < spaceCount3; i++) {
            response += " ";
        }
        response+=":" + to_string(maindata[i].lastSale);
        if (i < maindata.size() - 1) {
            response += "\n";
        }
    }
    // Send the response to the client
    send(clientSocket, response.c_str(), response.length(), 0);
        //cout at server side for checking
        cout << "Received from client - Username: " << username << ", Action: Request Display Available Stock" << endl;
    } 
    //second action that use do is sell stock format is username:symbol:quantity:price
    else if (strcmp(action, "2") == 0) {
        token = strtok(nullptr, ":");
        if (token != nullptr) {
            //symbol variable
            const char* symbol = token;
            token = strtok(nullptr, ":");
            if (token != nullptr) {
                //quantity variable
                const char* quantity = token;
                token = strtok(nullptr, ":");
                if (token != nullptr) {
                    //price variable
                    const char* price = token;
                    // Process the buy stock action
                    cout << "Received from client - Username: " << username << ", Action: " << action
                         << ", Symbol: " << symbol << ", Quantity: " << quantity << ", Price: " << price << endl;
                    

                    //matching logic and pending logic here
                    Order order = {username, symbol,stod(price),stoi(quantity)};
                    placeSellOrder(order);

                    // Handle the purchase logic and send a response
                    const char* response = "Stock listed in pending list.";
                    send(clientSocket, response, strlen(response), 0);
                }
            }
        }
    } 
    else if (strcmp(action, "4") == 0) {
        string response = displayPendingOrders();
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    else {
        // Process other actions as needed
        cout << "Received from client - Username: " << username << ", Action: " << action << endl;

        // Send a response
        const char* response = "Message received in else method.";
        send(clientSocket, response, strlen(response), 0);
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
