#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H
#include "multimedia_decode_module/ffmpegmultimedia.h"
#include <QMutex>
class PlayThread:public QObject
{
    Q_OBJECT
public:
    explicit PlayThread(QObject *parent = nullptr);
    ~PlayThread();

    void openFile(QString url);//打开视频文件
    void initdevice();//初始化设备
    void initTimer();//初始化计时器

    void fastplay(qreal i);

    QMutex mutex;
public slots:
    void dowork();//启动
    void turnto(qreal i = 0);//跳转到指定秒数
    void pause();//播放或暂停
    void fastforward();
    void fastback();
    void close();//停止
    void volumeControl(qreal i);
signals:
    void updateImage(QImage img);//更新Image
    void positionChanged(qreal i);//播放进度变化
    void durationChanged(qreal i);//视频时长发生变化
    void pauseSignal();//暂停信号
    void playSignal();//播放信号
private:
    lzq::FFMpegMultimedia *ffmpeg;
    QTimer *ptimer;
    QAudioSink *output;
    QAudioFormat format;
    bool playFlag;//播放启动
    bool playState;//播放状态
    qreal fast;
    qreal volume;//音量
public:
    QIODevice *device;
};

#endif // PLAYTHREAD_H
