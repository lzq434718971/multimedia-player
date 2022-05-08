#include "ffmpegmultimedia.h"

namespace lzq {

void FFMpegMultimedia::initBuffer()
{
    
}

void FFMpegMultimedia::seekAndSetCodecCtx(int64_t pts)
{
    //av_seek_frame(_fmtCtx, _videoStreamId, ptsToFrame(pts),AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    //avcodec_flush_buffers(_videoCodecCtx);
    //av_read_frame(_fmtCtx, _videoPacket);
    //avcodec_send_packet(_videoCodecCtx, _videoPacket);
    //avcodec_receive_frame(_videoCodecCtx, _videoFrame);
}

void FFMpegMultimedia::readPacket()
{
    //qDebug() << "当前pts:" << _pts;
    //int targetFrame = realPtsToFrame(_pts);
    //int currentFrame = ptsToFrame(_videoPacket->pts);
    ////qDebug() << "当前帧:" << currentFrame << ";目标帧:" << targetFrame;
    if (_videoPacket->pts > _pts)
    {
        return;
    }
    av_packet_unref(_videoPacket);
    
    //一直找到能解码的包或者到达结尾才循环结束.
    while (true)
    {
        int msg;
        msg = readAValidPacketTo(_videoPacket, _videoStreamId);
        if (msg < 0)
        {
            qDebug() << "视频流结尾.";
            av_packet_unref(_videoPacket);
            break;
        }

        //currentFrame = ptsToFrame(_videoPacket->pts);

        // 如果解码到了目标位置,就把图像真正地解码到qimage中.
        if (true)
        {
            msg = decodeImagePacketTo(_testBuffer, _videoPacket);
            if (msg < 0)
            {
                qDebug() << "找到包后解码对应图像时出错，msg = " << msg;
                av_packet_unref(_videoPacket);
                //解码失败则继续找到能解码的包.
                continue;
            }
            //成功解码了就退出循环.
            //qDebug() << "解码此帧成功.pts = " << _videoPacket->pts;
            //一直往下寻找，直到满足_videoPacket->pts > _pts使函数不再递归调用.
            //if (_videoFrame->best_effort_timestamp > _pts)
            //{
                break;
            //}
        }
        //av_packet_unref(_videoPacket);
    }
    //av_packet_unref(_videoPacket);
}

qint64 FFMpegMultimedia::realTimeToPts(qreal timestamp, AVRational base)
{
    return timestamp / av_q2d(base);
}
inline qreal FFMpegMultimedia::ptsToRealTime(qint64 pts, AVRational base)
{
    return pts * av_q2d(base);
}
qint64 FFMpegMultimedia::ptsToFrame(qint64 pts)
{
    if (_ptsToFrame.find(pts) != _ptsToFrame.end())
    {
        return _ptsToFrame.find(pts)->second;
    }
    qDebug() << "没有查询到pts:" << pts << "的对应帧!返回0";
    return -1;
}

inline qint64 FFMpegMultimedia::realPtsToFrame(qint64 pts)
{
    return ptsToRealTime(pts,_videoStream->time_base)/getFrameInterval();
}

int FFMpegMultimedia::readAValidPacketTo(AVPacket*& target, int streamId)
{
    while (true)
    {
        if (av_read_frame(_fmtCtx, target) < 0)
        {
            av_packet_unref(target);
            return -1;
        }

        if (target->stream_index == streamId)
        {
            int msg;
            msg = avcodec_send_packet(_videoCodecCtx, target);
            if (msg != 0)
            {
                qDebug() << "send packet error:" + QString::number(msg);
                av_packet_unref(target);
                continue;
            }
            return 0;
        }
    }
}

int FFMpegMultimedia::decodeImagePacketTo(QImage& target, AVPacket* packet)
{
    QImage output = QImage(_videoCodecPar->width, _videoCodecPar->height, QImage::Format_RGB888);
    int outputLineSize[4];
    av_image_fill_linesizes(outputLineSize, AV_PIX_FMT_RGB24, _videoCodecPar->width);
    uint8_t* outputDst[] = { output.bits() };

    int msg;
    //msg = avcodec_send_packet(_videoCodecCtx, packet);
    //if (msg != 0)
    //{
    //    //qDebug() << "send packet error:" + QString::number(msg);
    //    //av_packet_unref(_videoPacket);
    //    _testBuffer = output;
    //    return -1;
    //}
    msg = avcodec_receive_frame(_videoCodecCtx, _videoFrame);
    if (msg != 0)
    {
        //qDebug() << "receive frame error msg:" + QString::number(msg);
        //av_packet_unref(_videoPacket);
        _testBuffer = output;
        return -1;
    }

    sws_scale(_imgConvert, _videoFrame->data, _videoFrame->linesize, 0, _videoCodecPar->height, outputDst, outputLineSize);
    target = output;
    return 0;
}

void FFMpegMultimedia::readTotalAudio()
{
    _totalAudio = QByteArray();

    AVPacket* packet = av_packet_alloc();                                                       //分配一个数据包

    AVFrame* frame = av_frame_alloc();

    SwrContext* swrctx = NULL;
    swrctx = swr_alloc_set_opts(swrctx, av_get_default_channel_layout(getChannelCount()), AV_SAMPLE_FMT_S16, getAudioSampleRate(),
        _audioCodecCtx->channel_layout, _audioCodecCtx->sample_fmt, _audioCodecCtx->sample_rate, NULL, NULL);
    swr_init(swrctx);

    int i = 0;
    while (true)
    {
        if (av_read_frame(_fmtCtx, packet) < 0)
        {
            av_packet_unref(packet);
            break;
        }

        //if (packet->stream_index == _videoStreamId && packet->pts != std::numeric_limits<qint64>::min())
        //{
        //    //qDebug() << "记录:" << packet->pts << ":" << i;
        //    //顺便记录pts与读入顺序的对应关系
        //    recorePtsToFrameSet(packet, i);
        //    i++;
        //}

        if (packet->stream_index == _audioStreamId)
        {
            if (int msg = avcodec_send_packet(_audioCodecCtx, packet) != 0)
            {
                qDebug() << "send audio packet error:" + QString::number(msg);
                av_packet_unref(packet);
                continue;
            }
            while (int msg = avcodec_receive_frame(_audioCodecCtx, frame) == 0)
            {
                int byteCnt = frame->nb_samples * _audio_bit_per_sample / 8 * getChannelCount();

                unsigned char* pcm = new uint8_t[byteCnt];

                uint8_t* data[2] = { 0 };
                data[0] = pcm;

                int ret = swr_convert(swrctx,
                    data, frame->nb_samples,        //输出
                    (const uint8_t**)frame->data, frame->nb_samples);    //输入
                _totalAudio.append((char*)pcm, byteCnt);
                delete[] pcm;
            }
        }
        av_packet_unref(packet);
    }

    av_seek_frame(_fmtCtx, -1, 0, AVSEEK_FLAG_BACKWARD);

    av_packet_free(&packet);
    av_frame_free(&frame);
}

void FFMpegMultimedia::recorePtsToFrameSet(AVPacket* pkt, qint64 index)
{
    _ptsToFrame.insert(map<qint64, qint64>::value_type(pkt->pts, index));
}

FFMpegMultimedia::FFMpegMultimedia()
{
    _fmtCtx = avformat_alloc_context();

    _videoPacket = av_packet_alloc();
    _videoFrame = av_frame_alloc();

    _isAttached = false;

    //默认缓存当前，前一张和后一张图像.
    _pBufferNum = 1;
    _lBufferNum = 1;

    _pts = 0;
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

    codecCtx->time_base = stream->time_base;

    if (codec == nullptr) {
        qCritical() << "can`t find codec.";
        return;
    }
    if (avcodec_open2(codecCtx, codec, nullptr) != 0) {                                          //开启编解码器
        qCritical() << "can`t open codec";
        return;
    }
}

void FFMpegMultimedia::open(QString path)
{
    _ptsToFrame = map<qint64, qint64>();
    _buffer = list<QImage>(1 + _pBufferNum + _lBufferNum);

    int state;
    //打开视频文件
    if(state = avformat_open_input(&_fmtCtx,path.toLocal8Bit(),nullptr,nullptr)!=0)
    {
        qCritical()<<"文件打开失败:"<< path <<" 错误码:"<<state;
        return;
    }

    //解析流信息
    if(avformat_find_stream_info(_fmtCtx,nullptr)!=0)
    {
        qCritical()<<"文件流解析失败:"<<path;
        return;
    }
    _videoStreamId = -1;
    _audioStreamId = -1;
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

    _imgConvert = sws_getContext(_videoCodecPar->width, _videoCodecPar->height, (AVPixelFormat)_videoCodecPar->format, _videoCodecPar->width, _videoCodecPar->height, AV_PIX_FMT_RGB24, NULL, NULL, NULL, NULL);

    readTotalAudio();
    //readAValidPacketTo(_videoPacket, _videoStreamId);
    //readPacket();
}

qreal FFMpegMultimedia::getDurationInSeconds()
{
    return 0;
}

void FFMpegMultimedia::seek(qreal timestamp)
{
    //int excState;
    //excState = av_seek_frame(_fmtCtx, -1, realTimeToPts(timestamp,_videoStream->time_base), AVSEEK_FLAG_BACKWARD);
    //if (excState < 0)
    //{
    //    qInfo() << "帧跳转无效" <<Qt::endl;
    //    return;
    //}

    av_packet_unref(_videoPacket);
    _pts = realTimeToPts(timestamp, _videoStream->time_base);
    av_seek_frame(_fmtCtx, _videoStreamId, _pts, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(_videoCodecCtx);
    //readAValidPacketTo(_videoPacket, _videoStreamId);
    //qDebug() << _videoPacket->pts;

    //释放跳转前留下的packet.
    //av_packet_unref(_videoPacket);
    //初始化跳转后的packet方便readPacket判断.
    //int msg = readAValidPacketTo(_videoPacket, _videoStreamId);
    //readPacket();

    //int msg;
    //do
    //{
    //    msg = readAValidPacketTo(_videoPacket, _videoStreamId);
    //    if (msg == 0)
    //    {
    //        int targetFrame = realPtsToFrame(_pts);
    //        int currentFrame = ptsToFrame(_videoPacket->pts);
    //        while (currentFrame < targetFrame)
    //        {
    //            av_packet_unref(_videoPacket);
    //            msg = readAValidPacketTo(_videoPacket, _videoStreamId);
    //            if (msg != 0)
    //            {
    //                break;
    //            }
    //            currentFrame = ptsToFrame(_videoPacket->pts);
    //        }
    //    }
    //}
    //while(true)
    //avcodec_flush_buffers(_videoCodecCtx);
    //readPacket();
}

void FFMpegMultimedia::nextFrame()
{
    //av_packet_unref(_videoPacket);
    _pts += realTimeToPts(getFrameInterval(), _videoStream->time_base);
    //readPacket();
}

qreal FFMpegMultimedia::getCurrentTimeStamp()
{
    return ptsToRealTime(_pts,_videoStream->time_base);
}

QImage FFMpegMultimedia::getImage()
{
    readPacket();
    return _testBuffer;
}

inline int alignByte(int index, int cell)
{
    index -= index % cell;
    return index;
}
QByteArray FFMpegMultimedia::getPCM()
{
    //return _totalAudio;
    //每个采样的位数.
    int cell = getChannelCount() * getAudioBytePerSample();

    //需要进行索引对齐，对齐到每个采样的大小上，否则数据会出问题
    int sIndex = ptsToRealTime(_pts, _videoStream->time_base) * getAudioSampleRate() * cell;
    sIndex = alignByte(sIndex, cell);

    int len = getFrameInterval() * getAudioSampleRate() * cell;
    len = alignByte(len, cell);

    return _totalAudio.mid(sIndex, len);
}

qreal FFMpegMultimedia::getFrameRate()
{
    return av_q2d(_videoStream->avg_frame_rate);
}

qreal FFMpegMultimedia::getFrameInterval()
{
    return qreal(1)/getFrameRate();
}
qint32 FFMpegMultimedia::getAudioSampleRate()
{
    return _audioCodecCtx->sample_rate;
}
qint32 FFMpegMultimedia::getChannelCount()
{
    return _audioCodecCtx->channels;
}
qint32 FFMpegMultimedia::getAudioBytePerSample()
{
    return _audio_bit_per_sample / 8;
}
} // namespace lzq
