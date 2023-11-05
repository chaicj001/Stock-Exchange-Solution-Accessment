#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

int main() {
    ReadStock stockData;
    stockData.loadcsv();
    stockData.printStock();
    vector <Stock> maindata = stockData.returnStock();
    return 0;
}
