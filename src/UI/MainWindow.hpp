#pragma once
#include <QtWidgets>

#include <Core/Snowflake.hpp>
#include "Input/MessageInput.hpp"
#include "Discord/Entities.hpp"

namespace Acheron {
namespace Core {
class Session;
class ClientInstance;
class TypingTracker;
} // namespace Core
namespace Discord {
struct TypingStart;
}
namespace UI {
class ChatView;
class ChatModel;
class ChannelTreeModel;
class ChannelFilterProxyModel;
class AccountsWindow;
class AccountsModel;
class ChannelTreeView;
struct ChannelNode;
class TypingIndicator;
} // namespace UI
} // namespace Acheron

namespace Acheron {
namespace UI {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(Core::Session *session, QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onChannelSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onTypingStart(const Discord::TypingStart &event);
    void onChannelPermissionsChanged(Core::Snowflake channelId);

private:
    void switchActiveInstance(Core::ClientInstance *instance);
    void setupPermanentConnections(Core::ClientInstance *instance);

private:
    void setupUi();
    void setupMenu();

    ChatView *chatView;
    ChatModel *chatModel;

    ChannelTreeView *channelTree;
    ChannelTreeModel *channelTreeModel;
    ChannelFilterProxyModel *channelFilterProxy;

    AccountsModel *accountsModel;

    MessageInput *messageInput;
    TypingIndicator *typingIndicator;
    Core::TypingTracker *typingTracker;

    AccountsWindow *accountsWindow = nullptr;

private slots:
    void openAccountsWindow();

private:
    Core::Session *session;
    Core::ClientInstance *currentInstance = nullptr;

    Core::Snowflake cachedGuildId = Core::Snowflake::Invalid;
    QHash<Core::Snowflake, QList<Discord::Role>> guildRolesCache;
    QHash<Core::Snowflake, QColor> userColorCache; // current guild
};

} // namespace UI
} // namespace Acheron