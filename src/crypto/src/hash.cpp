#include "hash.h"


namespace btclite {
namespace crypto {

namespace hashfuncs {

util::Hash256 Sha256(const uint8_t in[], size_t length)
{
    util::Hash256 result;
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    
    hash_func->update(in, length);
    hash_func->final(reinterpret_cast<uint8_t*>(&result));
    
    return result;
}

util::Hash256 Sha256(const std::vector<uint8_t>& in)
{
    return Sha256(in.data(), in.size());
}

util::Hash256 DoubleSha256(const uint8_t in[], size_t length)
{
    util::Hash256 result;
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    
    hash_func->update(in, length);
    hash_func->final(reinterpret_cast<uint8_t*>(&result));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(&result), result.size());
    hash_func->final(reinterpret_cast<uint8_t*>(&result));
    
    return result;
}

util::Hash256 DoubleSha256(const std::vector<uint8_t>& in)
{
    return DoubleSha256(in.data(), in.size());
}

util::Hash256 DoubleSha256(const std::string& in)
{
    return DoubleSha256(reinterpret_cast<const uint8_t*>(in.data()), in.size());;
}

} // namespace hashfuncs


HashOStream::HashOStream()
    : vec_(), byte_sink_(vec_)
{
}

util::Hash256 HashOStream::Sha256()
{     
    return hashfuncs::Sha256(vec_);
}

util::Hash256 HashOStream::DoubleSha256()
{
    return hashfuncs::DoubleSha256(vec_);
}

size_t HashOStream::Size() const
{
    return vec_.size();
}

const HashOStream::Container& HashOStream::vec() const
{
    return vec_;
}

void HashOStream::Clear()
{
    vec_.clear();
}

SipHasher::SipHasher()
    : mac_(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
      rng_(), key_(rng_.random_vec(16)), is_set_key_(false) 
{
}

SipHasher::SipHasher(uint64_t k0, uint64_t k1)
    : mac_(Botan::MessageAuthenticationCode::create_or_throw("SipHash")),
      rng_(), key_(sizeof(k0)+sizeof(k1)), is_set_key_(false) 
{
    std::memcpy(&key_[0], &k0, sizeof(k0));
    std::memcpy(&key_[sizeof(k0)], &k1, sizeof(k1));
}

SipHasher& SipHasher::Update(uint64_t in)
{
    if (!is_set_key_) {
        mac_->set_key(key_);
        is_set_key_ = true;
    }
    mac_->update(reinterpret_cast<const uint8_t*>(&in), sizeof(uint64_t));
    
    return *this;
}

SipHasher& SipHasher::Update(const uint8_t *in, size_t length)
{
    if (!is_set_key_) {
        mac_->set_key(key_);
        is_set_key_ = true;
    }
    mac_->update(in, length);
    
    return *this;
}

uint64_t SipHasher::Final()
{
    uint64_t ret;
    mac_->final(reinterpret_cast<uint8_t *>(&ret));
    is_set_key_ = false;
    
    return ret;
}

} // namespace crypto
} // namespace btclite
