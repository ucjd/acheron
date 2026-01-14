#include "ClientInstance.hpp"

#include "Storage/DatabaseManager.hpp"
#include "Storage/GuildRepository.hpp"
#include "Storage/ChannelRepository.hpp"
#include "Storage/RoleRepository.hpp"
#include "Storage/MemberRepository.hpp"

namespace Acheron {
namespace Core {
ClientInstance::ClientInstance(const AccountInfo &info, QObject *parent)
    : QObject(parent),
      account(info),
      roleRepo(info.id),
      guildRepo(info.id),
      channelRepo(info.id),
      memberRepo(info.id)
{
    client = new Discord::Client(info.token, info.gatewayUrl, info.restUrl, this);
    messageManager = new MessageManager(info.id, client, this);

    Storage::DatabaseManager::instance().openCacheDatabase(info.id);

    userManager = new UserManager(info.id, this);

    permissionManager = new PermissionManager(info.id, this);

    connect(client, &Discord::Client::stateChanged, this, &ClientInstance::stateChanged);

    connect(client, &Discord::Client::ready, this, [this](const Discord::Ready &ready) {
        QString connName = Storage::DatabaseManager::instance().getCacheConnectionName(account.id);
        QSqlDatabase db = QSqlDatabase::database(connName);

        account.username = ready.user->username;
        account.displayName = ready.user->globalName;
        account.avatar = ready.user->avatar;

        db.transaction();
        for (size_t i = 0; i < ready.guilds->size(); i++) {
            const auto &guild = ready.guilds->at(i);

            guildRepo.saveGuild(guild.asGuild(), db);

            if (guild.roles.hasValue())
                roleRepo.saveRoles(guild.properties->id.get(), guild.roles.get(), db);

            for (const auto &channel : guild.channels.get()) {
                // we dont get a guild_id from the gateway here
                Discord::Channel copy = channel;
                copy.guildId = guild.properties->id;
                channelRepo.saveChannel(copy, db);

                if (copy.permissionOverwrites.hasValue())
                    channelRepo.savePermissionOverwrites(copy.id.get(),
                                                         copy.permissionOverwrites.get(), db);
            }

            if (ready.mergedMembers.hasValue()) {
                const auto &members = ready.mergedMembers->at(i);
                for (const auto &member : members)
                    memberRepo.saveMember(guild.properties->id.get(), member.userId.get(), member);
            }
        }

        userManager->saveUser(ready.user);

        db.commit();

        emit detailsUpdated(account);
        emit this->ready(ready);
    });

    connect(client, &Discord::Client::readySupplemental, this,
            [this](const Discord::ReadySupplemental &data) {
                QString connName =
                        Storage::DatabaseManager::instance().getCacheConnectionName(account.id);
                QSqlDatabase db = QSqlDatabase::database(connName);

                db.transaction();

                for (size_t i = 0; i < data.mergedMembers->size(); i++) {
                    const auto &guild = data.guilds->at(i);
                    const auto &members = data.mergedMembers->at(i);

                    for (const auto &member : members)
                        memberRepo.saveMember(guild.id, member.userId.get(), member);
                }

                db.commit();
            });

    connect(client, &Discord::Client::messageCreated, messageManager,
            &MessageManager::onMessageCreated);
    connect(client, &Discord::Client::messageSendFailed, messageManager,
            &MessageManager::onMessageSendFailed);
}

ClientInstance::~ClientInstance()
{
    Storage::DatabaseManager::instance().closeCacheDatabase(account.id);
}

void ClientInstance::start()
{
    discord()->start();
}

void ClientInstance::stop()
{
    discord()->stop();
}

Discord::Client *ClientInstance::discord() const
{
    return client;
}

MessageManager *ClientInstance::messages() const
{
    return messageManager;
}

UserManager *ClientInstance::users() const
{
    return userManager;
}

PermissionManager *ClientInstance::permissions() const
{
    return permissionManager;
}

ConnectionState ClientInstance::state() const
{
    return client->getState();
}

Snowflake ClientInstance::accountId() const
{
    return account.id;
}

const AccountInfo &ClientInstance::accountInfo() const
{
    return account;
}
} // namespace Core
} // namespace Acheron
