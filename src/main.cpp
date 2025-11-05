#include <iostream>
#include "Enum.h"
    

CUSTOM_ENUM(Test, A, B, C, D );
CUSTOM_ENUM(Rest, A, B, C, D );

int main(){
    Test test = Test::A;
    Rest rest = Rest::A;
    std::cout << reflect::to_string(test) << std::endl;
    std::cout << reflect::to_string(rest) << std::endl;
    return 0;
}