#include <iostream>
#include "parser.h"

int main(int argc, char *argv[]) {
    fprintf(stderr, ">>> ");
    getNextToken();
    MainLoop();
    return 0;
}
