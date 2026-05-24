enum TokenType { NUMBER, VARIABLE, OPERATOR, OPEN_BRACKET, CLOSE_BRACKET };

struct Token {
    TokenType type;
    string value;
};

vector<Token> tokenize(const string& expr) {
    vector<Token> tokens;
    size_t i = 0;

    while (i < expr.size()) {
        char c = expr[i];
        if (isspace(c)) { ++i; continue; }

        if (isdigit(c)) {
            string num;
            while (i < expr.size() && isdigit(expr[i]))
                num += expr[i++];
            tokens.push_back({NUMBER, num});
            continue;
        }
        if (isalpha(c) || c == '_') {
            string ident;
            while (i < expr.size() && (isalnum(expr[i]) || expr[i] == '_'))
                ident += expr[i++];
            tokens.push_back({VARIABLE, ident});
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/') {
            tokens.push_back({OPERATOR, string(1, c)});
            ++i; continue;
        }

        if (c == '(' || c == '[' || c == '{') {
            tokens.push_back({OPEN_BRACKET, string(1, c)});
            ++i; continue;
        }

        if (c == ')' || c == ']' || c == '}') {
            tokens.push_back({CLOSE_BRACKET, string(1, c)});
            ++i; continue;
        }

        cerr << "Syntax error: unexpected character '" << c << "'\n";
        exit(1);
    }
    return tokens;
}
