#include "protocol/message.h"

#include "network/include/params.h"


bool MessageHeader::Init(const uint8_t *raw_data)
{
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    
    std::memset(command_.begin(), 0, kCommandSize);
    vec.reserve(kSize);
    vec.assign(raw_data, raw_data + kSize);
    Deserialize(byte_source);
    
    return IsValid();
}

bool MessageHeader::IsValid() const
{
    if (magic_ != btclite::network::SingletonParams::GetInstance().msg_magic()) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ << ") is invalid";
        return false;
    }
    
    std::string cmd = command();
    if (cmd != kMsgVersion &&
            cmd != kMsgVerack &&
            cmd != kMsgAddr &&
            cmd != kMsgInv &&
            cmd != kMsgGetData &&
            cmd != kMsgMerkleBlock &&
            cmd != kMsgGetBlocks &&
            cmd != kMsgGetHeaders &&
            cmd != kMsgTx &&
            cmd != kMsgHeaders &&
            cmd != kMsgBlock &&
            cmd != kMsgGetAddr &&
            cmd != kMsgMempool &&
            cmd != kMsgPing &&
            cmd != kMsgPong &&
            cmd != kMsgNotFound &&
            cmd != kMsgFilterLoad &&
            cmd != kMsgFilterAdd &&
            cmd != kMsgFilterClear &&
            cmd != kMsgReject &&
            cmd != kMsgSendHeaders &&
            cmd != kMsgFeeFilter &&
            cmd != kMsgSendCmpct &&
            cmd != kMsgCmpctBlock &&
            cmd != kMsgGetBlockTxn &&
            cmd != kMsgBlockTxn) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::command_(" << cmd << ") is invalid";
        return false;
    }
    
    if (payload_length_ > kMaxMessageSize) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::payload_length_(" << payload_length_ << ") is invalid";
        return false;
    }
    
    return true;
}