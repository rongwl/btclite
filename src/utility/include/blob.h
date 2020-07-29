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

/* Template class for fixed-sized opaque blobs. */
template <uint32_t nBITS>
class Blob : public Bytes<nBITS/8> {
public:    
    Blob()
    {
        Clear();
    }
    
    explicit Blob(std::initializer_list<uint8_t> init)
    {   
        Clear();
        for (auto it1 = this->begin(), it2 = init.begin(); it1 != this->end() && it2 != init.end(); ++it1, ++it2)
            *it1 = *it2;
    }

    //-------------------------------------------------------------------------
    bool IsNull() const
    {
        for (auto it : *this) {
            if (it != 0)
                return false;
        }
        return true;
    }
    
    void Clear()
    {
        std::memset(&this->front(), 0, width_);
    }
    
    size_t Size() const
    {
        return width_;
    }
    
    size_t SerializedSize() const
    {
        return Size();
    }
    
    int Compare(const Blob& b) const
    {
        return std::memcmp(this->data(), b.data(), width_);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s.write(reinterpret_cast<const char*>(&this->front()), this->size());
    }
    
    template <typename Stream>
    void Deserialize(Stream& s)
    {
        s.read(reinterpret_cast<char*>(&this->front()), this->size());
    }
    
    //-------------------------------------------------------------------------
    uint64_t Uint64(int pos) const
    {
        auto it = this->begin() + pos * 8;
        return *(it) | 
               (*(it+1) << 8) | 
               (*(it+2) << 16) | 
               (*(it+3) << 24) | 
               (*(it+4) << 32) | 
               (*(it+5) << 40) | 
               (*(it+6) << 48) | 
               (*(it+7) << 56);
    }

private:
    uint32_t width_ = nBITS / 8;
};


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
