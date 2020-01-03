#include "protocol/reject.h"


namespace btclite {
namespace network {
namespace protocol {

bool reject::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

bool reject::IsValid() const
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

void reject::Clear()
{
    message_.clear();
    ccode_ = CCode::kRejectUnknown;
    reason_.clear();
    data_.Clear();
}

size_t reject::SerializedSize() const
{
    size_t result = btclite::utility::serialize::VarIntSize(message_.size()) +
                    btclite::utility::serialize::VarIntSize(reason_.size()) +
                    message_.size() + reason_.size() + sizeof(ccode_);
    if (message_ == ::kMsgBlock || message_ == ::kMsgTx)
        result += data_.size();
    
    return result;
}

bool reject::operator==(const reject& b) const
{
    if (message_ == ::kMsgBlock || message_ == ::kMsgTx)
        return (message_ == b.message_ &&
                ccode_ == b.ccode_ &&
                reason_ == b.reason_ &&
                data_ == b.data_);
    
    return (message_ == b.message_ &&
            ccode_ == b.ccode_ &&
            reason_ == b.reason_);
}

bool reject::operator!=(const reject& b) const
{
    return !(*this == b);
}

} // namespace protocol
} // namespace network
} // namespace btclite
