#pragma once

#include <deque>
#include <memory>

#include "Snowflake.hpp"
#include "Storage/MessageRepository.hpp"
#include "Discord/Entities.hpp"
#include "Discord/Client.hpp"

namespace Acheron::Core::Markdown {
class Parser;
}

namespace Acheron {
namespace Core {

class UserManager;

struct MessageRequestResult
{
    // not using Result cuz i want to see the type and id even if it failed
    bool success;
    Discord::Client::MessageLoadType type;
    Snowflake channelId;
    QList<Discord::Message> messages;
};

class MessageManager : public QObject
{
    Q_OBJECT
public:
    explicit MessageManager(Snowflake accountId, Discord::Client *client,
                            UserManager *userManager, QObject *parent = nullptr);
    ~MessageManager() override;

    void requestLoadChannel(Snowflake channelId);
    void requestLoadHistory(Snowflake channelId, Snowflake beforeId);
    void sendMessage(Snowflake channelId, const QString &content);

signals:
    void messagesReceived(const MessageRequestResult &result);
    void messageErrored(const QString &nonce);

public slots:
    void onMessageCreated(const Discord::Message &message);
    void onMessageSendFailed(const QString &nonce, const QString &error);

private slots:
    void onApiMessagesReceived(const QList<Discord::Message> &messages,
                               Discord::Client::MessageLoadType type, Snowflake channelId);

private:
    void cacheMessages(Snowflake channelId, const QList<Discord::Message> &msgs);
    QList<Discord::Message> getCachedMessages();

    Storage::MessageRepository repo;

    Discord::Client *client;
    UserManager *userManager;
    std::unique_ptr<Markdown::Parser> parser;

    QCache<Snowflake, Discord::Message> messageCache;
    QHash<Snowflake, std::deque<Snowflake>> channelMessages;
    QSet<Snowflake> fetchedChannels;
    QHash<Snowflake, Snowflake> lowestKnownId;
    QSet<Snowflake> historyDebounce;
};

} // namespace Core
} // namespace Acheron
