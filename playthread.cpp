#include "playthread.h"

PlayThread::PlayThread(QObject *parent){
    ffmpeg = new lzq::FFMpegMultimedia;
    playFlag = false;
    playState=false;

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
    output->setVolume(0.2);
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
        ptimer=nullptr;
    }

    playFlag = true;
    ffmpeg->open(url);
    fast = 1;
    initdevice();
    initTimer();
    emit durationChanged(ffmpeg->getDurationInSeconds());
    mutex.unlock();
}

void PlayThread::dowork(){
    playState = true;
    connect(ptimer,&QTimer::timeout,this,[&]()
    {
            this->mutex.lock();
        QImage img = ffmpeg->getImage();
        QByteArray pcm1 = ffmpeg->getPCM();
        ffmpeg->nextFrame();
        this->mutex.unlock();
        emit updateImage(img);
        emit positionChanged(ffmpeg->getCurrentTimeStamp());
        while(true)
        {
            if(pcm1.size()<=output->bytesFree())
            {
                device->write(pcm1);
                break;
            }
            QThread::msleep(10);
        }
    });
    emit playSignal();
}

void PlayThread::turnto(qreal i){
    mutex.lock();
    if(i < 0){
        i = 0;
    }
    if(i > ffmpeg->getDurationInSeconds()){
        i = ffmpeg->getDurationInSeconds();
    }
    ffmpeg->seek(i);
    if(!playState){
        initTimer();
        dowork();
    }
    mutex.unlock();
}
//音量控制
void PlayThread::volumeControl(qreal i)
{
    output->setVolume(i);
    qDebug()<<"音量为"<<i;

}

void PlayThread::pause(){
    if(playState){
        ptimer->deleteLater();
        ptimer=nullptr;
        playState = false;
        qDebug()<<"ptimer1 stop";
        emit pauseSignal();
    }else{
        initTimer();
        dowork();
        playState = true;
        qDebug()<<"restart";
        emit playSignal();
    }
}

void PlayThread::close(){
     mutex.lock();
     ffmpeg->close();
     device->deleteLater();
     device=nullptr;
     ptimer->deleteLater();
     ptimer=nullptr;
     playFlag=false;
     playState=false;
     QImage img;
     emit updateImage(img);
     mutex.unlock();
}

void PlayThread::fastplay(qreal i){
    fast = i;
    device->deleteLater();
    device=nullptr;
    ptimer->deleteLater();
    ptimer=nullptr;
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
