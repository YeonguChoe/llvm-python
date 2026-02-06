// Lexer: Analyze text in source code into tokens

#include "lexer.h"
#include <cctype>
#include <cstdio>

using namespace std;

string IdentifierStr;
double NumVal;

int gettok() {
    // pointer to currently checking character
    static int LastChar = ' ';

    // skip blank
    while (isspace(LastChar) != 0) {
        LastChar = getchar();
    }

    // process keyword (def, extern) and identifier
    if (isalpha(LastChar)) {
        IdentifierStr = LastChar;
        LastChar = getchar();

        // if it is alphabet or number, append to identifier string
        // recognizing a word
        while (isalnum(LastChar)) {
            IdentifierStr.push_back(LastChar);
            LastChar = getchar();
        }

        // check if it is a function
        if (IdentifierStr == "def") {
            return Token::tok_def;
        }

        // check if it is an external function
        if (IdentifierStr == "extern") {
            return Token::tok_extern;
        }

        if (IdentifierStr == "if") {
            return Token::tok_if;
        }

        if (IdentifierStr == "then") {
            return Token::tok_then;
        }

        if (IdentifierStr == "else") {
            return Token::tok_else;
        }

        if (IdentifierStr == "for") {
            return Token::tok_for;
        }

        if (IdentifierStr == "in") {
            return Token::tok_in;
        }

        return Token::tok_identifier;
    }

    // process number
    if (LastChar == '.' or isdigit(LastChar)) {
        string NumStr = "";

        bool dot_seen = false;

        if (LastChar == '.') {
            dot_seen = true;
            int next_char = getchar();

            // digit after '.' is not number
            if (isdigit(next_char) == 0) {
                NumVal = 0.0;
                LastChar = next_char;
                return Token::tok_number;
            }
            // digit after '.' is a number
            else {
                NumStr.push_back('.');
                LastChar = next_char;
            }
        }

        while (isdigit(LastChar) or (dot_seen == false and LastChar == '.')) {
            if (LastChar == '.') {
                dot_seen = true;
            }
            NumStr.push_back(LastChar);
            LastChar = getchar();
        }
        NumVal = strtod(NumStr.c_str(), nullptr);
        return Token::tok_number;
    }

    // process comment
    if (LastChar == '#') {
        // find '\n' or '\r' or EOF
        while (LastChar != '\n' and LastChar != '\r' and LastChar != EOF) {
            // getchar reads from stdin
            LastChar = getchar();
        }
        if (LastChar == EOF) {
            return Token::tok_eof;
        } else {
            return gettok();
        }
    }

    if (LastChar == EOF) {
        return Token::tok_eof;
    } else {
        // copy value
        int found_token = LastChar;
        // update LastChar and keep the value that way
        LastChar = getchar();
        return found_token;
    }
}
