#include <iostream>
using namespace std;


int number=0000;
int createnumber(){
    number++;
    return number;
}

int main() {
    cout << createnumber() << endl;
    cout << createnumber() << endl;
    return 0;
}
