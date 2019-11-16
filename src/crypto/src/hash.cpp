#include "hash.h"


namespace btclite {
namespace crypto {
namespace hash {

void Sha256(const uint8_t in[], size_t length, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in, length);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 Sha256(const uint8_t in[], size_t length)
{
    Hash256 hash;
    
    Sha256(in, length, &hash);
    
    return hash;
}

void Sha256(const std::vector<uint8_t>& in, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 Sha256(const std::vector<uint8_t>& in)
{
    Hash256 hash;
    
    Sha256(in, &hash);
    
    return hash;
}

void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in, length);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 DoubleSha256(const uint8_t in[], size_t length)
{
    Hash256 hash;
    
    DoubleSha256(in, length, &hash);
    
    return hash;
}

void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 DoubleSha256(const std::vector<uint8_t> &in)
{
    Hash256 hash;
    
    DoubleSha256(in, &hash);
    
    return hash;
}

void DoubleSha256(const std::string &in, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 DoubleSha256(const std::string &in)
{
    Hash256 hash;
    
    DoubleSha256(in, &hash);
    
    return hash;
}

} // namespace hash
} // namespace crypto
} // namespace btclite

void HashOStream::Sha256(Hash256 *out) const
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(vec_);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 HashOStream::Sha256() const
{
    Hash256 hash;
    
    Sha256(&hash);
    
    return hash;
}

void HashOStream::DoubleSha256(Hash256 *out) const
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(vec_);
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

Hash256 HashOStream::DoubleSha256() const
{
    Hash256 hash;
    
    DoubleSha256(&hash);
    
    return hash;
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

