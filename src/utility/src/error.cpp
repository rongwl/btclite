#include "error.h"

#include <unordered_map>


namespace btclite {
namespace util {

std::error_code make_error_code(ErrorCode e)
{
    return std::error_code(static_cast<std::underlying_type_t<ErrorCode> >(e),
                           BtcliteErrCategory::instance());
}

std::string BtcliteErrCategory::message(int code) const noexcept
{
    static const std::unordered_map<int, std::string> messages =
        {
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::kSuccess), 
            "success" },
            
            // args
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::kShowHelp), 
            ""}, 
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::kInvalidOption), 
            "unrecognized option" },
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::kInvalidArg),
            "unsupported argument" },
        };
        
    const auto message = messages.find(code);
    return message != messages.end() ? message->second : "invalid code";
}

} // namespace util
} // namespace btclite
