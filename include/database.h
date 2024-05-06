#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <tuple>
#include <optional>

class Database {
public:
    virtual ~Database() {}

    virtual bool createNewsgroup(const std::string& name) = 0;
    virtual bool deleteNewsgroup(int id) = 0;
    virtual std::vector<std::pair<int, std::string>> listNewsgroups() const = 0;

    virtual bool createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) = 0;
    virtual bool deleteArticle(int newsgroupId, int articleId) = 0;
    virtual std::tuple<bool, std::string, std::string, std::string> getArticle(int newsgroupId, int articleId) const = 0;
    virtual std::optional<std::vector<std::pair<int, std::string>>> listArticles(int newsgroupId) const = 0;
};

#endif
