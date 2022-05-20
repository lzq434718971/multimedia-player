#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H
#include "multimedia_decode_module/ffmpegmultimedia.h"
class PlayThread:public QObject
{
    Q_OBJECT
public:
    explicit PlayThread(QObject *parent = nullptr);
    ~PlayThread();

    void openFile(QString url);//打开视频文件
    void initdevice();//初始化设备
    void initTimer();//初始化计时器
    void close();//停止
    void fastplay(qreal i);
public slots:
    void dowork();//启动
    void turnto(qreal i = 0);//跳转到指定秒数
    void pause();//播放或暂停
    void fastforward();
    void fastback();
signals:
    void updateImage(QImage img);//更新Image
private:
    lzq::FFMpegMultimedia *ffmpeg;
    QTimer *ptimer;
    QAudioSink *output;
    QAudioFormat format;
    bool playFlag;//播放启动
    bool playState;//播放状态
    qreal fast=1;
public:
    QIODevice *device;
};

#endif // PLAYTHREAD_H
