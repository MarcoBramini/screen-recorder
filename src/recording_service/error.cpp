#include "error.h"
#include <fmt/core.h>

extern "C" {
#include <libavutil/avutil.h>
}

std::string Error::build_error_message(const std::string &methodName,
                                const std::map<std::string, std::string> &methodParams,
                                const std::string &errorDescription) {
    // Format method params to string
    std::string params;
    for (const auto &param: methodParams) {
        params.append(fmt::format("{}:{}, ", param.first, param.second));
    }
    // Trim last comma and space
    params = params.substr(0, params.length() - 2);

    return fmt::format("{}({}): {}", methodName, params, errorDescription);
}


std::string Error::unpackAVError(int avErrorCode) {
    char a[AV_ERROR_MAX_STRING_SIZE] = {0};
    return av_make_error_string(a, AV_ERROR_MAX_STRING_SIZE, avErrorCode);
}