#include "hash.h"


namespace btclite {
namespace crypto {

SHA256& SHA256::Write(const uint8_t *data, size_t len)
{
    const uint8_t* end = data + len;
    size_t bufsize = bytes_ % 64;
    if (bufsize && bufsize + len >= 64) {
        // Fill the buffer, and process it.
        memcpy(buf_ + bufsize, data, 64 - bufsize);
        bytes_ += 64 - bufsize;
        data += 64 - bufsize;
        Transform(buf_, 1);
        bufsize = 0;
    }
    if (end - data >= 64) {
        size_t blocks = (end - data) / 64;
        Transform(data, blocks);
        data += 64 * blocks;
        bytes_ += 64 * blocks;
    }
    if (end > data) {
        // Fill the buffer with what remains.
        memcpy(buf_ + bufsize, data, end - data);
        bytes_ += end - data;
    }
    return *this;
}

void SHA256::Finalize(uint8_t out[OUTPUT_SIZE])
{
    static const uint8_t pad[64] = {0x80};
    uint8_t sizedesc[8];
    
    util::ToBigEndian(static_cast<uint64_t>(bytes_ << 3), sizedesc);
    Write(pad, 1 + ((119 - (bytes_ % 64)) % 64));
    Write(sizedesc, sizeof(sizedesc));
    util::ToBigEndian(s_[0], out);
    util::ToBigEndian(s_[1], out+4);
    util::ToBigEndian(s_[2], out+8);
    util::ToBigEndian(s_[3], out+12);
    util::ToBigEndian(s_[4], out+16);
    util::ToBigEndian(s_[5], out+20);
    util::ToBigEndian(s_[6], out+24);
    util::ToBigEndian(s_[7], out+28);
}

void SHA256::Transform(const uint8_t *chunk, size_t blocks)
{
    while (blocks--) {
        uint32_t a = s_[0], b = s_[1], c = s_[2], d = s_[3], e = s_[4], f = s_[5], g = s_[6], h = s_[7];
        uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

        Round(a, b, c, d, e, f, g, h, 0x428a2f98, w0 = util::FromBigEndian<uint32_t>(chunk + 0));
        Round(h, a, b, c, d, e, f, g, 0x71374491, w1 = util::FromBigEndian<uint32_t>(chunk + 4));
        Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf, w2 = util::FromBigEndian<uint32_t>(chunk + 8));
        Round(f, g, h, a, b, c, d, e, 0xe9b5dba5, w3 = util::FromBigEndian<uint32_t>(chunk + 12));
        Round(e, f, g, h, a, b, c, d, 0x3956c25b, w4 = util::FromBigEndian<uint32_t>(chunk + 16));
        Round(d, e, f, g, h, a, b, c, 0x59f111f1, w5 = util::FromBigEndian<uint32_t>(chunk + 20));
        Round(c, d, e, f, g, h, a, b, 0x923f82a4, w6 = util::FromBigEndian<uint32_t>(chunk + 24));
        Round(b, c, d, e, f, g, h, a, 0xab1c5ed5, w7 = util::FromBigEndian<uint32_t>(chunk + 28));
        Round(a, b, c, d, e, f, g, h, 0xd807aa98, w8 = util::FromBigEndian<uint32_t>(chunk + 32));
        Round(h, a, b, c, d, e, f, g, 0x12835b01, w9 = util::FromBigEndian<uint32_t>(chunk + 36));
        Round(g, h, a, b, c, d, e, f, 0x243185be, w10 = util::FromBigEndian<uint32_t>(chunk + 40));
        Round(f, g, h, a, b, c, d, e, 0x550c7dc3, w11 = util::FromBigEndian<uint32_t>(chunk + 44));
        Round(e, f, g, h, a, b, c, d, 0x72be5d74, w12 = util::FromBigEndian<uint32_t>(chunk + 48));
        Round(d, e, f, g, h, a, b, c, 0x80deb1fe, w13 = util::FromBigEndian<uint32_t>(chunk + 52));
        Round(c, d, e, f, g, h, a, b, 0x9bdc06a7, w14 = util::FromBigEndian<uint32_t>(chunk + 56));
        Round(b, c, d, e, f, g, h, a, 0xc19bf174, w15 = util::FromBigEndian<uint32_t>(chunk + 60));

        Round(a, b, c, d, e, f, g, h, 0xe49b69c1, w0 += sigma1(w14) + w9 + sigma0(w1));
        Round(h, a, b, c, d, e, f, g, 0xefbe4786, w1 += sigma1(w15) + w10 + sigma0(w2));
        Round(g, h, a, b, c, d, e, f, 0x0fc19dc6, w2 += sigma1(w0) + w11 + sigma0(w3));
        Round(f, g, h, a, b, c, d, e, 0x240ca1cc, w3 += sigma1(w1) + w12 + sigma0(w4));
        Round(e, f, g, h, a, b, c, d, 0x2de92c6f, w4 += sigma1(w2) + w13 + sigma0(w5));
        Round(d, e, f, g, h, a, b, c, 0x4a7484aa, w5 += sigma1(w3) + w14 + sigma0(w6));
        Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc, w6 += sigma1(w4) + w15 + sigma0(w7));
        Round(b, c, d, e, f, g, h, a, 0x76f988da, w7 += sigma1(w5) + w0 + sigma0(w8));
        Round(a, b, c, d, e, f, g, h, 0x983e5152, w8 += sigma1(w6) + w1 + sigma0(w9));
        Round(h, a, b, c, d, e, f, g, 0xa831c66d, w9 += sigma1(w7) + w2 + sigma0(w10));
        Round(g, h, a, b, c, d, e, f, 0xb00327c8, w10 += sigma1(w8) + w3 + sigma0(w11));
        Round(f, g, h, a, b, c, d, e, 0xbf597fc7, w11 += sigma1(w9) + w4 + sigma0(w12));
        Round(e, f, g, h, a, b, c, d, 0xc6e00bf3, w12 += sigma1(w10) + w5 + sigma0(w13));
        Round(d, e, f, g, h, a, b, c, 0xd5a79147, w13 += sigma1(w11) + w6 + sigma0(w14));
        Round(c, d, e, f, g, h, a, b, 0x06ca6351, w14 += sigma1(w12) + w7 + sigma0(w15));
        Round(b, c, d, e, f, g, h, a, 0x14292967, w15 += sigma1(w13) + w8 + sigma0(w0));

        Round(a, b, c, d, e, f, g, h, 0x27b70a85, w0 += sigma1(w14) + w9 + sigma0(w1));
        Round(h, a, b, c, d, e, f, g, 0x2e1b2138, w1 += sigma1(w15) + w10 + sigma0(w2));
        Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc, w2 += sigma1(w0) + w11 + sigma0(w3));
        Round(f, g, h, a, b, c, d, e, 0x53380d13, w3 += sigma1(w1) + w12 + sigma0(w4));
        Round(e, f, g, h, a, b, c, d, 0x650a7354, w4 += sigma1(w2) + w13 + sigma0(w5));
        Round(d, e, f, g, h, a, b, c, 0x766a0abb, w5 += sigma1(w3) + w14 + sigma0(w6));
        Round(c, d, e, f, g, h, a, b, 0x81c2c92e, w6 += sigma1(w4) + w15 + sigma0(w7));
        Round(b, c, d, e, f, g, h, a, 0x92722c85, w7 += sigma1(w5) + w0 + sigma0(w8));
        Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1, w8 += sigma1(w6) + w1 + sigma0(w9));
        Round(h, a, b, c, d, e, f, g, 0xa81a664b, w9 += sigma1(w7) + w2 + sigma0(w10));
        Round(g, h, a, b, c, d, e, f, 0xc24b8b70, w10 += sigma1(w8) + w3 + sigma0(w11));
        Round(f, g, h, a, b, c, d, e, 0xc76c51a3, w11 += sigma1(w9) + w4 + sigma0(w12));
        Round(e, f, g, h, a, b, c, d, 0xd192e819, w12 += sigma1(w10) + w5 + sigma0(w13));
        Round(d, e, f, g, h, a, b, c, 0xd6990624, w13 += sigma1(w11) + w6 + sigma0(w14));
        Round(c, d, e, f, g, h, a, b, 0xf40e3585, w14 += sigma1(w12) + w7 + sigma0(w15));
        Round(b, c, d, e, f, g, h, a, 0x106aa070, w15 += sigma1(w13) + w8 + sigma0(w0));

        Round(a, b, c, d, e, f, g, h, 0x19a4c116, w0 += sigma1(w14) + w9 + sigma0(w1));
        Round(h, a, b, c, d, e, f, g, 0x1e376c08, w1 += sigma1(w15) + w10 + sigma0(w2));
        Round(g, h, a, b, c, d, e, f, 0x2748774c, w2 += sigma1(w0) + w11 + sigma0(w3));
        Round(f, g, h, a, b, c, d, e, 0x34b0bcb5, w3 += sigma1(w1) + w12 + sigma0(w4));
        Round(e, f, g, h, a, b, c, d, 0x391c0cb3, w4 += sigma1(w2) + w13 + sigma0(w5));
        Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a, w5 += sigma1(w3) + w14 + sigma0(w6));
        Round(c, d, e, f, g, h, a, b, 0x5b9cca4f, w6 += sigma1(w4) + w15 + sigma0(w7));
        Round(b, c, d, e, f, g, h, a, 0x682e6ff3, w7 += sigma1(w5) + w0 + sigma0(w8));
        Round(a, b, c, d, e, f, g, h, 0x748f82ee, w8 += sigma1(w6) + w1 + sigma0(w9));
        Round(h, a, b, c, d, e, f, g, 0x78a5636f, w9 += sigma1(w7) + w2 + sigma0(w10));
        Round(g, h, a, b, c, d, e, f, 0x84c87814, w10 += sigma1(w8) + w3 + sigma0(w11));
        Round(f, g, h, a, b, c, d, e, 0x8cc70208, w11 += sigma1(w9) + w4 + sigma0(w12));
        Round(e, f, g, h, a, b, c, d, 0x90befffa, w12 += sigma1(w10) + w5 + sigma0(w13));
        Round(d, e, f, g, h, a, b, c, 0xa4506ceb, w13 += sigma1(w11) + w6 + sigma0(w14));
        Round(c, d, e, f, g, h, a, b, 0xbef9a3f7, w14 + sigma1(w12) + w7 + sigma0(w15));
        Round(b, c, d, e, f, g, h, a, 0xc67178f2, w15 + sigma1(w13) + w8 + sigma0(w0));

        s_[0] += a;
        s_[1] += b;
        s_[2] += c;
        s_[3] += d;
        s_[4] += e;
        s_[5] += f;
        s_[6] += g;
        s_[7] += h;
        chunk += 64;
    }
}


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
