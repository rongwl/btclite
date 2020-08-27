#include "stream.h"


namespace btclite {
namespace util {

MemoryStream::MemoryStream()
    : vec_(), byte_sink_(vec_), byte_source_(vec_)
{
}

uint8_t* MemoryStream::Data()
{
    return vec_.data();
}

void MemoryStream::Clear()
{
    vec_.clear();
}

size_t MemoryStream::Size()
{
    return vec_.size();
}

} // namespace util
} // namespace btclite
