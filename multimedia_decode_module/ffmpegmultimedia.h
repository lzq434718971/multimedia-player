#ifndef LZQ_FFMPEGMULTIMEDIA_H
#define LZQ_FFMPEGMULTIMEDIA_H

#include <QtDebug>
#include <QtMultimedia>
#include <vector>
#include <queue>
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

    //暂被废弃
    //AVPacket* _videoPacket;
    //AVFrame* _videoFrame;

    //解析音/视频统一用的载体
    AVPacket* _packet;
    AVFrame* _frame;

    SwsContext* _imgConvert;
    SwrContext* _audioConvert;

    int _videoStreamId;
    int _audioStreamId;

    bool _isAttached;

    //当前的pts(自动对齐到音频pts)
    qint64 _pts;
    //因为出现了_pts通过累加帧间隔来计算会导致误差越来越大的问题（导致跳帧）
    //因此改用累计帧数的做法，并通过帧数计算_pts
    qint64 _frameCount;

    QByteArray _totalAudio;

    unsigned int _audio_bit_per_sample = 16;

    map<qint64, qint64> _ptsToFrame;

    //缓存关键帧(及对应的小段序列)的数量(当前关键帧所在的段默认会缓存)
    //向前缓存数
    int _pKFBufferNum;
    //向后缓存数
    int _lKFBufferNum;

    //缓存图像的数量,缓存中的图像的时间戳是连续的，list中靠前的帧时间戳较小
    //这样方便通过一个给定的目标时间戳判断目标帧是否在缓存中，也可以仅通过给定时间戳找到要求的帧
    int _iBufferNum;
    list<AVFrame*> _imageBuffer;
    //记录当前缓存中图像帧pts的下界，处理在刚跳转时第一帧的pts可能在pts之后的情况.
    qint64 _bufferPtsBottom;

    //缓存少量音频，会根据当前pts释放无用的数据.
    QByteArray _tempPCM;
    //记录当前剩余音频数据的头部pts.
    qint64 _tempPCMHead;

    //队列中会存放:如果正常向前逐帧播放，将会访问到的帧.
    //将要播放的图像队列
    list<AVFrame*> _imageQueue;
    //将要播放的音频队列
    list<AVFrame*> _audioQueue;

    //AVFrame池，避免频繁分配AVFrame，pool中的frame从前面出后面进.
    list<AVFrame*> _framePool;

    QImage _testBuffer;


    void initBuffer();

    /**
     *  简单地读取一帧到_frame中,会使_fmtCtx状态发生变化(移动一帧),只适用于一个包对应一帧的情况.
     **/
    int readPacket();

    /**
     * 结合队列，找到下一帧图像.
     **/
    AVFrame* popNextImageFrame();

    /**
     * 结合队列，找到下一帧音频.
     **/
    AVFrame* popNextAudioFrame();

    /**
     * 操作视频播放队列(_imageQueue).
     **/
    //会复制传入的帧保存在队列中.
    void pushImageQueue(AVFrame* frame);
    AVFrame* popImageQueue();

    /**
     * 操作音频播放队列(_audioQueue).
     **/
     //会复制传入的帧保存在队列中.
    void pushAudioQueue(AVFrame* frame);
    AVFrame* popAudioQueue();

    void pushFrameQueue(list<AVFrame*>& queue, AVFrame* frame);
    AVFrame* popFrameQueue(list<AVFrame*>& queue);

    /**
     * 添加一帧到缓存中，会产生复制.
     **/
    void addImageBuffer(AVFrame* frame);

    /**
     * 移除buffer最前端的frame(index最小).
     **/
    void removeFrontImageBuffer();

    /**
     * 将imageBuffer中的所有内容移到_framePool中
     **/
    void freeImageBuffer();

    /**
     * 清空音频缓存,清除tempPCM中的内容
     **/
    void freeAudioBuffer();

    /**
     * 将待播放的音视频队列(_imageQueue,_audioQueue)都清空.
     **/
    void freeQueue(list<AVFrame*>& queue);

    /**
     * 将tempPCM中，已经播过的数据(小于pts的)数据段剔除.
     **/
    void chopTempPCMBeforePts();

    /**
     * 读入一定数量的audioPacket，使其可以构成一帧需要的声音.
     **/
    void readLeastAudioPacketForOneFrame();

    /**
     * 输出音频时用到的函数，通过给定pts获取在音频数组中的具体索引.
     **/
    int ptsToByteIndex(qint64 pts);

    /**
     * 输出音频时用到的函数，通过给定时间获取在音频数组中的具体索引.
     **/
    int realTimeToByteIndex(qreal sec);

    /**
     * 在图像缓存中查找是否有对应的帧,如果有，则返回该帧，否则返回nullptr.
     **/
    AVFrame* findFrameInImageBuffer(qint64 pts);

    /**
     * 在确定图像缓存中有对应的帧后，找到最接近的一帧返回，该函数一定会返回一个AVFrame.
     **/
    AVFrame* findNearestImageBuffer(qint64 pts);

    /**
     * 初始化一个流，会把初始化得到的各种变量赋给传入的引用中.
     **/
    void commonStreamInit(AVFormatContext* context, AVMediaType streamType,
        int& streamID, AVCodecParameters*& codecPar, const AVCodec*& codec, AVCodecContext*& codecCtx, AVStream*& stream);

    //跳转到指定帧，并设置好解码器上下文
    void seekAndSetCodecCtx(int64_t pts);

    //解析到_pts后最近的一帧
    void readVideoPacket();

    inline qint64 realTimeToPts(qreal timestamp, AVRational base);
    inline qreal ptsToRealTime(qint64 pts, AVRational base);
    qint64 ptsToFrame(qint64 pts);
    inline qint64 realPtsToFrame(qint64 pts);
    qint64 videoPtsToAudioPts(qint64 pts);
    qint64 audioPtsToVideoPts(qint64 pts);

    //根据当前打开的上下文格式,通过AVFrame获取一个QImage
    QImage AVFrameToQImage(AVFrame* frame);

    //根据当前打开的上下文格式,通过AVFrame获取一段PCM数据
    QByteArray AVFrameToPCM(AVFrame* frame);

    //读取一个最近的streamId流中的packet，如果到流结尾还没有找到则返回-1(可能包含不能成功解码的packet)
    int readAValidPacketTo(AVPacket*& target, int streamId);
    //解码packet到target中，可能会失败.
    //int decodeImagePacketTo(QImage& target, AVPacket* packet);
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

    /**
     * 返回每个采样占用的字节数（考虑了通道数）;
     **/
    qint32 virtual getAudioBytePerSample();

    ///**
    // * 操作视频播放队列(_imageQueue).
    // **/
    // //会复制传入的帧保存在队列中.
    //void pushImageQueue(AVFrame* frame);
    //AVFrame* popImageQueue();

    //void testFreeFrame(AVFrame* frame);

    ///**
    // *  简单地读取一帧到_frame中,会使_fmtCtx状态发生变化(移动一帧),只适用于一个包对应一帧的情况.
    // **/
    //int readPacket();

    ////解析音/视频统一用的载体
    //AVPacket* _packet;
    //AVFrame* _frame;
};

} // namespace lzq

#endif // LZQ_FFMPEGMULTIMEDIA_H
