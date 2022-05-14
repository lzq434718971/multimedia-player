#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QtMultimedia>
#include <QDebug>
#include "multimedia_decode_module/bufferblock.h"
#include "multimedia_decode_module/ffmpegmultimedia.h"

using namespace lzq;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FFMpegMultimedia ffmpeg;

    ffmpeg.open("video/Final.Fantasy.VII.Advent.Children.2005.1080p.BrRip.x264.BOKUTOX.YIFY.mp4");

    QByteArray pcm1;

    QAudioFormat format;
    format.setSampleRate(ffmpeg.getAudioSampleRate());
    format.setSampleFormat(QAudioFormat::Int16);
    format.setChannelCount(ffmpeg.getChannelCount());
    qDebug() << "每帧字节数:" << ffmpeg.getAudioBytePerSample();

    QAudioSink output=QAudioSink(format);

    QIODevice *device = output.start();

    QThread thread;

    QTimer *timer_play = new QTimer();
    timer_play->setTimerType(Qt::PreciseTimer);
    timer_play->setInterval(ffmpeg.getFrameInterval()*1000);
    timer_play->start();
    //timer_play->moveToThread(&thread);
    

    MainWindow w;

    

    int i = 0;
    //ffmpeg.seek(75);
    QMetaObject::Connection playConnect;
    playConnect = QObject::connect(timer_play, &QTimer::timeout, [&]
    {
        i++;
        //qDebug() << "第" << i << "帧--------------";
        //if (i % 1 == 0)
        //{
        //    qDebug() << "跳转到------------------------------------";
        //    //ffmpeg.seek(12);
        //    ffmpeg.readPacket();
        //    ffmpeg.pushImageQueue(ffmpeg._frame);
        //}
        if (i % 100 == 0)
        {
            qDebug() << "跳转到------------------------------------";
            ffmpeg.seek(i / 10);
            //ffmpeg.seek(10);
        }
        //ffmpeg.seek(ffmpeg.getCurrentTimeStamp() - ffmpeg.getFrameInterval());
        pcm1 = ffmpeg.getPCM();
        w.test = ffmpeg.getImage();
        ////w.test.save(".\\images\\frame"+QString::number(i)+".jpg");
        w.repaint();
        ffmpeg.nextFrame();
        while (true)
        {
            int freeB = output.bytesFree();
            //int actualWrite = device->write(pcm1);
            if (freeB < pcm1.size())
            {
                QThread::msleep(10);
                continue;
            }
            device->write(pcm1);
            break;
        }
        
        //qDebug() << "是否完整写入?:" << (device->write(pcm1) == pcm1.size());
        //while (pcm1.size() > freeB)
        //{
        //    //qDebug() << freeB;
        //    continue;
        //}
    });

    //thread.start();

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MultimediaPlayer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    w.show();

    return a.exec();
}
