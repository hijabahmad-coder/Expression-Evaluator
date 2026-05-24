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
vector<Token> toPostfix(const vector<Token>& tokens) {
    vector<Token> output;
    stack<Token> opStack;

    bool expectOperand = true;   
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& tok = tokens[i];

        if (tok.type == NUMBER || tok.type == VARIABLE) {
            if (!expectOperand) {
                cerr << "Syntax error: unexpected operand '" << tok.value << "'\n";
                exit(1);
            }
            output.push_back(tok);
            expectOperand = false;

        } else if (tok.type == OPEN_BRACKET) {
            if (!expectOperand) {
                cerr << "Syntax error: unexpected '(" << tok.value << "'\n";
                exit(1);
            }
            opStack.push(tok);


        } else if (tok.type == CLOSE_BRACKET) {
            if (expectOperand) {
                cerr << "Syntax error: empty or mismatched brackets\n";
                exit(1);
            }
            char needed = matchingOpen(tok.value[0]);
            bool found = false;
            while (!opStack.empty()) {
                Token top = opStack.top(); opStack.pop();
                if (top.type == OPEN_BRACKET) {
                    if (top.value[0] != needed) {
                        cerr << "Syntax error: mismatched bracket '"
                                  << top.value << "' closed by '" << tok.value << "'\n";
                        exit(1);
                    }
                    found = true;
                    break;
                }
                output.push_back(top);
            }
            if (!found) {
                cerr << "Syntax error: unmatched closing bracket '" << tok.value << "'\n";
                exit(1);
            }
            expectOperand = false;

        } else if (tok.type == OPERATOR) {
            if (expectOperand) {
                cerr << "Syntax error: unexpected operator '" << tok.value << "'\n";
                exit(1);
            }
            while (!opStack.empty() &&
                   opStack.top().type == OPERATOR &&
                   precedence(opStack.top().value) >= precedence(tok.value)) {
                output.push_back(opStack.top());
                opStack.pop();
            }
            opStack.push(tok);
            expectOperand = true;
        }
    }

    if (expectOperand && !tokens.empty()) {
        cerr << "Syntax error: expression ends with an operator\n";
        exit(1);
    }
    while (!opStack.empty()) {
        Token top = opStack.top(); opStack.pop();
        if (top.type == OPEN_BRACKET || top.type == CLOSE_BRACKET) {
            cerr << "Syntax error: unmatched opening bracket '" << top.value << "'\n";
            exit(1);
        }
        output.push_back(top);
    }

    return output;
}
vector<string> collectVariables(const vector<Token>& tokens) {
    vector<string> vars;
    set<string> seen;
    for (const auto& t : tokens) {
        if (t.type == VARIABLE && seen.find(t.value) == seen.end()) {
            seen.insert(t.value);
            vars.push_back(t.value);
        }
    }
    return vars;
}
int evaluate(const vector<Token>& postfix,
                const map<string, int>& varValues) {
    stack<int> stk;

    for (const auto& tok : postfix) {
        if (tok.type == NUMBER) {
            stk.push(stoi(tok.value));

        } else if (tok.type == VARIABLE) {
            auto it = varValues.find(tok.value);
            if (it == varValues.end()) {
                cerr << "Runtime error: no value for variable '" << tok.value << "'\n";
                exit(2);
            }
            stk.push(it->second);

        } else if (tok.type == OPERATOR) {
            if (stk.size() < 2) {
                cerr << "Runtime error: not enough operands for '" << tok.value << "'\n";
                exit(2);
            }
            int b = stk.top(); stk.pop();
            int a = stk.top(); stk.pop();

            if (tok.value == "+") stk.push(a + b);
            else if (tok.value == "-") stk.push(a - b);
            else if (tok.value == "*") stk.push(a * b);
            else if (tok.value == "/") {
                if (b == 0) {
                    cerr << "Logical error: division by zero\n";
                    exit(3);
                }
                stk.push(a / b);
            }
        }
    }

    if (stk.size() != 1) {
        cerr << "Runtime error: malformed expression\n";
        exit(2);
    }
    return stk.top();
}
int main() {
    string expr;
    getline(cin, expr);

    if (expr.empty()) {
        cerr << "Syntax error: empty expression\n";
        return 1;
    }

    vector<Token> tokens = tokenize(expr);

    if (tokens.empty()) {
        cerr << "Syntax error: expression contains no tokens\n";
        return 1;
    }
    vector<Token> postfix = toPostfix(tokens);

    vector<string> vars = collectVariables(tokens);
    map<string, int> varValues;

    for (const auto& var : vars) {
        cerr << "Enter value for " << var << ": ";
        int val;
        if (!(cin >> val)) {
            cerr << "Runtime error: invalid input for variable '" << var << "'\n";
            return 2;
        }
        varValues[var] = val;
    }
    for (size_t i = 0; i < postfix.size(); ++i) {
        if (i) cout << " ";
        cout << postfix[i].value;
    }
    cout << "\n";

    int result = evaluate(postfix, varValues);

    if (result == static_cast<long long>(result))
        cout << static_cast<long long>(result) << "\n";
    else
        cout << result << "\n";

    return 0;
}


