#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qpainter.h>
#include <QThread>
#include <QTimer>
#include "multimedia_decode_module/ffmpegmultimedia.h"
#include "playthread.h"
extern "C"{
#include"SDL.h"
}
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QImage test;
    QByteArray pcm;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent* pev) override;
    void playUrl(QString url);//播放指定地址文件
signals:
    void play();//启动信号
    void turnto(qreal i);//跳转信号
    void fastplay(qreal i);//倍数设置
private slots:


private:
    Ui::MainWindow *ui;
    SDL_Window *window;
    SDL_Renderer *render;
    SDL_Texture *texture;
    PlayThread *playthread;
    QThread *thread;
    QTimer *timer;
};
#endif // MAINWINDOW_H
