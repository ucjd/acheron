#pragma once

#include <QObject>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include <QTimer>

#include <optional>

#include "Snowflake.hpp"
#include "Discord/Entities.hpp"
#include "Discord/Events.hpp"

namespace Acheron {
namespace Core {

class PermissionManager;

struct ChannelReadState
{
    bool isUnread = false;
    int mentionCount = 0;
    bool isMuted = false;
};

class ReadStateManager : public QObject
{
    Q_OBJECT
public:
    explicit ReadStateManager(Snowflake accountId, PermissionManager *perms,
                              QObject *parent = nullptr);

    void loadFromReady(const QList<Discord::ReadStateEntry> &readStates,
                       const QList<Discord::UserGuildSettings> &guildSettings);

    [[nodiscard]] ChannelReadState computeChannelReadState(Snowflake channelId, Snowflake guildId,
                                                           bool isDM = false) const;

    bool isChannelUnread(Snowflake channelId, Snowflake channelLastMessageId,
                         bool canSendMessages = true) const;
    int getMentionCount(Snowflake channelId) const;
    bool isChannelMuted(Snowflake channelId) const;
    bool isGuildMuted(Snowflake guildId) const;

    void onMessageAck(const Discord::MessageAck &ack);
    void onUserGuildSettingsUpdate(const Discord::UserGuildSettings &settings);
    void updateLocalReadState(Snowflake channelId, Snowflake lastMessageId);

    void setActiveChannel(Snowflake channelId);
    void markChannelAsRead(Snowflake channelId, Snowflake lastMessageId);
    void markChannelsAsRead(const QList<QPair<Snowflake, Snowflake>> &channelMessagePairs);
    void handleMessageCreated(Snowflake channelId, Snowflake messageId, bool isMention);

    void updateChannelLastMessageId(Snowflake channelId, Snowflake messageId);
    [[nodiscard]] Snowflake getChannelLastMessageId(Snowflake channelId) const;

    const Discord::UserGuildSettings *getGuildSettings(Snowflake guildId) const;
    std::optional<Discord::ReadStateEntry> getReadStateEntry(Snowflake channelId) const;

    static bool isMuteActive(bool muted, const Discord::MuteConfig *muteConfig);
    static int daysSinceDiscordEpoch();

signals:
    void readStateUpdated(Snowflake channelId);
    void guildSettingsUpdated(Snowflake guildId);
    void ackRequested(Snowflake channelId, Snowflake messageId);
    void bulkAckRequested(const QList<QPair<Snowflake, Snowflake>> &channelMessagePairs);

private:
    void rebuildChannelOverrideCache(Snowflake guildSettingsKey);

    Snowflake accountId;
    PermissionManager *permissionManager;

    QHash<Snowflake, Discord::ReadStateEntry> channelReadStates;
    QHash<Snowflake, Snowflake> channelLastMessageIds;
    QHash<Snowflake, Discord::UserGuildSettings> guildSettingsMap; // Snowflake(0) for DMs

    QHash<Snowflake, Discord::ChannelOverride> channelOverrideCache;
    QHash<Snowflake, QSet<Snowflake>> guildOverrideChannels;

    Snowflake activeChannelId;
    QTimer activeChannelAckTimer;
    bool activeChannelAckPending = false;
};

} // namespace Core
} // namespace Acheron
