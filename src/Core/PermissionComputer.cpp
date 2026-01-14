#include "PermissionComputer.hpp"

namespace Acheron {
namespace Core {

QHash<Snowflake, Discord::Role> PermissionComputer::buildRoleMap(const QList<Discord::Role> &roles)
{
    QHash<Snowflake, Discord::Role> roleMap;
    roleMap.reserve(roles.size());
    for (const auto &role : roles)
        roleMap.insert(role.id.get(), role);
    return roleMap;
}

Discord::Permissions PermissionComputer::computeBasePermissions(
        Snowflake guildOwnerId, Snowflake userId, Snowflake guildId,
        const QList<Snowflake> &memberRoleIds, const QList<Discord::Role> &allRoles)
{
    if (guildOwnerId == userId)
        return Discord::ALL_PERMISSIONS;

    auto roleMap = buildRoleMap(allRoles);

    Discord::Permissions permissions = Discord::NO_PERMISSIONS;
    if (auto everyoneRole = roleMap.value(guildId); everyoneRole.id.hasValue())
        permissions = everyoneRole.permissions.get();

    for (const auto &roleId : memberRoleIds) {
        if (auto role = roleMap.value(roleId); role.id.hasValue())
            permissions |= role.permissions.get();
    }

    if (permissions & Discord::Permission::ADMINISTRATOR)
        return Discord::ALL_PERMISSIONS;

    return permissions;
}

Discord::Permissions PermissionComputer::computeOverwrites(
        Discord::Permissions basePermissions, Snowflake userId, Snowflake guildId,
        const QList<Snowflake> &memberRoleIds,
        const QList<Discord::PermissionOverwrite> &overwrites)
{
    if (basePermissions & Discord::Permission::ADMINISTRATOR)
        return Discord::ALL_PERMISSIONS;

    Discord::Permissions permissions = basePermissions;

    // @everyone
    for (const auto &ow : overwrites) {
        if (ow.type.get() == Discord::PermissionOverwrite::Type::Role && ow.id.get() == guildId) {
            permissions &= ~ow.deny.get();
            permissions |= ow.allow.get();
            break;
        }
    }

    Discord::Permissions allowRoles = Discord::NO_PERMISSIONS;
    Discord::Permissions denyRoles = Discord::NO_PERMISSIONS;

    for (const auto &roleId : memberRoleIds) {
        for (const auto &ow : overwrites) {
            if (ow.type.get() == Discord::PermissionOverwrite::Type::Role &&
                ow.id.get() == roleId) {
                denyRoles |= ow.deny.get();
                allowRoles |= ow.allow.get();
                break;
            }
        }
    }

    permissions &= ~denyRoles;
    permissions |= allowRoles;

    for (const auto &ow : overwrites) {
        if (ow.type.get() == Discord::PermissionOverwrite::Type::Member && ow.id.get() == userId) {
            permissions &= ~ow.deny.get();
            permissions |= ow.allow.get();
            break;
        }
    }

    return permissions;
}

Discord::Permissions PermissionComputer::computeChannelPermissions(
        Snowflake guildOwnerId, Snowflake userId, Snowflake guildId, bool isDM,
        const QList<Snowflake> &memberRoleIds, const QList<Discord::Role> &allRoles,
        const QList<Discord::PermissionOverwrite> &overwrites)
{
    if (isDM) {
        // clang-format off
        return Discord::Permission::VIEW_CHANNEL |
               Discord::Permission::SEND_MESSAGES |
               Discord::Permission::READ_MESSAGE_HISTORY;
        // clang-format on
    }

    if (guildOwnerId == userId)
        return Discord::ALL_PERMISSIONS;

    auto basePerms = computeBasePermissions(guildOwnerId, userId, guildId, memberRoleIds, allRoles);

    if (basePerms & Discord::Permission::ADMINISTRATOR)
        return Discord::ALL_PERMISSIONS;

    return computeOverwrites(basePerms, userId, guildId, memberRoleIds, overwrites);
}

} // namespace Core
} // namespace Acheron
