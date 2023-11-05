#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

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

    void customCsvParser(const string& line, vector<string>& tokens) {
    istringstream ss(line);
    string token;
    bool inQuotedField = false;
    string currentToken;

    while (getline(ss, token, ',')) {
        if (token.front() == '"') {
            inQuotedField = true;
            currentToken = token;
        } else if (token.back() == '"') {
            inQuotedField = false;
            currentToken += "," + token;
            currentToken.pop_back(); // Remove the trailing quote
            if (currentToken.front() == '"') {
                currentToken = currentToken.substr(1); // Remove the leading quote
            }
            tokens.push_back(currentToken);
        } else {
            if (inQuotedField) {
                currentToken += "," + token;
            } else {
                if (currentToken.empty()) {
                    tokens.push_back(token);
                } else {
                    currentToken += "," + token;
                    if (currentToken.back() == '$') {
                        currentToken.pop_back(); // Remove the dollar sign
                        tokens.push_back(currentToken);
                        currentToken.clear(); // Reset for the next field
                    }
                }
            }
        }
    }
}

    long long removeCommasAndQuotes(const string& value) {
        string result;
        for (int i = 0; i < value.size(); i++) {
            if (value[i] != ',' && value[i] != '"') {
                result += value[i];
            }
        }
        return stoll(result);
    }

    double removeDollarSign(const string& value) {
        string result;
        for (int i = 0; i < value.size(); i++) {
            if (value[i] != '$') {
                result += value[i];
            }
        }
        return stod(result);
    }

    void loadcsv() {
        ifstream inputFile("stock_info.csv"); 
        if (!inputFile) {
            cerr << "Failed to open the CSV file." << endl;
            return;
        } else {
            cout << "Opened the CSV file." << endl;
        }
        string line;
        getline(inputFile, line); // Skip the first line
        while (getline(inputFile, line)) {
            vector<string> tokens;
            customCsvParser(line, tokens);
            cout << tokens[0] << " " << tokens[1] << " " << tokens[2] << " " << tokens[3] << endl;
        }
        inputFile.close();
    }

    void printStock() {
        for (int i = 0; i < stockList.size(); i++) {
            cout << stockList[i].symbol << " " << stockList[i].name << " " << stockList[i].marketCap << " " << stockList[i].lastSale << endl;
        }
    }
};

int main() {
    ReadStock stockData;
    stockData.loadcsv();
    //stockData.printStock();
    return 0;
}
