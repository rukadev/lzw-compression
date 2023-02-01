#include <iostream>
#include "LZW.h"

int main() {
    std::string test1 = "First second third test second third second";
    std::string test2 = "FirstFirstFirstFirstdkjsfdjfkjdskfjdsklFirstksjfkdlsjakFirstkasjdkfjsdljFirstjlkasjdkfjslaFirstaskdfjlskjaFirstaksdjfksjdaFirstsaklj";

    std::string comp = compress(test2);
    std::string decomp = decompress(comp);
    std::cout << comp << std::endl;
    std::cout << decomp << std::endl;
    return 0;
}