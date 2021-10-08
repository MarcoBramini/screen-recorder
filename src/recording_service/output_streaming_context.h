#ifndef PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H
#define PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include <string>
#include <utility>

class OutputStreamingContext {
    AVFormatContext *avfc;

    const AVCodec *videoAvc;
    AVStream *videoAvs;
    AVCodecContext *videoAvcc;

    const AVCodec *audioAvc;
    AVStream *audioAvs;
    AVCodecContext *audioAvcc;

    std::string filename;
public:

    OutputStreamingContext(AVFormatContext *avfc,std::string filename) : avfc(avfc),filename(std::move(filename)) {}

    AVFormatContext *getAvfc() {
        return avfc;
    }

    void setAvfc(AVFormatContext *avfc) {
        OutputStreamingContext::avfc = avfc;
    }

    const AVCodec * getVideoAvc() {
        return videoAvc;
    }

    void setVideoAvc(const AVCodec *videoAvc) {
        OutputStreamingContext::videoAvc = videoAvc;
    }

    AVStream *getVideoAvs() const {
        return videoAvs;
    }

    void setVideoAvs(AVStream *videoAvs) {
        OutputStreamingContext::videoAvs = videoAvs;
    }

    AVCodecContext *getVideoAvcc() const {
        return videoAvcc;
    }

    void setVideoAvcc(AVCodecContext *videoAvcc) {
        OutputStreamingContext::videoAvcc = videoAvcc;
    }

    const AVCodec *getAudioAvc() const {
        return audioAvc;
    }

    void setAudioAvc(const AVCodec *audioAvc) {
        OutputStreamingContext::audioAvc = audioAvc;
    }

    AVStream *getAudioAvs() const {
        return audioAvs;
    }

    void setAudioAvs(AVStream *audioAvs) {
        OutputStreamingContext::audioAvs = audioAvs;
    }

    AVCodecContext *getAudioAvcc() const {
        return audioAvcc;
    }

    void setAudioAvcc(AVCodecContext *audioAvcc) {
        OutputStreamingContext::audioAvcc = audioAvcc;
    }

    const std::string &getFilename() const {
        return filename;
    }

    void setFilename(const std::string &filename) {
        OutputStreamingContext::filename = filename;
    }
};

#endif //PDS_SCREEN_RECORDING_OUTPUT_STREAMING_CONTEXT_H
