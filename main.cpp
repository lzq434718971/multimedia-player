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


    MainWindow w;

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
