#import <AVFoundation/AVFoundation.h>
#import "avfoundation_bridge.h"

#import <vector>

int avfoundation_list_video_devices(std::vector<InputDeviceVideo> *devices) {
    int index = 0;

    AVCaptureDeviceDiscoverySession *captureDeviceDiscoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[
            AVCaptureDeviceTypeBuiltInMicrophone, AVCaptureDeviceTypeBuiltInWideAngleCamera,
            AVCaptureDeviceTypeExternalUnknown]                                                                             mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];

    NSArray *audioInDevs = [captureDeviceDiscoverySession devices];
    for (AVCaptureDevice *device in audioInDevs) {
        const char *name = [[device localizedName] UTF8String];
        (*devices).emplace_back(std::to_string(index), std::string(name),0,0);
        index++;
    }

    uint32_t numScreens = 0;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
    CGGetActiveDisplayList(0, NULL, &numScreens);
    if (numScreens > 0) {
        CGDirectDisplayID screens[numScreens];
        CGGetActiveDisplayList(numScreens, screens, &numScreens);
        for (int i = 0; i < numScreens; i++) {
            (*devices).emplace_back(std::to_string(index), std::string("Capture screen " + std::to_string(i)),0,0);
            index++;
        }
    }
#endif

    return numScreens;
}

int avfoundation_list_audio_devices(std::vector<InputDeviceAudio> *devices) {
    AVCaptureDeviceDiscoverySession *captureDeviceDiscoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[
                    AVCaptureDeviceTypeBuiltInMicrophone, AVCaptureDeviceTypeBuiltInWideAngleCamera,
                    AVCaptureDeviceTypeExternalUnknown]
                                                                                                                            mediaType:AVMediaTypeAudio
                                                                                                                             position:AVCaptureDevicePositionUnspecified];


    NSArray *audioInDevs = [captureDeviceDiscoverySession devices];
    int index = 0;
    for (AVCaptureDevice *device in audioInDevs) {
        const char *name = [[device localizedName] UTF8String];
        (*devices).emplace_back(std::to_string(index), std::string(name));
        index++;
    }
    return index;
}
