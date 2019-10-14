#ifndef BTCLITE_CONSTANTS_H
#define BTCLITE_CONSTANTS_H


#include <array>
#include <cstddef>
#include <cstdint>

#include "btclite-config.h"


#define BTCLITE_USER_AGENT "/" PACKAGE_NAME ":" PACKAGE_VERSION "/"

constexpr size_t kMaxVardataSize = 0x02000000;
constexpr size_t kMaxBlockSize = 1000000;
constexpr size_t kMaxMessageSize = 0x02000000;
// Maximum length of strSubVer in `version` message
constexpr size_t kMaxSubVersionSize = 256;

constexpr uint8_t kVarint16bits = 0xfd;
constexpr uint8_t kVarint32bits = 0xfe;
constexpr uint8_t kVarint64bits = 0xff;

constexpr uint64_t kSatoshiPerBitcoin = 100000000;
constexpr uint64_t kMaxSatoshiAmount = 21000000 * kSatoshiPerBitcoin;

constexpr uint32_t kMainMagic = 0xD9B4BEF9;
constexpr uint32_t kTestnetMagic = 0x0709110B;
constexpr uint32_t kRegtestMagic = 0xDAB5BFFA;

constexpr size_t kIpByteSize = 16;
constexpr std::array<uint8_t, kIpByteSize> kNullIp = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
constexpr uint8_t kPchIpv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
// 0xFD + sha256("bitcoin")[0:5]
constexpr uint8_t kBtcIpPrefix[] = { 0xFD, 0x6B, 0x88, 0xC0, 0x87, 0x24 };

// The maximum number of peer connections
constexpr uint8_t kMaxPeerConnections = 125;
// Maximum number of automatic outgoing nodes
constexpr uint8_t kMaxOutboundConnections = 8;
constexpr uint8_t kMaxInboundConnections = kMaxPeerConnections - kMaxOutboundConnections;
constexpr uint64_t kMaxOutboundTimeframe = 60 * 60 * 24;
constexpr size_t kMaxSendBuffer = 1000;
constexpr size_t kMaxReceiveBuffer = 5000;

constexpr uint64_t kDefaultMisbehavingBantime = 60 * 60 * 24;

// Expiration time for orphan transactions in seconds
constexpr int64_t kBrphanTxExpireTime = 20 * 60;
// Minimum time between orphan transactions expire time checks in seconds
constexpr int64_t kOrphanTxExpireInterval = 5 * 60;

constexpr uint32_t kNoMsgTimeout = 60;
constexpr uint32_t kNoSendingTimeout = 2*60;
constexpr uint32_t kNoReceivingTimeoutBip31 = 2*60;
constexpr uint32_t kNoReceivingTimeout = 90*60;
constexpr uint32_t kNoPingTimeout = 2*60;
constexpr uint32_t kNoConnectionTimeout = 60;

// Time after which to disconnect, after waiting for a ping response (or inactivity).
constexpr int kConnTimeoutInterval = 20 * 60;

constexpr uint32_t kMaxTimedataSamples = 200;

// how old addresses can maximally be
constexpr uint16_t kPeerHorizonDays = 30;
// after how many failed attempts we give up on a new node
constexpr uint16_t kPeerRetries = 3;
// how many successive failures are allowed ...
constexpr uint16_t kMaxPeerFailures = 10;
// in at least this many days
constexpr uint16_t kMinPeerFailDays = 7;
//! the maximum percentage of nodes to return in a getaddr call
constexpr uint16_t kMaxGetaddrPct = 23;
//! the maximum number of nodes to return in a getaddr call
constexpr uint32_t kMaxGetaddrCount = 2500;

constexpr uint32_t kOutboundConnectionTimeout = 500;
constexpr uint32_t kOutboundConnectionInterval = 500;

// message types
constexpr char kMsgVersion[] = "version";
constexpr char kMsgVerack[] = "verack";
constexpr char kMsgAddr[] = "addr";
constexpr char kMsgInv[] = "inv";
constexpr char kMsgGetData[] = "getdata";
constexpr char kMsgMerkleBlock[] = "merkleblock";
constexpr char kMsgGetBlocks[] = "getblocks";
constexpr char kMsgGetHeaders[] = "getheaders";
constexpr char kMsgTx[] = "tx";
constexpr char kMsgHeaders[] = "headers";
constexpr char kMsgBlock[] = "block";
constexpr char kMsgGetAddr[] = "getaddr";
constexpr char kMsgMempool[] = "mempool";
constexpr char kMsgPing[] = "ping";
constexpr char kMsgPong[] = "pong";
constexpr char kMsgNotFound[] = "notfound";
constexpr char kMsgFilterLoad[] = "filterload";
constexpr char kMsgFilterAdd[] = "filteradd";
constexpr char kMsgFilterClear[] = "filterclear";
constexpr char kMsgReject[] = "reject";
constexpr char kMsgSendHeaders[] = "sendheaders";
constexpr char kMsgFeeFilter[] = "feefilter";
constexpr char kMsgSendCmpct[] = "sendcmpct";
constexpr char kMsgCmpctBlock[] = "cmpctblock";
constexpr char kMsgGetBlockTxn[] = "getblocktxn";
constexpr char kMsgBlockTxn[] = "blocktxn";


#endif // BTCLITE_CONSTANTS_H
