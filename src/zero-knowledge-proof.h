// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee
#ifndef ZERO_PROOF_H
#define ZERO_PROOF_H

#ifdef WIN32
#include <windows.h>
#endif
#include <stdint.h>
#include <string>
#include "base58.h"
#include "key.h"
#include "bignum.h"
#include "sync.h"
#include "net.h"
#include "script.h"
#include "scrypt.h"
#include "aes/aes.h"
#include <list>
#include <fstream>

class CWallet;
class CBlock;
class CBlockIndex;
class CKeyItem;
class CReserveKey;
class COutPoint;
class CKeyID;
class CBitcoinAddress;

class CAddress;
class CInv;
class CRequestTracker;
class CNode;
class CZeroKnowledgeProof;
class CZeroKnowledgeProofMintKey;

using namespace std;
using namespace boost;

struct ZeroKnowledgeProofPack{
   int nProofType;
   int64_t n6Coins;
   string nProofStr;
};

struct ZeroKnowledgeProofMintPack{
   string sZkpHash;
   string sMintKey;
   string sMintK2;
};

struct ZeroKnowledgeProofMintOutPack{
   string sZkpHash, sToAddress;
   int nAmount;
   string sMintKey;
};

struct txOutPairPack{
   int idx;   uint64_t v_nValue;
   string sAddr;
};

extern const std::string sZKPMint_Flags;   //"ZKP-Mint";
extern const std::string sZeroKnowledgeProofPoolAddress;

extern const int64_t Min_Burn_Coin_Amount;                   // 100 * COIN;
extern const int64_t Max_Burn_Coin_Amount_1K;             // 1000 * COIN;
extern const int64_t Max_Burn_Coin_Amount_10K;           // 1 W
extern const int64_t Max_Burn_Coin_Amount_100K;         // 10 W
extern const int64_t Max_Burn_Coin_Amount_1M;            // 100 W
extern const int64_t Max_Burn_Coin_Amount_10M;          // 1000 W
extern const int64_t Max_Burn_Coin_Amount_100M;        // 1 E
extern const int64_t Max_Burn_Coin_Amount_500M;        // 5 E
extern const int ZKP_Rule_Active_Block;                           // 1166100 + 43210;
extern const int ZKP_Rule_Active_Block_A_10K;                      // 1166100 + (100000 * 2);
extern const int ZKP_Rule_Active_Block_B_100K;                    // 1166100 + (100000 * 3);
extern const int ZKP_Rule_Active_Block_C_1M;                       // 1166100 + (100000 * 4);
extern const int ZKP_Rule_Active_Block_D_10M;                     // 1166100 + (100000 * 5);
extern const int ZKP_Rule_Active_Block_E_100M;                    // 1166100 + (100000 * 6);

extern string sBitChainIdentAddress;
extern int writeBufToFile(char* pBuf, int bufLen, string fName);
extern string signMessage(const string strAddress, const string strMessage);
extern int signMessageWithoutBase64(const string strAddress, const string strMessage, string& sRzt);
//extern bool getTxinAddressAndAmount(const CTxIn& txin, string& sPreTargetAddr, int64_t& iAmnt);

inline std::string inttostr(int n)
{
  return strprintf("%d", n);
}

std::string int64tostr(int64_t n);
int64_t strToInt64(const char *s, int iBase);
int64_t strToInt64(const string s, int iBase);
std::string getNewAddress(const string strAccount);
bool IsValidBitcoinAddress(const string sAddr);
bool IsZkpRuleActived(int nHeight);
int64_t GetMaxZkpMintCoinAmount(int nHeight);
bool GetMaxZkpMintCoinAmount(int nHeight, int64_t& i6Amount);
bool IsValidBurnOrMintCoinAmount(int nHeight, int64_t i6Amount);
std::string getCoinAddressFromCScript(const CScript& scriptPubKey);
bool zkpBurn(int iBurnCoins, const string sPubKey, string& rztTxHash);
bool IsZkpBurnTx(const CTransaction& tx, bool checkExists, string& sRztZkpHash, bool& bExists);
int GetTxOutBurnedCoins(const std::vector<CTxOut> &vout, int64_t& u6Rzt);
int  GetVectorPairTxOutDetails(const std::vector<std::pair<CScript, int64_t> >& aVecSend, std::vector<txOutPairPack > &outPair);
int  GetTxOutDetails(const std::vector<CTxOut> &vout, std::vector<txOutPairPack > &outPair);
bool isTxOutToThisAddress(const CTxOut txout, const std::string sAddress);
bool isTxOutToThisAddress(const CTransaction& tx, const std::string sAddress, int64_t& i6OutCoin, bool bGetOneReturn=false);
bool zkpClassExists(const CZeroKnowledgeProof& zkp);
bool zkpClassExists(int nProofType, int64_t i6Coin, const std::string nProofStr);
int getCZKPAndCZKPMintKeyFromTx(const CTransaction& tx, CZeroKnowledgeProof& czkp, CZeroKnowledgeProofMintKey& czmk);
uint256 getZkpHashFromParams(const CZeroKnowledgeProof& zkp, bool checkExists=false);
uint256 getZkpHashFromParams(int nProofType, int64_t i6Coin, const std::string nProofStr, bool checkExists=false);
bool ReadZeroKnowledgeProof(uint256 zkpHash, CZeroKnowledgeProof& zkp);
bool ReadZeroKnowledgeProof(const std::string& sZkpHash, CZeroKnowledgeProof& zkp);
bool isUsedZkpMintKey(const CZeroKnowledgeProof& zkp, const string sMintKey);
void addZkpMintKey(CZeroKnowledgeProof& zkp, const string sMintKey, bool bSaveToDB);
bool delZkpMintKey(CZeroKnowledgeProof& zkp, const string sMintKey, bool bSaveToDB);
//bool isValidZkpMintReq(const CZeroKnowledgeProof& zkp, int64_t nAmount, const string& sMintKey);
bool delCZeroKnowledgeProofMintKeyFromTx(const CTransaction& tx);
bool unSpentZkpAndDelCZKPMintKeyFromTx(const CTransaction& tx);  //bool unSpentZkpAndDelCZeroKnowledgeProofMintKeyFromTx(const CTransaction& tx);
bool isValidZkpMintKey(int64_t i6Amount, const string& sToAddress, const string& sProofStr, const string& sMintKey);
bool isZkpMintTx(const CTransaction& tx, int nHeight=0);
int addOrEraseZkpTx(const CTransaction& tx, int nHeight, bool bAdd, bool bJustRecvTx, bool bInOtherChain=false);
bool isValidZkpMintTx(const CTransaction& tx, const string sCallFrom, int nHeight, bool bJustRecvTx, bool bSaveMintKeyToDb, bool bUpdateSpentToDb, bool bInOtherChain, bool bJustCheck=false);
bool zkpmint(const string& sZkpHash, const std::vector<std::pair<CScript, int64_t> >& aVecSend, const string& sMintKey, string& sRztTxHash);
bool genZkpMintKey(const string& sZkpHash, int64_t i6Amount, const string& sToAddress, std::string& sRztMintKey);
bool saveZeroKnowledgeProofMintTxToDB(const CTransaction& tx);
bool delZeroKnowledgeProofMintTxFromDB(const CTransaction& tx);

class CZeroKnowledgeProof
{
public:
    unsigned int nVersion;
    int64_t n6Coins;
	//std::string nProofStr;   // a AES encrypt string or a coin address
	CKeyID nProofKeyID;
    int nProofType;
    unsigned int nLockTime;
    int64_t n6Spent, n6EstSpent;
	//std::vector<std::string> nUsedMintKeys;
	int nStatus;                    // 0 = Just receive, not in a block and not active, 1 = actived
	std::string nReservedStr;

    CZeroKnowledgeProof()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(n6Coins);
        READWRITE(nProofKeyID);   //READWRITE(nProofStr);
        READWRITE(nProofType);
        READWRITE(nLockTime);
		if( !(nType & SER_GETHASH) )
		{
			READWRITE(n6Spent);      READWRITE(n6EstSpent);      READWRITE(nStatus);      READWRITE(nReservedStr);
		}
    )

    void SetNull()
    {
        nVersion = 1;       n6Coins = 0;    n6Spent = 0;      n6EstSpent=0;      nStatus = 0;   //nUsedMintKeys.clear();
        nProofKeyID = CKeyID(0);     nProofType = 0;	  nLockTime = 0;       nReservedStr = ""; // nProofStr = "";
    }

    bool IsNull() const
    {
        return ( (n6Coins <= 0) || (n6Spent >= n6Coins) );
    }
	bool NotActive() const
	{
		return nStatus <= 0;
	}
	
	bool CheckSpent(int64_t i6Spent) const
	{
		bool rzt=false;    int64_t i62 = i6Spent + n6EstSpent;
		if( (i6Spent > 0) && (i62 <= n6Coins) ){ rzt=true; }
		if( fDebug ){ printf("CZeroKnowledgeProof::CheckSpent(%s), i62=[%s], rzt=[%d] \n", int64tostr(i6Spent).c_str(), int64tostr(i62).c_str(), rzt); }
		return rzt;	
	}
	bool CanSpent(int64_t i6New) const
	{
		bool rzt=false;    int64_t i62 = i6New + n6Spent;
		if( (i6New > 0) && (i62 <= n6Coins) ){ rzt=true; }
		if( fDebug ){ printf("CZeroKnowledgeProof::CanSpent(%s), i62=[%s], rzt=[%d] \n", int64tostr(i6New).c_str(), int64tostr(i62).c_str(), rzt); }
		return rzt;
	}
	int64_t GetBalance() const
	{
		return (n6Coins - n6Spent);
	}

	bool SetKeyID(const string& sAddress)
	{
		bool rzt=false;      CBitcoinAddress address(sAddress);
		if( address.IsValid() )
		{
			rzt = address.GetKeyID(nProofKeyID);
		}
		return rzt;
	}
	string GetProofKeys() const
	{
		CBitcoinAddress address(nProofKeyID);    
		return address.ToString();
	}
    CZeroKnowledgeProof(int64_t n6CoinsIn, string nProofStrIn)
    {
        SetNull();      n6Coins = n6CoinsIn;   SetKeyID(nProofStrIn);   //nProofStr = nProofStrIn;
    }
    CZeroKnowledgeProof(int64_t n6CoinsIn, string nProofStrIn, int nStatusIn)
    {
        SetNull();      n6Coins = n6CoinsIn;   SetKeyID(nProofStrIn);     nStatus = nStatusIn;
    }
    CZeroKnowledgeProof(int nProofTypeIn, int64_t n6CoinsIn, string nProofStrIn)
    {
        SetNull();      nProofType = nProofTypeIn;   n6Coins = n6CoinsIn;   SetKeyID(nProofStrIn);
    }

    CZeroKnowledgeProof(int nProofTypeIn, int64_t n6CoinsIn, string nProofStrIn, string nReservedStrIn, int nLockTimeIn)
    {
        SetNull();
        nProofType = nProofTypeIn;   n6Coins = n6CoinsIn;   SetKeyID(nProofStrIn);   nReservedStr = nReservedStrIn;
        nLockTime = nLockTimeIn;
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }
	
	std::string GetHashString() const
	{
		return GetHash().GetHex();
	}

    /*std::string ToString() const
    {
        if (IsNull())
            return "null";
        else
            return strprintf("(nFile=%u, nBlockPos=%u, nTxPos=%u)", nFile, nBlockPos, nTxPos);
    }

    void print() const
    {
        printf("%s", ToString().c_str());
    }*/
};

class CZeroKnowledgeProofMintKey
{
public:
    unsigned int nVersion;
	std::string nMintKeyStr;
    int nStatus;                    // 0 = Just receive, not in a block and not active, 1 = used
	std::string nReservedStr;

    CZeroKnowledgeProofMintKey()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nMintKeyStr);
		if( !(nType & SER_GETHASH) )
		{
			READWRITE(nStatus);      READWRITE(nReservedStr);
		}
    )

    void SetNull()
    {
        nVersion = 1;       nMintKeyStr = "";     nStatus = 0;       nReservedStr = "";
    }

    bool IsNull() const
    {
        return ( nMintKeyStr.length() < 32 );
    }

    CZeroKnowledgeProofMintKey(string nMintKeyStrIn)
    {
        SetNull();      nMintKeyStr = nMintKeyStrIn;
    }
	
    CZeroKnowledgeProofMintKey(string nMintKeyStrIn, int nStatusIn)
    {
        SetNull();
        nMintKeyStr = nMintKeyStrIn;      nStatus =  nStatusIn;
    }

    CZeroKnowledgeProofMintKey(string nMintKeyStrIn, int nStatusIn, string nReservedStrIn)
    {
        SetNull();
        nMintKeyStr = nMintKeyStrIn;      nStatus =  nStatusIn;      nReservedStr = nReservedStrIn;
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }
};

#endif
