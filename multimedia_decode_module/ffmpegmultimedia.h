#ifndef LZQ_FFMPEGMULTIMEDIA_H
#define LZQ_FFMPEGMULTIMEDIA_H

#include <QtDebug>
#include "multimediafile.h""

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avio.h"
}

namespace lzq {

/**
 * @brief 利用ffmpeg实现多媒体文件解析，仅支持解析单视频+音频流
 */
class FFMpegMultimedia:public MultimediaFile
{
private:
    AVFormatContext* _fmtCtx;

    AVCodecParameters* _videoCodecPar;
    AVCodecParameters* _audioCodecPar;

    const AVCodec* _videoCodec;
    const AVCodec* _audioCodec;

    AVCodecContext* _videoCodecCtx;
    AVCodecContext* _audioCodecCtx;

    AVStream* _videoStream;
    AVStream* _audioStream;

    int _videoStreamId;
    int _audioStreamId;

    bool _isAttached;

public:
    FFMpegMultimedia();

    void virtual open(QString path) override;
    qreal virtual getDurationInSeconds() override;
    void virtual seek(qreal timestamp) override;
    void virtual nextFrame() override;
    qreal virtual getCurrentTimeStamp() override;
    QImage virtual getImage() override;
    QByteArray virtual getPCM() override;
    qreal virtual getFrameRate() override;
    qreal virtual getFrameInterval() override;
};

} // namespace lzq

#endif // LZQ_FFMPEGMULTIMEDIA_H
