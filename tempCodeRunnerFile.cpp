        for (csv::CSVRow& row : reader) {
            string symbol = row[0].get<>();
            string name = row[1].get<>();
            int quantity = row[2].get<int>();
            double price = row[3].get<double>();

            stockList.push_back(Stock{symbol, name, quantity, price});
        }

        for (const Stock& stock : stockList) {
            cout << "Symbol: " << stock.symbol << ", Name: " << stock.name
                    << ", Quantity: " << stock.quantity << ", Price: $" << stock.price << endl;
        }
            inputFile.close();
        }
        void printStock(){
            for (int i = 0; i < stockList.size(); i++) {
                cout << stockList[i].symbol << " " << stockList[i].name << " " << stockList[i].quantity << " " << stockList[i].price << endl;
            }
        }