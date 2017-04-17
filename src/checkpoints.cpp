// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "txdb.h"
#include "main.h"
#include "uint256.h"


static const int nCheckpointSpan = 10;

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    //
    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    //
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        ( 0,      uint256("0x00000ac7d764e7119da60d3c832b1d4458da9bc9ef9d5dd0d91a15f690a46d99") )  // hashGenesisBlock
		( 1,      uint256("0x0000017759b8575c7190e664ce9bb5a82bc678e0f226ad130759e43c14a83b35"))
		( 2,      uint256("0x10eba14c001a5fa2464832760bb66f41c31f055bf2e1dccba0e2046e69c81d31"))
		( 10,     uint256("0x565224cacb31625ad815d445f9c36b003d0ee6a4e4505b05c39837de88b8a20c"))
		( 20,     uint256("0x8bc4daca2cb78e35a66cb0d761fe34d9230eef05984d285773755578a359ed82"))
		( 1000,   uint256("0x78977497cfdad7b5d5ec381ddf15f8ce2298ac286c9cc2aa4a7c766bc1e89a94"))
		( 2000,   uint256("0xf4c10af07e7e8b94be2fd7b8a7d367b83b793a93e4e8b8c7083b6b1a87cf2cd0"))
		( 4000,   uint256("0x6b5211171399027ae444712519f42b20ed2e0a3d9305797f74c515374ca931db"))
		( 8000,   uint256("0x4c562b38c8c4d87b336c8d28055920a52f64dea3eacf1b932974dd6579b26ae9"))
		( 12000,   uint256("0x5ed556c6f6d173b3a6ad1ed63dc10d94036fb2cbcd5f1e5cbb27a0dfea6e2fcb"))
		( 16000,   uint256("0x3200daf45480aa4cced79b68c54486755e4b59075d8d3b7ef26153c81af1e1e0"))
		( 20000,   uint256("0x70e7dc00555bf59616fb0b8a707a08a4b0d5dd192694708e71b23a6423e21a6e"))
		( 25000,   uint256("0xdd0a3ab0f69b8eefb9641c254be2087610a3445602007e3ff458e9b8226a4e85"))
		( 30000,   uint256("0x33e0d940a1dd6baed53b17f7dc59cf312cdc6b160b7d33ade37cc3430ff511d8"))
		( 35000,   uint256("0xf17c97c47c99ccc093b5adf7a5275f6244260299e2eb57bdc59322a4235dfeef"))
		( 40000,   uint256("0x9ae3c2f7349d2917d31c62a547d4f659263e67f41d24b336a8b282980ae84056"))
		( 50000,   uint256("0xa119d2b0fedd1017fff012780fa406f1462ef7027dcb0ad46bc80a68fa6ef9c0"))
		( 75000,   uint256("0xaf42c5638688a6032d0c6004da6c9b788395bb37527a44ba15c6186c2dfa009c"))
		( 100000,   uint256("0x35b3f3084d49fd9736afe3b5ba7db1665526cec1f904ac6aff801375ca575484"))
		( 125000,   uint256("0xcee5ea23bf48cf75e7a99602703f067c88b7cfb2e06e3d35893755e00396f5a4"))
		( 150000,   uint256("0x3a2313e6a1883c8c8ab2b7c8186bdb65ed2d44ab74486bf6a089111142cba47e"))
		( 175000,   uint256("0x2594bd804fe437eff75872d00ef7ef095652077a7c734d1c41f77782d158daff"))
		( 200000,   uint256("0x37d597d7287752a156f416890a35dbefc3432dc786c9a82c8c072c4960c49f8f"))
		( 225000,   uint256("0x33b3f4b1c6c8edbbbade0828664927356ba180a6030aac78322e0dea027f5c50"))
		( 250000,   uint256("0xd7920231e8b04dea477e0095bf6de5e1da0887bc8c3b1e88d89a80e2b535bb11"))
		( 275000,   uint256("0xe824d002d31a14730ffdf7e8ef17718d45ef60f128476540a8b013141e22d0e0"))
		( 300000,   uint256("0xdb97bfe515debe7ebaab356ea45ec50f0d774ba3b39b5e6bf220006a4f445ecd"))
		( 325000,   uint256("0x823e35b41b1256bc70dad3f37eb1460e9f5025a0a92df916b8924ca7653a2311"))
		( 350000,   uint256("0xc3dc3fd261ba35a3699623b6667465f989c4aed76c5dbaacd5b54c40f37bf82c"))
		( 375000,   uint256("0xed52a1c784219fda8d5b15eedc1ba7cc8c0c69fae9beea6018dde58ac3ff39c7"))
		( 400000,   uint256("0xf1081ae3713cc18e9fc3daff5a72827b1b49978f8504d41c52924b759136e4bb")) 
		( 425000,   uint256("0x7a32a7d364a91361932c74a7965de096b7f6a0b746422065126ce0f1984f079d")) 
		( 450000,   uint256("0x20bf4ec2917859cb891848589b95d674d34a55029d69cbf4bce54a97b61cad55"))
		( 475000,   uint256("0x098a54ca0e397ffb9886ef3041300d20663bde9acc4e7ada4e2958aaf474e8c7"))
		( 500000,   uint256("0x075a2b612793fac3ecdf4af678e04abc8079b0c7a31b9407c7b43f8231470c25"))
		( 525000,   uint256("0x00343be8e7ceec9368cd61494d3477b89a2cb1c38c6e9ef9b27b77be3ca30ec1"))
		( 550000,   uint256("0x1c7372416f7d20ae57f95bcb89f439377310c6303348c304016800b43e3258f4"))
		( 575000,   uint256("0xced3c637a7b098c47d7e60bb7c126548c4f407910d01c8ec15c10d0f3492df0d"))
		( 600000,   uint256("0x7d7f9ed5107a325f8f913698fc64e87b9acdffa356536ff81ba89d6441debf7a"))
		( 625000,   uint256("0x61ebd7b2118ab921b900b0ff2bc19b24d444c3876afbf5ebde467332c1886d70"))
		( 650000,   uint256("0xba0173aaf192eb2d39634a58c8cb0b9e596784da8f7cdfc8f29f15a6549fb401"))
		( 675000,   uint256("0x48d1bb57d9baf8541a49883cc0369a022df0574cc744479f72236c4a87554eb9"))
		( 700000,   uint256("0x9127fd471b9db479508db8ca2c05458e5d10a3b67bdc330b8a140ba86cc3a712"))
		( 725000,   uint256("0xf69dac0c8b7791d8c5d9a1f7c4b6dc109757e9ad532d1dcd803959b09f219ca7"))
		( 750000,   uint256("0x2dec9b2d07785918cfe1a31d2b32b68fa530f004630697d2641e92bfc373f8fc"))
		( 800000,   uint256("0x69724787cfb8a349e19f124ce89efe786076fc63162b948b2ccec80792dbb65a"))
		( 900000,   uint256("0xf82e73ad0e3ce041a4fe2ec3a31e35d1af35d156180d176281fbf85092da5af4"))
		( 1000000,   uint256("0xfd83b59c9c3f611a1fabd552bf41ce9d82da54212e1c71775169aabff0acfb4e"))
		( 1100000,   uint256("0xa0afd30f9435ed844cb52afd6ce7820a01c637e74d79fd29309c84312504b104"))
		( 1200000,   uint256("0x6f03cb287986809e5b3df11e6d368a8277a89601665577de5135e2e3e365b9a0"))
		( 1205000,   uint256("0x94833548583b5ea19b2321ee2752514698c53b5dd9304cf0237e0cd45f24e796"))
		( 1209300,   uint256("0x0a79b1f3145bceea4b1eba70a5989c31637e1b5ca3c0573bf67e9b12417a68ff"))
		( 1210300,   uint256("0x7a1ce85b0c4f995caedae5b4cd3a944b4c0b788fec5a5789f79b76a62b1fcbcf"))
		( 1215000,   uint256("0x9d39587c1d35fecac7370df0aeab4059466fdef83427e5f609548c939d624651"))
		( 1218000,   uint256("0xa65c0c2745d311c09d001d15a27dc6ab0e24f9ad3611901fae02d97a03df6f8d"))
    ;

    // TestNet has no checkpoints
    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        ( 0, hashGenesisBlockTestNet )
		/*( 1,      uint256("0x000003f85af837f52e8520b53c13563af57e555b9e1003428acf5c2a2b71b76d"))
		( 10,     uint256("0xce8b138a8f5a07815b09bb982eeff2c0b43ef694f5eff3b23ab52ff9cc3681ca"))
		( 20,     uint256("0x14d03bdf522e9fa92852539d3c5ebee178ef02c17c10a8e33d7b8db83cc63cb3")) */
        ;

    bool CheckHardened(int nHeight, const uint256& hash)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    int GetTotalBlocksEstimate()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->first;
    }
	

    uint256 GetTotalBlocksEstimateHash()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.rbegin()->second;
    }
	int insertToCheckpoints(int idx, uint256 hash)
	{
		//std::map<int, uint256> map_W;
		MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);
		//checkpoints.insert(idx, hash);
		return checkpoints.size();
	}
	
    bool IsCheckpoint(uint256 hash)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            if(i.second == hash)
            {
                return true;
            }
        }
        return false;
    }

    void LoadCheckpoints(CNode* pfrom)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            if(i.second != hashGenesisBlock)
            {
                if( !mapOrphanBlocks.count(i.second) && !mapBlockIndex.count(i.second) )
                {
                    pfrom->AskFor(CInv(pfrom->zipblock > 0 ? MSG_BLKZP : MSG_BLOCK, i.second));
                }
            }
        }
    }

    int GetNumCheckpoints()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        return checkpoints.size();
    }

    int GetNumLoadedCheckpoints()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        int Ret=0;
        MapCheckpoints::const_iterator i = checkpoints.begin();
        for(;i != checkpoints.end();i++)
        {
            if( mapOrphanBlocks.count(i->second) || mapBlockIndex.count(i->second) )
            {
                Ret++;
            }
        }
        return Ret;
    }

    void InsertPlaceHoldersBetweenCheckpoints()
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        MapCheckpoints::const_iterator i = checkpoints.begin();
        int64_t prevIndex=0;
        uint256 prevHash = 0;
        while(i != checkpoints.end())
        {
            if (mapOrphanBlocks.count(i->second))
            {
                CBlock placeHolderBlock;
                //placeHolderBlock.hashOverride = mapOrphanBlocks[i->second]->hashPrevBlock;
                placeHolderBlock.hashPrevBlock = prevHash;
                //placeHolderBlock.numPlacesHeld = i->first - prevIndex;
                //placeHolderBlock.headerOnly = true;
                //placeHolderBlock.placeHolderBlock = true;
                ProcessBlock(NULL, &placeHolderBlock);
            }
            prevHash = i->second;
            prevIndex = i->first;
            // Check if already in tree - when loading an existing block index this may be the case.
            if (mapBlockIndex.count(prevHash))
            {
                prevHash = pindexBest->GetBlockHash();
                prevIndex = pindexBest->nHeight;
            }

            i++;
        }
    }
	
    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        MapCheckpoints& checkpoints = (fTestNet ? mapCheckpointsTestnet : mapCheckpoints);

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }

    // ppcoin: synchronized checkpoint (centrally broadcasted)
    uint256 hashSyncCheckpoint = 0;
    uint256 hashPendingCheckpoint = 0;
    CSyncCheckpoint checkpointMessage;
    CSyncCheckpoint checkpointMessagePending;
    uint256 hashInvalidCheckpoint = 0;
    CCriticalSection cs_hashSyncCheckpoint;

    // ppcoin: get last synchronized checkpoint
    CBlockIndex* GetLastSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            error("GetSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        else
            return mapBlockIndex[hashSyncCheckpoint];
        return NULL;
    }

    // ppcoin: only descendant of current sync-checkpoint is allowed
    bool ValidateSyncCheckpoint(uint256 hashCheckpoint)
    {
        if (!mapBlockIndex.count(hashSyncCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for current sync-checkpoint %s", hashSyncCheckpoint.ToString().c_str());
        if (!mapBlockIndex.count(hashCheckpoint))
            return error("ValidateSyncCheckpoint: block index missing for received sync-checkpoint %s", hashCheckpoint.ToString().c_str());

        CBlockIndex* pindexSyncCheckpoint = mapBlockIndex[hashSyncCheckpoint];
        CBlockIndex* pindexCheckpointRecv = mapBlockIndex[hashCheckpoint];

        if (pindexCheckpointRecv->nHeight <= pindexSyncCheckpoint->nHeight)
        {
            // Received an older checkpoint, trace back from current checkpoint
            // to the same height of the received checkpoint to verify
            // that current checkpoint should be a descendant block
            CBlockIndex* pindex = pindexSyncCheckpoint;
            while (pindex->nHeight > pindexCheckpointRecv->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("ValidateSyncCheckpoint: pprev null - block index structure failure");
            if (pindex->GetBlockHash() != hashCheckpoint)
            {
                hashInvalidCheckpoint = hashCheckpoint;
                return error("ValidateSyncCheckpoint: new sync-checkpoint %s is conflicting with current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
            }
            return false; // ignore older checkpoint
        }

        // Received checkpoint should be a descendant block of the current
        // checkpoint. Trace back to the same height of current checkpoint
        // to verify.
        CBlockIndex* pindex = pindexCheckpointRecv;
        while (pindex->nHeight > pindexSyncCheckpoint->nHeight)
            if (!(pindex = pindex->pprev))
                return error("ValidateSyncCheckpoint: pprev2 null - block index structure failure");
        if (pindex->GetBlockHash() != hashSyncCheckpoint)
        {
            hashInvalidCheckpoint = hashCheckpoint;
            return error("ValidateSyncCheckpoint: new sync-checkpoint %s is not a descendant of current sync-checkpoint %s", hashCheckpoint.ToString().c_str(), hashSyncCheckpoint.ToString().c_str());
        }
        return true;
    }

    bool WriteSyncCheckpoint(const uint256& hashCheckpoint)
    {
        CTxDB txdb;
        txdb.TxnBegin();
        if (!txdb.WriteSyncCheckpoint(hashCheckpoint))
        {
            txdb.TxnAbort();
            return error("WriteSyncCheckpoint(): failed to write to db sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
        if (!txdb.TxnCommit())
            return error("WriteSyncCheckpoint(): failed to commit to db sync checkpoint %s", hashCheckpoint.ToString().c_str());

        Checkpoints::hashSyncCheckpoint = hashCheckpoint;
        return true;
    }

    bool AcceptPendingSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint != 0 && mapBlockIndex.count(hashPendingCheckpoint))
        {
            if (!ValidateSyncCheckpoint(hashPendingCheckpoint))
            {
                hashPendingCheckpoint = 0;
                checkpointMessagePending.SetNull();
                return false;
            }

            CTxDB txdb;
            CBlockIndex* pindexCheckpoint = mapBlockIndex[hashPendingCheckpoint];
            if (!pindexCheckpoint->IsInMainChain())
            {
                CBlock block;
                if (!block.ReadFromDisk(pindexCheckpoint))
                    return error("AcceptPendingSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                if (!block.SetBestChain(txdb, pindexCheckpoint))
                {
                    hashInvalidCheckpoint = hashPendingCheckpoint;
                    return error("AcceptPendingSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
                }
            }

            if (!WriteSyncCheckpoint(hashPendingCheckpoint))
                return error("AcceptPendingSyncCheckpoint(): failed to write sync checkpoint %s", hashPendingCheckpoint.ToString().c_str());
            hashPendingCheckpoint = 0;
            checkpointMessage = checkpointMessagePending;
            checkpointMessagePending.SetNull();
            printf("AcceptPendingSyncCheckpoint : sync-checkpoint at %s\n", hashSyncCheckpoint.ToString().c_str());
            // relay the checkpoint
            if (!checkpointMessage.IsNull())
            {
                BOOST_FOREACH(CNode* pnode, vNodes)
                    checkpointMessage.RelayTo(pnode);
            }
            return true;
        }
        return false;
    }

    // Automatically select a suitable sync-checkpoint 
    uint256 AutoSelectSyncCheckpoint()
    {
        const CBlockIndex *pindex = pindexBest;
        // Search backward for a block within max span and maturity window
        while (pindex->pprev && (pindex->GetBlockTime() + nCheckpointSpan * GetTargetSpacing(nBestHeight) > pindexBest->GetBlockTime() || pindex->nHeight + nCheckpointSpan > pindexBest->nHeight))
            pindex = pindex->pprev;
        return pindex->GetBlockHash();
    }

    // Check against synchronized checkpoint
    bool CheckSync(const uint256& hashBlock, const CBlockIndex* pindexPrev)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        int nHeight = pindexPrev->nHeight + 1;

        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

        if (nHeight > pindexSync->nHeight)
        {
            // trace back to same height as sync-checkpoint
            const CBlockIndex* pindex = pindexPrev;
            while (pindex->nHeight > pindexSync->nHeight)
                if (!(pindex = pindex->pprev))
                    return error("CheckSync: pprev null - block index structure failure");
            if (pindex->nHeight < pindexSync->nHeight || pindex->GetBlockHash() != hashSyncCheckpoint)
                return false; // only descendant of sync-checkpoint can pass check
        }
        if (nHeight == pindexSync->nHeight && hashBlock != hashSyncCheckpoint)
            return false; // same height with sync-checkpoint
        if (nHeight < pindexSync->nHeight && !mapBlockIndex.count(hashBlock))
            return false; // lower height than sync-checkpoint
        return true;
    }

    bool WantedByPendingSyncCheckpoint(uint256 hashBlock)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (hashPendingCheckpoint == 0)
            return false;
        if (hashBlock == hashPendingCheckpoint)
            return true;
        if (mapOrphanBlocks.count(hashPendingCheckpoint) 
            && hashBlock == WantedByOrphan(mapOrphanBlocks[hashPendingCheckpoint]))
            return true;
        return false;
    }

    // ppcoin: reset synchronized checkpoint to last hardened checkpoint
    bool ResetSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        const uint256& hash = mapCheckpoints.rbegin()->second;
        if (mapBlockIndex.count(hash) && !mapBlockIndex[hash]->IsInMainChain())
        {
            // checkpoint block accepted but not yet in main chain
            printf("ResetSyncCheckpoint: SetBestChain to hardened checkpoint %s\n", hash.ToString().c_str());
            CTxDB txdb;
            CBlock block;
            if (!block.ReadFromDisk(mapBlockIndex[hash]))
                return error("ResetSyncCheckpoint: ReadFromDisk failed for hardened checkpoint %s", hash.ToString().c_str());
            if (!block.SetBestChain(txdb, mapBlockIndex[hash]))
            {
                return error("ResetSyncCheckpoint: SetBestChain failed for hardened checkpoint %s", hash.ToString().c_str());
            }
        }
        else if(!mapBlockIndex.count(hash))
        {
            // checkpoint block not yet accepted
            hashPendingCheckpoint = hash;
            checkpointMessagePending.SetNull();
            printf("ResetSyncCheckpoint: pending for sync-checkpoint %s\n", hashPendingCheckpoint.ToString().c_str());
        }

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, mapCheckpoints)
        {
            const uint256& hash = i.second;
            if (mapBlockIndex.count(hash) && mapBlockIndex[hash]->IsInMainChain())
            {
                if (!WriteSyncCheckpoint(hash))
                    return error("ResetSyncCheckpoint: failed to write sync checkpoint %s", hash.ToString().c_str());
                printf("ResetSyncCheckpoint: sync-checkpoint reset to %s\n", hashSyncCheckpoint.ToString().c_str());
                return true;
            }
        }

        return false;
    }

    void AskForPendingSyncCheckpoint(CNode* pfrom)
    {
        LOCK(cs_hashSyncCheckpoint);
        if (pfrom && hashPendingCheckpoint != 0 && (!mapBlockIndex.count(hashPendingCheckpoint)) && (!mapOrphanBlocks.count(hashPendingCheckpoint)))
            pfrom->AskFor(CInv(pfrom->zipblock > 0 ? MSG_BLKZP : MSG_BLOCK, hashPendingCheckpoint));
    }

    bool SetCheckpointPrivKey(std::string strPrivKey)
    {
        // Test signing a sync-checkpoint with genesis block
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = !fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        std::vector<unsigned char> vchPrivKey = ParseHex(strPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end())); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return false;

        // Test signing successful, proceed
        CSyncCheckpoint::strMasterPrivKey = strPrivKey;
        return true;
    }

    bool SendSyncCheckpoint(uint256 hashCheckpoint)
    {
        CSyncCheckpoint checkpoint;
        checkpoint.hashCheckpoint = hashCheckpoint;
        CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
        sMsg << (CUnsignedSyncCheckpoint)checkpoint;
        checkpoint.vchMsg = std::vector<unsigned char>(sMsg.begin(), sMsg.end());

        if (CSyncCheckpoint::strMasterPrivKey.empty())
            return error("SendSyncCheckpoint: Checkpoint master key unavailable.");
        std::vector<unsigned char> vchPrivKey = ParseHex(CSyncCheckpoint::strMasterPrivKey);
        CKey key;
        key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end())); // if key is not correct openssl may crash
        if (!key.Sign(Hash(checkpoint.vchMsg.begin(), checkpoint.vchMsg.end()), checkpoint.vchSig))
            return error("SendSyncCheckpoint: Unable to sign checkpoint, check private key?");

        if(!checkpoint.ProcessSyncCheckpoint(NULL))
        {
            printf("WARNING: SendSyncCheckpoint: Failed to process checkpoint.\n");
            return false;
        }

        // Relay checkpoint
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                checkpoint.RelayTo(pnode);
        }
        return true;
    }

    // Is the sync-checkpoint outside maturity window?
    bool IsMatureSyncCheckpoint()
    {
        LOCK(cs_hashSyncCheckpoint);
        // sync-checkpoint should always be accepted block
        assert(mapBlockIndex.count(hashSyncCheckpoint));
        const CBlockIndex* pindexSync = mapBlockIndex[hashSyncCheckpoint];

		int vCoinbaseMaturity = nCoinbaseMaturity;
		//if( pindexSync->nHeight <= 10 )
		//	vCoinbaseMaturity = 0;

        return (nBestHeight >= pindexSync->nHeight + vCoinbaseMaturity ||
                pindexSync->GetBlockTime() + nStakeMinAge < GetAdjustedTime());
    }
}

// ppcoin: sync-checkpoint master key
const std::string CSyncCheckpoint::strMasterPubKey = "04a18357665ed7a802dcf252ef528d3dc786da38653b51d1ab8e9f4820b55aca807892a056781967315908ac205940ec9d6f2fd0a85941966971eac7e475a27826";

std::string CSyncCheckpoint::strMasterPrivKey = "";

// ppcoin: verify signature of sync-checkpoint message
bool CSyncCheckpoint::CheckSignature()
{
    CKey key;
    if (!key.SetPubKey(ParseHex(CSyncCheckpoint::strMasterPubKey)))
        return error("CSyncCheckpoint::CheckSignature() : SetPubKey failed");
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CSyncCheckpoint::CheckSignature() : verify signature failed");

    // Now unserialize the data
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedSyncCheckpoint*)this;
    return true;
}

// ppcoin: process synchronized checkpoint
bool CSyncCheckpoint::ProcessSyncCheckpoint(CNode* pfrom)
{
    if (!CheckSignature())
        return false;

    LOCK(Checkpoints::cs_hashSyncCheckpoint);
    if (!mapBlockIndex.count(hashCheckpoint))
    {
        // We haven't received the checkpoint chain, keep the checkpoint as pending
        Checkpoints::hashPendingCheckpoint = hashCheckpoint;
        Checkpoints::checkpointMessagePending = *this;
        printf("ProcessSyncCheckpoint: pending for sync-checkpoint %s\n", hashCheckpoint.ToString().c_str());
        // Ask this guy to fill in what we're missing
        if (pfrom)
        {
            pfrom->PushGetBlocks(pindexBest, hashCheckpoint);
            // ask directly as well in case rejected earlier by duplicate
            // proof-of-stake because getblocks may not get it this time
            pfrom->AskFor(CInv(pfrom->zipblock > 0 ? MSG_BLKZP : MSG_BLOCK, mapOrphanBlocks.count(hashCheckpoint)? WantedByOrphan(mapOrphanBlocks[hashCheckpoint]) : hashCheckpoint));
        }
        return false;
    }

    if (!Checkpoints::ValidateSyncCheckpoint(hashCheckpoint))
        return false;

    CTxDB txdb;
    CBlockIndex* pindexCheckpoint = mapBlockIndex[hashCheckpoint];
    if (!pindexCheckpoint->IsInMainChain())
    {
        // checkpoint chain received but not yet main chain
        CBlock block;
        if (!block.ReadFromDisk(pindexCheckpoint))
            return error("ProcessSyncCheckpoint: ReadFromDisk failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        if (!block.SetBestChain(txdb, pindexCheckpoint))
        {
            Checkpoints::hashInvalidCheckpoint = hashCheckpoint;
            return error("ProcessSyncCheckpoint: SetBestChain failed for sync checkpoint %s", hashCheckpoint.ToString().c_str());
        }
    }

    if (!Checkpoints::WriteSyncCheckpoint(hashCheckpoint))
        return error("ProcessSyncCheckpoint(): failed to write sync checkpoint %s", hashCheckpoint.ToString().c_str());
    Checkpoints::checkpointMessage = *this;
    Checkpoints::hashPendingCheckpoint = 0;
    Checkpoints::checkpointMessagePending.SetNull();
    printf("ProcessSyncCheckpoint: sync-checkpoint at %s\n", hashCheckpoint.ToString().c_str());
    return true;
}
