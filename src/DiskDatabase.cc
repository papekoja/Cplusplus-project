#include "DiskDatabase.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

DiskDatabase::DiskDatabase(const std::string& rootPath) : dbRoot(rootPath) {
    if (!std::filesystem::exists(dbRoot)) {
        std::filesystem::create_directories(dbRoot);
    }
}

DiskDatabase::~DiskDatabase() {
    out << "DiskDatabase::~DiskDatabase()" << std::endl;
    out << "we shuttin down" << std::endl;
}

bool DiskDatabase::createNewsgroup(const std::string& name) {
    int newsgroupId = std::hash<std::string>{}(name);
    auto newsgroupPath = dbRoot / std::to_string(newsgroupId);
    if (!std::filesystem::exists(newsgroupPath)) {
        std::filesystem::create_directory(newsgroupPath);
        std::ofstream out(newsgroupPath / "meta.txt");
        out << "Name: " << name << std::endl;
        return true;
    }
    return false;
}

bool DiskDatabase::deleteNewsgroup(int id) {
    auto newsgroupPath = dbRoot / std::to_string(id);
    out << "Deleting newsgroup with id " << id << "path: " << newsgroupPath << std::endl;
    if (std::filesystem::exists(newsgroupPath)) {
        std::filesystem::remove_all(newsgroupPath);
        return true;
    }
    return false;
}

std::vector<std::pair<int, std::string>> DiskDatabase::listNewsgroups() const {
    std::vector<std::pair<int, std::string>> groups;
    for (const auto& entry : std::filesystem::directory_iterator(dbRoot)) {
        if (entry.is_directory()) {
            std::ifstream in(entry.path() / "meta.txt");
            std::string name;
            std::getline(in, name);
            out << "listNewsgroups: " << name << std::endl;
            groups.emplace_back(std::stoi(entry.path().filename().string()), name.substr(6));  // Extract name after "Name: "
        }
    }
    return groups;
}

bool DiskDatabase::createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) {
    auto path = dbRoot / std::to_string(newsgroupId);
    if (std::filesystem::exists(path)) {
        int articleId = std::hash<std::string>{}(title + author + text);
        std::ofstream out(path / (std::to_string(articleId) + ".txt"));
        out << "Title: " << title << "\nAuthor: " << author << "\nText: " << text << std::endl;
        return true;
    }
    return false;
}

bool DiskDatabase::deleteArticle(int newsgroupId, int articleId) {
    auto articlePath = dbRoot / std::to_string(newsgroupId) / (std::to_string(articleId) + ".txt");
    if (std::filesystem::exists(articlePath)) {
        std::filesystem::remove(articlePath);
        return true;
    }
    return false;
}

std::tuple<bool, std::string, std::string, std::string> DiskDatabase::readArticle(int newsgroupId, int articleId) const {
    auto articlePath = dbRoot / std::to_string(newsgroupId) / (std::to_string(articleId) + ".txt");
    if (std::filesystem::exists(articlePath)) {
        std::ifstream in(articlePath);
        std::string title, author, text, line;
        std::getline(in, title);
        std::getline(in, author);
        while (std::getline(in, line)) text += line + "\n";
        return {true, title.substr(7), author.substr(8), text}; // Strip the prefixes "Title: " and "Author: "
    }
    return {false, "", "", ""};
}

std::vector<std::pair<int, std::string>> DiskDatabase::listArticles(int newsgroupId) const {
    std::vector<std::pair<int, std::string>> articles;
    auto path = dbRoot / std::to_string(newsgroupId);
    if (std::filesystem::exists(path)) {
        for (const auto& file : std::filesystem::directory_iterator(path)) {
            if (!file.is_directory()) {
                int id = std::stoi(file.path().stem());
                std::ifstream in(file.path());
                std::string title;
                std::getline(in, title);
                articles.emplace_back(id, title.substr(7)); // Strip "Title: "
            }
        }
    }
    return articles;
}
