#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QtMultimedia>
#include <QDebug>
#include "multimedia_decode_module/ffmpegmultimedia.h"

using namespace lzq;

QByteArray generatePCM()
{
    //幅度，因为sampleSize = 16bit
    qint16 amplitude = INT16_MAX;
    //单声道
    int channels = 1;
    //采样率
    int samplerate = 8000;
    //持续时间ms
    int duration = 20;
    //总样本数
    int n_samples = int(channels * samplerate * (duration / 1000.0));
    //声音频率
    int frequency = 100;

    bool reverse = false;
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    for (int i = 0; i < 1000; i++) {
        for (int j = 0; j < n_samples; j++) {
            qreal radians = qreal(2.0 * M_PI * j  * frequency / qreal(samplerate));
            qint16 sample = qint16(qSin(radians) * amplitude);
            out << sample;
        }

        if (!reverse) {
            if (frequency < 2000) {
                frequency += 100;
            } else reverse = true;
        } else {
            if (frequency > 100) {
                frequency -= 100;
            } else reverse = false;
        }
    }

    QFile file("raw");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    return data;
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    FFMpegMultimedia ffmpeg;

//    ffmpeg.open("video/1.mp4");

//    QByteArray pcm1;

//    QAudioFormat format;
//    format.setSampleRate(ffmpeg.getAudioSampleRate());
//    format.setSampleFormat(QAudioFormat::Int16);
//    format.setChannelCount(ffmpeg.getChannelCount());

//    QAudioSink output=QAudioSink(format);

//    QIODevice *device = output.start();

//    QThread thread;

//    QTimer *timer_play = new QTimer();
//    timer_play->setTimerType(Qt::PreciseTimer);
//    timer_play->setInterval(ffmpeg.getFrameInterval()*1000);
//    timer_play->start();
    //timer_play->moveToThread(&thread);

//    ffmpeg->moveToThread(thread);
//    thread.start();

    MainWindow w;
//    int i = 0;
//    ffmpeg.seek(0);
//    QMetaObject::Connection playConnect;
//    playConnect = QObject::connect(timer_play, &QTimer::timeout, [&]
//    {
//        i++;
//        //if (i == 1)
//        //{
//        //    qDebug() << "跳转到------------------------------------";
//        //    ffmpeg.seek(10);
//        //}
//        //if (i == 10)
//        //{
//        //    qDebug() << "跳转到------------------------------------";
//        //    ffmpeg.seek(10.99);
//        //}
//        //ffmpeg.seek(ffmpeg.getCurrentTimeStamp() - ffmpeg.getFrameInterval());
//        pcm1 = ffmpeg.getPCM();
//        w.test = ffmpeg.getImage();
//        //w.test.save(".\\images\\frame"+QString::number(i)+".jpg");
//        w.repaint();
//        ffmpeg.nextFrame();
//        int freeB = output.bytesFree();
//        //qDebug() << freeB;
//        //while (pcm1.size() > freeB)
//        //{
//        //    //qDebug() << freeB;
//        //    continue;
//        //}
//        device->write(pcm1);
//    });



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
