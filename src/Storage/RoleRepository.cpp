#include "RoleRepository.hpp"
#include "DatabaseManager.hpp"
#include "Core/Logging.hpp"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

namespace Acheron {
namespace Storage {

RoleRepository::RoleRepository(Core::Snowflake accountId)
    : BaseRepository(DatabaseManager::getCacheConnectionName(accountId))
{
}

void RoleRepository::saveRole(Core::Snowflake guildId, const Discord::Role &role, QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.prepare(R"(
        INSERT OR REPLACE INTO roles
        (id, guild_id, name, permissions, position, color, hoist, icon,
         unicode_emoji, managed, mentionable)
        VALUES (:id, :guild_id, :name, :permissions, :position, :color, :hoist,
                :icon, :unicode_emoji, :managed, :mentionable)
    )");

    q.bindValue(":id", static_cast<qint64>(role.id.get()));
    q.bindValue(":guild_id", static_cast<qint64>(guildId));
    q.bindValue(":name", role.name.get());
    q.bindValue(":permissions", static_cast<qint64>(role.permissions.get()));
    q.bindValue(":position", role.position.get());
    q.bindValue(":color", role.color.hasValue() ? QVariant(role.color.get()) : QVariant());
    q.bindValue(":hoist", role.hoist.hasValue() ? QVariant(role.hoist.get()) : QVariant());
    q.bindValue(":icon", role.icon.hasValue() && !role.icon.isNull() ? QVariant(role.icon.get())
                                                                     : QVariant());
    q.bindValue(":unicode_emoji", role.unicodeEmoji.hasValue() && !role.unicodeEmoji.isNull()
                                          ? QVariant(role.unicodeEmoji.get())
                                          : QVariant());
    q.bindValue(":managed", role.managed.hasValue() ? QVariant(role.managed.get()) : QVariant());
    q.bindValue(":mentionable",
                role.mentionable.hasValue() ? QVariant(role.mentionable.get()) : QVariant());

    if (!q.exec())
        qCWarning(LogDB) << "RoleRepository: Save role failed:" << q.lastError().text();
}

void RoleRepository::saveRoles(Core::Snowflake guildId, const QList<Discord::Role> &roles,
                               QSqlDatabase &db)
{
    if (roles.isEmpty())
        return;

    QSqlQuery q(db);
    q.prepare(R"(
        INSERT OR REPLACE INTO roles
        (id, guild_id, name, permissions, position, color, hoist, icon,
         unicode_emoji, managed, mentionable)
        VALUES (:id, :guild_id, :name, :permissions, :position, :color, :hoist,
                :icon, :unicode_emoji, :managed, :mentionable)
    )");

    for (const auto &role : roles) {
        q.bindValue(":id", static_cast<qint64>(role.id.get()));
        q.bindValue(":guild_id", static_cast<qint64>(guildId));
        q.bindValue(":name", role.name.get());
        q.bindValue(":permissions", static_cast<qint64>(role.permissions.get()));
        q.bindValue(":position", role.position.get());
        q.bindValue(":color", role.color.hasValue() ? QVariant(role.color.get()) : QVariant());
        q.bindValue(":hoist", role.hoist.hasValue() ? QVariant(role.hoist.get()) : QVariant());
        q.bindValue(":icon", role.icon.hasValue() && !role.icon.isNull() ? QVariant(role.icon.get())
                                                                         : QVariant());
        q.bindValue(":unicode_emoji", role.unicodeEmoji.hasValue() && !role.unicodeEmoji.isNull()
                                              ? QVariant(role.unicodeEmoji.get())
                                              : QVariant());
        q.bindValue(":managed",
                    role.managed.hasValue() ? QVariant(role.managed.get()) : QVariant());
        q.bindValue(":mentionable",
                    role.mentionable.hasValue() ? QVariant(role.mentionable.get()) : QVariant());

        if (!q.exec())
            qCWarning(LogDB) << "RoleRepository: Save role failed:" << q.lastError().text();
    }
}

std::optional<Discord::Role> RoleRepository::getRole(Core::Snowflake guildId,
                                                     Core::Snowflake roleId)
{
    auto db = getDb();
    QSqlQuery q(db);
    q.prepare(R"(
        SELECT id, name, permissions, position, color, hoist, icon,
               unicode_emoji, managed, mentionable
        FROM roles WHERE guild_id = :guild_id AND id = :id
    )");
    q.bindValue(":guild_id", static_cast<qint64>(guildId));
    q.bindValue(":id", static_cast<qint64>(roleId));

    if (!q.exec() || !q.next())
        return std::nullopt;

    Discord::Role role;
    role.id = static_cast<Core::Snowflake>(q.value(0).toLongLong());
    role.name = q.value(1).toString();
    role.permissions = Discord::Permissions::fromInt(q.value(2).toLongLong());
    role.position = q.value(3).toInt();
    if (!q.value(4).isNull())
        role.color = q.value(4).toInt();
    if (!q.value(5).isNull())
        role.hoist = q.value(5).toBool();
    if (!q.value(6).isNull())
        role.icon = q.value(6).toString();
    if (!q.value(7).isNull())
        role.unicodeEmoji = q.value(7).toString();
    if (!q.value(8).isNull())
        role.managed = q.value(8).toBool();
    if (!q.value(9).isNull())
        role.mentionable = q.value(9).toBool();

    return role;
}

QList<Discord::Role> RoleRepository::getRolesForGuild(Core::Snowflake guildId)
{
    QList<Discord::Role> roles;
    auto db = getDb();
    QSqlQuery q(db);
    q.prepare(R"(
        SELECT id, name, permissions, position, color, hoist, icon,
               unicode_emoji, managed, mentionable
        FROM roles WHERE guild_id = :guild_id
        ORDER BY position ASC
    )");
    q.bindValue(":guild_id", static_cast<qint64>(guildId));

    if (!q.exec()) {
        qCWarning(LogDB) << "RoleRepository: Get roles failed:" << q.lastError().text();
        return roles;
    }

    while (q.next()) {
        Discord::Role role;
        role.id = static_cast<Core::Snowflake>(q.value(0).toLongLong());
        role.name = q.value(1).toString();
        role.permissions = Discord::Permissions::fromInt(q.value(2).toLongLong());
        role.position = q.value(3).toInt();
        if (!q.value(4).isNull())
            role.color = q.value(4).toInt();
        if (!q.value(5).isNull())
            role.hoist = q.value(5).toBool();
        if (!q.value(6).isNull())
            role.icon = q.value(6).toString();
        if (!q.value(7).isNull())
            role.unicodeEmoji = q.value(7).toString();
        if (!q.value(8).isNull())
            role.managed = q.value(8).toBool();
        if (!q.value(9).isNull())
            role.mentionable = q.value(9).toBool();
        roles.append(role);
    }

    return roles;
}

void RoleRepository::deleteRole(Core::Snowflake guildId, Core::Snowflake roleId, QSqlDatabase &db)
{
    QSqlQuery q(db);
    q.prepare("DELETE FROM roles WHERE guild_id = :guild_id AND id = :id");
    q.bindValue(":guild_id", static_cast<qint64>(guildId));
    q.bindValue(":id", static_cast<qint64>(roleId));

    if (!q.exec())
        qCWarning(LogDB) << "RoleRepository: Delete role failed:" << q.lastError().text();
}

} // namespace Storage
} // namespace Acheron
