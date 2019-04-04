#ifndef BTCLITE_PEERS_H
#define BTCLITE_PEERS_H

#include "network_address.h"
#include "peers.pb.h"
#include "sync.h"

namespace btclite {

class Peers {
public:
	Peers()
	{}
	~Peers()
	{}
	
	//-------------------------------------------------------------------------
	// Add a single address.
    bool Add(const btclite::NetAddr &addr, const btclite::NetAddr& source, int64_t nTimePenalty = 0);
	
	//! Add multiple addresses.
    bool Add(const std::vector<btclite::NetAddr> &vAddr, const btclite::NetAddr& source, int64_t nTimePenalty = 0);
	
	void Clear();
	
	//-------------------------------------------------------------------------
	// Mark an entry as accessible.
    void Good(const btclite::NetAddr &addr, int64_t nTime);
	
	// Mark an entry as connection attempted to.
    void Attempt(const btclite::NetAddr &addr, bool fCountFailure, int64_t nTime);
	
	// Choose an address to connect to.
    btclite::NetAddr Select(bool newOnly = false);
	
private:
	// critical section to protect the inner data structures
    mutable CriticalSection cs_;
	proto_peers::Peers peers_;
	
	//! Find an entry.
    btclite::NetAddr* Find(const btclite::NetAddr& addr, int *pnId = nullptr);

    // find an entry, creating it if necessary.
    // nTime and nServices of the found node are updated, if necessary.
    btclite::NetAddr* Create(const btclite::NetAddr &addr, const btclite::NetAddr &addrSource, int *pnId = nullptr);

    // Move an entry from the "new" table(s) to the "tried" table
    void MakeTried(btclite::NetAddr& info, int nId);

    // Delete an entry. It must not be in tried, and have refcount 0.
    void Delete(int nId);

    // Clear a position in a "new" table. This is the only place where entries are actually deleted.
    void ClearNew(int nUBucket, int nUBucketPos);

    // Mark an entry "good", possibly moving it from "new" to "tried".
    void Good_(const btclite::NetAddr &addr, int64_t nTime);

    // Add an entry to the "new" table.
    bool Add_(const btclite::NetAddr &addr, const btclite::NetAddr& source, int64_t nTimePenalty);

    // Mark an entry as attempted to connect.
    void Attempt_(const btclite::NetAddr &addr, bool fCountFailure, int64_t nTime);

    // Select an address to connect to, if newOnly is set to true, only the new table is selected from.
    btclite::NetAddr Select_(bool newOnly);
};

}

#endif // BTCLITE_PEERS_H
