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


#endif // BTCLITE_CONSTANTS_H
