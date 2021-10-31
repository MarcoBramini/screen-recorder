#include "recording_service.h"

extern "C" {
#include <libavutil/imgutils.h>
}

int RecordingService::convert_video(AVFrame *videoInputFrame, AVFrame *videoOutputFrame) {
    videoOutputFrame->format = OUTPUT_VIDEO_PIXEL_FMT;
    videoOutputFrame->width = outputVideoAvcc->width;
    videoOutputFrame->height = outputVideoAvcc->height;
    av_frame_get_buffer(videoOutputFrame, 0);

    if (sws_scale(videoConverter, videoInputFrame->data, videoInputFrame->linesize, 0, videoInputFrame->height,
                  videoOutputFrame->data, videoOutputFrame->linesize) < 0) {
        throw std::runtime_error("failed converting video frame");
    }
    return 0;
}

int64_t last_encoded_video_dts = -1;

int RecordingService::encode_video(int64_t framePts, AVFrame *videoInputFrame) {
    AVPacket *output_packet = av_packet_alloc();
    if (!output_packet) {
        //Handle
        std::cout << "error allocating output AVPacket" << std::endl;
        return -1;
    }

    if (videoInputFrame != nullptr) {
        videoInputFrame->pict_type = AV_PICTURE_TYPE_NONE;
        videoInputFrame->pts = framePts;
        //videoInputFrame->pts = av_rescale_q(framePts, {1, 1000}, outputVideoAvcc->time_base);
    }

    int response = avcodec_send_frame(outputVideoAvcc, videoInputFrame);

    while (response >= 0) {
        response = avcodec_receive_packet(outputVideoAvcc, output_packet);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            //std::cout << "eagain" << std::endl;
            break;
        } else if (response < 0) {
            //Handle
            std::cout << "error receiving packet from video encoder" << std::endl;
            return -1;
        }

        output_packet->stream_index = outputVideoAvs->index;

        av_packet_rescale_ts(output_packet, outputVideoAvcc->time_base, outputVideoAvs->time_base);

        if (output_packet->dts <= last_encoded_video_dts) {
            continue;
        }
        last_encoded_video_dts = output_packet->dts;
        //std::cout << "write video dts" << output_packet->dts << std::endl;
        response = av_interleaved_write_frame(outputAvfc, output_packet);
        if (response != 0) {
            //Handle
            std::cout << "error writing output frame" << std::endl;
            return -1;
        }
    }
    av_packet_unref(output_packet);
    av_packet_free(&output_packet);
    return 0;
}

int RecordingService::transcode_video(AVPacket *videoInputPacket, int64_t packetPts) {
    AVFrame *videoInputFrame = av_frame_alloc();
    if (videoInputFrame == nullptr) {
        //Handle
        return -1;
    }

    int response = avcodec_send_packet(inputVideoAvcc, videoInputPacket);
    if (response < 0) {
        std::cout << "Error while sending packet to video decoder: " << unpackAVError(response) << std::endl;
        return -1;
    }

    while (response >= 0) {
        response = avcodec_receive_frame(inputVideoAvcc, videoInputFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            std::cout << "error receiving packet from video decoder" << std::endl;
            return -1;
        }

        if (response >= 0) {
            AVFrame *convertedFrame = av_frame_alloc();
            if (convert_video(videoInputFrame, convertedFrame) < 0) {
                throw std::runtime_error("Error converting video frame");
            }
            if (encode_video(packetPts, convertedFrame)) {
                throw std::runtime_error("Error converting audio frame");
            }
            av_frame_free(&convertedFrame);
        }
    }
    av_frame_free(&videoInputFrame);
    return 0;
}

int RecordingService::convert_audio(AVFrame *audioInputFrame) {
    uint8_t **audioData;
    int ret = av_samples_alloc_array_and_samples(&audioData, nullptr, outputAudioAvcc->channels,
                                                 audioInputFrame->nb_samples, OUTPUT_AUDIO_SAMPLE_FMT, 0);
    if (ret < 0) {
        throw std::runtime_error("Fail to alloc samples by av_samples_alloc_array_and_samples.");
    }

    ret = swr_convert(audioConverter, audioData, audioInputFrame->nb_samples,
                      (const uint8_t **) (audioInputFrame->extended_data),
                      audioInputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error("Fail to swr_convert.");
    }

    if (av_audio_fifo_space(audioConverterBuffer) < audioInputFrame->nb_samples)
        throw std::runtime_error("audio buffer is too small.");

    ret = av_audio_fifo_write(audioConverterBuffer, (void **) audioData, audioInputFrame->nb_samples);
    if (ret < 0) {
        throw std::runtime_error("Fail to write fifo");
    }

    av_freep(&audioData[0]);
    return 0;
}

int64_t last_encoded_audio_dts = -1;

int RecordingService::encode_audio_from_buffer(int64_t framePts, bool shouldFlush) {

    AVPacket *output_packet = av_packet_alloc();
    if (!output_packet) {
        std::cout << "could not allocate memory for output packet" << std::endl;
        return -1;
    }

    AVFrame *outputFrame = nullptr;
    if (!shouldFlush) {
        outputFrame = av_frame_alloc();
        if (outputFrame == nullptr) {
            throw std::runtime_error("Error allocating new frame");
        }
    }

    while (av_audio_fifo_size(audioConverterBuffer) >= outputAudioAvcc->frame_size) {
        if (!shouldFlush) {
            // Build frame from buffer
            outputFrame->nb_samples = outputAudioAvcc->frame_size;
            outputFrame->channels = outputAudioAvcc->channels;
            outputFrame->channel_layout = outputAudioAvcc->channel_layout;
            outputFrame->format = OUTPUT_AUDIO_SAMPLE_FMT;
            outputFrame->sample_rate = outputAudioAvcc->sample_rate;

            outputFrame->pts = framePts;

            if (av_frame_get_buffer(outputFrame, 0) < 0) {
                throw std::runtime_error("Error reading from audio buffer");
            }

            if (av_audio_fifo_read(audioConverterBuffer, (void **) outputFrame->data, outputAudioAvcc->frame_size) <
                0) {
                throw std::runtime_error("Error reading from audio buffer");
            }
        }

        int response = avcodec_send_frame(outputAudioAvcc, outputFrame);

        while (response >= 0) {
            response = avcodec_receive_packet(outputAudioAvcc, output_packet);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                break;
            } else if (response < 0) {
                std::cout << "Error while receiving packet from encoder: " << std::endl;
                return -1;
            }

            output_packet->stream_index = outputAudioAvs->index;

            if (output_packet->dts <= last_encoded_audio_dts) {
                continue;
            }
            last_encoded_audio_dts = output_packet->dts;
            //std::cout << "write audio dts" << output_packet->pts << std::endl;
            response = av_interleaved_write_frame(outputAvfc, output_packet);
            if (response != 0) {
                std::cout << "Error " << response << " while receiving packet from decoder: " << std::endl;
                return -1;
            }
        }

    }
    av_frame_unref(outputFrame);
    av_frame_free(&outputFrame);
    av_packet_unref(output_packet);
    av_packet_free(&output_packet);

    return 0;
}

int RecordingService::transcode_audio(AVPacket *audioInputPacket, int64_t packetPts) {
    AVFrame *audioInputFrame = av_frame_alloc();
    if (audioInputFrame == nullptr) {
        //Handle
        return -1;
    }

    int response = avcodec_send_packet(inputAudioAvcc, audioInputPacket);
    if (response < 0) {
        std::cout << "Error while sending packet to decoder: " << unpackAVError(response) << std::endl;
        return response;
    }

    while (response >= 0) {
        response = avcodec_receive_frame(inputAudioAvcc, audioInputFrame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            std::cout << "Error while receiving frame from input: " << std::endl;
            return response;
        }

        if (response >= 0) {
            if (convert_audio(audioInputFrame) < 0) {
                throw std::runtime_error("Error converting audio frame");
            }

            if (encode_audio_from_buffer(packetPts, false) < 0) {
                throw std::runtime_error("Error converting audio frame");
            }
        }
    }
    av_frame_free(&audioInputFrame);
    return 0;
}