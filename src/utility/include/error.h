#ifndef BTCLITE_ERROR_H
#define BTCLITE_ERROR_H

#include <system_error>


namespace btclite {
namespace util {

enum class ErrorCode {
    kSuccess = 0,
    
    // args
    kShowHelp,
    kInvalidOption,
    kInvalidArg
    
};

std::error_code make_error_code(ErrorCode e);

class BtcliteErrCategory
    : public std::error_category
{
public:
    static const BtcliteErrCategory& instance()
    {
        static BtcliteErrCategory instance;
        return instance;
    }
    
    virtual const char* name() const noexcept
    {
        return "btclite";
    }
    
    virtual std::string message(int code) const noexcept;
    virtual std::error_condition default_error_condition(int code) const noexcept
    {
        return std::error_condition(code, *this);
    }
};


class Exception : public std::runtime_error {
public :
    Exception(const std::error_code& code)
        : code_(code), runtime_error(code.message()) {}
    Exception(const std::error_code& code, const std::string& msg)
        : code_(code), runtime_error(msg) {}
    
    const std::error_code& code() const
    {
        return code_;
    }
private:
    std::error_code code_;
};

} // namespace util
} // namespace btclite

namespace std {

template <>
struct is_error_code_enum<btclite::util::ErrorCode> : public true_type {};

} // namespace std

#endif // BTCLITE_ERROR_H
