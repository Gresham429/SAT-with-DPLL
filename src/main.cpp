#include "../include/display.h"
#include <iostream>

int main()
{
    int op = 0;

    do
    {
        std::cin >> op;
        display(op);
    } while (op);
}