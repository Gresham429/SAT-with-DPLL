#include "../include/display.h"
#include <iostream>

int main()
{
    int op = 0;

    do
    {
        printMenu();
        std::cin >> op;
        display(op);

        std::cout << std::endl;
    } while (op);

    return 0;
}