//
// Created by Marco Bramini on 01/11/21.
//

#ifndef PDS_SCREEN_RECORDING_ERROR_H
#define PDS_SCREEN_RECORDING_ERROR_H

#include <string>
#include <map>

class Error {
public:
    static std::string build_error_message(const std::string &methodName,
                                           const std::map<std::string, std::string> &methodParams,
                                           const std::string &errorDescription);

    static std::string unpackAVError(int avErrorCode);
};

#endif //PDS_SCREEN_RECORDING_ERROR_H
