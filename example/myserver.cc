/* myserver.cc: sample server program */
#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using std::string;
using std::cout;
using std::cerr;
using std::cout;
using std::endl;
using std::string;


#include "InMemoryDatabase.h"
#include "DiskDatabase.h"
InMemoryDatabase db = InMemoryDatabase();
// DiskDatabase db = DiskDatabase("db");
/*
 * Read an integer from a client.
 */
int readCommand(const std::shared_ptr<Connection> &conn) {
    Protocol command = conn->read();

    Protocol next = conn->read();
    while(next != Protocol::COM_END) {
        next = conn->read();
    }
    
}

/*
 * Send a string to a client.
 */
void writeString(const std::shared_ptr<Connection> &conn, const string &s) {
    for (char c : s) {
        conn->write(c);
    }
    conn->write('$');
}

Server init(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: myserver port-number" << endl;
        exit(1);
    }

    int port = -1;
    try {
        port = std::stoi(argv[1]);
    } catch (std::exception &e) {
        cerr << "Wrong format for port number. " << e.what() << endl;
        exit(2);
    }

    Server server(port);
    if (!server.isReady()) {
        cerr << "Server initialization error." << endl;
        exit(3);
    }
    return server;
}

void process_request(std::shared_ptr<Connection> &conn) {
    int nbr = readNumber(conn);

    string result;
    if (nbr > 0) {
        std::string newsgroupName = "Tech News";
        db.createNewsgroup(newsgroupName);
        std::string title = "New article";
        std::string author = "Author";
        std::string text = "Text";
        db.createArticle(0, title, author, text);
        auto article = db.readArticle(0, 0);
        // print article
        cout << std::get<1>(article) << " " << std::get<2>(article) << " " << std::get<3>(article) << endl;
        result = std::get<1>(article) + " " + std::get<2>(article) + " " + std::get<3>(article);


    } else if (nbr == 0) {
        result = "zero";
    } else {
        result = "negative";
    }
    writeString(conn, result);
}

void serve_one(Server &server) {
    auto conn = server.waitForActivity();
    if (conn != nullptr) {
        try {
            process_request(conn);
        } catch (ConnectionClosedException &) {
            server.deregisterConnection(conn);
            cout << "Client closed connection" << endl;
        }
    } else {
        conn = std::make_shared<Connection>();
        server.registerConnection(conn);
        cout << "New client connects" << endl;
    }
}

int main(int argc, char *argv[]) {
        auto server = init(argc, argv);
        while (true) {
                serve_one(server);
        }
        return 0;
}
