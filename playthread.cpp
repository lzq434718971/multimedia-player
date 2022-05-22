#include "playthread.h"

PlayThread::PlayThread(QObject *parent){
    ffmpeg = new lzq::FFMpegMultimedia;
    playFlag = false;
    playState = false;
}
PlayThread::~PlayThread(){

    delete ffmpeg;
    delete output;
    delete ptimer;
}
void PlayThread::initdevice(){

    qDebug() <<"initdevice";

    format.setSampleRate(ffmpeg->getAudioSampleRate()*fast);
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelCount(ffmpeg->getChannelCount());

    output = new QAudioSink(format);

    device = output->start();
}

void PlayThread::initTimer(){

    ptimer = new QTimer();
    ptimer->setTimerType(Qt::PreciseTimer);
    ptimer->setInterval(ffmpeg->getFrameInterval()*1000/fast);
    ptimer->start();
}

void PlayThread::openFile(QString url){
    mutex.lock();
    if(playFlag)
    {
        ptimer->deleteLater();
        ptimer = nullptr;
    }

    playFlag = true;
    ffmpeg->open(url);
    fast = 1;
    initdevice();
    initTimer();
    mutex.unlock();
}

void PlayThread::dowork(){
    playState = true;
    connect(ptimer,&QTimer::timeout,this,[=]()
    {
        QImage img = ffmpeg->getImage();
        QByteArray pcm1 = ffmpeg->getPCM();
        ffmpeg->nextFrame();
//        qDebug() << ffmpeg->getCurrentTimeStamp();
        emit updateImage(img);
        while(true)
        {
            if(pcm1.size()<=output->bytesFree())
            {
                device->write(pcm1);
                pcm1.clear();
                break;
            }
            QThread::msleep(10);
        }

    });
}

void PlayThread::turnto(qreal i){
    if(i < 0){
        i = 0;
    }
    if(i > ffmpeg->getDurationInSeconds()){
        i = ffmpeg->getDurationInSeconds();
    }
    ffmpeg->seek(i);
    if(!playState){
        qDebug()<<"快进恢复播放";
        initTimer();
        dowork();
    }
}

void PlayThread::pause(){
    if(playState){
        ptimer->deleteLater();
        ptimer = nullptr;
        playState = false;
        qDebug()<<"ptimer1 stop";
    }else{
        initTimer();
        dowork();
        playState = true;
        qDebug()<<"restart";
    }
}

void PlayThread::close(){
    mutex.lock();
    ffmpeg->close();
    device->deleteLater();
    device = nullptr;
    ptimer->deleteLater();
    ptimer = nullptr;
    playFlag = false;
    playState = false;
    QImage img;
    emit updateImage(img);
    mutex.unlock();
}

void PlayThread::fastplay(qreal i){
    fast = i;
    device->deleteLater();
    device = nullptr;
    ptimer->deleteLater();
    ptimer = nullptr;
    initdevice();
    initTimer();
    dowork();
}

void PlayThread::fastforward(){
    qreal i = ffmpeg->getCurrentTimeStamp() + 5;
    turnto(i);
}

void PlayThread::fastback(){
    qreal i = ffmpeg->getCurrentTimeStamp() - 5;
    turnto(i);
}
