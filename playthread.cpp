#include "playthread.h"

PlayThread::PlayThread(QObject *parent){
    ffmpeg = new lzq::FFMpegMultimedia;
    playFlag = false;

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
    if(playFlag)
    {
        ptimer->deleteLater();
    }

    playFlag = true;
    ffmpeg->open(url);
    ffmpeg->seek(0);
    fast = 1;
    initdevice();
    initTimer();
}

void PlayThread::dowork(){
    playState = true;
    connect(ptimer,&QTimer::timeout,this,[=]()
    {
        QImage img = ffmpeg->getImage();
        QByteArray pcm1 = ffmpeg->getPCM();
        ffmpeg->nextFrame();
        emit updateImage(img);
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
        initTimer();
        dowork();
    }
}

void PlayThread::pause(){
    if(playState){
        ptimer->deleteLater();
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
    ffmpeg->close();
}

void PlayThread::fastplay(qreal i){
    fast = i;
    device->deleteLater();
    ptimer->deleteLater();
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
