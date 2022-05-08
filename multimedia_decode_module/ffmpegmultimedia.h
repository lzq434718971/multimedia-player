#ifndef LZQ_FFMPEGMULTIMEDIA_H
#define LZQ_FFMPEGMULTIMEDIA_H

#include <QtDebug>
#include <QtMultimedia>
#include <vector>
#include <list>
#include <map>
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
#include "libswresample/swresample.h"
}

using namespace std;

namespace lzq {

/**
 * @brief 利用ffmpeg实现多媒体文件解析，仅支持解析单视频+音频流，以视频的帧作为分隔单位.
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

    AVPacket* _videoPacket;
    AVFrame* _videoFrame;

    SwsContext* _imgConvert;

    int _videoStreamId;
    int _audioStreamId;

    bool _isAttached;

    //当前的pts(自动对齐到音频pts)
    qint64 _pts;

    QByteArray _totalAudio;

    unsigned int _audio_bit_per_sample = 16;

    map<qint64, qint64> _ptsToFrame;

    //缓存关键帧(及对应的小段序列)的数量(当前关键帧所在的段默认会缓存)
    //向前缓存数
    int _pKFBufferNum;
    //向后缓存数
    int _lKFBufferNum;
    list<list<AVPacket*>> _KFBuffer;

    //缓存图像的数量
    //向前缓存数
    int _pBufferNum;
    //向后缓存数
    int _lBufferNum;
    list<QImage> _buffer;

    QImage _testBuffer;


    void initBuffer();
    //跳转到指定帧，并设置好解码器上下文
    void seekAndSetCodecCtx(int64_t pts);
    //解析到_pts后最近的一帧
    void readPacket();
    inline qint64 realTimeToPts(qreal timestamp, AVRational base);
    inline qreal ptsToRealTime(qint64 pts, AVRational base);
    qint64 ptsToFrame(qint64 pts);
    inline qint64 realPtsToFrame(qint64 pts);
    //读取一个最近的streamId流中的packet，如果到流结尾还没有找到则返回-1(可能包含不能成功解码的packet)
    int readAValidPacketTo(AVPacket*& target, int streamId);
    //解码packet到target中，可能会失败.
    int decodeImagePacketTo(QImage& target, AVPacket* packet);
    void readTotalAudio();
    void recorePtsToFrameSet(AVPacket* pkt,qint64 index);
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

    qint32 virtual getAudioSampleRate();
    qint32 virtual getChannelCount();
    qint32 virtual getAudioBytePerSample();
};

} // namespace lzq

#endif // LZQ_FFMPEGMULTIMEDIA_H
