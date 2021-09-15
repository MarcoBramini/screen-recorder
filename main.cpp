#include <vector>

extern "C" {
//#include <libavdevice/avdevice.h>
//#include <libavformat/avformat.h>
};


struct InputDevice{
    int id;
    char* name;
};

const char* platform_env_var = "PLATFORM";

std::vector<InputDevice> get_video_input_list_linux(){
    std::vector<InputDevice> devices;
    return devices;
}

std::vector<InputDevice> get_video_input_list_macos(){
    std::vector<InputDevice> devices;
    return devices;
}

std::vector<InputDevice> get_video_input_list_windows(){
    std::vector<InputDevice> devices;
    return devices;
}

// [{id:0,name:"Display1"},{id:1,name:"Webcam"}]
std::vector<InputDevice> get_video_input_list(){
    char* platform = std::getenv(platform_env_var);

    if (strcmp(platform, "MACOS") == 0) {
        return get_video_input_list_macos();
    }
    else if (strcmp(platform, "LINUX") == 0) {
        return get_video_input_list_linux();
    }
    else if (strcmp(platform, "WINDOWS") == 0) {
        return get_video_input_list_windows();
    }else{
        throw std::invalid_argument("unknown platform");
    }
}

std::vector<InputDevice> get_audio_input_list_linux(){
    std::vector<InputDevice> devices;
    return devices;
}

std::vector<InputDevice> get_audio_input_list_macos(){
    std::vector<InputDevice> devices;
    return devices;
}

std::vector<InputDevice> get_audio_input_list_windows(){
    std::vector<InputDevice> devices;
    return devices;
}

// [{id:0,name:"Integrated microphone"},{id:1,name:"Airpods mic"}]
std::vector<InputDevice> get_audio_input_list(){
    char* platform = std::getenv(platform_env_var);

    if (strcmp(platform, "MACOS") == 0) {
        return get_audio_input_list_macos();
    }
    else if (strcmp(platform, "LINUX") == 0) {
        return get_audio_input_list_linux();
    }
    else if (strcmp(platform, "WINDOWS") == 0) {
        return get_audio_input_list_windows();
    }else{
        throw std::invalid_argument("unknown platform");
    }
}


int main() {
    // For testing only, remove asap
    setenv(platform_env_var, "LINUX",0);

    std::vector<InputDevice> videoDevices = get_video_input_list();
    std::vector<InputDevice> audioDevices = get_audio_input_list();

    return 0;
}


//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//
//    AVDictionary *options = NULL;
//    av_dict_set(&options, "list_devices", "true", 0);
//
//    avdevice_register_all();
//    AVInputFormat *fmt = av_find_input_format("avfoundation");
//
//    int r = avformat_open_input(nullptr,"0:0",fmt, &options);
//    av_log(NULL, AV_LOG_ERROR, "%s\n" ,av_err2str(r));
