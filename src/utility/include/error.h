#ifndef BTCLITE_ERROR_H
#define BTCLITE_ERROR_H

#include <system_error>


enum class ErrorCode {
    success = 0,
    
    // args
    show_help,
    invalid_option,
    invalid_argument
    
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

namespace std {

template <>
struct is_error_code_enum<ErrorCode> : public true_type {};

} // namespace std


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

#endif // BTCLITE_ERROR_H
