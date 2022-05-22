#include "videowidget.h"

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget{parent}
{

}

void VideoWidget::mouseMoveEvent(QMouseEvent*event){

}

void VideoWidget::enterEvent(QEnterEvent*event){
    qDebug()<<"鼠标移入";
    emit hideControlWidget();
}

void VideoWidget::leaveEvent(QEvent*event){
    qDebug()<<"鼠标移出";
    emit showControlWidget();
}

