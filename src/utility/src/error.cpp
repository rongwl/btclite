#include "error.h"

#include <unordered_map>


namespace btclite {
namespace util {

std::error_code make_error_code(ErrorCode e)
{
    return std::error_code(static_cast<std::underlying_type_t<ErrorCode> >(e),
                           BtcliteErrCategory::instance());
}

const BtcliteErrCategory& BtcliteErrCategory::instance()
{
    static BtcliteErrCategory instance;
    return instance;
}

const char* BtcliteErrCategory::name() const noexcept
{
    return "btclite";
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

std::error_condition BtcliteErrCategory::default_error_condition(int code) const noexcept
{
    return std::error_condition(code, *this);
}

Exception::Exception(const std::error_code& code)
    : code_(code), runtime_error(code.message())
{
}

Exception::Exception(const std::error_code& code, const std::string& msg)
    : code_(code), runtime_error(msg)
{
}

const std::error_code& Exception::code() const
{
    return code_;
}

} // namespace util
} // namespace btclite
