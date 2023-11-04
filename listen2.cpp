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
    long long volume;
    double lastSale;

    Stock(const string& s, const string& n, long long m, double l)
        : symbol(s), name(n), volume(m), lastSale(l) {}

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
    int orderid;
    //what is determine as same orderkey price and symbol only
    bool operator==(const OrderKey& other) const {
        return symbol == other.symbol && price == other.price;
    }
    bool equal(const OrderKey& other) const {
        return symbol == other.symbol && price == other.price && quantity == other.quantity && orderid == other.orderid;
    }
};

/*struct OrderKeyHash {
    std::size_t operator()(const OrderKey& key) const {
        size_t h1 = std::hash<string>{}(key.symbol);
        size_t h2 = std::hash<double>{}(key.price);
        // this code make the hash function more unique to prevent overflow in later
        //size_t h3 = std::hash<size_t>{}(reinterpret_cast<size_t>(&key));  
        return h1 ^ (h2 << 1);
    }
};*/

struct OrderKeyHash {
    std::size_t operator()(const OrderKey& key) const {
        size_t h1 = std::hash<string>{}(key.symbol);
        size_t h2 = std::hash<double>{}(key.price);
        size_t h3 = std::hash<int>{}(key.orderid);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

//from option 2 and 3 from this struct after taking input from user
struct Order {
    int orderid;
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
            string volume;
            string lastSale;

            getline(ss, symbol, ',');
            getline(ss, name, ',');
            getline(ss, volume, '$'); // Read until the before $ character
            getline(ss, lastSale, ' ');  // Read until the before empty character
            
            //checking code 
            //cout << symbol << " " << name << "volume: " << volume << "lastSale: " << lastSale << endl;


            // Remove commas and quotes from volume and dollar sign from last sale using algorithm library > faster and efficient
            volume.erase(remove(volume.begin(), volume.end(), ','), volume.end());
            volume.erase(remove(volume.begin(), volume.end(), '"'), volume.end());
        
            //checking code
            //cout << symbol << " " << name << " " << volume << " " << lastSale << endl;

            stockList.push_back(Stock(symbol, name, stoll(volume), stod(lastSale)));
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
            cout << stockList[i].symbol << " " << stockList[i].name << " " << stockList[i].volume << " " << stockList[i].lastSale << endl;
        }
    }
    vector<Stock> returnStock(){
        return stockList;
    }
};

class StockServer {
public:
    unordered_map<OrderKey, vector<Order>, OrderKeyHash> pendingBuyOrders;
    unordered_map<OrderKey, vector<Order>, OrderKeyHash> pendingSellOrders;
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
        OrderKey key = {order.symbol, order.price, order.quantity, order.orderid};
        pendingBuyOrders[key].push_back(order);
    }

    void placeSellOrder(const Order& order) {
        OrderKey key = {order.symbol, order.price, order.quantity, order.orderid};
        pendingSellOrders[key].push_back(order);
    }

string displayPendingOrders() {
    stringstream output;

    output << "\nPending Buy Orders:" << endl;
    for (const auto& entry : pendingBuyOrders) {
        const OrderKey& key = entry.first;
        const vector<Order>& orders = entry.second;

        for (const Order& order : orders) {
            output <<"OrderId: " << order.orderid
            << ", Username: " << order.username << ", Symbol: " << key.symbol << ", Price: " << key.price << ", Quantity: " << key.quantity << endl;
        }
    }

    output << "Pending Sell Orders:" << endl;
    for (const auto& entry : pendingSellOrders) {
        const OrderKey& key = entry.first;
        const vector<Order>& orders = entry.second;

        for (const Order& order : orders) {
            output <<"OrderId: " << order.orderid
            <<", Username: " << order.username << ", Symbol: " << key.symbol << ", Price: " << key.price << ", Quantity: " << key.quantity << endl;
        }
    }

    return output.str();
}
//buy match sell order
int matchSellOrders(const Order& order) {
    int totalMatchedQuantity = 0;

    // Create a modified key with the same symbol and price
    OrderKey modifiedBuyOrderKey = { order.symbol, order.price };

    if (pendingSellOrders.find(modifiedBuyOrderKey) != pendingSellOrders.end()) {
        // Important code that goes into the pendingSellOrders to find the same price and symbol
        vector<Order>& sellOrders = pendingSellOrders[modifiedBuyOrderKey];
        for (int j = 0; j < sellOrders.size(); j++) {
            Order& sellOrder = sellOrders[j];

            if (order.price >= sellOrder.price && order.symbol == sellOrder.symbol) {
                // Match the orders based on symbol and price
                // Implement the logic to process matched orders
                // Access order and sellOrder to perform the trade
                // Update the quantities, execute the trade, etc.

                // Calculate the matched quantity
                int matchedQuantity = min(order.quantity, sellOrder.quantity);

                int  orderquantityleft = order.quantity - matchedQuantity;
                int sellquantityleft= sellOrder.quantity -= matchedQuantity;

                // Accumulate the matched quantity
                totalMatchedQuantity += matchedQuantity;

                // Update the stock price in maindata
                for (auto& stock : maindata) {
                    if (stock.symbol == order.symbol) {
                        // Assuming lastSale is a double attribute in the Stock class
                        stock.lastSale = order.price;
                        stock.volume += matchedQuantity;
                    }
                }

                // Remove matched buy or sell orders, this will only trigger one 
                    if (orderquantityleft > 0) {                        
                        int oldorderid = order.orderid;
                        string username = order.username;
                        string symbol = order.symbol;
                        double price = order.price;
                        Order neworder = { oldorderid, username, symbol, price, orderquantityleft };
                        placeBuyOrder(neworder);
                        cout << "in buy else:" << neworder.username << " " << neworder.symbol << " " << neworder.price << " " << neworder.quantity << endl;
                        break; 
                        } 
                    else if (sellquantityleft> 0) {
                        pendingSellOrders.erase(pendingSellOrders.find( { sellOrder.symbol, sellOrder.price, sellOrder.orderid}));
                        int oldorderid = sellOrder.orderid;
                        string username = sellOrder.username;
                        string symbol = sellOrder.symbol;
                        double price = sellOrder.price;
                        Order neworder = { oldorderid, username, symbol, price, sellquantityleft };
                        sellOrder.quantity = 0; // The sell order is fully matched, set quantity to 0
                        placeSellOrder(neworder);
                        cout << "in sell else:" << " " << neworder.username << " " << neworder.symbol << " " << neworder.price << " " << neworder.quantity << endl;
                        break;
                        } 
            }
        }

       /*If the order is still not fully matched, add it back to the pendingSellOrders
        if (order.quantity > 0) {
            placeSellOrder(order);
        }*/

        // Remove the modifiedBuyOrderKey from pendingSellOrders if it's empty
        if (sellOrders.empty()) {
            pendingSellOrders.erase(modifiedBuyOrderKey);
        }
    }
    else{
        placeSellOrder(order);
    }

    return totalMatchedQuantity;
}



bool checksymbol(string symbol){
    for (int i=0;i<maindata.size();i++){
        if (maindata[i].symbol==symbol){
            return true;
        }
    }
    return false;
}

private:    

    int port;
    SOCKET serverSocket;
    int nextOrderId = 0;

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
    } 
    if (strcmp(action, "1") == 0) {
    // Construct the stock data as a response
    string response;
    response += "\nSymbol :Company Name                               :Volume        :Last Price \n";
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
        response+=":" + to_string(maindata[i].volume);
        int spaceCount3 = 15 - to_string(maindata[i].volume).length();
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
                    cout << "Received from client - Username: " << username << ", Action: Sell" << ", Symbol: " << symbol << ", Quantity: " << quantity << ", Price: " << price << endl;
                    
                    //check symbol is valid or not
                    if (checksymbol(symbol)==false){
                        const char* response = "ALERT: Symbol is not valid!";
                        send(clientSocket, response, strlen(response), 0);
                        closesocket(clientSocket);
                        return;
                    }
                    //matching logic and pending logic here
                    int orderid= nextOrderId++;
                    Order order = {orderid,username, symbol,stod(price),stoi(quantity)};
                    //placeSellOrder(order);

                    // Handle the purchase logic and send a response
                    int  matchreturn= matchSellOrders(order);
                    if (matchreturn== stoi(quantity)) {
                        const char* response = "SELL: ALL Stock sold.";
                        send(clientSocket, response, strlen(response), 0);
                    } else if (matchreturn >0){
                        string response = "SELL: Stock partially sold.";
                        response+= "Quantity sold: " + to_string(matchreturn);
                        send(clientSocket, response.c_str(), response.length(), 0);
                    } 
                    else{
                        const char* response = "SELL: Stock listed in pending list.";
                        send(clientSocket, response, strlen(response), 0);
                    }
                }
            }
        }
    } 
    else if (strcmp(action, "3") == 0) {
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
                    cout << "Received from client - Username: " << username << ", Action: Buy" << ", Symbol: " << symbol << ", Quantity: " << quantity << ", Price: " << price << endl;
                    
                    //check symbol is valid or not
                    if (checksymbol(symbol)==false){
                        const char* response = "ALERT: Symbol is not valid!";
                        send(clientSocket, response, strlen(response), 0);
                        closesocket(clientSocket);
                        return;
                    }
                    
                    //matching logic and pending logic here
                    int orderid= nextOrderId++;
                    Order order = {orderid,username,symbol,stod(price),stoi(quantity)};
                    //placeBuyOrder(order);

                    // Handle the purchase logic and send a response
                    int  matchreturn= matchSellOrders(order);
                    if (matchreturn== stoi(quantity)) {
                        const char* response = "Buy: ALL Stock purchase";
                        send(clientSocket, response, strlen(response), 0);
                    } else if (matchreturn >0){
                        string response = "Buy: Stock partially purchase.";
                        response+= "Quantity purchase: " + to_string(matchreturn);
                        send(clientSocket, response.c_str(), response.length(), 0);
                    } 
                    else{
                        const char* response = "Buy: Stock listed in pending list.";
                        send(clientSocket, response, strlen(response), 0);
                    }
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
