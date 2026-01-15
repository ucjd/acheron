#pragma once

#include <QSortFilterProxyModel>

#include "Core/Session.hpp"
#include "Core/Snowflake.hpp"

namespace Acheron {
namespace UI {

class ChannelTreeModel;

class ChannelFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ChannelFilterProxyModel(Core::Session *session, QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

public slots:
    void invalidateFilter();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    bool hasVisibleChildren(const QModelIndex &parent) const;
    Core::Snowflake getUserIdForNode(const QModelIndex &index) const;

    Core::Session *session;
    ChannelTreeModel *channelModel;
};

} // namespace UI
} // namespace Acheron
