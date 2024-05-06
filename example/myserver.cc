/* myserver.cc: sample server program */
#include "connection.h"
#include "connectionclosedexception.h"
#include "server.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>

using std::string;
using std::cout;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::uint8_t;

#include "InMemoryDatabase.h"
#include "DiskDatabase.h"
#include "protocol.h"
#include <command.h>

InMemoryDatabase db = InMemoryDatabase();
// DiskDatabase db = DiskDatabase("db");

void writeCommand(const std::shared_ptr<Connection> &conn, Protocol command);

int readNumber(const std::shared_ptr<Connection> &conn) {
    unsigned char byte1 = conn->read();
    unsigned char byte2 = conn->read();
    unsigned char byte3 = conn->read();
    unsigned char byte4 = conn->read();
    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

void writeParNumber(const std::shared_ptr<Connection> &conn, int value) {
    writeCommand(conn, Protocol::PAR_NUM);
    cout << "Writing number " << value << endl;
    conn->write((value >> 24) & 0xFF);
    conn->write((value >> 16) & 0xFF);
    conn->write((value >> 8) & 0xFF);
    conn->write(value & 0xFF);
}

void writeNumber(const std::shared_ptr<Connection> &conn, int value) {
    cout << "Writing number " << value << endl;
    conn->write((value >> 24) & 0xFF);
    conn->write((value >> 16) & 0xFF);
    conn->write((value >> 8) & 0xFF);
    conn->write(value & 0xFF);
}

/*
 * Read an integer from a client.
 */
Command readCommand(const std::shared_ptr<Connection> &conn){
    Protocol commandType = static_cast<Protocol>(conn->read());
    cout << "Command type: " << static_cast<int>(commandType) << "\n";
    
    std::vector<Param> params;
    try {
        unsigned char byte;
        while((byte = conn->read()) != static_cast<unsigned char>(Protocol::COM_END)) {
            Protocol paramType = static_cast<Protocol>(byte);
            if (paramType == Protocol::PAR_STRING) {
                int length = readNumber(conn);
                string paramValue;
                for (unsigned char i = 0; i < length; i++) {
                    paramValue += static_cast<char>(conn->read());
                }
                params.push_back(Param(paramType, paramValue));
            } else if (paramType == Protocol::PAR_NUM) {
                int paramValue = readNumber(conn);
                params.push_back(Param(paramType, paramValue));
            } else {
                cout << "Parameter type: " << static_cast<int>(paramType) << "\n";
                throw std::runtime_error("Unknown parameter type");
            }
        }
        
    } catch (const ConnectionClosedException&) {
        throw; // Rethrow to handle disconnection at a higher level
    }
    return Command(commandType, params);
}

/*
 * Send a string to a client.
 */
void writeParString(const std::shared_ptr<Connection> &conn, const string &s) {
    writeCommand(conn, Protocol::PAR_STRING);
    writeNumber(conn, s.size());
    cout << "Writing string " << s << endl;
    for (char c : s) {
        conn->write(c);
    }
}

void writeCommand(const std::shared_ptr<Connection> &conn, Protocol command) {
    cout << "Sending command: " << static_cast<int>(command) << "\n";
    conn->write(static_cast<unsigned char>(command));
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
    Command command = readCommand(conn);
    bool result;
    bool result1;
    bool result2;
    bool result3;
    std::vector<std::pair<int, string>> result4;
    std::vector<std::pair<int, string>> result5;
    std::tuple<bool, string, string, string> result6;
    std::optional<std::vector<std::pair<int, string>>> articlesOpt;

    switch (command.commandType) {
        case Protocol::COM_LIST_NG:
            result4 = db.listNewsgroups();
            writeCommand(conn, Protocol::ANS_LIST_NG);
            writeParNumber(conn, result4.size());
            for (auto &ng : result4) {
                writeParNumber(conn, ng.first);
                writeParString(conn, ng.second);
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_CREATE_NG:
            result = db.createNewsgroup(command.parameters[0].getString());
            writeCommand(conn, Protocol::ANS_CREATE_NG);
            if (result == 1) {
                writeCommand(conn, Protocol::ANS_ACK);
            } else {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_NG_ALREADY_EXISTS); 
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_DELETE_NG:
            result1 = db.deleteNewsgroup(command.parameters[0].getInt());
            writeCommand(conn, Protocol::ANS_DELETE_NG);
            if (result1 == 1) {
                writeCommand(conn, Protocol::ANS_ACK);
            } else {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_NG_DOES_NOT_EXIST); 
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_LIST_ART:
            articlesOpt = db.listArticles(command.parameters[0].getInt());
            writeCommand(conn, Protocol::ANS_LIST_ART);
            if (!articlesOpt.has_value()) {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_NG_DOES_NOT_EXIST); 
            } else {
                writeCommand(conn, Protocol::ANS_ACK);
                const auto &articles = articlesOpt.value();
                writeParNumber(conn, static_cast<int>(articles.size()));
                for (auto &art : articles) {
                    writeParNumber(conn, art.first);
                    writeParString(conn, art.second);
                }
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_CREATE_ART:
            result2 = db.createArticle(command.parameters[0].getInt(), command.parameters[1].getString(), command.parameters[2].getString(), command.parameters[3].getString());
            writeCommand(conn, Protocol::ANS_CREATE_ART);
            if (result2 == 1) {
                writeCommand(conn, Protocol::ANS_ACK);
            } else {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_NG_DOES_NOT_EXIST); 
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_DELETE_ART:
            result3 = db.deleteArticle(command.parameters[0].getInt(), command.parameters[1].getInt());
            writeCommand(conn, Protocol::ANS_DELETE_ART);
            if (result3 == 1) {
                writeCommand(conn, Protocol::ANS_ACK);
            } else {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_ART_DOES_NOT_EXIST); 
            }
            writeCommand(conn, Protocol::ANS_END);
            break;
        case Protocol::COM_GET_ART:
            result6 = db.getArticle(command.parameters[0].getInt(), command.parameters[1].getInt());
            writeCommand(conn, Protocol::ANS_GET_ART);

            if (std::get<0>(result6)) {  
                writeCommand(conn, Protocol::ANS_ACK);
                writeParString(conn, std::get<1>(result6)); 
                writeParString(conn, std::get<2>(result6));  
                writeParString(conn, std::get<3>(result6));
            } else {
                writeCommand(conn, Protocol::ANS_NAK);
                writeCommand(conn, Protocol::ERR_NG_DOES_NOT_EXIST);
            }
            writeCommand(conn, Protocol::ANS_END);
        default:
            break;
    }
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
        cout << "Waiting for activity" << endl;
        while (true) {
                serve_one(server);
        }
        return 0;
}
