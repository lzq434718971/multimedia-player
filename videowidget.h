#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H


#include <QMouseEvent>
#include <QEvent>
#include <QEnterEvent>
#include <QWidget>
#include <QTimer>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);


private:
    QTimer*timer;

signals:
    void showControlWidget();
    void hideControlWidget();

protected:
    void mouseMoveEvent(QMouseEvent*event);

};

#endif // VIDEOWIDGET_H
