#ifndef SINGLETABWIDGET_H
#define SINGLETABWIDGET_H

#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QWidget>

namespace Inspector {
namespace Internal {

class SingleTabWidget : public QWidget {
    Q_OBJECT
public:
    SingleTabWidget(QWidget *parent = 0);

    void setTitle(const QString &title);
    QString title() const { return m_title; }

    void addTab(const QString &name);
    void insertTab(int index, const QString &name);
    void removeTab(int index);
    int tabCount() const;

    void setCurrentIndex(int index);

signals:
    void currentIndexChanged(int index);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    QSize minimumSizeHint() const;

private:
    QString m_title;
    QStringList m_tabs;
    int m_currentIndex;
    QVector<int> m_currentTabIndices;
    int m_lastVisibleIndex;
};

} // namespace Internal
} // namespace Inspector

#endif // SINGLETABWIDGET_H
