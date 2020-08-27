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
    static const BtcliteErrCategory& instance();    
    virtual const char* name() const noexcept;    
    virtual std::string message(int code) const noexcept;
    virtual std::error_condition default_error_condition(int code) const noexcept;
};


class Exception : public std::runtime_error {
public :
    Exception(const std::error_code& code);
    Exception(const std::error_code& code, const std::string& msg);    
    const std::error_code& code() const;
    
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
