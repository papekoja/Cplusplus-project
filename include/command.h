#include "protocol.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

class Command {
public:
    Command(Protocol command, vector<Param> params);
};

class Param {
public:
    Param(Protocol paramtype);
};