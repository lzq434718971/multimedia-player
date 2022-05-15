#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "multimedia_decode_module/myopenglwidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->widget = findChild<MyOpenGLWidget*>("openGLWidget");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent* pev)
{
    //widget->test = test;
    //widget->repaint();
    QRectF target(0, 0, width(), height());
    QRectF source(0, 0, test.width(), test.height());

    QPainter painter(this);
    //painter.drawImage(QPoint(0, 0), test);
    painter.drawImage(target, test, source);
}

