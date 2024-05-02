#include "protocol.h"
#include <string>
#include <vector>
#include <variant>

using std::string;
using std::vector;
using std::variant;

class Param {
public:
    variant<int, string> value;
    Protocol paramType;
    Param(Protocol paramType, const string& value) : paramType(paramType), value(value) {}
    Param(Protocol paramType, int value) : paramType(paramType), value(value) {}

    string getString() {
        return std::get<string>(value);
    }

    int getInt() {
        return std::get<int>(value);
    }
};

class Command {
public:
    Command(Protocol commandType, vector<Param> params)
        : commandType(commandType), parameters(params) {}
    Protocol commandType;
    vector<Param> parameters;
};

