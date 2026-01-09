#pragma once

#include <QObject>

#include <curl/curl.h>

namespace Acheron {
namespace Discord {

class ClientIdentity;

struct HttpResponse
{
    int statusCode = 0;
    QByteArray body;
    QString error;
    bool success = false;
};

using HttpCallback = std::function<void(const HttpResponse &)>;

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(const QString &baseUrl, const QString &token, ClientIdentity &identity,
                        QObject *parent = nullptr);

    void get(const QString &endpoint, const QUrlQuery &query, HttpCallback callback);

private:
    enum class Method { GET, POST, PUT, PATCH, DELETE_ }; // thanks windows.h for the DELETE macro

    static void lock_cb(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr);
    static void unlock_cb(CURL *handle, curl_lock_data data, void *userptr);
    void setupSharing();

    void executeRequest(Method method, const QString &endpoint, const QByteArray &data,
                        HttpCallback callback);

    CURLSH *share;
    static inline std::array<std::mutex, 3> shareMutexes;

    QString baseUrl;
    QString token;
    ClientIdentity &identity;
};

} // namespace Discord
} // namespace Acheron
