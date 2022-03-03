#include "ffmpeg_objects_deleter.h"

void FFMpegObjectsDeleter::operator()(AVCodecContext *avcc) {
    avcodec_close(avcc);
    avcodec_free_context(&avcc);
}

void FFMpegObjectsDeleter::operator()(AVFormatContext *avfc) {
    avformat_close_input(&avfc);
    avformat_free_context(avfc);
}

void FFMpegObjectsDeleter::operator()(AVFrame *avf) {
    av_frame_free(&avf);
}

void FFMpegObjectsDeleter::operator()(AVPacket *avp) {
    av_packet_unref(avp);
    av_packet_free(&avp);
}

void FFMpegObjectsDeleter::operator()(SwrContext *swrc) {
    swr_free(&swrc);
}

void FFMpegObjectsDeleter::operator()(AVAudioFifo *avaf) {
    av_audio_fifo_free(avaf);
}

void FFMpegObjectsDeleter::operator()(uint8_t ***d) {
    av_freep(d[0]);
    av_freep(d);
}

void FFMpegObjectsDeleter::operator()(SwsContext *swsc) {
    sws_freeContext(swsc);
}

void FFMpegObjectsDeleter::operator()(AVFilterGraph *avfg) {
    avfilter_graph_free(&avfg);
}

void FFMpegObjectsDeleter::operator()(AVFilterInOut *avfio) {
    avfilter_inout_free(&avfio);
}
