/* myclient.cc: sample client program */
#include "connection.h"
#include "connectionclosedexception.h"

#include <cstdlib>
#include <iostream>
#include <protocol.h>
#include <stdexcept>
#include <string>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

void handleListNewsgroups(const Connection &conn);
void handleCreateNewsgroup(const Connection &conn);
void handleDeleteNewsgroup(const Connection &conn);
void handleListArticles(const Connection &conn);
void handleCreateArticle(const Connection &conn);
void handleDeleteArticle(const Connection &conn);
void handleGetArticle(const Connection &conn);
void handleEnd();
void expect(const Connection &conn, Protocol expected);

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
 * Read a number from the server.
 */
int readNumber(const Connection &conn) {
    unsigned char byte1 = conn.read();
    unsigned char byte2 = conn.read();
    unsigned char byte3 = conn.read();
    unsigned char byte4 = conn.read();
    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
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

Protocol readProtocol(const Connection &conn) {
    unsigned char byte = conn.read();
    return static_cast<Protocol>(byte);
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
    cout << "\n--------------------------------------------------------------------------------\n"
            "Hello and welcome to the application!\n"
            "To interact with the app please chose the action you want to take by typing a single number followed by return\n\n"
            "1 List newsgroups\n"
            "2 Create newsgroup\n"
            "3 Delete newsgroup\n"
            "4 List articles\n"
            "5 Create article\n"
            "6 Delete article\n"
            "7 Get article\n"
            "8 End\n";
    int nbr;

    while (true) {
        cout << "\nPlease type a number and return: ";
        cin >> nbr;
        if (nbr > 8 || nbr < 1) {
            cout << "Not a valid command: " << nbr << endl;
            exit(1);
        }

        Protocol command = static_cast<Protocol>(nbr);
        writeCommand(conn, command);
        switch (command) {
        case Protocol::COM_LIST_NG:
            handleListNewsgroups(conn);
            break;
        case Protocol::COM_CREATE_NG:
            handleCreateNewsgroup(conn);
            break;
        case Protocol::COM_DELETE_NG:
            handleDeleteNewsgroup(conn);
            break;
        case Protocol::COM_LIST_ART:
            handleListArticles(conn);
            break;
        case Protocol::COM_CREATE_ART:
            handleCreateArticle(conn);
            break;
        case Protocol::COM_DELETE_ART:
            handleDeleteArticle(conn);
            break;
        case Protocol::COM_GET_ART:
            handleGetArticle(conn);
            break;
        case Protocol::COM_END:
            handleEnd();
            break;
        default:
            cout << "Unknown command\n";
            break;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    Connection conn = init(argc, argv);
    return app(conn);
}

void handleListNewsgroups(const Connection &conn) {
    writeCommand(conn, Protocol::COM_END);
    expect(conn, Protocol::ANS_LIST_NG);
    try {
        int numberOfNewsgroups = readNumber(conn);
        for (int i = 0; i < numberOfNewsgroups; i++) {
            int id = readNumber(conn);
            string name = readString(conn);
            cout << "Newsgroup " << id << ": " << name << endl;
        }
    } catch (ConnectionClosedException &) {
        cout << "No reply from server. Exiting." << endl;
        exit(1);
    }
    expect(conn, Protocol::ANS_END);
}

void handleCreateNewsgroup(const Connection &conn) {
    string name;
    cout << "Enter name of newsgroup: ";
    cin >> name;
    writeStringParam(conn, name);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_CREATE_NG);
    try {
        Protocol body = readProtocol(conn);
        switch (body) {
        case Protocol::ANS_NAK:
            expect(conn, Protocol::ERR_NG_ALREADY_EXISTS);
            cerr << "Error: Newsgroup already exists" << endl;
            break;
        case Protocol::ANS_ACK:
            cout << "Newsgroup created" << endl;
            break;
        default:
            cerr << "Error: Unexpected answer " << static_cast<int>(body) << endl;
            exit(1);
        }
        expect(conn, Protocol::ANS_END);
    } catch (const ConnectionClosedException &) {
        cerr << "No reply from server. Exiting." << endl;
        exit(1);
    }
}

void handleDeleteNewsgroup(const Connection &conn) {
    int id;
    cout << "Enter id of newsgroup: ";
    cin >> id;
    writeNumberParam(conn, id);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_DELETE_NG);
    Protocol body = readProtocol(conn);
    switch (body) {
    case Protocol::ANS_NAK:
        expect(conn, Protocol::ERR_NG_DOES_NOT_EXIST);
        cerr << "Error: Newsgroup does not exist" << endl;
        break;
    case Protocol::ANS_ACK:
        cout << "Newsgroup deleted" << endl;
        break;
    default:
        cerr << "Error: Unexpected answer " << static_cast<int>(body) << endl;
        exit(1);
    }
    expect(conn, Protocol::ANS_END);
}

void handleListArticles(const Connection &conn) {
    cout << "Enter id of newsgroup: ";
    int groupId;
    cin >> groupId;
    writeNumberParam(conn, groupId);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_LIST_ART);
    int numberOfArticles;
    int id;
    try {
        Protocol body = readProtocol(conn);
        switch (body) {
        case Protocol::ANS_NAK:
            expect(conn, Protocol::ERR_NG_DOES_NOT_EXIST);
            cout << "Error: Newsgroup does not exist" << endl;
            break;
        case Protocol::ANS_ACK:
            numberOfArticles = readNumber(conn);
            for (int i = 0; i < numberOfArticles; i++) {
                id = readNumber(conn);
                string title = readString(conn);
                cout << "Article " << id << ": " << title << endl;
            }
            break;
        default:
            cerr << "Error: Unexpected answer " << static_cast<int>(body) << endl;
            exit(1);
        }
    } catch (ConnectionClosedException &) {
        cout << "No reply from server. Exiting." << endl;
        return;
    }
    expect(conn, Protocol::ANS_END);
}

void handleCreateArticle(const Connection &conn) {
    string title;
    string author;
    string text;
    int groupId;
    cout << "Enter id of newsgroup: ";
    cin >> groupId;
    writeNumberParam(conn, groupId);
    cout << "Enter title of article: ";
    cin >> title;
    writeStringParam(conn, title);
    cout << "Enter author of article: ";
    cin >> author;
    writeStringParam(conn, author);
    cout << "Enter text of article: ";
    cin >> text;
    writeStringParam(conn, text);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_CREATE_ART);
    Protocol body = readProtocol(conn);
    switch (body) {
    case Protocol::ANS_NAK:
        cout << "Error: Newsgroup does not exist" << endl;
        break;
    case Protocol::ANS_ACK:
        cout << "Article created" << endl;
        break;
    default:
        cout << "Error: Unexpected answer " << static_cast<int>(body) << endl;
        break;
    }
    expect(conn, Protocol::ANS_END);
}

void handleDeleteArticle(const Connection &conn) {
    int id;
    int groupId;
    cout << "Enter id of newsgroup: ";
    cin >> groupId;
    writeNumberParam(conn, groupId);
    cout << "Enter id of article: ";
    cin >> id;
    writeNumberParam(conn, id);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_DELETE_ART);
    Protocol body = readProtocol(conn);
    switch (body) {
    case Protocol::ANS_NAK:
        cout << "Error: Newsgroup does not exist" << endl;
        break;
    case Protocol::ANS_ACK:
        cout << "Article deleted" << endl;
        break;
    default:
        cout << "Error: Unexpected answer " << static_cast<int>(body) << endl;
        break;
    }
    expect(conn, Protocol::ANS_END);
}

void handleGetArticle(const Connection &conn) {
    int id;
    int groupId;
    cout << "Enter id of newsgroup: ";
    cin >> groupId;
    writeNumberParam(conn, groupId);
    cout << "Enter id of article: ";
    cin >> id;
    writeNumberParam(conn, id);
    writeCommand(conn, Protocol::COM_END);

    expect(conn, Protocol::ANS_GET_ART);
    Protocol body = readProtocol(conn);
    switch (body) {
    case Protocol::ANS_NAK:
        cout << "Error: Newsgroup does not exist" << endl;
        break;
    case Protocol::ANS_ACK:
        cout << "Article retrieved" << endl;
        break;
    default:
        cout << "Error: Unexpected answer " << static_cast<int>(body) << endl;
        break;
    }
    Protocol end = readProtocol(conn);
    if (end != Protocol::ANS_END) {
        cout << "Error: Expected answer ANS_END, got " << static_cast<int>(end) << endl;
        return;
    }
}

void handleEnd() {
    // exit the application
    cout << "Exiting application" << endl;
    exit(0);
}

void expect(const Connection &conn, Protocol expected) {
    Protocol actual = readProtocol(conn);
    if (actual != expected) {
        cerr << "Error: Expected answer " << static_cast<int>(expected) << ", got " << static_cast<int>(actual) << endl;
        exit(1);
    }
}
