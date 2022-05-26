#include "videowidget.h"
#include <QApplication>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget{parent}
{
    timer=new QTimer(this);
    timer->setInterval(5000);
    connect(timer,&QTimer::timeout, this,[=](){
//        setCursor(Qt::BlankCursor);
        emit hideControlWidget();
    });
}

void VideoWidget::mouseMoveEvent(QMouseEvent*event){
    timer->start();
//    setCursor(QCursor(Qt::ArrowCursor));
    emit showControlWidget();
}



