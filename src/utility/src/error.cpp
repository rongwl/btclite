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
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::success), 
            "success" },
            
            // args
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::show_help), 
            ""}, 
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::invalid_option), 
            "unrecognized option" },
            { static_cast<std::underlying_type_t<ErrorCode> >(ErrorCode::invalid_argument),
            "unsupported argument" },
        };
        
    const auto message = messages.find(code);
    return message != messages.end() ? message->second : "invalid code";
}

} // namespace util
} // namespace btclite
