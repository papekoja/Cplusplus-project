/* myclient.cc: sample client program */
#include "connection.h"
#include "connectionclosedexception.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <protocol.h>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
/*
 * Send an integer to the server as four bytes.
 */
void writeNumber(const Connection &conn, int value) {
    conn.write((value >> 24) & 0xFF);
    conn.write((value >> 16) & 0xFF);
    conn.write((value >> 8) & 0xFF);
    conn.write(value & 0xFF);
}

/*
 * Send a command to the server.
 */
void writeCommand(const Connection &conn, Protocol command) {
    conn.write(static_cast<unsigned char>(command));
}

/*
 * Send a string parameter to the server.
 */
void writeStringParam(const Connection &conn, const string &s) {
    conn.write(static_cast<unsigned char>(Protocol::PAR_STRING));
    writeNumber(conn, s.size());
    for (char c : s) {
        conn.write(c);
    }
}

/*
 * Send a number parameter to the server.
 */
void writeNumberParam(const Connection &conn, int value) {
    conn.write(static_cast<unsigned char>(Protocol::PAR_NUM));
    writeNumber(conn, value);
}

/*
 * Read a string from the server.
 */
string readString(const Connection &conn) {
    string s;
    char ch;
    while ((ch = conn.read()) != '$') {
        s += ch;
    }
    return s;
}

/* Creates a client for the given args, if possible.
 * Otherwise exits with error code.
 */
Connection init(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: myclient host-name port-number" << endl;
        exit(1);
    }

    int port = -1;
    try {
        port = std::stoi(argv[2]);
    } catch (std::exception &e) {
        cerr << "Wrong port number. " << e.what() << endl;
        exit(2);
    }

    Connection conn(argv[1], port);
    if (!conn.isConnected()) {
        cerr << "Connection attempt failed" << endl;
        exit(3);
    }

    return conn;
}

int app(const Connection &conn) {
    cout << "Type a number: ";
    int nbr;
    while (cin >> nbr) {
        try {
            cout << nbr << " is ...";
            writeNumber(conn, nbr);
            string reply = readString(conn);
            cout << " " << reply << endl;
            cout << "Type another number: ";
        } catch (ConnectionClosedException &) {
            cout << " no reply from server. Exiting." << endl;
            return 1;
        }
    }
    cout << "\nexiting.\n";
    return 0;
}

int main(int argc, char *argv[]) {
    Connection conn = init(argc, argv);
    return app(conn);
}
