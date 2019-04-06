#include "hash.h"

void DoubleSha256(const uint8_t in[], size_t length, Hash256 *out)
{
	std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
	hash_func->update(in, length);
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	hash_func->clear();
	hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	std::reverse(out->begin(), out->end()); // from big endian to little endian
}

void DoubleSha256(const std::vector<uint8_t> &in, Hash256 *out)
{
	std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
	hash_func->update(in);
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	hash_func->clear();
	hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	std::reverse(out->begin(), out->end()); // from big endian to little endian
}

void DoubleSha256(const std::string &in, Hash256 *out)
{
	std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
	hash_func->update(in);
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	hash_func->clear();
	hash_func->update(reinterpret_cast<const uint8_t*>(out), out->size());
	out->SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(out));
	std::reverse(out->begin(), out->end()); // from big endian to little endian
}

SipHasher& SipHasher::Update(uint64_t in)
{
	mac->set_key(key);
	
	return *this;
}

SipHasher& SipHasher::Update(const uint8_t *in, size_t length)
{
	mac->set_key(key);
	
	return *this;
}

uint64_t SipHasher::Final()
{
	return 0;
}

