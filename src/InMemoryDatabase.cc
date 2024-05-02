#include "InMemoryDatabase.h"
#include <iostream>

// id and newsgroup tuple
InMemoryDatabase::~InMemoryDatabase() {
    // Debug message for destructor call
    std::cout << "InMemoryDatabase destructor called.\n";
}

bool InMemoryDatabase::createNewsgroup(const std::string& name) {
    for (const auto& ng : newsgroups) {
        if (ng.second.name == name) {
            std::cout << "Failed to create newsgroup, name already exists: " << name << "\n";
            return false; // Newsgroup with this name already exists
        }
    }
    Newsgroup newsgroup;
    newsgroup.id = nextNewsgroupId++;
    newsgroup.name = name;
    newsgroups[newsgroup.id] = newsgroup;
    std::cout << "Newsgroup created: " << name << " with ID " << newsgroup.id << "\n";
    return true;
}

bool InMemoryDatabase::deleteNewsgroup(int id) {
    if (newsgroups.erase(id) == 0) {
        std::cout << "Failed to delete newsgroup, no such ID: " << id << "\n";
        return false; // No newsgroup with this ID found
    }
    std::cout << "Newsgroup deleted: ID " << id << "\n";
    return true;
}

std::vector<std::pair<int, std::string>> InMemoryDatabase::listNewsgroups() const {
    std::vector<std::pair<int, std::string>> result;
    for (const auto& ng : newsgroups) {
        result.push_back({ng.first, ng.second.name});
    }
    std::cout << "Listing newsgroups, count: " << result.size() << "\n";
    return result;
}

bool InMemoryDatabase::createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) {
    auto it = newsgroups.find(newsgroupId);
    if (it == newsgroups.end()) {
        std::cout << "Failed to create article, no such newsgroup ID: " << newsgroupId << "\n";
        return false; // No newsgroup with this ID
    }

    Article article;
    article.id = nextArticleId++;
    article.title = title;
    article.author = author;
    article.text = text;
    it->second.articles[article.id] = article;
    std::cout << "Article created in newsgroup " << newsgroupId << ": " << title << " with ID " << article.id << "\n";
    return true;
}

bool InMemoryDatabase::deleteArticle(int newsgroupId, int articleId) {
    auto it = newsgroups.find(newsgroupId);
    if (it == newsgroups.end()) {
        std::cout << "Failed to delete article, no such newsgroup ID: " << newsgroupId << "\n";
        return false; // No newsgroup with this ID
    }

    if (it->second.articles.erase(articleId) == 0) {
        std::cout << "Failed to delete article, no such article ID: " << articleId << "\n";
        return false; // No article with this ID in the newsgroup
    }

    std::cout << "Article deleted: ID " << articleId << " from newsgroup ID " << newsgroupId << "\n";
    return true;
}

std::tuple<bool, std::string, std::string, std::string> InMemoryDatabase::getArticle(int newsgroupId, int articleId) const {
    auto ng_it = newsgroups.find(newsgroupId);
    if (ng_it == newsgroups.end()) {
        std::cout << "No newsgroup found for ID: " << newsgroupId << "\n";
        return {false, "", "", ""}; // No newsgroup with this ID
    }

    auto art_it = ng_it->second.articles.find(articleId);
    if (art_it == ng_it->second.articles.end()) {
        std::cout << "No article found for ID: " << articleId << " in newsgroup ID: " << newsgroupId << "\n";
        return {false, "", "", ""}; // No article with this ID
    }

    std::cout << "Article retrieved: " << art_it->second.title << "\n";
    return {true, art_it->second.title, art_it->second.author, art_it->second.text};
}

std::vector<std::pair<int, std::string>> InMemoryDatabase::listArticles(int newsgroupId) const {
    std::vector<std::pair<int, std::string>> result;
    auto ng_it = newsgroups.find(newsgroupId);
    if (ng_it == newsgroups.end()) {
        std::cout << "No newsgroup found for listing articles, ID: " << newsgroupId << "\n";
        return result; // No newsgroup with this ID
    }

    for (const auto& article : ng_it->second.articles) {
        result.push_back({article.first, article.second.title});
    }
    std::cout << "Articles listed for newsgroup " << newsgroupId << ", count: " << result.size() << "\n";
    return result;
}
