#include "ImageManager.hpp"
#include "ImageManager.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "Logging.hpp"

namespace Acheron {
namespace Core {

ImageManager::ImageManager(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    cache.setMaxCost(300);
}

bool ImageManager::isCached(const QUrl &url, const QSize &size)
{
    ImageRequestKey k{ url, size };
    return pinnedImages.contains(k) || cache.contains(k);
}

void ImageManager::assign(QLabel *label, const QUrl &url, const QSize &size)
{
    if (!label)
        return;

    // just in case
    disconnect(this, &ImageManager::imageFetched, label, nullptr);

    QPixmap pixmap = get(url, size);
    label->setPixmap(pixmap);

    if (!isCached(url, size)) {
        connect(this, &ImageManager::imageFetched, label,
                [=](const QUrl &u, const QSize &s, const QPixmap &p) {
                    if (u == url && s == size)
                        label->setPixmap(p);
                });
    }
}

QPixmap ImageManager::get(const QUrl &url, const QSize &size, PinGroup pin)
{
    ImageRequestKey k{ url, size };

    auto pinnedIt = pinnedImages.constFind(k);
    if (pinnedIt != pinnedImages.constEnd()) {
        // probably wont happen rn but the same url could be pinned by multiple groups
        if (pin != PinGroup::None && !pinGroupKeys.contains(pin, k))
            pinGroupKeys.insert(pin, k);
        return pinnedIt.value();
    }

    if (cache.contains(k)) {
        QPixmap pixmap = *cache.object(k);
        if (pin != PinGroup::None) {
            // promote to pinned (eg, currently unpinned member list to -> chatview)
            pinnedImages.insert(k, pixmap);
            pinGroupKeys.insert(pin, k);
            cache.remove(k);
        }
        return pixmap;
    }

    request(url, size, pin);
    return placeholder(size);
}

QPixmap ImageManager::placeholder(const QSize &size)
{
    static QPixmap unscaled(":/resources/placeholder.png");
    QPixmap pixmap;
    const QString key = QString("placeholder:%1x%2").arg(size.width()).arg(size.height());
    if (!QPixmapCache::find(key, &pixmap)) {
        pixmap = unscaled.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(key, pixmap);
    }
    return pixmap;
}

void ImageManager::request(const QUrl &url, const QSize &size, PinGroup pin)
{
    ImageRequestKey k{ url, size };
    if (requests.contains(k)) {
        // promote
        if (pin != PinGroup::None) {
            auto it = pendingPins.find(k);
            if (it == pendingPins.end() || it.value() == PinGroup::None)
                pendingPins.insert(k, pin);
        }
        return;
    }

    requests.insert(k);
    if (pin != PinGroup::None)
        pendingPins.insert(k, pin);

    fetchFromNetwork(url, size, pin);
}

void ImageManager::fetchFromNetwork(const QUrl &url, const QSize &size, PinGroup pin)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, url, size]() {
        ImageRequestKey k{ url, size };
        PinGroup pin = pendingPins.value(k, PinGroup::None);
        pendingPins.remove(k);

        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            if (pixmap.loadFromData(reply->readAll())) {
                pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                if (pin != PinGroup::None) {
                    pinnedImages.insert(k, pixmap);
                    pinGroupKeys.insert(pin, k);
                } else {
                    cache.insert(k, new QPixmap(pixmap));
                }

                requests.remove(k);
                emit imageFetched(url, size, pixmap);
            } else {
                requests.remove(k);
            }
        } else {
            requests.remove(k);
        }
        reply->deleteLater();
    });
}

void ImageManager::unpinGroup(PinGroup group)
{
    if (group == PinGroup::None)
        return;

    QList<ImageRequestKey> keys = pinGroupKeys.values(group);
    pinGroupKeys.remove(group);

    for (const auto &k : keys) {
        // wont happen rn but the same url could be pinned by multiple groups
        bool stillPinned = false;
        for (auto it = pinGroupKeys.cbegin(); it != pinGroupKeys.cend(); ++it) {
            if (it.value() == k) {
                stillPinned = true;
                break;
            }
        }

        if (!stillPinned) {
            auto it = pinnedImages.find(k);
            if (it != pinnedImages.end()) {
                // send it back to the lru
                cache.insert(k, new QPixmap(it.value()));
                pinnedImages.erase(it);
            }
        }
    }
}

} // namespace Core
} // namespace Acheron
