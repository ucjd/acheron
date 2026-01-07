#include "ChatLayout.hpp"

namespace Acheron {
namespace UI {
namespace ChatLayout {

QRect dateSeparatorRectForRow(const QRect &rowRect)
{
    return QRect(rowRect.left(), rowRect.top(), rowRect.width(), separatorHeight());
}

QFont getFontForIndex(const QAbstractItemView *view, const QModelIndex &index)
{
    QFont font = view->font();
    QVariant modelFont = index.data(Qt::FontRole);
    if (modelFont.isValid() && !modelFont.isNull()) {
        font = qvariant_cast<QFont>(modelFont).resolve(font);
    }
    return font;
}

QRect avatarRectForRow(const QRect &rowRect, bool hasSeperator)
{
    int topOffset = hasSeperator ? separatorHeight() : 0;
    return QRect(rowRect.left() + padding(), rowRect.top() + padding() + topOffset, avatarSize(),
                 avatarSize());
}

QRect headerRectForRow(const QRect &rowRect, const QFontMetrics &fm, bool hasSeperator)
{
    int topOffset = hasSeperator ? separatorHeight() : 0;
    int left = rowRect.left() + padding() + avatarSize() + padding();
    int width = rowRect.right() - left - padding();
    return QRect(left, rowRect.top() + padding() + topOffset, width, fm.height());
}

QRect textRectForRow(const QRect &rowRect, bool showHeader, const QFontMetrics &fm,
                     bool hasSeperator)
{
    int topOffset = hasSeperator ? separatorHeight() : 0;

    int left = rowRect.left() + padding() + avatarSize() + padding();
    int width = rowRect.right() - left - padding();
    if (width < 10)
        width = 10;

    int top = rowRect.top() + topOffset;

    if (showHeader) {
        top += padding() + fm.height();
    } else {
        top += 1;
    }

    int height = rowRect.bottom() - top - padding() + 1;
    if (height < 0)
        height = 0;

    return QRect(left, top, width, height);
}

void setupDocument(QTextDocument &doc, const QString &htmlContent, const QFont &font, int textWidth)
{
    doc.setDefaultFont(font);
    doc.setHtml(htmlContent);
    doc.setTextWidth(textWidth);
    doc.setDocumentMargin(0);
    QTextOption opt = doc.defaultTextOption();
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    doc.setDefaultTextOption(opt);
}

int hitTestCharIndex(QAbstractItemView *view, const QModelIndex &index, const QPoint &viewportPos)
{
    if (!index.isValid() || !view)
        return -1;

    const bool showHeader = index.data(ChatModel::ShowHeaderRole).toBool();
    const QString content = index.data(ChatModel::ContentRole).toString();
    const QString html = index.data(ChatModel::HtmlRole).toString();
    const bool hasSeparator = index.data(ChatModel::DateSeparatorRole).toBool();

    QFont docFont = getFontForIndex(view, index);
    QFontMetrics fm(docFont);

    QRect rowRect = view->visualRect(index);
    QRect textRect = textRectForRow(rowRect, showHeader, fm, hasSeparator);

    QTextDocument doc;
    setupDocument(doc, html, docFont, textRect.width());

    QPointF local = viewportPos - textRect.topLeft();

    if (local.y() < 0 || local.y() > doc.size().height())
        return -1;

    return doc.documentLayout()->hitTest(local, Qt::ExactHit);
}

QRectF charRectInDocument(const QTextDocument &doc, int charIndex)
{
    if (charIndex < 0)
        return QRectF();
    QTextBlock block = doc.findBlock(charIndex);
    if (!block.isValid())
        return QRectF();

    int blockPos = block.position();
    int offset = charIndex - blockPos;
    QTextLayout *layout = block.layout();
    if (!layout)
        return QRectF();

    QTextLine line = layout->lineForTextPosition(offset);
    if (!line.isValid())
        return QRectF();

    qreal x1 = line.cursorToX(offset);
    qreal x2 = line.cursorToX(offset + 1);
    if (qFuzzyCompare(x1, x2))
        x2 = x1 + 6;
    qreal y = doc.documentLayout()->blockBoundingRect(block).top() + line.y();
    return QRectF(x1, y, x2 - x1, line.height());
}

QString getLinkAt(const QAbstractItemView *view, const QModelIndex &index, const QPoint &mousePos)
{
    if (!index.isValid() || !view)
        return {};

    QString html = index.data(ChatModel::HtmlRole).toString();
    if (html.isEmpty())
        return {};

    QRect rowRect = view->visualRect(index);

    bool showHeader = index.data(ChatModel::ShowHeaderRole).toBool();
    bool hasSeparator = index.data(ChatModel::DateSeparatorRole).toBool();
    QFont font = view->font();
    QFontMetrics fm(font);

    QRect textRect = textRectForRow(rowRect, showHeader, fm, hasSeparator);

    if (!textRect.contains(mousePos))
        return {};

    QTextDocument doc;
    setupDocument(doc, html, font, textRect.width());

    QPointF localPos = mousePos - textRect.topLeft();

    return doc.documentLayout()->anchorAt(localPos);
}

std::optional<AttachmentData> getAttachmentAt(const QAbstractItemView *view,
                                              const QModelIndex &index, const QPoint &mousePos)
{
    AttachmentData result;
    result.isLoading = false;

    if (!index.isValid() || !view)
        return std::nullopt;

    QList<AttachmentData> attachments =
            index.data(ChatModel::AttachmentsRole).value<QList<AttachmentData>>();

    if (attachments.isEmpty())
        return result;

    QRect rowRect = view->visualRect(index);
    bool showHeader = index.data(ChatModel::ShowHeaderRole).toBool();
    bool hasSeparator = index.data(ChatModel::DateSeparatorRole).toBool();
    QString html = index.data(ChatModel::HtmlRole).toString();
    QFont font = view->font();
    QFontMetrics fm(font);

    QRect textRect = textRectForRow(rowRect, showHeader, fm, hasSeparator);

    // calculate actual text height
    QTextDocument doc;
    setupDocument(doc, html, font, textRect.width());
    int realTextHeight = int(std::ceil(doc.size().height()));

    int attachmentTop = textRect.top() + realTextHeight + padding();

    for (const auto &att : attachments) {
        QRect imgRect(textRect.left(), attachmentTop, att.displaySize.width(),
                      att.displaySize.height());

        if (imgRect.contains(mousePos))
            return att;

        attachmentTop = imgRect.bottom() + padding();
    }

    return result;
}

} // namespace ChatLayout
} // namespace UI
} // namespace Acheron
