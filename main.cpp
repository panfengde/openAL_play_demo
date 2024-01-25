#include <AL/al.h>
#include <AL/alext.h>
#include <AL/alc.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <unistd.h>
#include <fstream>

using namespace std;

struct PCMFileInfo {
    int sampleRate;
    int bitsPerSample;
    int channels;
};

PCMFileInfo getPCMFileInfo(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        // 处理文件打开失败的情况
        return PCMFileInfo{-1, -1, -1};
    }

    // 读取样本率、位深度和通道数等信息
    file.seekg(24, std::ios::beg); // 移到文件头的第24字节处

    int sampleRate, bitsPerSample, channels;
    file.read(reinterpret_cast<char*>(&sampleRate), sizeof(sampleRate));
    file.seekg(34, std::ios::beg); // 移到文件头的第34字节处
    file.read(reinterpret_cast<char*>(&bitsPerSample), sizeof(bitsPerSample));
    file.seekg(22, std::ios::beg); // 移到文件头的第22字节处
    file.read(reinterpret_cast<char*>(&channels), sizeof(channels));

    file.close();


    return PCMFileInfo{sampleRate, bitsPerSample, channels};
}

// 将文件切割为多个buffer，播放一个后，追加剩余的buffer，并从停止的buffer位置继续播放
void addStreamPlay(string path) {
    int buffeerNumbers = 8;
    ALCdevice* device = NULL;
    ALCcontext* context = NULL;

    device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "fail to open device\n");
        return;
    }
    context = alcCreateContext(device, NULL);
    if (!context) {
        fprintf(stderr, "fail to create context.\n");
        return;
    }
    alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR) {
        return;
    }
    //音频播放源
    ALuint source;
    //音频数据
    // ALuint buffer;
    //音频格式
    ALenum audioFormat = AL_FORMAT_STEREO_FLOAT32;
    //声道数目
    ALshort channels = 2;
    //音频采样率
    ALsizei sample_rate = 44100;
    //是否循环播放
    ALboolean loop = 0;
    //播放源的位置
    ALfloat position[] = {0.0f, 0.0f, 0.0f};
    //播放的速度
    ALfloat velocity[] = {0.0f, 0.0f, 0.0f};
    ALuint bufferStore[buffeerNumbers];

    //alGenBuffers(1, &buffer);
    alGenBuffers(buffeerNumbers, bufferStore);
    alGenSources(1, &source);

    FILE* f = fopen(path.c_str(), "rb");
    //FILE *f = fopen("/Users/panfeng/coder/mediaLean/FFmpegLearn/FFmpegCmake/outfile.pcm", "rb");

    long aBuffeerLength;
    // 将文件位置指针移动到文件末尾
    fseek(f, 0, SEEK_END);
    // 获取当前文件位置指针的位置（文件大小）
    long length = ftell(f);
    aBuffeerLength = static_cast<long>(std::floor(static_cast<double>(length) / buffeerNumbers));

    //将文件位置指针重新设置到文件的开头类似于 fseek(f, 0, SEEK_SET)
    rewind(f);

    for (int i = 0; i < buffeerNumbers - 7; i++) {
        char* aBuffer = (char *)malloc(aBuffeerLength);
        fread(aBuffer, sizeof(char), aBuffeerLength, f);
        alBufferData(bufferStore[i], audioFormat, aBuffer, aBuffeerLength, sample_rate);
        alSourceQueueBuffers(source, 1, &bufferStore[i]);
    }

    if (alGetError() != AL_NO_ERROR) {
        cout << "erro:" << alGetError() << endl;
        return;
    }
    //为source绑定数据
    //alSourcei(source, AL_BUFFER, buffer);
    //音高倍数
    alSourcef(source, AL_PITCH, 0.5f);
    //声音的增益
    alSourcef(source, AL_GAIN, 0.2f);
    //设置位置
    alSourcefv(source, AL_POSITION, position);
    //设置声音移动速度
    alSourcefv(source, AL_VELOCITY, velocity);
    //设置是否循环播放
    alSourcei(source, AL_LOOPING, loop);
    //播放音乐
    alSourcePlay(source);

    /*
    ALint source_state;
    ALint buffersProcessed = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    //只要不手动听下来，都是AL_PLAYING状态
    while (source_state == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    }*/

    for (int i = buffeerNumbers - 7; i < buffeerNumbers; i++) {
        char* aBuffer = (char *)malloc(aBuffeerLength);
        fread(aBuffer, sizeof(char), aBuffeerLength, f);
        alBufferData(bufferStore[i], audioFormat, aBuffer, aBuffeerLength, sample_rate);
        alSourceQueueBuffers(source, 1, &bufferStore[i]);
    }

    //注意这里的AL_BYTE_OFFSET ，是以字符大小，重新定义播放位置！！！
    alSourcei(source, AL_BYTE_OFFSET, aBuffeerLength * sizeof(char));
    alSourcePlay(source);

    //巧妙的阻塞程序，直到从标准输入（通常是终端）获取到输入字符为止
    getchar();

    //释放资源
    alcMakeContextCurrent(NULL);
    alDeleteSources(1, &source);
    alDeleteBuffers(buffeerNumbers, bufferStore);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

// 将文件切割为多个buffer并播放
void StreamPlay(string path) {
    int buffeerNumbers = 8;
    ALCdevice* device = NULL;
    ALCcontext* context = NULL;
    device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "fail to open device\n");
        return;
    }
    context = alcCreateContext(device, NULL);
    if (!context) {
        fprintf(stderr, "fail to create context.\n");
        return;
    }
    alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR) {
        return;
    }
    //音频播放源
    ALuint source;
    //音频数据
    // ALuint buffer;
    //音频格式
    ALenum audioFormat = AL_FORMAT_STEREO_FLOAT32;
    //声道数目
    ALshort channels = 2;
    //音频采样率
    ALsizei sample_rate = 44100;
    //是否循环播放
    ALboolean loop = 0;
    //播放源的位置
    ALfloat position[] = {0.0f, 0.0f, 0.0f};
    //播放的速度
    ALfloat velocity[] = {0.0f, 0.0f, 0.0f};
    ALuint bufferStore[buffeerNumbers];

    //alGenBuffers(1, &buffer);
    alGenBuffers(buffeerNumbers, bufferStore);
    alGenSources(1, &source);

    FILE* f = fopen(path.c_str(), "rb");
    //FILE *f = fopen("/Users/panfeng/coder/mediaLean/FFmpegLearn/FFmpegCmake/outfile.pcm", "rb");

    long aBuffeerLength;
    // 将文件位置指针移动到文件末尾
    fseek(f, 0, SEEK_END);
    // 获取当前文件位置指针的位置（文件大小）
    long length = ftell(f);
    aBuffeerLength = static_cast<long>(std::floor(static_cast<double>(length) / buffeerNumbers));

    // 重新将位置设为文件初始位置
    rewind(f);
    for (int i = 0; i < buffeerNumbers; i++) {
        char* aBuffer = (char *)malloc(aBuffeerLength);
        //读取buffer
        fread(aBuffer, sizeof(char), aBuffeerLength, f);
        //写入音频设备的buffer存储中
        alBufferData(bufferStore[i], audioFormat, aBuffer, aBuffeerLength, sample_rate);
        //设备关联buffer
        alSourceQueueBuffers(source, 1, &bufferStore[i]);
    }
    fclose(f);

    if (alGetError() != AL_NO_ERROR) {
        cout << "erro:" << alGetError() << endl;
        return;
    }
    // 为source绑定数据
    // alSourcei(source, AL_BUFFER, buffer);
    //音高倍数
    alSourcef(source, AL_PITCH, 0.5f);
    //声音的增益
    alSourcef(source, AL_GAIN, 0.2f);
    //设置位置
    alSourcefv(source, AL_POSITION, position);
    //设置声音移动速度
    alSourcefv(source, AL_VELOCITY, velocity);
    //设置是否循环播放
    //alSourcei(source, AL_LOOPING, loop);//循环
    alSourcei(source, AL_LOOPING, 0); //不循环
    //播放音乐
    alSourcePlay(source);

    /*//查询播放状态
    ALint source_state;
    ALint buffersProcessed = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    while (source_state == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffersProcessed);
    }*/

    //巧妙的阻塞程序，直到从标准输入（通常是终端）获取到输入字符为止
    getchar();
    //释放资源

    alcMakeContextCurrent(NULL);
    alDeleteSources(1, &source);
    alDeleteBuffers(buffeerNumbers, bufferStore);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

// 将文件作为一个buffer播放
void AllPlay(string path) {
    ALCdevice* device = NULL;
    ALCcontext* context = NULL;
    device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "fail to open device\n");
        return;
    }
    context = alcCreateContext(device, NULL);
    if (!context) {
        fprintf(stderr, "fail to create context.\n");
        return;
    }
    alcMakeContextCurrent(context);
    if (alGetError() != AL_NO_ERROR) {
        return;
    }
    //音频播放源
    ALuint source;
    //音频数据
    ALuint buffer;
    //音频格式
    ALenum audioFormat = AL_FORMAT_STEREO_FLOAT32;
    //声道数目
    ALshort channels = 2;
    //音频采样率
    ALsizei sample_rate = 44100;
    //是否循环播放
    ALboolean loop = 1;
    //播放源的位置
    ALfloat position[] = {0.0f, 0.0f, 0.0f};
    //播放的速度
    ALfloat velocity[] = {0.0f, 0.0f, 0.0f};
    alGenBuffers(1, &buffer);
    alGenSources(1, &source);

    FILE* f = fopen(path.c_str(), "rb");
    //FILE *f = fopen("/Users/panfeng/coder/mediaLean/FFmpegLearn/FFmpegCmake/outfile.pcm", "rb");

    // 将文件位置指针移动到文件末尾
    fseek(f, 0, SEEK_END);
    // 获取当前文件位置指针的位置（文件大小）
    long length = ftell(f);
    //将文件位置指针重新设置到文件的开头类似于 fseek(f, 0, SEEK_SET)
    rewind(f);
    //从堆（heap）申请内存大小
    char* data = (char *)malloc(length);
    //从f中读区length数量，每个大小为sizeof(char)的数据，存放在打他中
    fread(data, sizeof(char), length, f);

    alBufferData(buffer, audioFormat, data, length, sample_rate);
    if (alGetError() != AL_NO_ERROR) {
        return;
    }
    //为source绑定数据
    alSourcei(source, AL_BUFFER, buffer);
    //音高倍数
    alSourcef(source, AL_PITCH, 0.5f);
    //声音的增益
    alSourcef(source, AL_GAIN, 0.2f);
    //设置位置
    alSourcefv(source, AL_POSITION, position);
    //设置声音移动速度
    alSourcefv(source, AL_VELOCITY, velocity);
    //设置是否循环播放
    alSourcei(source, AL_LOOPING, loop);
    //播放音乐
    alSourcePlay(source);

    //巧妙的阻塞程序，直到从标准输入（通常是终端）获取到输入字符为止
    getchar();

    //释放资源
    free(data);
    alcMakeContextCurrent(NULL);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);

    alcDestroyContext(context);
    alcCloseDevice(device);
}

int main() {
    // 获取可用音频设备列表
    /*
    const ALCchar* devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    if (devices) {
        std::cout << "Available audio devices:" << std::endl;
        while (*devices) {
            std::cout << devices << std::endl;
            devices += std::strlen(devices) + 1;
        }
    }
    */
    AllPlay("/Users/panfeng/coder/mediaLean/openALLearn/music.pcm");
    // AllPlay("/Users/panfeng/coder/mediaLean/openALLearn/newx.pcm");
}

