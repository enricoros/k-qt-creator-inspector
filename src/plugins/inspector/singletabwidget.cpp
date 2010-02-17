#include "singletabwidget.h"

#include "inspectorstyle.h"

#include <utils/stylehelper.h>

#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <QtGui/QMouseEvent>
#include <QtGui/QMenu>

#include <QtDebug>

namespace Inspector {
namespace Internal {

// keep this in sync with combotreewidget.cpp
static const int MIN_LEFT_MARGIN = 50;
static const int MARGIN = 12;
static const int OTHER_HEIGHT = 38;
static const int OVERFLOW_DROPDOWN_WIDTH = InspectorStyle::defaultBarHeight();

static void drawFirstLevelSeparator(QPainter *painter, QPoint top, QPoint bottom)
{
    QLinearGradient grad(top, bottom);
    grad.setColorAt(0, QColor(255, 255, 255, 20));
    grad.setColorAt(0.4, QColor(255, 255, 255, 60));
    grad.setColorAt(0.7, QColor(255, 255, 255, 50));
    grad.setColorAt(1, QColor(255, 255, 255, 40));
    painter->setPen(QPen(grad, 0));
    painter->drawLine(top, bottom);
    grad.setColorAt(0, QColor(0, 0, 0, 30));
    grad.setColorAt(0.4, QColor(0, 0, 0, 70));
    grad.setColorAt(0.7, QColor(0, 0, 0, 70));
    grad.setColorAt(1, QColor(0, 0, 0, 40));
    painter->setPen(QPen(grad, 0));
    painter->drawLine(top - QPoint(1,0), bottom - QPoint(1,0));
}

SingleTabWidget::SingleTabWidget(QWidget *parent) :
    QWidget(parent),
    m_currentIndex(-1),
    m_lastVisibleIndex(-1)
{
}

void SingleTabWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

QSize SingleTabWidget::minimumSizeHint() const
{
    return QSize(0, InspectorStyle::defaultBarHeight());
}

void SingleTabWidget::addTab(const QString &name)
{
    m_tabs.append(name);
    if (m_currentIndex == -1)
        setCurrentIndex(m_tabs.size() - 1);
    update();
}

void SingleTabWidget::insertTab(int index, const QString &name)
{
    m_tabs.insert(index, name);
    if (m_currentIndex == -1)
        setCurrentIndex(m_tabs.size() - 1);
    else if (m_currentIndex >= index)
        setCurrentIndex(m_currentIndex + 1);
    update();
}

void SingleTabWidget::removeTab(int index)
{
    m_tabs.removeAt(index);
    if (index <= m_currentIndex) {
        --m_currentIndex;
        if (m_currentIndex < 0 && m_tabs.size() > 0)
            m_currentIndex = 0;
        if (m_currentIndex < 0)
            emit currentIndexChanged(-1);
        else
            emit currentIndexChanged(m_currentIndex);
    }
    update();
}

int SingleTabWidget::tabCount() const
{
    return m_tabs.size();
}

void SingleTabWidget::setCurrentIndex(int newIndex)
{
    if (newIndex != m_currentIndex) {
        m_currentIndex = newIndex;
        update();
        emit currentIndexChanged(m_currentIndex);
    }
}

void SingleTabWidget::mousePressEvent(QMouseEvent *event)
{
    int eventX = event->x();
    // clicked on the top level part of the bar
    QFontMetrics fm(font());
    int x = 2 * MARGIN + qMax(fm.width(m_title), MIN_LEFT_MARGIN);
    if (eventX <= x)
        return;
    int i;
    for (i = 0; i <= m_lastVisibleIndex; ++i) {
        int otherX = x + 2 * MARGIN + fm.width(m_tabs.at(m_currentTabIndices.at(i)));
        if (eventX > x && eventX < otherX) {
            break;
        }
        x = otherX;
    }
    if (i <= m_lastVisibleIndex) {
        if (m_currentIndex != m_currentTabIndices.at(i))
            setCurrentIndex(m_currentTabIndices.at(i));
        event->accept();
        return;
    } else if (m_lastVisibleIndex < m_tabs.size() - 1) {
        // handle overflow menu
        if (eventX > x && eventX < x + OVERFLOW_DROPDOWN_WIDTH) {
            QMenu overflowMenu;
            QList<QAction *> actions;
            for (int i = m_lastVisibleIndex + 1; i < m_tabs.size(); ++i) {
                actions << overflowMenu.addAction(m_tabs.at(m_currentTabIndices.at(i)));
            }
            if (QAction *action = overflowMenu.exec(mapToGlobal(QPoint(x+1, 1)))) {
                int index = m_currentTabIndices.at(actions.indexOf(action) + m_lastVisibleIndex + 1);
                setCurrentIndex(index);
            }
        }
    }
    event->ignore();
}

void SingleTabWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QRect r = rect();

    // draw top level tab bar
    r.setHeight(InspectorStyle::defaultBarHeight());

    QPoint offset = window()->mapToGlobal(QPoint(0, 0)) - mapToGlobal(r.topLeft());
    QRect gradientSpan = QRect(offset, window()->size());
    Utils::StyleHelper::horizontalGradient(&painter, gradientSpan, r);

    painter.setPen(Utils::StyleHelper::borderColor());

    QColor lighter(255, 255, 255, 40);
    painter.drawLine(r.bottomLeft(), r.bottomRight());
    painter.setPen(lighter);
    painter.drawLine(r.topLeft(), r.topRight());

    QFontMetrics fm(font());
    int baseline = (r.height() + fm.ascent()) / 2 - 1;

    // top level title
    painter.setPen(Utils::StyleHelper::panelTextColor());
    painter.drawText(MARGIN, baseline, m_title);

    QLinearGradient grad(QPoint(0, 0), QPoint(0, r.height() + OTHER_HEIGHT - 1));
    grad.setColorAt(0, QColor(247, 247, 247));
    grad.setColorAt(1, QColor(205, 205, 205));

    // top level tabs
    int x = 2 * MARGIN + qMax(fm.width(m_title), MIN_LEFT_MARGIN);

    // calculate sizes
    QList<int> nameWidth;
    int width = x;
    int indexSmallerThanOverflow = -1;
    int indexSmallerThanWidth = -1;
    for (int i = 0; i < m_tabs.size(); ++i) {
        int w = fm.width(m_tabs.at(i));
        nameWidth << w;
        width += 2 * MARGIN + w;
        if (width < r.width())
            indexSmallerThanWidth = i;
        if (width < r.width() - OVERFLOW_DROPDOWN_WIDTH)
            indexSmallerThanOverflow = i;
    }
    m_lastVisibleIndex = -1;
    m_currentTabIndices.resize(m_tabs.size());
    if (indexSmallerThanWidth == m_tabs.size() - 1) {
        // => everything fits
        for (int i = 0; i < m_tabs.size(); ++i)
            m_currentTabIndices[i] = i;
        m_lastVisibleIndex = m_tabs.size()-1;
    } else {
        // => we need the overflow thingy
        if (m_currentIndex <= indexSmallerThanOverflow) {
            // easy going, simply draw everything that fits
            for (int i = 0; i < m_tabs.size(); ++i)
                m_currentTabIndices[i] = i;
            m_lastVisibleIndex = indexSmallerThanOverflow;
        } else {
            // now we need to put the current tab into
            // visible range. for that we need to find the place
            // to put it, so it fits
            width = x;
            int index = 0;
            bool handledCurrentIndex = false;
            for (int i = 0; i < m_tabs.size(); ++i) {
                if (index != m_currentIndex) {
                    if (!handledCurrentIndex) {
                        // check if enough room for current tab after this one
                        if (width + 2 * MARGIN + nameWidth.at(index)
                                + 2 * MARGIN + nameWidth.at(m_currentIndex)
                                < r.width() - OVERFLOW_DROPDOWN_WIDTH) {
                            m_currentTabIndices[i] = index;
                            ++index;
                            width += 2 * MARGIN + nameWidth.at(index);
                        } else {
                            m_currentTabIndices[i] = m_currentIndex;
                            handledCurrentIndex = true;
                            m_lastVisibleIndex = i;
                        }
                    } else {
                        m_currentTabIndices[i] = index;
                        ++index;
                    }
                } else {
                    ++index;
                    --i;
                }
            }
        }
    }

    // actually draw top level tabs
    for (int i = 0; i <= m_lastVisibleIndex; ++i) {
        int actualIndex = m_currentTabIndices.at(i);
        QString tabName = m_tabs.at(actualIndex);
        if (actualIndex == m_currentIndex) {
            QColor bgColor = actualIndex ? InspectorStyle::invertedBackColor() : palette().color(QPalette::Window);
            painter.setPen(Utils::StyleHelper::borderColor());
            painter.drawLine(x - 1, 0, x - 1, r.height() - 1);
            painter.fillRect(QRect(x, 0,
                                   2 * MARGIN + fm.width(tabName),
                                   r.height() + 1),
                             bgColor);
            x += MARGIN;
            QColor textColor = actualIndex ? InspectorStyle::invertedTextColor() : Qt::black;
            painter.setPen(textColor);
            painter.drawText(x, baseline, tabName);
            x += nameWidth.at(actualIndex);
            x += MARGIN;
            painter.setPen(Utils::StyleHelper::borderColor());
            painter.drawLine(x, 0, x, r.height() - 1);
        } else {
            if (i == 0)
                drawFirstLevelSeparator(&painter, QPoint(x, 0), QPoint(x, r.height()-1));
            x += MARGIN;
            painter.setPen(Utils::StyleHelper::panelTextColor());
            painter.drawText(x + 1, baseline, tabName);
            x += nameWidth.at(actualIndex);
            x += MARGIN;
            drawFirstLevelSeparator(&painter, QPoint(x, 0), QPoint(x, r.height()-1));
        }
    }
}

} // namespace Internal
} // namespace Inspector
