#ifndef LZQ_MULTIMEDIAFILE_H
#define LZQ_MULTIMEDIAFILE_H
#include<QString>
#include<QImage>

namespace lzq {

class MultimediaFile
{
public:
    MultimediaFile();

    /**
     * @brief 按照路径绑定文件,每个实例仅调用一次
     * @param path 文件路径
     */
    void virtual open(QString path)=0;

    /**
     * @brief 获取该多媒体文件的总时长
     * @return 返回多媒体文件总长度
     */
    qreal virtual getDurationInSeconds()=0;

    /**
     * @brief 跳转当前时刻到timestamp
     * @param timestamp 一个时间，以秒为单位
     */
    void virtual seek(qreal timestamp)=0;

    /**
     * @brief 时间戳跳转到下一帧
     */
    void virtual nextFrame()=0;

    /**
     * @brief 获取当前时间戳
     * @return 一个时间，以秒为单位
     */
    qreal virtual getCurrentTimeStamp()=0;

    /**
     * @brief 获取当前帧的图像数据
     * @return 当前帧图像数据
     */
    QImage virtual getImage()=0;

    /**
     * @brief 获取当前帧的音频数据
     * @return 当前帧音频数据
     */
    QByteArray virtual getPCM()=0;

    /**
     * @brief 获取帧率
     * @return 帧率
     */
    qreal virtual getFrameRate()=0;

    /**
     * @brief 获取每帧间隔时间
     * @return 一个时间，以秒为单位
     */
    qreal virtual getFrameInterval()=0;

};

} // namespace lzq

#endif // LZQ_MULTIMEDIAFILE_H
