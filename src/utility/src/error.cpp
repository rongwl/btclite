#include "error.h"

#include <unordered_map>


std::error_code make_error_code(ErrorCode e)
{
    return std::error_code(e, BtcliteErrCategory::instance());
}

std::string BtcliteErrCategory::message(int code) const noexcept
{
    static const std::unordered_map<int, std::string> messages =
        {
            { ErrorCode::success, "success" },
            
            // args
            { ErrorCode::show_help, ""}, 
            { ErrorCode::invalid_option, "unrecognized option" },
            { ErrorCode::invalid_argument, "unsupported argument" },
        };
        
    const auto message = messages.find(code);
    return message != messages.end() ? message->second : "invalid code";
}
