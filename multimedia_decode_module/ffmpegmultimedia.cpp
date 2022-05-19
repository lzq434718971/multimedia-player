#include "ffmpegmultimedia.h"

namespace lzq {

int alignByte(int index, int cell)
{
    int rem = index % cell;
    if (rem > cell / 2)
    {
        return index - rem + cell;
    }
    return index - rem;
}

void FFMpegMultimedia::simpPropInit()
{
    _imgConvert = NULL;
    _audioConvert = NULL;

    _bufferPtsBottom = 0;

    _tempPCM = QByteArray();
    _tempPCMHead = 0;

    _isAttached = false;

    _frameCount = 0;

    _interPts = 0;
}

int FFMpegMultimedia::readPacket()
{
    //获取新帧前清空当前的包占据的内存
    av_packet_unref(_packet);
    if (av_read_frame(_fmtCtx, _packet) < 0)
    {
        av_packet_unref(_packet);
        return -4;
    }

    AVCodecContext* codecCtx;

    if (_packet->stream_index == _videoStreamId)
    {
        codecCtx = _videoCodecCtx;
    }
    else if (_packet->stream_index == _audioStreamId)
    {
        codecCtx = _audioCodecCtx;
    }
    else
    {
        //qDebug() << "不存在流:" << _packet->stream_index;
        return -3;
    }

    if (int msg = avcodec_send_packet(codecCtx, _packet) != 0)
    {
        qDebug() << "发送包时错误:" + QString::number(msg);
        av_packet_unref(_packet);
        return -1;
    }
    if (int msg = avcodec_receive_frame(codecCtx, _frame) != 0)
    {
        //qDebug() << "接收帧时错误:" + QString::number(msg);
        av_packet_unref(_packet);
        return -2;
    }
    return 0;
}

AVFrame* FFMpegMultimedia::popNextImageFrame()
{
    if (_imageQueue.empty())
    {
        //qDebug() << "视频未命中";
        while (true)
        {
            int msg;
            msg = readPacket();
            if (msg >= 0)
            {
                if (_packet->stream_index == _videoStreamId)
                {
                    //qDebug() << "立即返回解析到的视频帧";
                    pushImageQueue(_frame);
                    return popImageQueue();
                }
                else if (_packet->stream_index == _audioStreamId)
                {
                    pushAudioQueue(_frame);
                    //qDebug() << "累积音频帧:" << _audioQueue.size();
                }
            }
            else if(msg == -4)
            {
                //已经到流结尾
                return nullptr;
            }
        }
    }
    else
    {
        //qDebug() << "视频命中";
        return popImageQueue();
    }
    return nullptr;
}

AVFrame* FFMpegMultimedia::popNextAudioFrame()
{
    if (_audioQueue.empty())
    {
        //qDebug() << "音频未命中";
        while (true)
        {
            int msg;
            msg = readPacket();
            if (msg >= 0)
            {
                if (_packet->stream_index == _audioStreamId)
                {
                    //qDebug() << "立即返回解析到的音频帧";
                    pushAudioQueue(_frame);
                    return popAudioQueue();
                }
                else if (_packet->stream_index == _videoStreamId)
                {
                    pushImageQueue(_frame);
                    //qDebug() << "累积视频帧:" << _imageQueue.size();
                }
            }
            else if(msg == -4)
            {
                //已经到流结尾
                return nullptr;
            }
        }
    }
    else
    {
        //qDebug() << "音频命中";
        return popAudioQueue();
    }
}

void FFMpegMultimedia::seekAndSetCodecCtx(int64_t pts)
{
    //av_seek_frame(_fmtCtx, _videoStreamId, ptsToFrame(pts),AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD);
    //avcodec_flush_buffers(_videoCodecCtx);
    //av_read_frame(_fmtCtx, _videoPacket);
    //avcodec_send_packet(_videoCodecCtx, _videoPacket);
    //avcodec_receive_frame(_videoCodecCtx, _videoFrame);
}

void FFMpegMultimedia::readVideoPacket()
{
    //qDebug() << "当前pts:" << _pts;
    //int targetFrame = realPtsToFrame(_pts);
    //int currentFrame = ptsToFrame(_videoPacket->pts);
    ////qDebug() << "当前帧:" << currentFrame << ";目标帧:" << targetFrame;
    //if (_videoPacket->pts > _pts)
    //{
    //    return;
    //}
    //av_packet_unref(_videoPacket);
    //一直找到能解码的包或者到达结尾才循环结束.
    while (true)
    {
        int msg;
        //msg = readAValidPacketTo(_videoPacket, _videoStreamId);
        if (msg < 0)
        {
            qDebug() << "视频流结尾.";
            //av_packet_unref(_videoPacket);
            break;
        }

        //currentFrame = ptsToFrame(_videoPacket->pts);

        //qDebug() << "packet pts :"<<_videoPacket->pts;
        // 如果解码到了目标位置,就把图像真正地解码到qimage中.
        if (true)
        {
            //msg = decodeImagePacketTo(_testBuffer, _videoPacket);
            if (msg < 0)
            {
                qDebug() << "找到包后解码对应图像时出错，msg = " << msg;
                //av_packet_unref(_videoPacket);
                //解码失败则继续找到能解码的包.
                continue;
            }
            //成功解码了就退出循环.
            //qDebug() << "解码此帧成功.pts = " << _videoPacket->pts;
            //一直往下寻找，直到满足_videoPacket->pts > _pts使函数不再递归调用.
            //qDebug() << _videoFrame->best_effort_timestamp << ">>" << _pts;
            //if (_videoFrame->best_effort_timestamp > _pts)
            //{
                break;
            //}
        }
        //else
        //{
        //    //跳过该帧.
        //    avcodec_flush_buffers(_videoCodecCtx);
        //}
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

qint64 FFMpegMultimedia::videoPtsToAudioPts(qint64 pts)
{
    return realTimeToPts(ptsToRealTime(pts,_videoStream->time_base),_audioStream->time_base);
}

qint64 FFMpegMultimedia::audioPtsToVideoPts(qint64 pts)
{
    return realTimeToPts(ptsToRealTime(pts, _audioStream->time_base), _videoStream->time_base);
}

FFMpegMultimedia::TimeStamp FFMpegMultimedia::videoPtsToTimeStamp(qint64 pts)
{
    return ptsToRealTime(pts, _videoStream->time_base) * AV_TIME_BASE;
}

FFMpegMultimedia::TimeStamp FFMpegMultimedia::audioPtsToTimeStamp(qint64 pts)
{
    return ptsToRealTime(pts,_audioStream->time_base) * AV_TIME_BASE;
}

qreal FFMpegMultimedia::timeStampToRealTime(TimeStamp ts)
{
    return ts/qreal(AV_TIME_BASE);
}

qint64 FFMpegMultimedia::timeStampToVideoPts(TimeStamp ts)
{
    return realTimeToPts(timeStampToRealTime(ts),_videoStream->time_base);
}

qint64 FFMpegMultimedia::timeStampToAudioPts(TimeStamp ts)
{
    return realTimeToPts(timeStampToRealTime(ts),_audioStream->time_base);
}

FFMpegMultimedia::TimeStamp FFMpegMultimedia::realTimeToTimeStamp(qreal sec)
{
    return sec*AV_TIME_BASE;
}

QImage FFMpegMultimedia::AVFrameToQImage(AVFrame* frame)
{
    QImage output = QImage(_videoCodecPar->width, _videoCodecPar->height, QImage::Format_RGB888);
    int outputLineSize[4];
    av_image_fill_linesizes(outputLineSize, AV_PIX_FMT_RGB24, _videoCodecPar->width);
    uint8_t* outputDst[] = { output.bits() };

    sws_scale(_imgConvert, frame->data, frame->linesize, 0, _videoCodecPar->height, outputDst, outputLineSize);
    return output;
}

QByteArray FFMpegMultimedia::AVFrameToPCM(AVFrame* frame)
{
    int byteCnt = frame->nb_samples * _audio_bit_per_sample / 8 * getChannelCount();

    unsigned char* pcm = new uint8_t[byteCnt];

    uint8_t* data[2] = { 0 };
    data[0] = pcm;

    int len = swr_convert(_audioConvert,
        data, frame->nb_samples,        //输出
        (const uint8_t**)frame->data, frame->nb_samples);    //输入

    QByteArray ret = QByteArray();
    ret.append((char*)pcm, byteCnt);

    delete[] pcm;
    return ret;
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

        qDebug() << "音频包pts:" <<packet->pts;

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
                qDebug() << " 音频帧预测pts:" << frame->best_effort_timestamp;
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

FFMpegMultimedia::FFMpegMultimedia():MultimediaFile()
{
    _isOpening = false;
    simpPropInit();

    _iBufferNum = 30;

    _fmtCtx = avformat_alloc_context();

    _packet = av_packet_alloc();
    _frame = av_frame_alloc();

    _imageBuffer = list<AVFrame*>();

    //初始化帧池
    _framePool = list<AVFrame*>(_iBufferNum);
    for (list<AVFrame*>::iterator i = _framePool.begin(); i != _framePool.end(); i++)
    {
        *i = av_frame_alloc();
    }
}

FFMpegMultimedia::~FFMpegMultimedia()
{
    if (_isOpening)
    {
        close();
    }

    av_frame_free(&_frame);
    av_packet_free(&_packet);

    for (list<AVFrame*>::iterator i = _framePool.begin(); i != _framePool.end(); i++)
    {
        av_frame_free(&(*i));
    }

    avformat_free_context(_fmtCtx);
}

void FFMpegMultimedia::pushImageQueue(AVFrame* frame)
{
    pushFrameQueue(_imageQueue, frame);
}

AVFrame* FFMpegMultimedia::popImageQueue()
{
    return popFrameQueue(_imageQueue);
}

//void FFMpegMultimedia::testFreeFrame(AVFrame* frame)
//{
//    freeQueue(_imageQueue);
//    _imageQueue.resize(0);
//}

void FFMpegMultimedia::pushAudioQueue(AVFrame* frame)
{
    pushFrameQueue(_audioQueue, frame);
}

AVFrame* FFMpegMultimedia::popAudioQueue()
{
    return popFrameQueue(_audioQueue);
}

void FFMpegMultimedia::pushFrameQueue(list<AVFrame*>& queue, AVFrame* frame)
{
    AVFrame* copyFrame = av_frame_alloc();
    //av_frame_copy(copyFrame, frame);
    //av_frame_copy_props(copyFrame, frame);
    av_frame_ref(copyFrame, frame);
    queue.push_back(copyFrame);
}

AVFrame* FFMpegMultimedia::popFrameQueue(list<AVFrame*>& queue)
{
    if (queue.empty())
    {
        return nullptr;
    }
    AVFrame* ret = queue.front();
    queue.pop_front();
    return ret;
}

void FFMpegMultimedia::addImageBuffer(AVFrame* frame)
{
    //buffer中的图像超出缓存上限.
    if (_imageBuffer.size() >= _iBufferNum)
    {
        removeFrontImageBuffer();
    }
    AVFrame* emptyFrame = _framePool.front();
    _framePool.pop_front();
    av_frame_ref(emptyFrame, frame);
    _imageBuffer.push_back(emptyFrame);
}

void FFMpegMultimedia::removeFrontImageBuffer()
{
    AVFrame* frame = _imageBuffer.front();
    av_frame_unref(frame);
    _imageBuffer.pop_front();
    _framePool.push_back(frame);
    //在移除最早的缓存时，更新缓存pts下界
    if (!_imageBuffer.empty())
    {
        if (_imageBuffer.front()->best_effort_timestamp >= 0)
        {
            _bufferPtsBottom = videoPtsToTimeStamp(_imageBuffer.front()->best_effort_timestamp);
        }
        //qDebug() << "缓存pts下界:" << _bufferPtsBottom;
    }
}

void FFMpegMultimedia::freeImageBuffer()
{
    while (!_imageBuffer.empty())
    {
        removeFrontImageBuffer();
    }
}

void FFMpegMultimedia::freeAudioBuffer()
{
    _tempPCM.clear();
    //_tempPCMHead = _pts;
    _tempPCMHead = _interPts;
}

void FFMpegMultimedia::freeQueue(list<AVFrame*>& queue)
{
    while (!queue.empty())
    {
        AVFrame* toDel = popFrameQueue(queue);
        av_frame_free(&toDel);
    }
    //freeImageBuffer();
}

void FFMpegMultimedia::chopTempPCMBeforePts()
{
    int sIndex = ptsToByteIndex(_tempPCMHead);
    int eIndex = ptsToByteIndex(_bufferPtsBottom);
    _tempPCM = _tempPCM.mid(eIndex - sIndex,-1);
    //_tempPCMHead = _pts;
    //_tempPCMHead = _interPts;
    _tempPCMHead = max(_bufferPtsBottom,_tempPCMHead);
}

void FFMpegMultimedia::readLeastAudioPacketForOneFrame()
{
    //需要组成的音频数据长度
    //int len = ptsToByteIndex(realTimeToPts(getFrameInterval(), _videoStream->time_base));
    int len = realTimeToByteIndex(getFrameInterval());
    //qDebug() << "当前音频缓存长度:" << _tempPCM.size();
    //qDebug() << "需求音频缓存长度:" << len;
    int i = 0;
    //如果音频还不够长，就需要读入更多包延长缓存数据.
    while ((_tempPCM.size() - ptsToByteIndex(_interPts - _tempPCMHead)) <len)
    {
        i++;
        //qDebug() << "读入" << i << "帧补充音频缓存";

        AVFrame* additionalFrame = popNextAudioFrame();

        if (additionalFrame == nullptr)
        {
            return;
        }

        QByteArray additionalPCM = AVFrameToPCM(additionalFrame);
        _tempPCM.push_back(additionalPCM);
        //用完frame后需要释放
        av_frame_free(&additionalFrame);
    }
    //qDebug() << "读取音频包完毕-----";
    //qDebug() << "当前音频缓存长度:" << _tempPCM.size();
    //qDebug() << "需求音频缓存长度:" << len;
}

int FFMpegMultimedia::ptsToByteIndex(TimeStamp pts)
{
    //每个采样的字节数.
    int cell = getAudioBytePerSample();

    //需要进行索引对齐，对齐到每个采样的大小上，否则数据会出问题
    int index = timeStampToRealTime(pts) * getAudioSampleRate() * cell;
    index = alignByte(index, cell);
    return index;
}

int FFMpegMultimedia::realTimeToByteIndex(qreal sec)
{
    //每个采样的字节数.
    int cell = getAudioBytePerSample();

    //需要进行索引对齐，对齐到每个采样的大小上，否则数据会出问题
    int index = round(sec * getAudioSampleRate() * cell);
    index = alignByte(index, cell);
    return index;
}

AVFrame* FFMpegMultimedia::findFrameInImageBuffer(qint64 pts)
{
    if (_imageBuffer.empty())
    {
        return nullptr;
    }
    qint64 maxPts = videoPtsToTimeStamp(_imageBuffer.back()->best_effort_timestamp);
    qint64 minPts = _bufferPtsBottom;
    if (pts >= minPts && pts <= maxPts)
    {
        return findNearestImageBuffer(pts);
    }
    return nullptr;
}

AVFrame* FFMpegMultimedia::findNearestImageBuffer(qint64 pts)
{
    qint64 minPtsDis = std::numeric_limits<qint64>::max();
    AVFrame* ret = nullptr;
    for (list<AVFrame*>::iterator i = _imageBuffer.begin(); i != _imageBuffer.end(); i++)
    {
        qint64 dis = abs(pts - videoPtsToTimeStamp((*i)->best_effort_timestamp));
        if (minPtsDis > dis)
        {
            minPtsDis = dis;
            ret = *i;
        }
    }
    //ret = _imageBuffer.back();
    return ret;
}

void FFMpegMultimedia::commonStreamInit(AVFormatContext* context,AVMediaType streamType,
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
        qInfo() << "没有找到流，类型需求:" << streamType;
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
        qInfo() << "can`t find codec.";
        return;
    }
    if (avcodec_open2(codecCtx, codec, nullptr) != 0) {                                          //开启编解码器
        qInfo() << "can`t open codec";
        return;
    }
}

int FFMpegMultimedia::open(QString path)
{
    //_fmtCtx = avformat_alloc_context();
    if (_isOpening)
    {
        close();
    }
    _isOpening = true;

    int state;
    //打开视频文件
    if(state = avformat_open_input(&_fmtCtx,path.toLocal8Bit(),nullptr,nullptr)!=0)
    {
        qInfo()<<"文件打开失败:"<< path <<" 错误码:"<<state;
        return -1;
    }

    //解析流信息
    if(avformat_find_stream_info(_fmtCtx,nullptr)!=0)
    {
        qInfo()<<"文件流解析失败:"<<path;
        return -2;
    }
    _videoStreamId = -1;
    _audioStreamId = -1;
    commonStreamInit(_fmtCtx, AVMEDIA_TYPE_VIDEO, _videoStreamId, _videoCodecPar, _videoCodec, _videoCodecCtx, _videoStream);
    commonStreamInit(_fmtCtx, AVMEDIA_TYPE_AUDIO, _audioStreamId, _audioCodecPar, _audioCodec, _audioCodecCtx, _audioStream);
    if(_videoStreamId == -1 && _audioStreamId == -1)
    {
        qInfo()<<"没有找到视频/音频流:"<<path;
        return -3;
    }
    else if(_videoStreamId == -1)
    {
        qInfo()<<"仅包含音频:"<<path;
    }
    else if(_audioStreamId == -1)
    {
        qInfo()<<"仅包含视频:"<<path;
    }

    _videoCodecCtx->thread_count = 2;

    _imgConvert = sws_getContext(_videoCodecPar->width, _videoCodecPar->height, (AVPixelFormat)_videoCodecPar->format, _videoCodecPar->width, _videoCodecPar->height, AV_PIX_FMT_RGB24, NULL, NULL, NULL, NULL);

    _audioConvert = swr_alloc_set_opts(_audioConvert, av_get_default_channel_layout(getChannelCount()), AV_SAMPLE_FMT_S16, getAudioSampleRate(),
        _audioCodecCtx->channel_layout, _audioCodecCtx->sample_fmt, _audioCodecCtx->sample_rate, NULL, NULL);
    swr_init(_audioConvert);

    return 0;
    //readTotalAudio();
    //readAValidPacketTo(_videoPacket, _videoStreamId);
    //readPacket();
}

void FFMpegMultimedia::close()
{
    _isOpening = false;

    freeQueue(_imageQueue);
    freeQueue(_audioQueue);
    freeAudioBuffer();
    freeImageBuffer();

    sws_freeContext(_imgConvert);
    swr_free(&_audioConvert);

    av_frame_unref(_frame);
    av_packet_unref(_packet);
    avcodec_free_context(&_videoCodecCtx);
    avcodec_free_context(&_audioCodecCtx);
    avformat_close_input(&_fmtCtx);
    avformat_free_context(_fmtCtx);

    simpPropInit();
}

qreal FFMpegMultimedia::getDurationInSeconds()
{
    return _fmtCtx->duration / double(AV_TIME_BASE);
}

void FFMpegMultimedia::seek(qreal timestamp)
{
    if (timestamp<0 || timestamp>getDurationInSeconds())
    {
        qInfo() << "seek时间戳非法";
        return;
    }
    qint64 targetPts = realTimeToTimeStamp(timestamp);
    _interPts = realTimeToTimeStamp(timestamp);
    _frameCount = getCurrentTimeStamp() / getFrameInterval();
    //缓存未命中.
    if (_imageBuffer.empty() || _interPts<_bufferPtsBottom || _interPts>(videoPtsToTimeStamp(_imageBuffer.back()->best_effort_timestamp)+5*AV_TIME_BASE))
    {
        freeAudioBuffer();
        freeQueue(_audioQueue);
        freeImageBuffer();
        freeQueue(_imageQueue);

        av_seek_frame(_fmtCtx, _videoStreamId, realTimeToPts(timestamp,_videoStream->time_base), AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(_videoCodecCtx);
        avcodec_flush_buffers(_audioCodecCtx);
        av_frame_unref(_frame);
        av_packet_unref(_packet);

        AVFrame* audioHead;
        bool first = true;
        qint64 packetHead;
        do
        {
            readPacket();
            if (_packet->stream_index == _audioStreamId)
            {
                qint64 packetSizeTS = audioPtsToTimeStamp(_frame->pkt_duration);
                packetHead = audioPtsToTimeStamp(_frame->best_effort_timestamp);
                if (first)
                {
                    _tempPCMHead = packetHead;
                    first = false;
                }
                _tempPCM.push_back(AVFrameToPCM(_frame));
                //_tempPCMHead = _bufferPtsBottom;
                if (packetHead +audioPtsToTimeStamp(_frame->pkt_duration) >= _interPts)
                {
                    //av_frame_unref(_frame);
                    break;
                }
            }
            else if (_packet->stream_index == _videoStreamId)
            {
                pushImageQueue(_frame);
                while (!_imageQueue.empty() && videoPtsToTimeStamp(_imageQueue.front()->best_effort_timestamp) <= _interPts)
                {
                    //_bufferPtsBottom = videoPtsToTimeStamp(_imageQueue.front()->best_effort_timestamp);
                    AVFrame* pastImg = popImageQueue();
                    addImageBuffer(pastImg);
                    av_frame_free(&pastImg);
                }
            }
        } while (true);

        qint64 bottomVideoPts = -1;
        if (!_imageBuffer.empty())
        {
            bottomVideoPts = _imageBuffer.front()->best_effort_timestamp;
        }

        if (bottomVideoPts >= 0 && videoPtsToTimeStamp(bottomVideoPts) <= _interPts)
        {
            _bufferPtsBottom = videoPtsToTimeStamp(bottomVideoPts);
        }
        else
        {
            _bufferPtsBottom = _interPts;
        }
        chopTempPCMBeforePts();
        //_bufferPtsBottom = _interPts;
    }
}

void FFMpegMultimedia::nextFrame()
{
    if (_interPts > realTimeToTimeStamp(getDurationInSeconds()))
    {
        //已经到视频结尾
        return;
    }
    _frameCount++;
    _interPts = _frameCount / getDurationInSeconds() * getFrameInterval() * realTimeToTimeStamp(getDurationInSeconds());
    while (!_audioQueue.empty())
    {
        qint64 epts = _audioQueue.front()->best_effort_timestamp + _audioQueue.front()->pkt_duration;
        if (audioPtsToTimeStamp(epts) <= _interPts)
        {
            AVFrame* frame = popAudioQueue();
            _tempPCMHead = audioPtsToTimeStamp(epts);
            av_frame_free(&frame);
        }
        else
        {
            break;
        }
    }
    while (!_imageQueue.empty())
    {
        if (videoPtsToTimeStamp(_imageQueue.front()->best_effort_timestamp)<=_interPts)
        {
            AVFrame* frame = popImageQueue();
            addImageBuffer(frame);
            av_frame_free(&frame);
        }
        else
        {
            break;
        }
    }
}

qreal FFMpegMultimedia::getCurrentTimeStamp()
{
    return timeStampToRealTime(_interPts);
}

QImage FFMpegMultimedia::getImage()
{
    /**
     * 这个版本的getImage函数假定下一个要播放的帧在缓存区或者缓存区往后很近的位置.
     * 没有命中缓存区时会将缓存区后移，直到要找的帧在缓存区中.
     **/
    AVFrame* retFrame = findFrameInImageBuffer(_interPts);
    if (nullptr == retFrame)
    {
        //未命中时后移缓存区，并递归执行当前函数，直到命中.
        retFrame = popNextImageFrame();

        if (nullptr == retFrame)
        {
            //已经到流结尾
            return QImage();
        }

        addImageBuffer(retFrame);
        av_frame_free(&retFrame);
        return getImage();
    }
    else
    {
        //命中时输出结果.
        //qDebug() << "播放帧:" << ptsToRealTime(retFrame->best_effort_timestamp,_videoStream->time_base);
        QImage ret = AVFrameToQImage(retFrame);
        return ret;
    }
    //AVFrame* test = popNextImageFrame();
    //qDebug() << test->best_effort_timestamp;
    //return AVFrameToQImage(test);
    //return QImage();
}

QByteArray FFMpegMultimedia::getPCM()
{
    //return AVFrameToPCM(popNextAudioFrame());
    //int len = ptsToByteIndex(realTimeToPts(getFrameInterval(), _videoStream->time_base));
    int len = realTimeToByteIndex(getFrameInterval());
    //qDebug() << "音频切帧前:" << ptsToRealTime(_tempPCMHead, _videoStream->time_base);
    chopTempPCMBeforePts();
    //qDebug()<< "音频切帧后:"<< ptsToRealTime(_tempPCMHead, _videoStream->time_base);
    readLeastAudioPacketForOneFrame();

    qint64 sIndex = ptsToByteIndex(_interPts - _tempPCMHead);
    return _tempPCM.mid(sIndex, len);
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
    return _audio_bit_per_sample / 8 * getChannelCount();
}
} // namespace lzq
