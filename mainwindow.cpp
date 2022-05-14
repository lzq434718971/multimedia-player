#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "multimedia_decode_module/myopenglwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent* pev)
{
    MyOpenGLWidget* widget = findChild<MyOpenGLWidget*>("openGLWidget");
    widget->test = test;
    widget->repaint();
//    QRectF target(0, 0, width(), height());
//    QRectF source(0, 0, test.width(), test.height());

//    QPainter painter(this);
//    //painter.drawImage(QPoint(0, 0), test);
//    painter.drawImage(target, test, source);
}

