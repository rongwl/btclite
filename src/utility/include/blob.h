#ifndef BTCLITE_BLOB_H
#define BTCLITE_BLOB_H

#include <array>
#include <cstring>
#include <iterator>

#include "string_encoding.h"

template <size_t size>
using Bytes = std::array<uint8_t, size>;

/** Template class for fixed-sized opaque blobs. (Little endian) */
template <uint32_t nBITS>
class Blob : public Bytes<nBITS/8> {
public:
    static constexpr int WIDTH = nBITS / 8;
    
    Blob()
    {
        Clear();
    }
    
    Blob(std::initializer_list<uint8_t> init)
    {   
        Clear();
        for (auto it1 = this->begin(), it2 = init.begin(); it1 != this->end() && it2 != init.end(); it1++, it2++)
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
        std::memset(&this->front(), 0, WIDTH);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& s) const
    {
        s.write(reinterpret_cast<const char*>(&this->front()), this->size());
    }
    
    template <typename Stream>
    void UnSerialize(Stream& s)
    {
        s.read(reinterpret_cast<char*>(&this->front()), this->size());
    }
    
    //-------------------------------------------------------------------------    
    std::string ToString() const
    {
        return Hex();
    }

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
    
    std::string Hex() const
    {
        return HexEncode(this->rbegin(), this->rend());
    }
    void SetHex(const std::string& str)
    {
        Clear();
        HexDecode(str, this->begin(), this->end());
    }
};


// modified from boost.iostreams example
// boost.org/doc/libs/1_55_0/libs/iostreams/doc/tutorial/container_source.html
template <typename Container, typename SinkType, typename CharType>
class ContainerSink {
public:
    typedef CharType char_type;

    ContainerSink(Container& container)
      : container_(container)
    {
        static_assert(sizeof(SinkType) == sizeof(CharType), "invalid size");
    }

    std::streamsize write(const char_type* buffer, std::streamsize size)
    {
        const auto safe_sink = reinterpret_cast<const SinkType*>(buffer);
        container_.insert(container_.end(), safe_sink, safe_sink + size);
        return size;
    }

private:
    Container& container_;
};

template <typename Container>
using ByteSink = ContainerSink<Container, uint8_t, char>;

#endif // BTCLITE_BLOB_H
