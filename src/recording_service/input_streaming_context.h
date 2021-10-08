#ifndef PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H
#define PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

/// Contains the information about one input device.
/// At least one between video and audio properties must be used.
/// If input device only support video/audio data, only video/audio properties will be set.
class InputStreamingContext {
    AVFormatContext *avfc;

    // Video properties
    const AVCodec *videoAvc;
    AVStream *videoAvs;
    AVCodecContext *videoAvcc;
    int videoIndex;

    // Audio properties
    const AVCodec *audioAvc;
    AVStream *audioAvs;
    AVCodecContext *audioAvcc;
    int audioIndex;

public:
    InputStreamingContext(AVFormatContext *avfc, const AVCodec *videoAvc, AVStream *videoAvs, AVCodecContext *videoAvcc,
                          int videoIndex, const AVCodec *audioAvc, AVStream *audioAvs, AVCodecContext *audioAvcc,
                          int audioIndex) : avfc(avfc), videoAvc(videoAvc), videoAvs(videoAvs), videoAvcc(videoAvcc),
                                            videoIndex(videoIndex), audioAvc(audioAvc), audioAvs(audioAvs),
                                            audioAvcc(audioAvcc), audioIndex(audioIndex) {}

    AVFormatContext *getAvfc() const {
        return avfc;
    }

    AVFormatContext **getAvfcPtr() {
        return &avfc;
    }

    void setAvfc(AVFormatContext *avfc) {
        InputStreamingContext::avfc = avfc;
    }

    const AVCodec *getVideoAvc() const {
        return videoAvc;
    }

    void setVideoAvc(const AVCodec *videoAvc) {
        InputStreamingContext::videoAvc = videoAvc;
    }

    AVStream *getVideoAvs() const {
        return videoAvs;
    }

    void setVideoAvs(AVStream *videoAvs) {
        InputStreamingContext::videoAvs = videoAvs;
    }

    AVCodecContext *getVideoAvcc() const {
        return videoAvcc;
    }

    AVCodecContext **getVideoAvccPtr() {
        return &videoAvcc;
    }

    void setVideoAvcc(AVCodecContext *videoAvcc) {
        InputStreamingContext::videoAvcc = videoAvcc;
    }

    int getVideoIndex() const {
        return videoIndex;
    }

    void setVideoIndex(int videoIndex) {
        InputStreamingContext::videoIndex = videoIndex;
    }

    const AVCodec *getAudioAvc() const {
        return audioAvc;
    }

    void setAudioAvc(const AVCodec *audioAvc) {
        InputStreamingContext::audioAvc = audioAvc;
    }

    AVStream *getAudioAvs() const {
        return audioAvs;
    }

    void setAudioAvs(AVStream *audioAvs) {
        InputStreamingContext::audioAvs = audioAvs;
    }

    AVCodecContext *getAudioAvcc() const {
        return audioAvcc;
    }

    AVCodecContext **getAudioAvccPtr() {
        return &audioAvcc;
    }

    void setAudioAvcc(AVCodecContext *audioAvcc) {
        InputStreamingContext::audioAvcc = audioAvcc;
    }

    int getAudioIndex() const {
        return audioIndex;
    }

    void setAudioIndex(int audioIndex) {
        InputStreamingContext::audioIndex = audioIndex;
    }
};

#endif //PDS_SCREEN_RECORDING_INPUT_STREAMING_CONTEXT_H
