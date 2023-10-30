#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;


class Stock {
public:
    string symbol;
    string name;
    int quantity;
    double price;

    Stock(const string& s, const string& n, int q, double p)
        : symbol(s), name(n), quantity(q), price(p) {}

    void customCsvParser(const string& line, vector<string>& tokens) {
    istringstream ss(line);
    string token;

    while (getline(ss, token, ',')) {
        tokens.push_back(token);
    }
};


class ReadStock {
public:
    vector <Stock> stockList;
    void loadcsv() {
        ifstream inputFile("stock_infor.csv"); 
        if (!inputFile) {
            cerr << "Failed to open the CSV file." << endl;
            return;
        }
        else{
            cout << "Opened the CSV file." << endl;
        }
        string line;
        while (getline(inputFile, line)) {
        vector<string> tokens;
        customCsvParser(line, tokens);

        if (tokens.size() == 4) {
            Stock stock;
            stock.symbol = tokens[0];
            stock.name = tokens[1];
            stock.quantity = stoi(tokens[2]);
            stock.price = stod(tokens[3]);
            stockList.push_back(stock);
        }
        inputFile.close();
    }
    void printStock(){
        for (int i = 0; i < stockList.size(); i++) {
            cout << stockList[i].symbol << " " << stockList[i].name << " " << stockList[i].quantity << " " << stockList[i].price << endl;
        }
    }
};

int main() {
    ReadStock stockData;
    stockData.loadcsv();
    stockData.printStock();
    return 0;
}
