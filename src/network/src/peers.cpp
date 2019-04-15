#include "peers.h"


bool btclite::Peers::Add(const btclite::NetAddr &addr, const btclite::NetAddr& source, int64_t nTimePenalty)
{
    return true;
}

bool btclite::Peers::Add(const std::vector<btclite::NetAddr> &vAddr, const btclite::NetAddr& source, int64_t nTimePenalty)
{
    return true;
}

void btclite::Peers::Clear()
{

}

void btclite::Peers::Good(const btclite::NetAddr &addr, int64_t nTime)
{

}

void btclite::Peers::Attempt(const btclite::NetAddr &addr, bool fCountFailure, int64_t nTime)
{

}

btclite::NetAddr btclite::Peers::Select(bool newOnly)
{
    btclite::NetAddr();
}

btclite::NetAddr* btclite::Peers::Find(const btclite::NetAddr& addr, int *pnId)
{
    return nullptr;
}

btclite::NetAddr* btclite::Peers::Create(const btclite::NetAddr &addr, const btclite::NetAddr &addrSource, int *pnId)
{
    return nullptr;
}

void btclite::Peers::MakeTried(btclite::NetAddr& info, int nId)
{

}

void btclite::Peers::Delete(int nId)
{

}

void btclite::Peers::ClearNew(int nUBucket, int nUBucketPos)
{

}

void btclite::Peers::Good_(const btclite::NetAddr &addr, int64_t nTime)
{

}

bool btclite::Peers::Add_(const btclite::NetAddr &addr, const btclite::NetAddr& source, int64_t nTimePenalty)
{
    return true;
}

void btclite::Peers::Attempt_(const btclite::NetAddr &addr, bool fCountFailure, int64_t nTime)
{

}

btclite::NetAddr btclite::Peers::Select_(bool newOnly)
{
    return btclite::NetAddr();
}
