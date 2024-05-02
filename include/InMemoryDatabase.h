#include "Database.h"
#include <unordered_map>
#include <memory>

class InMemoryDatabase : public Database {
private:
    struct Article {
        int id;
        std::string title, author, text;
    };

    struct Newsgroup {
        int id;
        std::string name;
        std::unordered_map<int, Article> articles;
    };

    int nextNewsgroupId = 0, nextArticleId = 0;
    std::unordered_map<int, Newsgroup> newsgroups;

public:
    InMemoryDatabase() = default;
    virtual ~InMemoryDatabase();
    bool createNewsgroup(const std::string& name) override;
    bool deleteNewsgroup(int id) override;
    std::vector<std::pair<int, std::string>> listNewsgroups() const override;

    bool createArticle(int newsgroupId, const std::string& title, const std::string& author, const std::string& text) override;
    bool deleteArticle(int newsgroupId, int articleId) override;
    std::tuple<bool, std::string, std::string, std::string> getArticle(int newsgroupId, int articleId) const override;
    std::vector<std::pair<int, std::string>> listArticles(int newsgroupId) const override;
};
