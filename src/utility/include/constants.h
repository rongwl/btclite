#ifndef BTCLITE_CONSTANTS_H
#define BTCLITE_CONSTANTS_H

#include <cstddef>
#include <cstdint>

constexpr size_t max_vardata_size = 0x02000000;
constexpr size_t max_block_size = 1000000;
constexpr size_t max_message_size = 0x02000000;

constexpr uint8_t varint_16bits = 0xfd;
constexpr uint8_t varint_32bits = 0xfe;
constexpr uint8_t varint_64bits = 0xff;

constexpr uint64_t satoshi_per_bitcoin = 100000000;
constexpr uint64_t max_satoshi_amount = 21000000 * satoshi_per_bitcoin;

constexpr uint32_t main_magic = 0xD9B4BEF9;
constexpr uint32_t testnet_magic = 0x0709110B;
constexpr uint32_t regtest_magic = 0xDAB5BFFA;

constexpr uint8_t pch_ipv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
// 0xFD + sha256("bitcoin")[0:5]
constexpr uint8_t btc_ip_prefix[] = { 0xFD, 0x6B, 0x88, 0xC0, 0x87, 0x24 };

// The maximum number of peer connections
constexpr uint8_t max_peer_connections = 125;
// Maximum number of automatic outgoing nodes
constexpr uint8_t max_outbound_connections = 8;
constexpr uint8_t max_inbound_connections = max_peer_connections - max_outbound_connections;
constexpr uint64_t max_outbound_timeframe = 60 * 60 * 24;
constexpr size_t max_send_buffer = 1000;
constexpr size_t max_receive_buffer = 5000;

constexpr uint64_t default_misbehaving_bantime = 60 * 60 * 24;

// Expiration time for orphan transactions in seconds
constexpr int64_t orphan_tx_expire_time = 20 * 60;
// Minimum time between orphan transactions expire time checks in seconds
constexpr int64_t orphan_tx_expire_interval = 5 * 60;

constexpr struct timeval node_checking_timeout = { 60, 0 };

// Time after which to disconnect, after waiting for a ping response (or inactivity).
constexpr int conn_timeout_interval = 20 * 60;

//! BIP 0031, pong message, is enabled for all versions AFTER this one
constexpr int bip0031_version = 60000;

#endif // BTCLITE_CONSTANTS_H
