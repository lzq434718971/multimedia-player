#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H


#include <QMouseEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QWidget>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

signals:
    void showControlWidget();
    void hideControlWidget();

protected:
    void mouseMoveEvent(QMouseEvent*event);
    void enterEvent(QEnterEvent*event);
    void leaveEvent(QEvent*event);
};

#endif // VIDEOWIDGET_H
