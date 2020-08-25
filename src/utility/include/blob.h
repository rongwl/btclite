#ifndef BTCLITE_BLOB_H
#define BTCLITE_BLOB_H

#include <array>
#include <cstring>
#include <iterator>

#include "string_encoding.h"


namespace btclite {
namespace util {

template <size_t size>
using Bytes = std::array<uint8_t, size>;


// merge from boost.iostreams example
// boost.org/doc/libs/1_55_0/libs/iostreams/doc/tutorial/container_sink.html
template <typename Container, typename SinkType, typename CharType>
class ContainerSink {
public:
    ContainerSink(Container& container)
      : container_(container)
    {
        static_assert(sizeof(SinkType) == sizeof(CharType), "invalid size");
    }

    ContainerSink& write(const CharType* buffer, std::streamsize size)
    {
        const auto safe_sink = reinterpret_cast<const SinkType*>(buffer);
        container_.insert(container_.end(), safe_sink, safe_sink + size);
        return *this;
    }

private:
    Container& container_;
};

template <typename Container>
using ByteSink = ContainerSink<Container, uint8_t, char>;

// merge from boost.iostreams example
// boost.org/doc/libs/1_55_0/libs/iostreams/doc/tutorial/container_source.html
template <typename Container, typename SourceType, typename CharType>
class ContainerSource
{
public:
    ContainerSource(const Container& container)
        : container_(container), position_(0)
    {
        static_assert(sizeof(SourceType) == sizeof(CharType), "invalid size");
    }

    std::streamsize read(CharType *buffer, std::streamsize size);

private:
    const Container& container_;
    typename Container::size_type position_;
};

template <typename Container, typename SourceType, typename CharType>
std::streamsize ContainerSource<Container, SourceType, CharType>::read(CharType *buffer, std::streamsize size)
{        
    if (position_ > container_.size())
        throw std::overflow_error("container source overflow");
    else if (position_ == container_.size())
        return -1;
    
    auto amount = container_.size() - position_;
    auto result = std::min(size, static_cast<std::streamsize>(amount));
    const auto value = static_cast<typename Container::size_type>(result);
    std::memcpy(buffer, container_.data() + position_, value);
    position_ += value;
    
    return result;
}

template <typename Container>
using ByteSource = ContainerSource<Container, uint8_t, char>;

} // namespace util
} // namespace btclite

#endif // BTCLITE_BLOB_H
