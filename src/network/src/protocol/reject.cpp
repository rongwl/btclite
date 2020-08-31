#include "protocol/reject.h"


namespace btclite {
namespace network {
namespace protocol {

Reject::Reject(const std::string& message, CCode ccode, const std::string& reason)
    : message_(message), ccode_(ccode), reason_(reason), data_() 
{
}
      
Reject::Reject(std::string&& message, CCode ccode, std::string&& reason) noexcept
    : message_(std::move(message)), ccode_(ccode), reason_(std::move(reason)),
      data_() 
{
}
      
Reject::Reject(const std::string& message, CCode ccode, const std::string& reason,
       const util::Hash256& data)
    : message_(message), ccode_(ccode), reason_(reason), data_(data) 
{
}
      
Reject::Reject(std::string&& message, CCode ccode, std::string&& reason,
       const util::Hash256& data) noexcept
    : message_(std::move(message)), ccode_(ccode), reason_(std::move(reason)),
      data_(data) 
{
}

bool Reject::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

std::string Reject::Command() const
{
    return msg_command::kMsgReject;
}

bool Reject::IsValid() const
{
    if (ccode_ != CCode::kRejectMalformed &&
        ccode_ != CCode::kRejectInvalid &&
        ccode_ != CCode::kRejectObsolete &&
        ccode_ != CCode::kRejectDuplicate &&
        ccode_ != CCode::kRejectNonstandard &&
        ccode_ != CCode::kRejectDust &&
        ccode_ != CCode::kRejectInsufficientfee &&
        ccode_ != CCode::kRejectCheckpoint)
        return false;
    
    return true;
}

void Reject::Clear()
{
    message_.clear();
    ccode_ = CCode::kRejectUnknown;
    reason_.clear();
    data_.fill(0);
}

size_t Reject::SerializedSize() const
{
    size_t result = util::VarIntSize(message_.size()) +
                    util::VarIntSize(reason_.size()) +
                    message_.size() + reason_.size() + sizeof(ccode_);
    if (message_ == msg_command::kMsgBlock || message_ == msg_command::kMsgTx)
        result += data_.size();
    
    return result;
}

util::Hash256 Reject::GetHash() const
{
    return crypto::GetHash(*this);
}

bool Reject::operator==(const Reject& b) const
{
    if (message_ == msg_command::kMsgBlock || message_ == msg_command::kMsgTx)
        return (message_ == b.message_ &&
                ccode_ == b.ccode_ &&
                reason_ == b.reason_ &&
                data_ == b.data_);
    
    return (message_ == b.message_ &&
            ccode_ == b.ccode_ &&
            reason_ == b.reason_);
}

bool Reject::operator!=(const Reject& b) const
{
    return !(*this == b);
}

const std::string& Reject::message() const
{
    return message_;
}

void Reject::set_message(const std::string& message)
{
    message_ = message;
}

void Reject::set_message(std::string&& message) noexcept
{
    message_ = std::move(message);
}

CCode Reject::ccode() const
{
    return ccode_;
}

void Reject::set_ccode(CCode ccode)
{
    ccode_ = ccode;
}

const std::string& Reject::reason() const
{
    return reason_;
}

void Reject::set_reason(const std::string& reason)
{
    reason_ = reason;
}

void Reject::set_reason(std::string&& reason) noexcept
{
    reason_ = std::move(reason);
}

const util::Hash256& Reject::data() const
{
    return data_;
}

void Reject::set_data(const util::Hash256& data)
{
    data_ = data;
} 

} // namespace protocol
} // namespace network
} // namespace btclite
