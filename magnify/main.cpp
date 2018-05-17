#include "magnify.h"

#include <iostream>


// задача по свертке строк, т.е. из строки 12w12abce1212abcr -> 2(12abc)2(12)wer
int main() {

    std::string str;
    std::cin >> str;
    std::cout << magnify(str) << std::endl;
    
    return 0;
}
