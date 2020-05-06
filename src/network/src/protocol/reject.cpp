#include "protocol/reject.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_reject {

bool Reject::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
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
    data_.Clear();
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

} // namespace private_reject

} // namespace protocol
} // namespace network
} // namespace btclite
