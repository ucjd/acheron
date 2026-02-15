#pragma once

#include <QString>
#include <QUrl>
#include <QCache>
#include <QPixmap>
#include <qlabel.h>

class QNetworkAccessManager;

namespace Acheron {
namespace Core {

enum class PinGroup {
    None,
    ChannelList,
    ChatView,
};

struct ImageRequestKey
{
    QUrl url;
    QSize size;

    bool operator==(const ImageRequestKey &other) const
    {
        return url == other.url && size == other.size;
    }
};

class ImageManager : public QObject
{
    Q_OBJECT
public:
    explicit ImageManager(QObject *parent = nullptr);

    [[nodiscard]] ImageRequestKey key(const QUrl &url, const QSize &size);

    [[nodiscard]] bool isCached(const QUrl &url, const QSize &size);
    void assign(QLabel *label, const QUrl &url, const QSize &size);
    QPixmap get(const QUrl &url, const QSize &size, PinGroup pin = PinGroup::None);
    [[nodiscard]] QPixmap placeholder(const QSize &size);

    void unpinGroup(PinGroup group);

signals:
    void imageFetched(const QUrl &url, const QSize &size, const QPixmap &pixmap);

private:
    void request(const QUrl &url, const QSize &size, PinGroup pin);
    void fetchFromNetwork(const QUrl &url, const QSize &size, PinGroup pin);

    QNetworkAccessManager *networkManager;

    QSet<ImageRequestKey> requests;
    QHash<ImageRequestKey, PinGroup> pendingPins;
    QCache<ImageRequestKey, QPixmap> cache;
    QHash<ImageRequestKey, QPixmap> pinnedImages;
    QMultiHash<PinGroup, ImageRequestKey> pinGroupKeys;
};

} // namespace Core
} // namespace Acheron

namespace std {
template <>
struct hash<Acheron::Core::ImageRequestKey>
{
    size_t operator()(const Acheron::Core::ImageRequestKey &key, size_t seed = 0) const
    {
        return qHashMulti(seed, key.url, key.size);
    }
};
} // namespace std
