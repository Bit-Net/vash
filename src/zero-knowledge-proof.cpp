// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee

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
#include "main.h"
#include <list>
#include <fstream>
#include "db.h"
#include "txdb.h"
#include "wallet.h"
#include "init.h"
#include "zero-knowledge-proof.h"

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

using namespace std;
using namespace boost;

const std::string sZKPMint_Flags = "ZKP-Mint";
const std::string sZeroKnowledgeProofPoolAddress = "VpnZKPjQRCVMu9qfekCrapVjnoQUt5Mrf3";
const int64_t Min_Burn_Coin_Amount            = 100 * COIN;
const int64_t Max_Burn_Coin_Amount_1K      = 1000 * COIN;
const int64_t Max_Burn_Coin_Amount_10K    = 10000 * COIN;            // 1 W
const int64_t Max_Burn_Coin_Amount_100K  = 100000 * COIN;          // 10 W
const int64_t Max_Burn_Coin_Amount_1M     = 1000000 * COIN;        // 100 W
const int64_t Max_Burn_Coin_Amount_10M   = 10000000 * COIN;      // 1000 W
const int64_t Max_Burn_Coin_Amount_100M = 100000000 * COIN;     // 1 E
const int64_t Max_Burn_Coin_Amount_500M = 500000000 * COIN;     // 5 E
const int ZKP_Rule_Active_Block    = 1166100 + 43210;                      // about actived at 2017.04.10
const int ZKP_Rule_Active_Block_A_10K = 1166100 + (100000 * 2);     // about actived at 2017.07.20
const int ZKP_Rule_Active_Block_B_100K = 1166100 + (100000 * 3);   // about actived at 2017.09.20
const int ZKP_Rule_Active_Block_C_1M = 1166100 + (100000 * 6);      // about actived at 2018.03.20
const int ZKP_Rule_Active_Block_D_10M = 1166100 + (100000 * 8);
const int ZKP_Rule_Active_Block_E_100M = 1166100 + (100000 * 9);

extern string s_BlockChain_Dir;
extern string sBitChainIdentAddress;
extern string sBitChainKeyAddress;
extern int writeBufToFile(char* pBuf, int bufLen, string fName);
extern string signMessage(const string strAddress, const string strMessage);
extern int signMessageWithoutBase64(const string strAddress, const string strMessage, string& sRzt);
extern bool verifyMessage(const string strAddress, const string strSign, const string strMessage);
extern bool getTxinAddressAndAmount(const CTxIn& txin, string& sPreTargetAddr, int64_t& iAmnt);

int gLastCheckZkpMintTxErrorIdx = 0;
inline std::string u64tostr(uint64_t n)
{
  return strprintf("%"PRIu64, n);
}

std::string int64tostr(int64_t n)
{
	std::ostringstream oss;      oss<<n;
	return oss.str();
}

int64_t strToInt64(const char *s, int iBase)
{
   return strtoll(s, NULL, iBase);
}
int64_t strToInt64(const string s, int iBase)
{
   return strtoll(s.c_str(), NULL, iBase);
}

int64_t GetMaxZkpMintCoinAmount(int nHeight)
{
	int64_t rzt = 0;
	if( nHeight >= ZKP_Rule_Active_Block )
	{
			rzt = Max_Burn_Coin_Amount_1K;
			//if( nHeight >= ZKP_Rule_Active_Block_D ){ rzt = Max_Burn_Coin_Amount_10M; }                 // 10,000,000 * COIN;     1000 W
			if( nHeight >= ZKP_Rule_Active_Block_C_1M ){ rzt = Max_Burn_Coin_Amount_1M; }                //  1,000,000 * COIN;      100 W
			else if( nHeight >= ZKP_Rule_Active_Block_B_100K ){ rzt = Max_Burn_Coin_Amount_100K; }    //     100,000 * COIN;      10 W
			else if( nHeight >= ZKP_Rule_Active_Block_A_10K ){ rzt = Max_Burn_Coin_Amount_10K; }        //        10,000 * COIN;      1 W
			else{ rzt = Max_Burn_Coin_Amount_1K; }
	}
	return rzt;
}

bool GetMaxZkpMintCoinAmount(int nHeight, int64_t& i6Amount)
{
	bool rzt=false;      i6Amount = 0;
	if( fTestNet ){ rzt=true;   i6Amount = Max_Burn_Coin_Amount_500M; }
	else{
		i6Amount = GetMaxZkpMintCoinAmount(nHeight);
		rzt = (i6Amount > 0);
	}
	return rzt;
}

bool IsValidBurnOrMintCoinAmount(int nHeight, int64_t i6Amount)
{
	bool rzt = false;
	if( fTestNet ){ rzt=true; }
	else{
		int64_t i6Max=0;      bool b = GetMaxZkpMintCoinAmount(nHeight, i6Max);
		if( b ){ rzt = (i6Amount >= Min_Burn_Coin_Amount) && (i6Amount <= i6Max); }
	}
	return rzt;
}

bool IsZkpRuleActived(int nHeight)
{
	return fTestNet || (nHeight >= ZKP_Rule_Active_Block);
}

std::string getNewAddress(const string strAccount)
{
    if (!pwalletMain->IsLocked())
        pwalletMain->TopUpKeyPool();

    // Generate a new key that is added to wallet
    CPubKey newKey;
    if (!pwalletMain->GetKeyFromPool(newKey, false)) return "";   //throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
    CKeyID keyID = newKey.GetID();
    if( strAccount.length() > 0 ){ pwalletMain->SetAddressBookName(keyID, strAccount); }
    return CBitcoinAddress(keyID).ToString();
}

bool IsValidBitcoinAddress(const string sAddr)
{
	CBitcoinAddress address(sAddr);
	return address.IsValid();
}

int GetTxOutBurnedCoins(const std::vector<CTxOut> &vout, int64_t& u6Rzt)
{
	int rzt = 0;      u6Rzt = 0;
	int j = vout.size();
	for( int i = 0; i < j; i++)  //BOOST_FOREACH(const CTxOut& txout, tx.vout)
	{
		const CTxOut& txout = vout[i];
		txnouttype type;      vector<CTxDestination> addresses;      int nRequired;
		if( ExtractDestinations(txout.scriptPubKey, type, addresses, nRequired) )
		{
			BOOST_FOREACH(const CTxDestination& addr, addresses)
			{
				string sAa = CBitcoinAddress(addr).ToString();
				if( sAa == sZeroKnowledgeProofPoolAddress ){
					rzt++;   u6Rzt += txout.nValue;
				}
			}
		}
	}
	return rzt;
}

//bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, std::vector<CTxDestination>& addressRet, int& nRequiredRet);
CBitcoinAddress getCBitCoinAddressFromCScript(const CScript& scriptPubKey)
{
	txnouttype type;      int nRequired;
	vector<CTxDestination> addresses;
	if( ExtractDestinations(scriptPubKey, type, addresses, nRequired) )
	{
		BOOST_FOREACH(const CTxDestination& addr, addresses)
		{
			return CBitcoinAddress(addr);
		}
	}
	CBitcoinAddress rzt;      return rzt;
}
std::string getCoinAddressFromCScript(const CScript& scriptPubKey)
{
	string rzt="";
	CBitcoinAddress cba = getCBitCoinAddressFromCScript(scriptPubKey);
	if( cba.IsValid() ){ rzt = cba.ToString(); }
	return rzt;
}

int  GetVectorPairTxOutDetails(const std::vector<std::pair<CScript, int64_t> >& aVecSend, std::vector<txOutPairPack > &outPair)
{
	// BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, aVecSend){ nAmount = nAmount + s.second; }
	int rzt = 0;
	int j = aVecSend.size();
	for( int i = 0; i < j; i++)
	{
		const PAIRTYPE(CScript, int64_t)& vp = aVecSend[i];
		// int nRequired;      txnouttype type;      vector<CTxDestination> addresses;
		txOutPairPack p = {i, vp.second, ""};
		p.sAddr = getCoinAddressFromCScript(vp.first);
		/*if( ExtractDestinations(vp.first, type, addresses, nRequired) )
		{
			BOOST_FOREACH(const CTxDestination& addr, addresses)
			{
				string sAa = CBitcoinAddress(addr).ToString();   p.sAddr = sAa;
				break;
			}
		}*/
		outPair.push_back(p);   rzt++;
	}
	return rzt;
}
int  GetTxOutDetails(const std::vector<CTxOut> &vout, std::vector<txOutPairPack > &outPair)
{
	int rzt = 0;
	
	int j = vout.size();
	for( int i = 0; i < j; i++)  //BOOST_FOREACH(const CTxOut& txout, tx.vout)
	{
		const CTxOut& txout = vout[i];
		//txnouttype type;      int nRequired;      vector<CTxDestination> addresses;
		txOutPairPack p = {i, txout.nValue, ""};
		p.sAddr = getCoinAddressFromCScript(txout.scriptPubKey);
		/*if( ExtractDestinations(txout.scriptPubKey, type, addresses, nRequired) )
		{
			BOOST_FOREACH(const CTxDestination& addr, addresses)
			{
				string sAa = CBitcoinAddress(addr).ToString();   p.sAddr = sAa;
				break;
			}
		}*/
		outPair.push_back(p);   rzt++;
	}
	return rzt;
}

bool isTxOutToThisAddress(const CTxOut txout, const std::string sAddress, int64_t& i6OutCoin)
{
	bool rzt=false;
	{
		txnouttype type;
		vector<CTxDestination> addresses;
		int nRequired;
		if( ExtractDestinations(txout.scriptPubKey, type, addresses, nRequired) )
		{
			BOOST_FOREACH(const CTxDestination& addr, addresses)
			{
				string sAa = CBitcoinAddress(addr).ToString();
				if( sAddress.find(sAa) != string::npos ){ rzt = true;   i6OutCoin = txout.nValue;   break; }
			}
		}
	}
	return rzt;
}

bool isTxOutToThisAddress(const CTransaction& tx, const std::string sAddress, int64_t& i6OutCoin, bool bGetOneReturn)
{
	bool rzt = false;     i6OutCoin = 0;
	BOOST_FOREACH(const CTxOut &txout, tx.vout)
	{
		int64_t i6OCoin=0;
		if( isTxOutToThisAddress(txout, sAddress, i6OCoin) )
		{
			rzt = true;     i6OutCoin = i6OutCoin + i6OCoin;
			if( bGetOneReturn ){ break; }
		}
	}
	//printf("isTxOutToThisAddress rzt=[%d], [%d] [%I64u] \n", rzt, bGetOneReturn, i6OutCoin);
	return rzt;
}

bool saveOrDelZeroKnowledgeProofToDB(const CZeroKnowledgeProof& zkp, bool bSave)
{
	bool rzt=false;   CTxDB txdb;
	if( bSave ){ rzt = txdb.AddZeroKnowledgeProof(zkp); }
	else{ rzt = txdb.EraseZeroKnowledgeProof(zkp); }
	//if( fDebug ){ printf("saveOrDelZeroKnowledgeProofToDB(%s, bSave=%d) rzt = [%d] \n", zkp.GetHash().ToString().c_str(), bSave, rzt); }
	return rzt;
}

bool saveZeroKnowledgeProofToDB(const CZeroKnowledgeProof& zkp)
{
	return saveOrDelZeroKnowledgeProofToDB(zkp, true);
}
bool delZeroKnowledgeProofFromDB(const CZeroKnowledgeProof& zkp)
{
	return saveOrDelZeroKnowledgeProofToDB(zkp, false);
}

bool saveOrDelZeroKnowledgeProofMintKeyToDB(const CZeroKnowledgeProofMintKey& zmk, bool bSave)
{
	CTxDB txdb;
	if( bSave ){ return txdb.AddCZeroKnowledgeProofMintKey(zmk); }
	else{ return txdb.EraseCZeroKnowledgeProofMintKey(zmk); }
}
bool saveZeroKnowledgeProofMintKeyToDB(const CZeroKnowledgeProofMintKey& zmk)
{
	return saveOrDelZeroKnowledgeProofMintKeyToDB(zmk, true);
}
bool delZeroKnowledgeProofMintKeyFromDB(const CZeroKnowledgeProofMintKey& zmk)
{
	return saveOrDelZeroKnowledgeProofMintKeyToDB(zmk, false);
}

bool saveOrDelZeroKnowledgeProofMintTxToDB(const CTransaction& tx, bool bSave)
{
	CTxDB txdb;
	if( bSave ){ return txdb.AddCZeroKnowledgeProofMintTx(tx); }
	else{ return txdb.EraseCZeroKnowledgeProofMintTx(tx); }
}
bool saveZeroKnowledgeProofMintTxToDB(const CTransaction& tx)
{
	return saveOrDelZeroKnowledgeProofMintTxToDB(tx, true);
}
bool delZeroKnowledgeProofMintTxFromDB(const CTransaction& tx)
{
	return saveOrDelZeroKnowledgeProofMintTxToDB(tx, false);
}
bool isZeroKnowledgeProofMintTxInDB(const CTransaction& tx)
{
	CTxDB txdb("r");
	return txdb.ContainsCZeroKnowledgeProofMintTx(tx);
}

bool ReadZeroKnowledgeProof(uint256 zkpHash, CZeroKnowledgeProof& zkp)
{
	CTxDB txdb("r");      CZeroKnowledgeProof czkp;
	bool rzt = txdb.ReadZeroKnowledgeProof(zkpHash, czkp);
	if( rzt ){ zkp = czkp; }
	return rzt;
}

bool ReadZeroKnowledgeProof(const std::string& sZkpHash, CZeroKnowledgeProof& zkp)
{
	uint256 zkpHash=0;   zkpHash.SetHex(sZkpHash);
	return ReadZeroKnowledgeProof(zkpHash, zkp);
}

bool ReadCZeroKnowledgeProofMintKey(uint256 hash, CZeroKnowledgeProofMintKey& zmk)
{
	CTxDB txdb("r");      CZeroKnowledgeProofMintKey czmk;
	bool rzt = txdb.ReadCZeroKnowledgeProofMintKey(hash, czmk);
	if( rzt ){ zmk = czmk; }
	return rzt;
}

bool ReadCZeroKnowledgeProofMintKey(const std::string& sZmkHash, CZeroKnowledgeProofMintKey& zmk)
{
	uint256 zmkHash=0;   zmkHash.SetHex(sZmkHash);
	return ReadCZeroKnowledgeProofMintKey(zmkHash, zmk);
}

int ReadZeroKnowledgeProofMintKeyStatus(const string& sMintKey)
{
	int rzt=-1;      CZeroKnowledgeProofMintKey zmk(sMintKey);
	uint256 zmkHash = zmk.GetHash();
	if( ReadCZeroKnowledgeProofMintKey(zmkHash, zmk) )
	{
		rzt = zmk.nStatus;
	}
	if( fDebug ){ printf("ReadZeroKnowledgeProofMintKeyStatus(%s), rzt=[%d] \n", sMintKey.c_str(), rzt); }
	return rzt;
}

bool IsNewZeroKnowledgeProofMintKey(const string& sMintKey)
{
	int i = ReadZeroKnowledgeProofMintKeyStatus(sMintKey);
	return (i <= 0);
}

bool IsUsedZeroKnowledgeProofMintKey(const string& sMintKey)
{
	int i = ReadZeroKnowledgeProofMintKeyStatus(sMintKey);
	return (i > 0);
}

bool IsExistsZeroKnowledgeProofMintKey(const string& sMintKey)
{
	int i = ReadZeroKnowledgeProofMintKeyStatus(sMintKey);
	return (i >= 0);
}

	/*BOOST_FOREACH(const std::string& aKey, zkp.nUsedMintKeys)
	{
		if( aKey == sMintKey ){ rzt=true;   break; }
	}
bool isUsedZkpMintKey(const CZeroKnowledgeProof& zkp, const string sMintKey)
{
	bool rzt=false;
	std::vector<std::string> vs(zkp.nUsedMintKeys);
	std::vector<std::string>::iterator iter = find(vs.begin(), vs.end(), sMintKey);
	if( iter != vs.end() ){ rzt = true; }
	return rzt;
}

void addZkpMintKey(CZeroKnowledgeProof& zkp, const string sMintKey, bool bSaveToDB)
{
	zkp.nUsedMintKeys.push_back(sMintKey);
	if( bSaveToDB ){ saveZeroKnowledgeProofToDB(zkp); }
}

bool delZkpMintKey(CZeroKnowledgeProof& zkp, const string sMintKey, bool bSaveToDB)
{
	bool rzt=false;
	std::vector<std::string>::iterator iter = std::find(zkp.nUsedMintKeys.begin(), zkp.nUsedMintKeys.end(), sMintKey);
	if( iter != zkp.nUsedMintKeys.end() )
	{
		zkp.nUsedMintKeys.erase(iter);      rzt = true;
		if( bSaveToDB ){ saveZeroKnowledgeProofToDB(zkp); }
	}
	return rzt;
}*/

int getZeroKnowledgeProofPack(const std::string betStr, ZeroKnowledgeProofPack& zkpp)
{
	int rzt = 0;
	string stxData = "";
	int iLen = betStr.length();   if( iLen > 512 ){ return rzt; }
	if( betStr.length() > 0 ){ stxData = betStr.c_str(); }
	if( stxData.length() > 0 )
	{
		int i = 0;
		if( !strchr(stxData.c_str(), '|') )
		{
			i++;   i++;  // 2
			zkpp.nProofType = 0;         // 0 = coin address verify,  1 = AES veryfy
			zkpp.nProofStr = stxData;
		}else{
		try{
		char *delim = "|";
					
		char * pp = (char *)stxData.c_str();
        char *reserve;
		char *pch = strtok_r(pp, delim, &reserve);
		while (pch != NULL)
		{
			i++;
			if( i == 1 ){ zkpp.nProofStr = pch; }
			else if( i == 2 ){ zkpp.nProofType = atoi(pch); }
			//else if( i == 3 ){ zkpp.b3 = atoi(pch); }
			else if( i >= 3 ){ break; }
			pch = strtok_r(NULL, delim, &reserve);
		}
		}catch (std::exception &e) {
			printf("getZeroKnowledgeProofPack:: err [%s]\n", e.what());
		}
		}
		rzt = i;
	}
	return rzt;
}

int getZeroKnowledgeProofPackFromTx(const CTransaction& tx, ZeroKnowledgeProofPack& zkpp)
{
	return getZeroKnowledgeProofPack(tx.vpndata, zkpp);
}

int getZkpMintPack(const std::string betStr, ZeroKnowledgeProofMintPack& zmkp)
{
	int rzt = 0;   // ZKP-Mint | ZKP Hash | H2A1vejMSzyaL78urwtbeyIwsPdmjQ0WleLKHwwTb/jakRHbyQuRCMvWdZXs2gxJYx8ShayrN9Du6D+gGWuF9yU= (88 chars)
	string stxData = "";
	int iLen = betStr.length();   if( iLen > 512 ){ return rzt; }
	if( betStr.length() > 0 ){ stxData = betStr.c_str(); }
	if( (stxData.length() > 0) && (stxData.find(sZKPMint_Flags) == 0) )
	{
		int i = 0;
		try{
		char *delim = "|";
					
		char * pp = (char *)stxData.c_str();
        char *reserve;
		char *pch = strtok_r(pp, delim, &reserve);
		while (pch != NULL)
		{
			i++;
			if( i == 2 ){ zmkp.sZkpHash = pch; }
			else if( i == 3 ){ zmkp.sMintKey = pch; }
			else if( i == 4 ){ zmkp.sMintK2 = pch; }
			else if( i >= 5 ){ break; }
			pch = strtok_r(NULL, delim, &reserve);
		}
		}catch (std::exception &e) {
			printf("getZkpMintPack:: err [%s]\n", e.what());
		}catch (...) {
            //PrintExceptionContinue(NULL, "getZkpMintPack()");
        }
		rzt = i;
	}
	return rzt;
}

int getZkpMintPackFromTx(const CTransaction& tx, ZeroKnowledgeProofMintPack& zmkp)
{
	return getZkpMintPack(tx.vpndata, zmkp);
}

bool isZkpMintTx(const CTransaction& tx, int nHeight)
{
	bool rzt = ( tx.vin.empty() && (tx.vpndata.length() > 10) && (tx.vpndata.find(sZKPMint_Flags) == 0) );  // "ZKP-Mint"
	/*if( fTestNet ){ return rzt; }
	else if( rzt )
	{
		if( nHeight < ZKP_Rule_Active_Block ){ rzt = false; }
	}*/
	//if( fDebug ){ printf("isZkpMintTx(%s) rzt [%d] \n", tx.GetHash().ToString().c_str(), rzt); }
	return rzt;
}

bool getZkpMintPackFromZkpMintTx(const CTransaction& tx, ZeroKnowledgeProofMintPack& zkpmp)
{
	bool rzt=false;         //  ZKP-Mint | ZKP Hash | H2A1vejMSzyaL78urwtbeyIwsPdmjQ0WleLKHwwTb/jakRHbyQuRCMvWdZXs2gxJYx8ShayrN9Du6D+gGWuF9yU= (88 chars)
	string sHash = tx.GetHash().ToString();
	if( isZkpMintTx(tx) )
	{
		int k = getZkpMintPackFromTx(tx, zkpmp); //ZeroKnowledgeProofMintPack zmkp{"","",""};
		rzt = k > 2;
		if( fDebug ){ printf("getZkpMintPackFromZkpMintTx(%s), rzt=[%d], k=[%d] \nZkpHash=[%s], MintKey=[%s] \n", sHash.c_str(), rzt, k, zkpmp.sZkpHash.c_str(), zkpmp.sMintKey.c_str()); }
	}//else if( fDebug ){ printf("getZkpMintPackFromZkpMintTx(%s) not a ZKP mint tx :( \n", sHash.c_str()); }
	return rzt;
}

bool zkpClassExists(const CZeroKnowledgeProof& zkp)
{
	CTxDB txdb("r");     uint256 hash = zkp.GetHash();     
	bool rzt = txdb.ContainsZeroKnowledgeProof(hash);
	//if( fDebug ) printf("zkpClassExists(%s) return [%d] \n",  hash.ToString().c_str(), rzt);
	return rzt;
}

bool zkpClassExists(int nProofType, int64_t i6Coin, const std::string nProofStr)
{
    CZeroKnowledgeProof zkp(nProofType, i6Coin, nProofStr, "", 0);
	return zkpClassExists(zkp);
}

uint256 getZkpHashFromParams(const CZeroKnowledgeProof& zkp, bool checkExists)
{
	uint256 hash = zkp.GetHash();
	if( checkExists )
	{
	    CTxDB txdb("r");
		if( !txdb.ContainsZeroKnowledgeProof(hash) ){ hash=0; }
	}
	return hash;
}
uint256 getZkpHashFromParams(int nProofType, int64_t i6Coin, const std::string nProofStr, bool checkExists)
{
    CZeroKnowledgeProof zkp(nProofType, i6Coin, nProofStr, "", 0);
	return getZkpHashFromParams(zkp, checkExists);
}

bool IsZkpBurnTx(const CTransaction& tx, bool checkExists, string& sRztZkpHash, bool& bExists)
{
	bool rzt=false;      sRztZkpHash = "";     bExists=false;   bool bZkpMintTx = isZkpMintTx(tx);
	if( (bZkpMintTx == false) && (tx.vpndata.length() > 30) )
	{
		string sData = tx.vpndata.c_str();  // copy
		bool bValid = IsValidBitcoinAddress(sData);
		if( fDebug ) printf("IsZkpBurnTx:: bZkpMintTx=[%d], bValidAddress=[%d], sData=[%s] \n", bZkpMintTx, bValid, sData.c_str());
		if( !bValid ){ return rzt; }      int64_t i6OutCoin = 0;
		bool bZkpTx = isTxOutToThisAddress(tx, sZeroKnowledgeProofPoolAddress, i6OutCoin, false);
		//if( fDebug ) printf("IsZkpBurnTx:: bZkpTx=[%d], i6OutCoin=[%I64u] [%s] \n", bZkpTx, i6OutCoin, sData.c_str());
		if( bZkpTx )
		{
			ZeroKnowledgeProofPack zkpp={0, 0, ""};     int k = getZeroKnowledgeProofPack(sData, zkpp);
			if( fDebug ) printf("IsZkpBurnTx:: k=[%d], zkpp.nProofStr=[%s] \n", k, zkpp.nProofStr.c_str());
			if( k > 0 )
			{
			    rzt = true;      CZeroKnowledgeProof zkp(i6OutCoin, zkpp.nProofStr);     sRztZkpHash = zkp.GetHashString();
				if( checkExists ){ uint256 hash = getZkpHashFromParams(zkp, checkExists);      bExists = hash > 0; }
			}
		}
	}		
	return rzt;
}

int addOrEraseZkpTx(const CTransaction& tx, int nHeight, bool bAdd, bool bJustRecvTx, bool bInOtherChain)
{
	int rzt = 0;      bool bZkpMintTx = isZkpMintTx(tx);   // zkpburn 100 VieLM7amheaY83qhDyJET57UwwgRgzh42v
	if( (bZkpMintTx == false) && (tx.vpndata.length() > 30) )
	{
		string sData = tx.vpndata.c_str();  // copy
		bool bValid = IsValidBitcoinAddress(sData);
		if( fDebug ) printf("addOrEraseZkpTx:: bAdd=[%d], nHeight=[%d], bValidAddress=[%d], bJustRecvTx=[%d], bInOtherChain=[%d], bZkpMintTx=[%d], sData=[%s] \n", bAdd, nHeight, bValid, bJustRecvTx, bInOtherChain, bZkpMintTx, sData.c_str());
		if( !bValid ){ return rzt; }      int64_t i6OutCoin = 0;
		bool bZkpTx = isTxOutToThisAddress(tx, sZeroKnowledgeProofPoolAddress, i6OutCoin, !bAdd);
		if( fDebug ) printf("addOrEraseZkpTx:: bZkpTx=[%d], i6OutCoin=[%I64u] [%s] \n", bZkpTx, i6OutCoin, sData.c_str());
		if( bZkpTx )
		{
			if( bAdd && !IsValidBurnOrMintCoinAmount(nHeight, i6OutCoin) )
			{
				printf("addOrEraseZkpTx() : invalid burn coin amount [%s] \n", int64tostr(i6OutCoin).c_str());   return rzt;
			}
			ZeroKnowledgeProofPack zkpp={0, 0, ""};     int k = getZeroKnowledgeProofPack(sData, zkpp);
			if( fDebug ) printf("addOrEraseZkpTx:: k=[%d], zkpp.nProofStr=[%s] \n", k, zkpp.nProofStr.c_str());
			if( k > 0 )
			{
			    CZeroKnowledgeProof zkp(i6OutCoin, zkpp.nProofStr);
				uint256 hash = zkp.GetHash();     bool bZkpExists = ReadZeroKnowledgeProof(hash, zkp);
				if( fDebug ) printf("addOrEraseZkpTx hash=[%s], bZkpExists=[%d] bAdd=[%d] bJustRecvTx=[%d] \n", hash.ToString().c_str(), bZkpExists, bAdd, bJustRecvTx);
			    if( bAdd )
			    {
					if( bZkpExists && (bJustRecvTx || bInOtherChain) ){ rzt++; }  // 2017.04.16 add
					else{
						if( bJustRecvTx || bInOtherChain ){ zkp.nStatus = 0; }  // Locked
						else{ zkp.nStatus = 1; }
						if( fDebug ) printf("addOrEraseZkpTx:: zkp.nStatus=[%d], zkpHash=[%s] \n", zkp.nStatus, zkp.GetHash().ToString().c_str());
						if( saveZeroKnowledgeProofToDB(zkp) ){ rzt++; }
					}
			    }else{	// disconnect
				    if( bZkpExists )
					{
						if( delZeroKnowledgeProofFromDB(zkp) ){ rzt++; }
					}
			    }
			}
		}
		if( fDebug ) printf("addOrEraseZkpTx bAdd=[%d], rzt=[%d] \n\n", bAdd, rzt);
	}
	return rzt;
}


bool BroadcastZkpMintTx(CWalletTx& txNew)
{
	bool rzt=false;
    if (!txNew.AcceptToMemoryPool())
    {
        // This must not fail. The transaction has already been signed and recorded.
        if( fDebug ) printf("BroadcastZkpMintTx() : Error: Transaction not valid\n");
        //return false;
    }else{
        uint256 hash = txNew.GetHash();    CTxDB txdb("r");
        if (!txdb.ContainsTx(hash))
        {
            if( fDebug ) printf("BroadcastZkpMintTx() : Relaying wtx %s\n", hash.ToString().substr(0,10).c_str());
            RelayTransaction((CTransaction)txNew, hash);      rzt=true;
        }
	}
	return rzt;
}

/*bool isValidZkpKey(const CZeroKnowledgeProof& zkp, int64_t nAmount, const string& sProofKey)
{
	bool rzt=false;
	if( zkp.nProofType == 0 )  // coin address sign
	{
		string sAmount = int64tostr(nAmount);
		rzt = verifyMessage(zkp.nProofStr, sProofKey, sAmount);   // bool verifyMessage(const string strAddress, const string strSign, const string strMessage)
	}else if( zkp.nProofType == 1 )  // AES
	{
		//
	}
	return rzt;
}

bool isValidZkpMintReq(const CZeroKnowledgeProof& zkp, int64_t nAmount, const string& sMintKey)
{
	bool rzt=false;
	if( (nAmount > 0) && ((zkp.n6Spent + nAmount) <= zkp.n6Coins) )
	{
		if( zkp.nProofType == 0 )  // coin address sign
		{
			string sAmount = int64tostr(nAmount);
			rzt = verifyMessage(zkp.nProofStr, sMintKey, sAmount);   // bool verifyMessage(const string strAddress, const string strSign, const string strMessage)
		}else if( zkp.nProofType == 1 )  // AES
		{
			//
		}
	}
	return rzt;
}*/

bool getTxOutAmountAndAddress(const std::vector<CTxOut> &vout, int64_t& i6Amount, std::string& sAddress)
{
	bool rzt=false;      i6Amount = 0;      sAddress = "";      std::vector<txOutPairPack > outPair;      outPair.clear();
	if( GetTxOutDetails(vout, outPair) > 0 )
	{
		BOOST_FOREACH (const txOutPairPack& opp, outPair)
		{
			i6Amount = i6Amount + opp.v_nValue;
			if( sAddress.length() > 30 ){ sAddress = sAddress + "," + opp.sAddr; }  //strprintf("%s,%s", sAddress.c_str(), opp.sAddr.c_str());
			else{ sAddress = opp.sAddr; }
		}
		rzt = i6Amount > 0;
	}
	if( fDebug ){ printf("getTxOutAmountAndAddress :: rzt [%d] i6Amount=[%d], [%s] \n", rzt, (int)(i6Amount / COIN), sAddress.c_str()); }
	return rzt;
}

bool isValidZkpMintKey(int64_t i6Amount, const string& sToAddress, const string& sProofStr, const string& sMintKey)
{
	bool rzt=false;
	string strMessage = int64tostr(i6Amount) + "," + sToAddress;  // 100,VtPAropTrEmytT4QLgwUN1PsoNBLGYHWmy,VtPAropTrEmytT4QLgwUN1PsoNBLGYHWmz,...
	rzt = verifyMessage(sProofStr, sMintKey, strMessage);   // bool verifyMessage(const string strAddress, const string strSign, const string strMessage)
	if( fDebug ){ printf("isValidZkpMintKey(%s) rzt=[%d], coinAddr=[%s], sMintKey=[%s] \n", strMessage.c_str(), rzt, sProofStr.c_str(), sMintKey.c_str()); }
	return rzt;
}

bool getCZeroKnowledgeProofMintKeyFromStr(const string& sMintKey, CZeroKnowledgeProofMintKey& zmk)
{
	bool rzt=false;
	zmk.SetNull();      zmk.nMintKeyStr = sMintKey;
	uint256 zmkHash = zmk.GetHash();
	rzt = ReadCZeroKnowledgeProofMintKey(zmkHash, zmk);
	return rzt;
}

bool getCZeroKnowledgeProofMintKeyFromZkpmPark(const ZeroKnowledgeProofMintPack& zkpmp, CZeroKnowledgeProofMintKey& zmk)
{
	return getCZeroKnowledgeProofMintKeyFromStr(zkpmp.sMintKey, zmk);
}

bool getCZeroKnowledgeProofMintKeyFromTx(const CTransaction& tx, ZeroKnowledgeProofMintPack& zkpmp, CZeroKnowledgeProofMintKey& zmk)
{
	bool rzt=false;         //  ZKP-Mint | ZKP Hash | H2A1vejMSzyaL78urwtbeyIwsPdmjQ0WleLKHwwTb/jakRHbyQuRCMvWdZXs2gxJYx8ShayrN9Du6D+gGWuF9yU= (88 chars)
	string sHash = tx.GetHash().ToString();
	if( getZkpMintPackFromZkpMintTx(tx, zkpmp) )
	{
		rzt = getCZeroKnowledgeProofMintKeyFromZkpmPark(zkpmp, zmk);
		if( fDebug ){ printf("getCZeroKnowledgeProofMintKeyFromTx(%s), rzt=[%d], zmk.nMintKeyStr = [%s] \n", sHash.c_str(), rzt, zmk.nMintKeyStr.c_str()); }
		//}else if( fDebug ){ printf("getCZeroKnowledgeProofMintKeyFromTx(%s) :: k[%d] <= 2 :( \n", sHash.c_str(), k); }
	}else if( fDebug ){ printf("getCZeroKnowledgeProofMintKeyFromTx(%s) fail :( \n", sHash.c_str()); }
	return rzt;
}

bool delCZeroKnowledgeProofMintKeyFromTx(const CTransaction& tx)
{
	bool rzt=false;
	CZeroKnowledgeProofMintKey czmk;      ZeroKnowledgeProofMintPack zkpmp={"","",""};
	if( getCZeroKnowledgeProofMintKeyFromTx(tx, zkpmp, czmk) )
	{
		rzt = delZeroKnowledgeProofMintKeyFromDB(czmk);
		if( fDebug ){ printf("delCZeroKnowledgeProofMintKeyFromTx(%s), zmkHash=[%s] [%s] \n", tx.GetHash().ToString().c_str(), czmk.GetHash().ToString().c_str(), zkpmp.sMintKey.c_str()); }
	}
	if( fDebug ){ printf("delCZeroKnowledgeProofMintKeyFromTx(%s), rzt=[%d] \n", tx.GetHash().ToString().c_str(), rzt); }
	return rzt;
}

int getCZKPAndCZKPMintKeyFromTx(const CTransaction& tx, CZeroKnowledgeProof& czkp, CZeroKnowledgeProofMintKey& czmk)
{
	int rzt=0;         //  ZKP-Mint | ZKP Hash | H2A1vejMSzyaL78urwtbeyIwsPdmjQ0WleLKHwwTb/jakRHbyQuRCMvWdZXs2gxJYx8ShayrN9Du6D+gGWuF9yU= (88 chars)
	string sHash = tx.GetHash().ToString();      ZeroKnowledgeProofMintPack zkpmp={"","",""};
	if( getZkpMintPackFromZkpMintTx(tx, zkpmp) )
	{
		if( ReadZeroKnowledgeProof(zkpmp.sZkpHash, czkp) ){ rzt=1; }
		if( rzt > 0 ){ if( getCZeroKnowledgeProofMintKeyFromZkpmPark(zkpmp, czmk) ){ rzt = 2; } }
		if( fDebug ){ printf("getCZKPAndCZKPMintKeyFromTx(%s), rzt=[%d] \n", sHash.c_str(), rzt); }
		//}else if( fDebug ){ printf("getCZeroKnowledgeProofMintKeyFromTx(%s) :: k[%d] <= 2 :( \n", sHash.c_str(), k); }
	}//else if( fDebug ){ printf("getCZKPAndCZKPMintKeyFromTx(%s) fail :( \n", sHash.c_str()); }
	return rzt;
}

bool unSpentZkpAndDelCZKPMintKeyFromTx(const CTransaction& tx)
{
	bool rzt=false;      string sHash = tx.GetHash().ToString();
	CZeroKnowledgeProofMintKey czmk;      CZeroKnowledgeProof czkp;
	int k = getCZKPAndCZKPMintKeyFromTx(tx, czkp, czmk);
	if( k > 1 )  // = 2
	{
		int64_t i6Amount=0;      std::string sToAddress="";
		bool b = getTxOutAmountAndAddress(tx.vout, i6Amount, sToAddress);
		if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), b=[%d], i6Amount=[%s], [%s] \n", sHash.c_str(), b, int64tostr(i6Amount).c_str(), sToAddress.c_str()); }
		if( b )
		{
			if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), i6Amount=[%s], n6Spent=[%s] \n", sHash.c_str(), int64tostr(i6Amount).c_str(), int64tostr(czkp.n6Spent).c_str()); }
			int64_t i6 = czkp.n6Spent - i6Amount;
			if( i6 >= 0 )
			{
				if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), i6(%s) >= 0 \n", sHash.c_str(), int64tostr(i6).c_str()); }
				czkp.n6Spent = i6;   czkp.n6EstSpent = i6;
				if( saveZeroKnowledgeProofToDB(czkp) )
				{
					rzt = delZeroKnowledgeProofMintKeyFromDB(czmk);
				}else if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), call saveZeroKnowledgeProofToDB() failed  :(\n", sHash.c_str()); }
			}else if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), i6(%s) < 0  :(\n", sHash.c_str(), int64tostr(i6).c_str()); }
		}
		if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), b=[%d], i6Amount=[%s], n6Spent=[%s] [%s] \n", sHash.c_str(), b, int64tostr(i6Amount).c_str(), int64tostr(czkp.n6Spent).c_str(), sToAddress.c_str()); }
		if( fDebug ){ printf("unSpentZkpAndDelCZKPMintKeyFromTx(%s), rzt=[%d], k=[%d] \n\n", sHash.c_str(), rzt, k); }
	}
	return rzt;
}

bool spentZeroKnowledgeProof(CZeroKnowledgeProof& czkp, int64_t i6Spent)
{
	bool rzt=false;
	int64_t i6 = i6Spent + czkp.n6Spent;
	if( i6 <= czkp.n6Coins )
	{
		czkp.n6Spent = i6;      czkp.n6EstSpent = i6;      rzt = saveZeroKnowledgeProofToDB(czkp);
	}
	if( fDebug ){ printf("spentZeroKnowledgeProof(%s), rzt=[%d], i6Spent=[%s] \n", czkp.GetHash().ToString().c_str(), rzt, int64tostr(i6Spent).c_str()); }
	return rzt;
}
bool isValidZkpMintTx(const CTransaction& tx, const string sCallFrom, int nHeight, bool bJustRecvTx, bool bSaveMintKeyToDb, bool bUpdateSpentToDb, bool bInOtherChain, bool bJustCheck)
{
	bool rzt = false;      string sHash = tx.GetHash().ToString();      gLastCheckZkpMintTxErrorIdx = 0;
	//CZeroKnowledgeProofMintKey czmk;      ZeroKnowledgeProofMintPack zkpmp{"","",""};
	CZeroKnowledgeProofMintKey czmk;      CZeroKnowledgeProof czkp;
	if( fDebug ){ printf("\n%s call isValidZkpMintTx(%s), nHeight=[%d], bJustRecvTx=[%d], bSaveMintKeyToDb=[%d], bUpdateSpentToDb=[%d], bInOtherChain=[%d], bJustCheck=[%d] \n", sCallFrom.c_str(), sHash.c_str(), nHeight, bJustRecvTx, bSaveMintKeyToDb, bUpdateSpentToDb, bInOtherChain, bJustCheck); }
	if( bInOtherChain && (GetArg("-validzkpminttxrzttrueifinotherchain", 0) > 0) ){ return true; }  // just for test
	int k = getCZKPAndCZKPMintKeyFromTx(tx, czkp, czmk);
	if( fDebug ){ printf("isValidZkpMintTx(%s), k=[%d] \n", sHash.c_str(), k); }
	if( k > 0 )  // 2 //if( getCZeroKnowledgeProofMintKeyFromTx(tx, zkpmp, czmk) )
	{
		string sMintKey = czmk.nMintKeyStr;  //zkpmp.sMintKey;   //strprintf("%s%s", zkpmp.sMintKey.c_str(), zkpmp.sMintK2.c_str());
		if( fDebug ){ printf("isValidZkpMintTx(%s), k=[%d], czkp.nStatus=[%d], czmk.nStatus=[%d], sMintKey=[%s] \n", sHash.c_str(), k, czkp.nStatus, czmk.nStatus, sMintKey.c_str()); }
		if( !bJustCheck )
		{
			if( bJustRecvTx && (k > 1) && (!isZeroKnowledgeProofMintTxInDB(tx)) ){ gLastCheckZkpMintTxErrorIdx = 1;   return rzt; }  // Mint Tx has record in db, return false
			// if( !bJustRecvTx && (czmk.nStatus > 0) ){ gLastCheckZkpMintTxErrorIdx = 2;   return rzt; }  // Mint Tx key has used, return false
			if( !bInOtherChain && !bJustRecvTx && (czmk.nStatus > 0) ){ gLastCheckZkpMintTxErrorIdx = 2;   return rzt; }  // Mint Tx key has used, return false
		}
		//CZeroKnowledgeProof czkp;
		//if( !ReadZeroKnowledgeProof(zkpmp.sZkpHash, czkp) ){ return rzt; }
		if( !bInOtherChain ){  if( czkp.NotActive() ){ gLastCheckZkpMintTxErrorIdx = 3;   return rzt; }  }  // nStatus <= 0;
		int64_t i6Amount=0;      std::string sToAddress="";
		bool ab = getTxOutAmountAndAddress(tx.vout, i6Amount, sToAddress);
		if( fDebug ){ printf("isValidZkpMintTx(%s), call getTxOutAmountAndAddress() return [%d], i6Amount=[%s], sToAddress=[%s] \n", sHash.c_str(), ab, int64tostr(i6Amount).c_str(), sToAddress.c_str()); }
		if( ab )
		{
			if( !IsValidBurnOrMintCoinAmount(nHeight, i6Amount) )
			{
				printf("isValidZkpMintTx() : invalid mint coin amount [%s] \n", int64tostr(i6Amount).c_str());   gLastCheckZkpMintTxErrorIdx = 4;   return rzt;
			}
			if( fDebug ){ printf("isValidZkpMintTx(%s), i6Amount=[%s :: %s :: %s] \n", sHash.c_str(), int64tostr(i6Amount).c_str(), int64tostr(czkp.n6Spent).c_str(), int64tostr(czkp.n6Coins).c_str()); }
			if( bJustRecvTx && !czkp.CheckSpent(i6Amount) ){ gLastCheckZkpMintTxErrorIdx = 5;   return rzt; }
			// if( !czkp.CanSpent(i6Amount) ){ gLastCheckZkpMintTxErrorIdx = 6;   return rzt; }
			if( !bInOtherChain && !czkp.CanSpent(i6Amount) ){ gLastCheckZkpMintTxErrorIdx = 6;   return rzt; }
			string signAddr = czkp.GetProofKeys();
			bool bc = isValidZkpMintKey(i6Amount, sToAddress, signAddr, sMintKey);
			if( fDebug ){ printf("isValidZkpMintTx(%s), call isValidZkpMintKey() return [%d], isValidZkpMintKey(%s, %s, %s, %s) \n", sHash.c_str(), bc, int64tostr(i6Amount).c_str(), sToAddress.c_str(), signAddr.c_str(), sMintKey.c_str()); }
			if( bc )
			{
				rzt = true;
				if( bSaveMintKeyToDb )
				{
					CZeroKnowledgeProofMintKey zmk(sMintKey);
					if( bJustRecvTx )
					{
						saveZeroKnowledgeProofMintTxToDB(tx);
						if( czkp.n6EstSpent <= czkp.n6Coins ){ czkp.n6EstSpent += i6Amount;      saveZeroKnowledgeProofToDB(czkp); }
					}
					else{	zmk.nStatus = 1;      delZeroKnowledgeProofMintTxFromDB(tx); }
					saveZeroKnowledgeProofMintKeyToDB(zmk);
				}
				if( bUpdateSpentToDb ){ rzt = spentZeroKnowledgeProof(czkp, i6Amount); }
			}else{ gLastCheckZkpMintTxErrorIdx = 7; }
		}
	}else{ gLastCheckZkpMintTxErrorIdx = -1; }
	if( fDebug ){ printf("%s call isValidZkpMintTx(%s), rzt=[%d], k=[%d] \n\n", sCallFrom.c_str(), sHash.c_str(), rzt, k); }
	return rzt;
}

bool zkpmint(const string& sZkpHash, const std::vector<std::pair<CScript, int64_t> >& aVecSend, const string& sMintKey, string& sRztTxHash)
{
    bool rzt=false;      sRztTxHash="";
	if( (nBestHeight < nNewBlkVerActiveNum) || (!IsZkpRuleActived(nBestHeight)) ){ sRztTxHash = "ZKP rules not actived, please wait";   return rzt; }
	CZeroKnowledgeProof zkp;
	bool ba = ReadZeroKnowledgeProof(sZkpHash, zkp);
	if( fDebug ){ printf("zkpmint(%s, ..., %s), call ReadZeroKnowledgeProof() return [%d] \n", sZkpHash.c_str(), sMintKey.c_str(), ba); }
	if( ba )
	{
		//if( IsExistsZeroKnowledgeProofMintKey(sMintKey) ){ return rzt; }
		//std::vector<txOutPairPack > outPair;      int k = GetVectorPairTxOutDetails(aVecSend, outPair);
		//if( k < 1 ){ return rzt; }
		//int64_t nAmount = 0;
		//BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, aVecSend){ nAmount = nAmount + s.second; }
		//if( isValidZkpMintReq(zkp, nAmount, sMintKey) )
		{
			//string sAmount = int64tostr(nAmount);
			CWalletTx txNew;       txNew.SetNull();      txNew.vpndata = strprintf("ZKP-Mint|%s|%s|", sZkpHash.c_str(), sMintKey.c_str());
			//CScript scriptPubKey;     scriptPubKey.SetDestination(CBitcoinAddress(sToAddress).Get());
			//txNew.vout.push_back(CTxOut(nAmount, scriptPubKey));
			BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, aVecSend){ txNew.vout.push_back(CTxOut(s.second, s.first)); }
			if( BroadcastZkpMintTx(txNew) )
			{
				rzt=true;      sRztTxHash = txNew.GetHash().GetHex();      //return wtx.GetHash().GetHex();
				//addZkpMintKey(zkp, sMintKey, true);
			}else{
				if( gLastCheckZkpMintTxErrorIdx == 1 ){ sRztTxHash = "ZKP mint key exists"; }
				else if( gLastCheckZkpMintTxErrorIdx == 2 ){ sRztTxHash = "ZKP mint key has been used"; }
				else if( gLastCheckZkpMintTxErrorIdx == 3 ){ sRztTxHash = "ZKP not actived, please wait for blockchain sync"; }
				else if( gLastCheckZkpMintTxErrorIdx == 4 ){ sRztTxHash = "ZKP mint coin amount out of rules limit range"; }
				else if( gLastCheckZkpMintTxErrorIdx == 5 ){ sRztTxHash = "ZKP mint coin amount estimate to be out of range"; }
				else if( gLastCheckZkpMintTxErrorIdx == 6 ){ sRztTxHash = "ZKP mint coin amount overrun the total amount"; }
				else if( gLastCheckZkpMintTxErrorIdx == 7 ){ sRztTxHash = "Invalid mint key"; }
				//sRztTxHash = "ZKP not exists, please wait for blockchain sync";
			}
		}
	}else{ sRztTxHash = "ZKP not exists, please wait for blockchain sync"; }
	return rzt;
}

bool genZkpMintKey(const CZeroKnowledgeProof& czkp, int64_t i6Amount, const string& sToAddress, std::string& sRztMintKey)
{
	bool rzt=false;      sRztMintKey = "";      string strMessage="", signAddr="";
	//if( fDebug ){ printf("genZkpMintKey:: i6Amount=[%s], czkp.nProofStr=[%s], sToAddress=[%s] \n", int64tostr(i6Amount).c_str(), czkp.nProofStr.c_str(), sToAddress.c_str()); }
	if( czkp.CanSpent(i6Amount) )
	{
		strMessage = int64tostr(i6Amount) + "," + sToAddress;
		//if( fDebug ){ printf("genZkpMintKey:: strMessage=[%s] \n", strMessage.c_str()); }
		signAddr = czkp.GetProofKeys();
		sRztMintKey = signMessage(signAddr, strMessage);  // string signMessage(const string strAddress, const string strMessage);
		rzt = sRztMintKey.length() > 80;
	}
	if( fDebug ){ printf("genZkpMintKey:: rzt=[%d], strMessage=[%s] signAddr=[%s] \nsRztMintKey=[%s] \n", rzt, strMessage.c_str(), signAddr.c_str(), sRztMintKey.c_str()); }
	return rzt;
}

bool genZkpMintKey(const string& sZkpHash, int64_t i6Amount, const string& sToAddress, std::string& sRztMintKey)
{
	bool rzt=false;      sRztMintKey = "";      CZeroKnowledgeProof czkp;
	//if( fDebug ){ printf("genZkpMintKey:: i6Amount=[%s], sToAddress=[%s] \n", int64tostr(i6Amount).c_str(), sToAddress.c_str()); }
	if( ReadZeroKnowledgeProof(sZkpHash, czkp) )
	{
		rzt = genZkpMintKey(czkp, i6Amount, sToAddress, sRztMintKey);
	}else if( fDebug ){ printf("genZkpMintKey:: rzt=[%d], sZkpHash [%s] not exists :(\n", rzt, sZkpHash.c_str()); }
	return rzt;
}

bool getVectorPairTxOutAmountAndAddress(const std::vector<std::pair<CScript, int64_t> >& aVecSend, int64_t& i6Amount, std::string& sAddress)
{
	bool rzt = false;      i6Amount = 0;      sAddress = "";      int j = aVecSend.size();
	for( int i = 0; i < j; i++)
	{
		const PAIRTYPE(CScript, int64_t)& vp = aVecSend[i];
		i6Amount = i6Amount + vp.second;      string sa = getCoinAddressFromCScript(vp.first);
		if( sAddress.length() < 30 ){ sAddress = sa; }
		else{ sAddress = sAddress + "," + sa; }
	}
	rzt = i6Amount > 0;
	if( fDebug ){ printf("getVectorPairTxOutAmountAndAddress:: rzt=[%d], i6Amount=[%d] [%s] \n", rzt, i6Amount / COIN, sAddress.c_str()); }
	return rzt;
}

bool zkpmint(const string& sZkpHash, const std::vector<std::pair<CScript, int64_t> >& aVecSend, string& sRztTxHash)
{
	bool rzt = false;      sRztTxHash="";
	if( (nBestHeight < nNewBlkVerActiveNum) || (!IsZkpRuleActived(nBestHeight)) ){ sRztTxHash = "ZKP rules not actived, please wait";   return rzt; }
	CZeroKnowledgeProof czkp;
	if( ReadZeroKnowledgeProof(sZkpHash, czkp) )
	{
		int64_t i6Amount=0;      std::string sToAddress="",  sMintKey = "";
		if( getVectorPairTxOutAmountAndAddress(aVecSend, i6Amount, sToAddress) )
		{
			if( genZkpMintKey(czkp, i6Amount, sToAddress, sMintKey) )
			{
				if( IsExistsZeroKnowledgeProofMintKey(sMintKey) ){ return rzt; }
				CWalletTx txNew;       txNew.SetNull();      txNew.vpndata = strprintf("%s|%s|%s|", sZKPMint_Flags.c_str(), sZkpHash.c_str(), sMintKey.c_str());
				BOOST_FOREACH (const PAIRTYPE(CScript, int64_t)& s, aVecSend){ txNew.vout.push_back(CTxOut(s.second, s.first)); }
				if( BroadcastZkpMintTx(txNew) )
				{
					rzt=true;      sRztTxHash = txNew.GetHash().GetHex();      //return wtx.GetHash().GetHex();
				}else{ sRztTxHash = "ZKP not exists, please wait for blockchain sync"; }
			}
		}
	}else{ sRztTxHash = "ZKP not exists, please wait for blockchain sync"; }
	return rzt;
}

bool zkpBurn(int iBurnCoins, const string sPubKey, string& rztTxHash)
{
    //"zkpburn <burn coins> <proof key>\nzkpburn 100 VpnZKPAEd1HnSzJqfQ6x2yThdKpEEQA1HA");  //"zkpburn <burn coins> <proof key> <proof type>\nzkpburn 100 VpnZKPAEd1HnSzJqfQ6x2yThdKpEEQA1HA 0");
	bool rzt = false;      int iProofType = 0;      rztTxHash = "";
	if( (nBestHeight < nNewBlkVerActiveNum) || (!IsZkpRuleActived(nBestHeight)) ){ rztTxHash = "ZKP rules not actived, please wait";   return rzt; }
    int64_t nAmount = iBurnCoins * COIN;
	if( nAmount < Min_Burn_Coin_Amount ){ rztTxHash = "Burn coins must big than 99";   return rzt; }
	if( !IsValidBurnOrMintCoinAmount(nBestHeight, nAmount) ){ rztTxHash = "Burn coins out of range";   return rzt; }
	if( zkpClassExists(iProofType, nAmount, sPubKey) ){ rztTxHash = "ZKP exists, please change (burn coins) or change (public key)";      return rzt; }

    CWalletTx wtx;      string stxData = sPubKey;  // + "|" + sProofType;
	int bEncrypt = 0;
    if (pwalletMain->IsLocked()){ rztTxHash = "Please enter the wallet passphrase with walletpassphrase first";      return rzt; }
    CBitcoinAddress address(sZeroKnowledgeProofPoolAddress);
	string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx, stxData, bEncrypt);
    if (strError != ""){ rztTxHash = strError;   return rzt; }
	if( addOrEraseZkpTx(wtx, nBestHeight, true, true) > 0 )   // int addOrEraseZkpTx(const CTransaction& tx, bool bAdd, bool bJustRecvTx)
	{
		rzt =  true;      uint256 hash = getZkpHashFromParams(iProofType, nAmount, sPubKey, true);
		rztTxHash = hash.GetHex();
	}
	return rzt;
}


bool addZkp(const int64_t i6OutCoin, const string sPubKey, const string sTagHash)
{
	bool rzt=false;
	CZeroKnowledgeProof zkp(i6OutCoin, sPubKey);
	uint256 hash = zkp.GetHash(), h2=0;   h2.SetHex(sTagHash);
	bool bSame = (hash == h2);
	if( fDebug ) printf("addZkp hash=[%s], bSame=[%d] \n", hash.ToString().c_str(), bSame);
	if( bSame )
	{
		bool bZkpExists = ReadZeroKnowledgeProof(hash, zkp);
		if( fDebug ) printf("addZkp hash=[%s], bZkpExists=[%d] \n", hash.ToString().c_str(), bZkpExists);
		if( !bZkpExists )
		{
			zkp.nStatus = 1;
			if( saveZeroKnowledgeProofToDB(zkp) ){ rzt=true; }
		}
	}
	return rzt;			
}

void initZkpForVer1703()
{
    int64_t i6OutCoin = 1000 * COIN;
	addZkp(i6OutCoin, "VauHn3vZ35Vc3jrZmeuNRrkLeSy6Aj4vGD", "6e851e11bdeb438c4eb644ab473c9e4384eb21b79d8794af5c5b5cada8b2cc7a");
	addZkp(i6OutCoin, "VtPgN26uyKdEm4UmsD15xxhJGXbPLJpGKk", "aa958765423d476a7555b3b464729d0102aa5cd2880e161ef0c3f06ed31de31a");
	addZkp(i6OutCoin, "VobHoSXkDTTvAARVLKrZyC7j1bYxKmYVYN", "a05da3a447dafdca31af6eb72f9549a0ac8f0b9493a444c25a319d0d494c513d");
	addZkp(i6OutCoin, "VjMtav9MdXohPwUo4SksWf39oDxpXa1G1x", "e82ded746a947bfa332691c3e46cebf3a7fecd4d23f15aec79dc1c7102427fa5");
	addZkp(i6OutCoin, "VceeRQ3cGaXAV48PNfspbxP6QFvTS6Czxf", "f9c88f8b0eac08f74ff6aaaa7874c8ed5f4ce6ccb99ba50f0c82d525959d4ff5");
	addZkp(i6OutCoin, "VhhCRzDFMG8yGEUTHwAt2GrU9aKNdAMgMU", "7ff7fcee7311681d816afe6c07a0da6dea8e4ed31272cffa5bb6fcfdc1daa388");
	addZkp(i6OutCoin, "VonznQbDuvJvmCfR7SKGCDEyPBMx4Qp3m1", "5ad43da306ca53b0304c197ca2ca10021ceff802f1560fdb3d47954c882082f9");
}