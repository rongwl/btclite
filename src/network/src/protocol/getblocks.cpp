#include "protocol/getblocks.h"


namespace btclite {
namespace network {
namespace protocol {

bool GetBlocks::RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const
{
    return true;
}

GetBlocks::GetBlocks(ProtocolVersion version, const consensus::BlockLocator& hashes,
          const util::Hash256& hash_stop)
    : version_(version), hashes_(hashes), hash_stop_(hash_stop) 
{
}
      
GetBlocks::GetBlocks(ProtocolVersion version, consensus::BlockLocator&& hashes,
          const util::Hash256& hash_stop) noexcept
    : version_(version), hashes_(std::move(hashes)), 
      hash_stop_(hash_stop) 
{
}

std::string GetBlocks::Command() const
{
    return msg_command::kMsgGetBlocks;
}

bool GetBlocks::IsValid() const
{
    return (version_ != kUnknownProtoVersion && 
            (!hashes_.empty() || hashes_.size() > kMaxBlockLoactorSize));
}

void GetBlocks::Clear() 
{
    version_ = kUnknownProtoVersion;
    hashes_.clear();
    hash_stop_.fill(0);
}

size_t GetBlocks::SerializedSize() const
{
    return sizeof(version_) + util::VarIntSize(hashes_.size()) +
           hashes_.size()*kHashSize + hash_stop_.size();
}

util::Hash256 GetBlocks::GetHash() const
{
    return crypto::GetHash(*this);
}

bool GetBlocks::operator==(const GetBlocks& b) const
{
    return (version_ == b.version_ && hashes_ == b.hashes_ &&
            hash_stop_ == b.hash_stop_);
}

bool GetBlocks::operator!=(const GetBlocks& b) const
{
    return !(*this == b);
}

ProtocolVersion GetBlocks::version() const
{
    return version_;
}

void GetBlocks::set_version(ProtocolVersion version)
{
    version_ = version;
}

const consensus::BlockLocator& GetBlocks::hashes() const
{
    return hashes_;
}

void GetBlocks::set_hashes(const consensus::BlockLocator& hashes)
{
    hashes_ = hashes;
}

void GetBlocks::set_hashes(consensus::BlockLocator&& hashes)
{
    hashes_ = std::move(hashes);
}

const util::Hash256& GetBlocks::hash_stop() const
{
    return hash_stop_;
}

void GetBlocks::set_hash_stop(const util::Hash256& hash_stop)
{
    hash_stop_ = hash_stop;
}

} // namespace protocol
} // namespace network
} // namespace btclite
