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

    // COM_LIST_NG = 1,    // list newsgroups
    // COM_CREATE_NG = 2,  // create newsgroup
    // COM_DELETE_NG = 3,  // delete newsgroup
    // COM_LIST_ART = 4,   // list articles
    // COM_CREATE_ART = 5, // create article
    // COM_DELETE_ART = 6, // delete article
    // COM_GET_ART = 7,    // get article

int app(const Connection &conn) {
    cout << "Hello and welcome to the application!\n
    To interact with the app please chose the action you want to take by typing a single number followed by return\n
    1 List newsgroups
    2 Create newsgroup
    3 Delete newsgroup
    4 List articles
    5 Create article
    6 Delete article
    7 Get article
    " << endl;
    int nbr;
    while (cin >> nbr && nbr >= 1 && nbr <= 7) {
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
