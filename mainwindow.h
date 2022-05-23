#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qpainter.h>
#include <QThread>
#include <QTimer>
#include <QKeyEvent>
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
    void itemRemove(); //文件列表项删除
    void playById(int i);//指定播放
    void volumeChange(qreal i);//音量改变
private slots:

    void on_addFileBtn_clicked();//添加文件

    void on_removeFileBtn_clicked();//删除文件

    void on_lastBtn_clicked();

    void on_nextBtn_clicked();



protected:
    void keyPressEvent(QKeyEvent *event); //键盘按下事件
    void keyReleaseEvent(QKeyEvent *event); //键盘松开事件

private:
    Ui::MainWindow *ui;
    SDL_Window *window;
    SDL_Renderer *render;
    SDL_Texture *texture;
    PlayThread *playthread;
    QThread *thread;
    QTimer *timer;
    bool isFullScreen;
    QStringList* myPlayList;//播放列表地址集合
    int indexPlay;//当前播放视频在列表的index
    qreal volumenum;
    bool ismute;
    qreal fastrate;//播放速度
};
#endif // MAINWINDOW_H
