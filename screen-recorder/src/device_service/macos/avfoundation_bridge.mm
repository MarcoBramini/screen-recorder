#import <AVFoundation/AVFoundation.h>
#import "avfoundation_bridge.h"

#import <vector>
#import <chrono>
#import <thread>
#include <stdexcept>

static const std::string DEVICE_ID_AVFOUNDATION = "avfoundation";

int check_screen_capture_permissions() {
    return CGPreflightScreenCaptureAccess();
}

int check_camera_capture_permissions() {
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo])
    {
    case AVAuthorizationStatusAuthorized:
    {
        // The user has previously granted access to the camera.
        return 1;
    }
    case AVAuthorizationStatusNotDetermined:
    {
        // The app hasn't yet asked the user for camera access.
        return 0;
    }
    default:
    {
        // The user has previously denied access or can't do it due to restrictions.
        throw std::runtime_error("Could not obtain capture permissions");
    }
    }
}

int check_microphone_capture_permissions() {
    // Request permission to access the microphone
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio])
    {
    case AVAuthorizationStatusAuthorized:
    {
        // The user has previously granted access to the microphone.
        return 1;
    }
    case AVAuthorizationStatusNotDetermined:
    {
        // The app hasn't yet asked the user for microphone access.
        return 0;
    }
    default:
    {
        // The user has previously denied access or can't do it due to restrictions.
        throw std::runtime_error("Could not obtain capture permissions");
    }
    }
}

void request_camera_capture_permissions(std::function<void(bool)> onComplete) {
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                        onComplete(granted);
                    }];
}
void request_microphone_capture_permissions(std::function<void(bool)> onComplete) {
    [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
                        onComplete(granted);
                    }];
}

void request_screen_capture_permissions(std::function<void(bool)> onComplete) {
    CGRequestScreenCaptureAccess();
    onComplete(true);
}

int avfoundation_list_video_devices(std::vector<InputDeviceVideo> *devices) {
    int index = 0;

    AVCaptureDeviceDiscoverySession *captureDeviceDiscoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:@[
                                                AVCaptureDeviceTypeBuiltInMicrophone, AVCaptureDeviceTypeBuiltInWideAngleCamera,
                                                AVCaptureDeviceTypeExternalUnknown]                                                                             mediaType:AVMediaTypeVideo position:AVCaptureDevicePositionUnspecified];

    NSArray *videoInDevs = [captureDeviceDiscoverySession devices];
    for (AVCaptureDevice *device in videoInDevs) {
        index++;
    }

    uint32_t numScreens = 0;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
    CGGetActiveDisplayList(0, NULL, &numScreens);
    if (numScreens > 0) {
        CGDirectDisplayID screens[numScreens];
        CGGetActiveDisplayList(numScreens, screens, &numScreens);
        for (int i = 0; i < numScreens; i++) {
            CGRect rect = CGDisplayBounds(screens[i]);
            (*devices).emplace_back(std::to_string(index), DEVICE_ID_AVFOUNDATION,
                                    std::string("Capture screen " + std::to_string(i)),
                                    rect.origin.x, rect.origin.y, rect.size.width, rect.size.height,
                                    CGDisplayIsMain(screens[i]), "");
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
        (*devices).emplace_back(std::to_string(index), DEVICE_ID_AVFOUNDATION, std::string(name));
        index++;
    }
    return index;
}
