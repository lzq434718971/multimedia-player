#include "ffmpegmultimedia.h"

namespace lzq {

FFMpegMultimedia::FFMpegMultimedia()
{
    _fmtCtx=avformat_alloc_context();

    _isAttached = false;
}

/**
 * 初始化一个流，会把初始化得到的各种变量赋给传入的引用中.
 **/
void commonStreamInit(AVFormatContext* context,AVMediaType streamType,
    int& streamID, AVCodecParameters*& codecPar, const AVCodec*& codec, AVCodecContext*& codecCtx, AVStream*& stream)
{
    //寻找视频流和音频流(本类仅支持单视频/音频流,解析第一个)
    streamID = -1;
    for (uint i = 0; i < context->nb_streams; i++)
    {
        if (context->streams[i]->codecpar->codec_type == streamType && streamID == -1)
        {
            streamID = i;
        }
        if (streamID != -1)
        {
            break;
        }
    }
    if (streamID == -1)
    {
        qCritical() << "没有找到流，类型需求:" << streamType;
        return;
    }
    else
    {
        //初始化AVCodecParam
        codecPar = context->streams[streamID]->codecpar;
    }

    codec = avcodec_find_decoder(codecPar->codec_id);

    codecCtx = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(codecCtx, codecPar);

    stream = context->streams[streamID];
}
void FFMpegMultimedia::open(QString path)
{
    //打开视频文件
    if(avformat_open_input(&_fmtCtx,path.toLocal8Bit(),nullptr,nullptr)!=0)
    {
        qCritical()<<"文件打开失败:"<<path;
        return;
    }

    //解析流信息
    if(avformat_find_stream_info(_fmtCtx,nullptr)!=0)
    {
        qCritical()<<"文件流解析失败:"<<path;
        return;
    }

    commonStreamInit(_fmtCtx, AVMEDIA_TYPE_VIDEO, _videoStreamId, _videoCodecPar, _videoCodec, _videoCodecCtx, _videoStream);
    commonStreamInit(_fmtCtx, AVMEDIA_TYPE_AUDIO, _audioStreamId, _audioCodecPar, _audioCodec, _audioCodecCtx, _audioStream);
    if(_videoStreamId == -1 && _audioStreamId == -1)
    {
        qCritical()<<"没有找到视频/音频流:"<<path;
        return;
    }
    else if(_videoStreamId == -1)
    {
        qInfo()<<"仅包含音频:"<<path;
    }
    else if(_audioStreamId == -1)
    {
        qInfo()<<"仅包含视频:"<<path;
    }
}

qreal FFMpegMultimedia::getDurationInSeconds()
{
    return 0;
}

void FFMpegMultimedia::seek(qreal timestamp)
{

}

void FFMpegMultimedia::nextFrame()
{

}

qreal FFMpegMultimedia::getCurrentTimeStamp()
{
    return 0;
}

QImage FFMpegMultimedia::getImage()
{
    return QImage();
}

QByteArray FFMpegMultimedia::getPCM()
{
    return QByteArray();
}

qreal FFMpegMultimedia::getFrameRate()
{
    return 0;
}

qreal FFMpegMultimedia::getFrameInterval()
{
    return 0;
}

} // namespace lzq
