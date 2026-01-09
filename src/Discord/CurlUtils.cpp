#include "CurlUtils.hpp"

#include "Core/Logging.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>

namespace Acheron {
namespace Discord {
namespace CurlUtils {

QString getCertificatePath()
{
    QString execDir = QCoreApplication::applicationDirPath();
    QString certPath = QDir(execDir).filePath("certs/cacert.pem");
    if (QFile::exists(certPath))
        return certPath;

    qCWarning(LogNetwork) << "Certificate file not found at" << certPath;
    return QString();
}

QString getUserAgent()
{
    return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
           "Chrome/142.0.0.0 Safari/537.36";
}

QString getImpersonateTarget()
{
    return "chrome142";
}

UserAgentProps getUserAgentProps()
{
    return { "Windows", "Chrome", "142.0.0.0", "10" };
}

} // namespace CurlUtils
} // namespace Discord
} // namespace Acheron
