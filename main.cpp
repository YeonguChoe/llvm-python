#include <iostream>

#include "lexer.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace std;

static cl::opt<string> InputFilename(cl::Positional, cl::desc("<input file>"), cl::Required);

int main(int argc, char *argv[]) {
    cl::ParseCommandLineOptions(argc, argv);

    // fill stdin
    freopen(InputFilename.c_str(), "r", stdin);

    int current_token;

    while (current_token = gettok(), current_token != tok_eof) {
    }

    return 0;
}
