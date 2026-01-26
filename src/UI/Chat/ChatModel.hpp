#pragma once

#include <QtWidgets>
#include "Core/Session.hpp"
#include "Core/MessageManager.hpp"

using Acheron::Core::ClientInstance;

namespace Acheron {

namespace Core {
class ImageManager;
class AttachmentCache;
} // namespace Core

struct AttachmentData
{
    Snowflake id;
    QUrl proxyUrl;
    QUrl originalUrl;
    QSize displaySize;
    QPixmap pixmap;
    bool isLoading;
    bool isImage;
    QString filename;
    qint64 fileSizeBytes;
    bool isSpoiler;
};

struct EmbedFieldData
{
    QString name;
    QString value;
    QString nameParsed;
    QString valueParsed;
    bool isInline = false;
};

struct EmbedImageData
{
    QUrl url;
    QPixmap pixmap;
    QSize displaySize;
};

enum class EmbedType {
    AgeVerificationSystemNotification,
    ApplicationNews,
    Article,
    AutoModerationMessage,
    AutoModerationNotification,
    Gift,
    Gifv,
    Image,
    Link,
    PollResult,
    PostPreview,
    Rich,
    SafetyPolicyNotice,
    SafetySystemNotification,
    Video,
};

struct EmbedData
{
    EmbedType type = EmbedType::Rich; // should this be default idk
    QString title;
    QString description;
    QString titleParsed;
    QString descriptionParsed;
    QString url;
    QDateTime timestamp;
    QColor color;

    QString authorName;
    QString authorUrl;
    QUrl authorIconUrl;
    QPixmap authorIcon;

    QString footerText;
    QUrl footerIconUrl;
    QPixmap footerIcon;

    QUrl thumbnailUrl;
    QPixmap thumbnail;
    QSize thumbnailSize;

    QList<EmbedImageData> images;

    QUrl videoThumbnailUrl;
    QPixmap videoThumbnail;
    QSize videoThumbnailSize;

    QString providerName;
    QString providerUrl;

    QList<EmbedFieldData> fields;
};

namespace UI {
class ChatModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ChatModel(Core::ImageManager *imageManager, Core::AttachmentCache *attachmentCache,
              QObject *parent = nullptr);

    enum Roles {
        ContentRole = Qt::UserRole + 1,
        UsernameRole,
        UserIdRole,
        AvatarRole,
        TimestampRole,
        EditedTimestampRole,
        CachedSizeRole,
        ShowHeaderRole,
        DateSeparatorRole,
        HtmlRole,
        AttachmentsRole,
        EmbedsRole,
        IsPendingRole,
        IsErroredRole,
        UsernameColorRole,
    };

    using AvatarUrlResolver = std::function<QUrl(const Discord::User &)>;
    void setAvatarUrlResolver(AvatarUrlResolver resolver);

    using RoleColorResolver = std::function<QColor(Snowflake userId, Snowflake guildId)>;
    void setRoleColorResolver(RoleColorResolver resolver);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    [[nodiscard]] Snowflake getOldestMessageId() const;
    [[nodiscard]] Snowflake getActiveChannelId() const;
    [[nodiscard]] bool isSpoilerRevealed(Snowflake attachmentId) const;

public slots:
    void setActiveChannel(Snowflake channelId, Snowflake guildId = Snowflake::Invalid);
    void handleIncomingMessages(const Core::MessageRequestResult &result);
    void handleMessageDeleted(Snowflake channelId, Snowflake messageId);
    void handleMessageErrored(const QString &nonce);
    void refreshUsersInView(const QList<Snowflake> &userIds);
    void revealSpoiler(Snowflake attachmentId);

    void triggerResize(int row)
    {
        if (row < 0 || row >= rowCount())
            return;
        QModelIndex idx = index(row, 0);
        emit dataChanged(idx, idx, { CachedSizeRole });
    }

private:
    void setMessages(const QList<Discord::Message> &messages);

    Core::ImageManager *imageManager;
    Core::AttachmentCache *attachmentCache;
    QVector<Discord::Message> messages;
    mutable QHash<Snowflake, QSize> sizeCache;
    mutable QHash<Snowflake, QList<EmbedData>> embedCache;

    Snowflake currentChannelId = Snowflake::Invalid;
    Snowflake currentGuildId = Snowflake::Invalid;

    AvatarUrlResolver avatarUrlResolver;
    RoleColorResolver roleColorResolver;

    mutable QMultiMap<QUrl, QPersistentModelIndex> pendingRequests;
    QSet<QString> pendingNonces;
    QSet<QString> erroredNonces;
    mutable QSet<Snowflake> revealedSpoilers;
};
} // namespace UI
} // namespace Acheron

Q_DECLARE_METATYPE(Acheron::AttachmentData)
Q_DECLARE_METATYPE(QList<Acheron::AttachmentData>)
Q_DECLARE_METATYPE(Acheron::EmbedFieldData)
Q_DECLARE_METATYPE(Acheron::EmbedImageData)
Q_DECLARE_METATYPE(Acheron::EmbedData)
Q_DECLARE_METATYPE(QList<Acheron::EmbedData>)
