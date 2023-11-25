# Stock Exchange Stimulator
**Repository includes a C++ implementation of a high-speed trading matching engine meeting specified requirements.**

**Clear documentation, efficient code, and thorough testing ensure reliability.**

**Utilizes TCP/IP for order communication.**

## Objective:

Creating a stock exchange program with matching engine for high-speed trading and large frequent trade.

**Features to Implement:**

1. Reading stock from the csv file with latest data. 
2. Accept buy and sell orders from the user.
3. Orders include: ID, Type (Buy/Sell), Price, Quantity.
4. Match orders efficiently based on price and time (first come first serve, order that been placed first will been solve during the matching).
5. Support partial fills (order that not been filled will been put into the pending list for waiting new buyer/seller).

## Communication Logic:

Use TCP/IP (windows inbuild socket function)for user order input and trade output.
## Efficiency:

Optimize for speed to handle a large volume of orders. (can be accept trade without any hustle)
## Source Code Include:

1. Source code for the matching engine.
2. User manual with compile/run instructions for installation and startup (TCP/IP specifications and configuration).
3. Test plan with varied scenarios and sample data.
4. Data generator program for high-speed order data via TCP/IP.
5. Correctness in order matching and partial fills.
6. Efficiency in handling a high volume of orders.
7. Proper TCP/IP connection for orders and trades.
8. Well-structured, documented, and readable code.
9. Comprehensive testing plan with diverse scenarios.







### Dev by ChekJu



