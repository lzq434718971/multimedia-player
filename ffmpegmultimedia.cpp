#include "ffmpegmultimedia.h"

namespace lzq {

FFMpegMultimedia::FFMpegMultimedia()
{
    _fmtCtx=avformat_alloc_context();

    _isAttached = false;
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

    //寻找视频流和音频流(本类仅支持单视频/音频流,解析第一个)
    _videoStreamId=_audioStreamId=-1;
    for(uint i=0;i<_fmtCtx->nb_streams;i++)
    {
        if(_fmtCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO && _videoStreamId==-1)
        {
            _videoStreamId=i;
        }
        if(_fmtCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO && _audioStreamId==-1)
        {
            _audioStreamId=i;
        }
        if(_videoStreamId != -1 && _audioStreamId != -1)
        {
            break;
        }
    }
    if(_videoStreamId == -1 && _audioStreamId == -1)
    {
        qCritical()<<"没有找到视频/音频流:"<<path;
        return;
    }
    else if(_videoStreamId == -1)
    {
        qInfo()<<"仅包含音频:"<<path;
        _audioCodecPar=_fmtCtx->streams[_audioStreamId]->codecpar;
    }
    else if(_audioStreamId == -1)
    {
        qInfo()<<"仅包含视频:"<<path;
        _videoCodecPar=_fmtCtx->streams[_videoStreamId]->codecpar;
    }
    else
    {
        _videoCodecPar=_fmtCtx->streams[_videoStreamId]->codecpar;
        _audioCodecPar=_fmtCtx->streams[_audioStreamId]->codecpar;
    }


}

} // namespace lzq
