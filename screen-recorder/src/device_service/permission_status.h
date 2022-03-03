#ifndef PDS_SCREEN_RECORDING_PERMISSION_STATUS_H
#define PDS_SCREEN_RECORDING_PERMISSION_STATUS_H

struct PermissionsStatus {
    bool screenCaptureAllowed;
    bool microphoneCaptureAllowed;
    bool cameraCaptureAllowed;
};

#endif // PDS_SCREEN_RECORDING_PERMISSION_STATUS_H
