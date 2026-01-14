#pragma once

#include <QObject>
#include <QCache>
#include <QPair>
#include <optional>

#include "Snowflake.hpp"
#include "Discord/Enums.hpp"
#include "Discord/Entities.hpp"

#include "Storage/GuildRepository.hpp"
#include "Storage/ChannelRepository.hpp"
#include "Storage/RoleRepository.hpp"
#include "Storage/MemberRepository.hpp"

namespace Acheron {
namespace Core {

class PermissionManager : public QObject
{
    Q_OBJECT
public:
    explicit PermissionManager(Snowflake accountId, QObject *parent = nullptr);

    Discord::Permissions getChannelPermissions(Snowflake userId, Snowflake channelId);
    bool hasChannelPermission(Snowflake userId, Snowflake channelId,
                              Discord::Permission permission);

private:
    Discord::Permissions computeChannelPermissions(Snowflake userId, Snowflake channelId);

    Storage::RoleRepository roleRepo;
    Storage::GuildRepository guildRepo;
    Storage::ChannelRepository channelRepo;
    Storage::MemberRepository memberRepo;

    QCache<QPair<Snowflake, Snowflake>, Discord::Permissions> permissionCache;
};

} // namespace Core
} // namespace Acheron
