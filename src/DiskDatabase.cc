#include "DiskDatabase.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <tuple>
#include <optional>
#include <vector>
#include <string>
#include <chrono>

DiskDatabase::DiskDatabase(const std::string& rootPath) : dbRoot(rootPath) {
    if (!std::filesystem::exists(dbRoot)) {
        std::filesystem::create_directories(dbRoot);
    }
}

DiskDatabase::~DiskDatabase() {
}

bool DiskDatabase::createNewsgroup(const std::string& name) {
    if (name.empty()) {
        std::cerr << "Failed to create newsgroup: Name cannot be empty.\n";
        return false;
    }
    int newsgroupId = std::hash<std::string>{}(name);
    auto newsgroupPath = dbRoot / std::to_string(newsgroupId);
    if (!std::filesystem::exists(newsgroupPath)) {
        std::filesystem::create_directory(newsgroupPath);
        std::ofstream out(newsgroupPath / "meta.txt");
        out << "Name: " << name << std::endl;
        std::cout << "Newsgroup created: " << name << " with ID " << newsgroupId << "\n";
        return true;
    }
    std::cerr << "Failed to create newsgroup: ID already exists.\n";
    return false;
}

bool DiskDatabase::deleteNewsgroup(int id) {
    auto newsgroupPath = dbRoot / std::to_string(id);
    if (std::filesystem::exists(newsgroupPath)) {
        std::filesystem::remove_all(newsgroupPath);
        std::cout << "Newsgroup deleted: ID " << id << "\n";
        return true;
    }
    std::cerr << "Failed to delete newsgroup: No such ID.\n";
    return false;
}

std::vector<std::pair<int, std::string>> DiskDatabase::listNewsgroups() const {
    std::vector<std::tuple<std::chrono::system_clock::time_point, int, std::string>> tempGroups; tempGroups;

    for (const auto& entry : std::filesystem::directory_iterator(dbRoot)) {
        if (entry.is_directory()) {
            std::ifstream in(entry.path() / "meta.txt");
            std::string name;
            if (std::getline(in, name) && name.starts_with("Name: ")) {
                auto file_time = std::filesystem::last_write_time(entry);
                auto sys_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                file_time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
                int id = std::stoi(entry.path().filename().string());
                tempGroups.emplace_back(sys_time, id, name.substr(6));
            }
        }
    }

    std::sort(tempGroups.begin(), tempGroups.end(), 
        [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    std::vector<std::pair<int, std::string>> groups;
    for (const auto& group : tempGroups) {
        groups.emplace_back(std::get<1>(group), std::get<2>(group));
    }

    std::cout << "Listing newsgroups, count: " << groups.size() << "\n";
    return groups;
}


bool DiskDatabase::createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) {
    auto path = dbRoot / std::to_string(newsgroupId);
    if (std::filesystem::exists(path)) {
        int articleId = std::hash<std::string>{}(title + author + text);
        std::ofstream out(path / (std::to_string(articleId) + ".txt"));
        out << "Title: " << title << "\nAuthor: " << author << "\nText: " << text << std::endl;
        std::cout << "Article created: " << title << " with ID " << articleId << " in newsgroup " << newsgroupId << "\n";
        return true;
    }
    std::cerr << "Failed to create article: No such newsgroup ID.\n";
    return false;
}

bool DiskDatabase::deleteArticle(int newsgroupId, int articleId) {
    auto articlePath = dbRoot / std::to_string(newsgroupId) / (std::to_string(articleId) + ".txt");
    if (std::filesystem::exists(articlePath)) {
        std::filesystem::remove(articlePath);
        std::cout << "Article deleted: ID " << articleId << " from newsgroup ID " << newsgroupId << "\n";
        return true;
    }
    std::cerr << "Failed to delete article: No such article ID.\n";
    return false;
}

std::tuple<bool, std::string, std::string, std::string> DiskDatabase::getArticle(int newsgroupId, int articleId) const {
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

std::optional<std::vector<std::pair<int, std::string>>> DiskDatabase::listArticles(int newsgroupId) const {
    auto path = dbRoot / std::to_string(newsgroupId);
    if (std::filesystem::exists(path)) {
        std::vector<std::pair<int, std::string>> articles;
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::ifstream in(entry.path());
                std::string title;
                if (std::getline(in, title) && title.starts_with("Title: ")) {
                    articles.emplace_back(std::stoi(entry.path().filename().string()), title.substr(7));
                }
            }
        }
        std::reverse(articles.begin(), articles.end());
        std::cout << "Listing articles in newsgroup " << newsgroupId << ", count: " << articles.size() << "\n";
        return articles;
    }
    std::cerr << "Failed to list articles: No such newsgroup ID.\n";
    return std::nullopt;
}
