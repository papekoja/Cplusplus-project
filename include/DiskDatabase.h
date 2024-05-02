#ifndef DISK_DATABASE_H
#define DISK_DATABASE_H

#include "Database.h"
#include <filesystem>
#include <string>
#include <vector>

class DiskDatabase : public Database {
private:
    std::filesystem::path dbRoot;

    struct Article {
        int id;
        std::string title, author, text;
        std::string filename() const { return std::to_string(id) + ".txt"; }
    };

    struct Newsgroup {
        int id;
        std::string name;
        std::string dirname() const { return std::to_string(id); }
    };

public:
    DiskDatabase(const std::string& rootPath);
    virtual ~DiskDatabase();
    bool createNewsgroup(const std::string& name) override;
    bool deleteNewsgroup(int id) override;
    std::vector<std::pair<int, std::string>> listNewsgroups() const override;

    bool createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) override;
    bool deleteArticle(int newsgroupId, int articleId) override;
    std::tuple<bool, std::string, std::string, std::string> getArticle(int newsgroupId, int articleId) const override;
    std::vector<std::pair<int, std::string>> listArticles(int newsgroupId) const override;
};

#endif
