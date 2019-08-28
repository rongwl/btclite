#include "hash.h"


void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in, length);
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in);
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

void DoubleSha256(const std::string &in, Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(in);
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

void HashWStream::Sha256(Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(vec_);
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
}

void HashWStream::DoubleSha256(Hash256 *out)
{
    std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
    hash_func->update(vec_);
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
    hash_func->clear();
    hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
    out->Clear();
    hash_func->final(reinterpret_cast<uint8_t*>(out));
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

