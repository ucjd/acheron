#include "TypingIndicator.hpp"
#include <QHBoxLayout>
#include <QMovie>

namespace Acheron {
namespace UI {

TypingIndicator::TypingIndicator(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(16, 0, 16, 0);
    layout->setSpacing(0);

    // dots = new QLabel(this);
    // auto* movie = new QMovie(":/icons/typing.gif");
    // dots->setMovie(movie);
    // movie->start();
    // layout->addWidget(dots);

    label = new QLabel(this);
    label->setTextFormat(Qt::RichText);
    label->setStyleSheet("font-weight: bold; font-size: 12px;");
    layout->addWidget(label);

    layout->addStretch();

    setFixedHeight(sizeHint().height());
    label->setVisible(false);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void TypingIndicator::setRoleColorResolver(RoleColorResolver resolver)
{
    roleColorResolver = std::move(resolver);
}

void TypingIndicator::setTypers(const QList<Core::TyperInfo> &typers)
{
    if (typers.isEmpty()) {
        label->setVisible(false);
        return;
    }

    label->setText(formatText(typers));
    label->setVisible(true);
}

QString TypingIndicator::coloredName(const Core::TyperInfo &typer)
{
    QString escapedName = typer.name.toHtmlEscaped();

    if (roleColorResolver && typer.guildId.has_value()) {
        QColor color = roleColorResolver(typer.userId, typer.guildId.value());
        if (color.isValid())
            return QStringLiteral("<span style=\"color: %1\">%2</span>").arg(color.name(), escapedName);
    }

    return escapedName;
}

QString TypingIndicator::formatText(const QList<Core::TyperInfo> &typers)
{
    if (typers.isEmpty())
        return {};

    const int count = typers.size();

    if (count == 1)
        return coloredName(typers[0]) + QStringLiteral(" is typing");

    if (count == 2)
        return coloredName(typers[0]) + QStringLiteral(" and ") + coloredName(typers[1]) + QStringLiteral(" are typing");

    if (count == 3)
        return coloredName(typers[0]) + QStringLiteral(", ") + coloredName(typers[1]) + QStringLiteral(", and ") + coloredName(typers[2]) + QStringLiteral(" are typing");

    return QStringLiteral("Several people are typing");
}

} // namespace UI
} // namespace Acheron