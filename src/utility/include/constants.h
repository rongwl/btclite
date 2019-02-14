#ifndef BTCLITE_CONSTANTS_H
#define BTCLITE_CONSTANTS_H

#include <cstddef>

constexpr std::size_t max_vardata_size = 0x02000000;
constexpr std::size_t max_block_size = 1000000;

constexpr uint8_t varint_16bits = 0xfd;
constexpr uint8_t varint_32bits = 0xfe;
constexpr uint8_t varint_64bits = 0xff;

constexpr uint64_t satoshi_per_bitcoin = 100000000;
constexpr uint64_t max_satoshi_amount = 21000000 * satoshi_per_bitcoin;

#endif // BTCLITE_CONSTANTS_H
