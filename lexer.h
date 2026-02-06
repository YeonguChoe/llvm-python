#ifndef LEXER_H
#define LEXER_H
#include <string>

// Token code
enum Token {
    tok_eof = -1, // end of file
    tok_def = -2, // function
    tok_extern = -3, // external function
    tok_identifier = -4, // identifier
    tok_number = -5, // number literal

    // Conditional statement
    tok_if = -6,
    tok_then = -7,
    tok_else = -8,

    // For loop
    tok_for = -9,
    tok_in = -10,
};

extern std::string IdentifierStr;
extern double NumVal;

int gettok();

#endif
