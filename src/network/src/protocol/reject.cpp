#include "protocol/reject.h"


namespace btclite {
namespace network {
namespace protocol {

Reject::Reject(const uint8_t *raw, size_t size)
    : Reject()
{
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    vec.reserve(size);
    vec.assign(raw, raw + size);
    Deserialize(byte_source);
}

bool Reject::RecvHandler(std::shared_ptr<Node> src_node) const
{
    return true;
}

bool Reject::IsValid() const
{
    if (ccode_ != kRejectMalformed &&
        ccode_ != kRejectInvalid &&
        ccode_ != kRejectObsolete &&
        ccode_ != kRejectDuplicate &&
        ccode_ != kRejectNonstandard &&
        ccode_ != kRejectDust &&
        ccode_ != kRejectInsufficientfee &&
        ccode_ != kRejectCheckpoint)
        return false;
    
    return true;
}

void Reject::Clear()
{
    message_.clear();
    ccode_ = 0;
    reason_.clear();
    data_.Clear();
}

size_t Reject::SerializedSize() const
{
    return btclite::utility::serialize::VarIntSize(message_.size()) + message_.size() +
           btclite::utility::serialize::VarIntSize(reason_.size()) + reason_.size() +
           sizeof(ccode_) + data_.size();
}

bool Reject::operator==(const Reject& b) const
{
    return (message_ == b.message_ &&
            ccode_ == b.ccode_ &&
            reason_ == b.reason_ &&
            data_ == b.data_);
}

bool Reject::operator!=(const Reject& b) const
{
    return !(*this == b);
}

} // namespace protocol
} // namespace network
} // namespace btclite
