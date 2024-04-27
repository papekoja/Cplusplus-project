#include "InMemoryDatabase.h"

InMemoryDatabase::~InMemoryDatabase() {
    
}
bool InMemoryDatabase::createNewsgroup(const std::string& name) {
    // Check if a newsgroup with the same name already exists
    for (const auto& ng : newsgroups) {
        if (ng.second.name == name) {
            return false; // Newsgroup with this name already exists
        }
    }
    Newsgroup newsgroup;
    newsgroup.id = nextNewsgroupId++;
    newsgroup.name = name;
    newsgroups[newsgroup.id] = newsgroup;
    return true;
}

bool InMemoryDatabase::deleteNewsgroup(int id) {
    if (newsgroups.erase(id) == 0) {
        return false; // No newsgroup with this ID found
    }
    return true;
}

std::vector<std::pair<int, std::string>> InMemoryDatabase::listNewsgroups() const {
    std::vector<std::pair<int, std::string>> result;
    for (const auto& ng : newsgroups) {
        result.push_back({ng.first, ng.second.name});
    }
    return result;
}

bool InMemoryDatabase::createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) {
    auto it = newsgroups.find(newsgroupId);
    if (it == newsgroups.end()) {
        return false; // No newsgroup with this ID
    }

    Article article;
    article.id = nextArticleId++;
    article.title = title;
    article.author = author;
    article.text = text;
    it->second.articles[article.id] = article;

    return true;
}

bool InMemoryDatabase::deleteArticle(int newsgroupId, int articleId) {
    auto it = newsgroups.find(newsgroupId);
    if (it == newsgroups.end()) {
        return false; // No newsgroup with this ID
    }

    if (it->second.articles.erase(articleId) == 0) {
        return false; // No article with this ID in the newsgroup
    }

    return true;
}

std::tuple<bool, std::string, std::string, std::string> InMemoryDatabase::readArticle(int newsgroupId, int articleId) const {
    auto ng_it = newsgroups.find(newsgroupId);
    if (ng_it == newsgroups.end()) {
        return {false, "", "", ""}; // No newsgroup with this ID
    }

    auto art_it = ng_it->second.articles.find(articleId);
    if (art_it == ng_it->second.articles.end()) {
        return {false, "", "", ""}; // No article with this ID
    }

    return {true, art_it->second.title, art_it->second.author, art_it->second.text};
}

std::vector<std::pair<int, std::string>> InMemoryDatabase::listArticles(int newsgroupId) const {
    std::vector<std::pair<int, std::string>> result;
    auto ng_it = newsgroups.find(newsgroupId);
    if (ng_it == newsgroups.end()) {
        return result; // No newsgroup with this ID
    }

    for (const auto& article : ng_it->second.articles) {
        result.push_back({article.first, article.second.title});
    }

    return result;
}
