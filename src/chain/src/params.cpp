#include "chain/include/params.h"


namespace btclite {
namespace chain {

using namespace util;

Params::Params(BtcNet btcnet)
    : consensus_params_(btcnet) 
{
    switch (btcnet) {
        case BtcNet::kMainNet :
        {
            base58_prefixes_[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,0);
            base58_prefixes_[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
            base58_prefixes_[SECRET_KEY] =     std::vector<unsigned char>(1,128);
            base58_prefixes_[EXT_PUBLIC_KEY] = { 0x04, 0x88, 0xB2, 0x1E };
            base58_prefixes_[EXT_SECRET_KEY] = { 0x04, 0x88, 0xAD, 0xE4 };
            
            bech32_hrp_ = "bc";
            
            checkpoints_ = {                
                { 11111, Hash256("0x0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d")},
                { 33333, Hash256("0x000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6")},
                { 74000, Hash256("0x0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20")},
                {105000, Hash256("0x00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97")},
                {134444, Hash256("0x00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe")},
                {168000, Hash256("0x000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763")},
                {193000, Hash256("0x000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317")},
                {210000, Hash256("0x000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e")},
                {216116, Hash256("0x00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e")},
                {225430, Hash256("0x00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932")},
                {250000, Hash256("0x000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214")},
                {279000, Hash256("0x0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40")},
                {295000, Hash256("0x00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983")},                
            };
            
            // Data as of block 0000000000000000002d6cca6761c99b3c2e936f9a0e304b7c7651a993f461de 
            // (height 506081).
            chain_tx_data_ = ChainTxData{ 1516903077, 295363220, 3.5 };
            
            break;
        }
        case BtcNet::kTestNet :
        {
            base58_prefixes_[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
            base58_prefixes_[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
            base58_prefixes_[SECRET_KEY] =     std::vector<unsigned char>(1,239);
            base58_prefixes_[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
            base58_prefixes_[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
            
            bech32_hrp_ = "tb";
            
            checkpoints_ = { {546, Hash256("000000002a936ca763904c3c35fce2f3556c559c0214345d31b1bcebf76acb70")} };
            
            // Data as of block 000000000000033cfa3c975eb83ecf2bb4aaedf68e6d279f6ed2b427c64caff9 (height 1260526)
            chain_tx_data_ = ChainTxData{ 1516903490, 17082348, 0.09 };
            
            break;
        }
        case BtcNet::kRegTest :
        {
            base58_prefixes_[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
            base58_prefixes_[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
            base58_prefixes_[SECRET_KEY] =     std::vector<unsigned char>(1,239);
            base58_prefixes_[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
            base58_prefixes_[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
            
            bech32_hrp_ = "bcrt";
            
            checkpoints_ = { {0, Hash256("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")} };
            chain_tx_data_ = ChainTxData{ 0, 0, 0 };
            
            break;
        }
        default :
            break;
    }
}

} // namespace chain

} // namespace btclite
