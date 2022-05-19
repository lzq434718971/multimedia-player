#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QtMultimedia>
#include <QDebug>
#include "multimedia_decode_module/ffmpegmultimedia.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //---------SDL----------
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    window = SDL_CreateWindowFrom((void*)ui->SDLwidget->winId());
    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(render,SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,ui->SDLwidget->width(), ui->SDLwidget->height());

    playthread = new PlayThread;
    thread = new QThread;
    playthread->moveToThread(thread);
    thread->start();

    //启动信号
    connect(this,&MainWindow::play,playthread,&PlayThread::dowork);
    //跳转信号
    connect(this,&MainWindow::turnto,playthread,&PlayThread::turnto);
    //跳转测试按钮
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::on_pushButton_clicked);
    //播放1视频测试按钮
    connect(ui->pushButton_2,&QPushButton::clicked,this,[=](){
        playthread->openFile("video/1.mp4");
        emit play();
    });
    //播放2视频测试按钮
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=](){
        playthread->openFile("E:/test/2.mp4");
        emit play();
    });
    //跳转测试按钮
    connect(ui->pauseButton,&QPushButton::clicked,playthread,&PlayThread::pause);

    connect(ui->fastplay,&QPushButton::clicked,this,[=]()
    {
        emit fastplay(2);
    });
    connect(this,&MainWindow::fastplay,playthread,&PlayThread::fastplay);

    //播放界面,每秒100帧
    connect(playthread,&PlayThread::updateImage,this,[=](QImage img)
    {
        test = img;
    });
    timer = new QTimer();
    timer->setInterval(10);
    connect(timer,SIGNAL(timeout()),this,SLOT(repaint()));
    timer->start();

}

MainWindow::~MainWindow()
{
    thread->quit();
    thread->wait();
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent* pev)
{
    ui->SDLwidget->setUpdatesEnabled(false);
    if(!render){
        qDebug() <<"render not create";
    }else{
        int texture_w=0, texture_h=0;
        SDL_QueryTexture(texture,NULL,NULL,&texture_w,&texture_h);
        if(texture_w != test.width() || texture_h != test.height()){
            if(texture){
                SDL_DestroyTexture(texture);
            }
            texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, test.width(), test.height());
        }
    }
    SDL_UpdateTexture(texture,NULL,test.bits(),test.bytesPerLine());
    SDL_RenderClear(render);
    SDL_RenderCopy(render,texture,NULL,NULL);
    SDL_RenderPresent(render);

}


void MainWindow::on_pushButton_clicked()
{
    emit turnto(120);
}


void MainWindow::on_pushButton_2_clicked()
{

}


void MainWindow::on_pushButton_3_clicked()
{

}


void MainWindow::on_pauseButton_clicked()
{

}

