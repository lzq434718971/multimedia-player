#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
    QRectF target(10.0, 20.0, 710.0, 420.0);
    QRectF source(300.0, 200.0, 700.0, 400.0);

    QPainter painter(this);
    //painter.drawImage(QPoint(0, 0), test);
    painter.drawImage(target, test, source);
}

