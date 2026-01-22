#include <iostream>

#include "lexer.h"
#include "parser.cpp"

using namespace std;

int main(int argc, char *argv[]) {
    fprintf(stderr, ">>> ");
    getNextToken();
    MainLoop();
    return 0;
}
