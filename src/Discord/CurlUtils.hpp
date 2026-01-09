#pragma once

#include <QString>

namespace Acheron {
namespace Discord {
namespace CurlUtils {

struct UserAgentProps
{
    QString os;
    QString browser;
    QString browserVersion;
    QString osVersion;
};

QString getCertificatePath();
QString getUserAgent();
QString getImpersonateTarget();
UserAgentProps getUserAgentProps();

} // namespace CurlUtils
} // namespace Discord
} // namespace Acheron
