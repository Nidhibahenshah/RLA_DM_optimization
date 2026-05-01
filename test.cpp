#include <iostream>
#include <climits>

int main() {
    std::cout << "char is " << (CHAR_MIN < 0 ? "signed" : "unsigned") << "\n";
    return 0;
}
