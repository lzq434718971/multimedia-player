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
    void initPlayList();//初始化播放列表
    void renewPlayListTxt();//更新列表txt文件
signals:
    void play();//启动信号
    void turnto(qreal i);//跳转信号
    void fastplay(qreal i);//倍数设置
    void itemRemove(); //文件列表项删除
    void playById(int i);//指定播放
    void volumeChange(qreal i);//音量改变
    void videoEnd();//当前视频播放完
    void permitBtn();//按钮可用信号
private slots:

    void on_addFileBtn_clicked();//添加文件

    void on_removeFileBtn_clicked();//删除文件

    void on_lastBtn_clicked();

    void on_nextBtn_clicked();

    void playNext();//播放下一首



    void on_hiddenBtn_clicked();

    void on_orderBtn_clicked();
    void setBtnTrue();

protected:
    void keyPressEvent(QKeyEvent *event) override; //键盘按下事件


private:
    Ui::MainWindow *ui;
    SDL_Window *window;
    SDL_Renderer *render;
    SDL_Texture *texture;
    PlayThread *playthread;
    QThread *thread;
    QTimer *timer;
    bool isFullScreen;//true全屏
    QStringList* myPlayList;//播放列表地址集合
    int indexPlay;//当前播放视频在列表的index
    qreal volumenum;
    bool ismute;//true静音
    qreal fastrate;//播放速度
    bool isListHide;//播放列表是否隐藏 true隐藏
    int order;//播放顺序 类型 0顺序 1单曲 2随机
    qreal durationCurrent;//当前视频时长
    bool isForbidBtn;//true说明禁用按钮 false说明按钮可用
};
#endif // MAINWINDOW_H
