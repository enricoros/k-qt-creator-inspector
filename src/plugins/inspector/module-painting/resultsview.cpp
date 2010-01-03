/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009-2010 Enrico Ros
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "resultsview.h"
#include "paintingmodel.h"
#include <QPainter>

using namespace Inspector::Internal;

void TemperatureResultsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // get the TemperatureItem
    const PaintingModel *model = static_cast<const PaintingModel *>(index.model());
    const TemperatureItem *item = model->result(index.row());
    if (!item) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // draw the TemperatureItem
    const QRect rect = option.rect;
    const int rt = rect.top();
    const int rl = rect.left();
    const int textRectHeight = (rect.height() - 10) / 3;

    // selection
    QColor textColor = option.palette.color(QPalette::Text);
    if (option.state & QStyle::QStyle::State_Selected) {
        painter->fillRect(rect, option.palette.color(QPalette::Highlight));
        if (!(option.state & QStyle::State_MouseOver))
            textColor = option.palette.color(QPalette::HighlightedText);
    } else if (option.state & QStyle::State_MouseOver) {
        QColor color = option.palette.color(QPalette::Highlight);
        color.setAlpha(color.alpha() / 4);
        painter->fillRect(rect, color);
    }
    QColor subtleTextColor = textColor;
    subtleTextColor.setAlpha(subtleTextColor.alpha() / 2);

    // preview pixmap
    const QPixmap preview = item->previewImage();
    painter->drawPixmap(rl + (88 - preview.width()) / 2, rt + (rect.height() - preview.height()) / 2, preview);

    // text: date + duration
    QFont normalFont = option.font;
    QFont smallFont = normalFont;
    smallFont.setPointSize(smallFont.pointSize() - 1);
    painter->setFont(normalFont);
    painter->setPen(textColor);
    painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->date().toString());
    int minutes = (int)(item->duration() / 60.0);
    int seconds = (int)(item->duration() - (minutes * 60));
    QString timeString = tr("%1' %2'' ").arg(minutes).arg(seconds);
    painter->setFont(smallFont);
    painter->drawText(QRect(rl + 88, rt + 5, rect.width() - 88, textRectHeight), Qt::AlignVCenter | Qt::AlignRight, timeString);

    // text: description
    painter->setFont(normalFont);
    painter->drawText(QRect(rl + 88, rt + 5 + textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->description());

    // text: options
    painter->setFont(smallFont);
    painter->setPen(subtleTextColor);
    painter->drawText(QRect(rl + 88, rt + 5 + 2 * textRectHeight, rect.width() - 88, textRectHeight), Qt::AlignVCenter, item->options());
}

QSize TemperatureResultsDelegate::sizeHint(const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    return QSize(TemperatureItem::previewWidth + 150, TemperatureItem::previewHeight + 8);
}


ResultsView::ResultsView(QWidget *parent)
  : QListView(parent)
  , m_delegate(new TemperatureResultsDelegate)
{
    // custom delegate for drawing TemperatureItems
    setItemDelegate(m_delegate);
}

ResultsView::~ResultsView()
{
    delete m_delegate;
}
