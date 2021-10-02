#include "recording_service.h"
#include <thread>

AVFormatContext *RecordingService::open_input_device(std::string deviceID, std::string url) {
    AVFormatContext *ctx = avformat_alloc_context();
    if (!ctx) {
        // Handle ctx allocation error
        std::cout << "error during AVFormatContext allocation" << std::endl;
        return nullptr;
    }

    int ret;

    auto *inputFormat = av_find_input_format(deviceID.c_str());

    AVDictionary *options = nullptr;
    ret = av_dict_set(&options, "pixel_format", "nv12", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }
    ret = av_dict_set(&options, "framerate", "60", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }
    ret = av_dict_set(&options, "capture_cursor", "true", 0);
    if (ret < 0) {
        std::cout << "error setting input options" << std::endl;
        return nullptr;
    }

    ret = avformat_open_input(&ctx, url.c_str(), inputFormat, &options);
    if (ret < 0) {
        // Handle open input error
        std::cout << "error opening input format" << std::endl;
        return nullptr;
    }

    ctx->probesize = 100000000;
    ret = avformat_find_stream_info(ctx, NULL);
    if (ret < 0) {
        // Handle find stream info error
        std::cout << "error finding stream info" << std::endl;
        return nullptr;
    }
    return ctx;
}

int RecordingService::open_input_stream_decoder(AVStream *inputStream, const AVCodec **inputCodec,
                                                AVCodecContext **inputCodecCtx) {
    *inputCodec = avcodec_find_decoder(inputStream->codecpar->codec_id);
    if (!*inputCodec) {
        std::cout << "failed to find the codec" << std::endl;
        return -1;
    }

    *inputCodecCtx = avcodec_alloc_context3(*inputCodec);
    if (!*inputCodecCtx) {
        std::cout << "failed to alloc memory for codec context" << std::endl;
        return -1;
    }

    if (avcodec_parameters_to_context(*inputCodecCtx, inputStream->codecpar) < 0) {
        std::cout << "failed to fill codec context" << std::endl;
        return -1;
    }

    if (avcodec_open2(*inputCodecCtx, *inputCodec, NULL) < 0) {
        std::cout << "failed to open codec" << std::endl;
        return -1;
    }
    return 0;
}

int
RecordingService::start_recording_loop(InputStreamingContext inputCtx, std::optional<InputStreamingContext> inputAuxCtx,
                                       OutputStreamingContext outputCtx) {
    // if inputAuxCtx == nullptr
    //  -> read frame from device (loop not blocked)
    //  -> open video thread
    //      -> transcode frame
    //      -> write interleaved frame
    //  -> open audio thread
    //      -> transcode frame
    //      -> write interleaved frame
    // -> else
    //  -> open video thread
    //      -> read frame from device
    //      -> transcode frame
    //      -> write interleaved frame
    //  -> open audio thread
    //      -> read frame from device
    //      -> transcode frame
    //      -> write interleaved frame
    return 0;
}

OutputStreamingContext RecordingService::open_output_file(const std::string &filename) {
    /*OutputStreamingContext output;
    output->filename = filename.c_str();

    avformat_alloc_output_context2(&output->avfc, NULL, NULL, output->filename);
    if (!output->avfc) {
        //Handle null context
    }

    if (output->avfc->oformat->flags & AVFMT_GLOBALHEADER)
        output->avfc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (!(output->avfc->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output->avfc->pb, output->filename, AVIO_FLAG_WRITE) < 0) {
            //Handle
            return std::nullopt;
        }
    }

    return output;*/
    return {};
}

int RecordingService::start_recording() {
    // Write output file header
    // Call start_recording_loop in a new thread

    std::thread thread([this]() {
        start_recording_loop(*inputCtx, *inputAuxCtx, *outputCtx);
    });

    return 0;
}

int RecordingService::pause_recording(){
    // Set pauseTimestamp

    // Set cond var isPaused to true
    return 0;
}

int RecordingService::resume_recording(){
    // Increment pausedTime by resumeTimestamp - pauseTimestamp interval

    // Set cond var isPaused to false
    return 0;
}

int RecordingService::stop_recording(){
    // Stop recording loop thread (set cond var stopRecording to true)
    // Flush encoders
    // Write output file trailer
    // Free res (?)
    return 0;
}

/// Initializes all the structures needed for the recording process.
/// Accepts device IDs as input in the following format:
/// - MacOS
///     videoInDevID:"avfoundation:1"
///     audioInDevID:"avfoundation:1"
/// - Linux
///     videoInDevID:"x11grab:...."
///     audioInDevID:"pulse:....."
/// - Windows
///     videoInDevID:"dshow:...."
///     audioInDevID:"dshow:...."
RecordingService::RecordingService(const std::string &videoInDevID, const std::string &audioInDevID,
                                   const std::string &outputFilename) {

    // Split input devices ID
    // If audio and video deviceID is equal
    //  -> call open_input_device(deviceID, videoID, audioID)
    //  -> get input streamID for audio and video
    //  -> call open_input_stream_decoder for audio and video
    //  -> build single InputStreamingContext
    // Else
    //  -> call open_input_device(deviceID, videoID, nullptr) and open_input_device(deviceID, nullptr, audioID)
    //  -> get input streamID for audio and video
    //  -> call open_input_stream_decoder for audio and video
    //  -> build double InputStreamingContext

    // Call open_output_file

    // Set class attributes
}
