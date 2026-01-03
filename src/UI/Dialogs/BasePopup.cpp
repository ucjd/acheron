#include "BasePopup.hpp"

namespace Acheron {
namespace UI {

BasePopup::BasePopup(QWidget *parent) : QDialog(parent, Qt::FramelessWindowHint | Qt::Dialog)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    auto *overlayLayout = new QVBoxLayout(this);
    overlayLayout->setAlignment(Qt::AlignCenter);
    overlayLayout->setContentsMargins(20, 20, 20, 20);

    m_container = new QFrame(this);
    m_container->setObjectName("ContentFrame");
    m_container->setAutoFillBackground(true);
    m_container->setFrameShape(QFrame::StyledPanel);
    m_container->setFrameShadow(QFrame::Raised);

    m_container->setMinimumWidth(300);
    m_container->setMaximumWidth(600);
    m_container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(25);
    shadow->setXOffset(0);
    shadow->setYOffset(8);
    shadow->setColor(QColor(0, 0, 0, 100));
    m_container->setGraphicsEffect(shadow);

    overlayLayout->addWidget(m_container);
}

void BasePopup::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 110));
}

void BasePopup::mousePressEvent(QMouseEvent *event)
{
    if (!m_container->geometry().contains(event->pos())) {
        reject();
    } else {
        QDialog::mousePressEvent(event);
    }
}

void BasePopup::showEvent(QShowEvent *event)
{
    if (parentWidget() && parentWidget()->window()) {
        QWidget *topLevel = parentWidget()->window();
        setGeometry(topLevel->geometry());
        topLevel->installEventFilter(this);
    }
    QDialog::showEvent(event);
}

bool BasePopup::eventFilter(QObject *obj, QEvent *event)
{
    if (parentWidget() && obj == parentWidget()->window()) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
            setGeometry(parentWidget()->window()->geometry());
        }
    }
    return QDialog::eventFilter(obj, event);
}

} // namespace UI
} // namespace Acheron
