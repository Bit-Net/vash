// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee
#ifndef BITCHAIN_H
#define BITCHAIN_H

#ifdef USE_BITNET 
#include "bitnet.h"
#endif

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

using namespace std;
using namespace boost;

extern std::string sQkl_domain;  // = "www.qkl.im";
extern std::string BitChain_Head;  // = "BitChainP:";
extern string s_BlockChain_Dir;
extern string sBitChainIdentAddress;
extern string sBitChainKeyAddress;
extern int writeBufToFile(char* pBuf, int bufLen, string fName);
extern string signMessage(const string strAddress, const string strMessage);
extern int signMessageWithoutBase64(const string strAddress, const string strMessage, string& sRzt);

/*int getFileBinData(string sFile, string& sRzt)
{
    int rzt = 0;
    ifstream is;
	sRzt.resize(0);
	is.open( sFile.c_str(), ios::binary);
	if( is.is_open() )
	{
        is.seekg(0, ios::end);   
        int iMaxSz = is.tellg();
		//printf("iMaxSz = %d \n", iMaxSz);
		if( (iMaxSz > 0) && (iMaxSz < ( 3 * 1024 * 1024 ) ) )
		{
            is.seekg(0, ios::beg);   
			sRzt.resize(iMaxSz);
			//printf("sRzt.size() = %d \n", sRzt.size());
			char* p = (char*)sRzt.c_str();
			is.read(&p[0], iMaxSz);
			//writeBufToFile((char*)sRzt.c_str(), iMaxSz, s_BlockChain_Dir + "getFileBinData.bin");
			rzt = iMaxSz;
		}
		is.close();
	}
	return rzt;
}*/

class CBitChain
{
public:
    unsigned int nVersion;
    unsigned int nTime;
	std::string nFromAddr;
	std::string nToAddr;
    int nChainType;
    char nAllowReply;
    std::string nNickName;
    std::string nTopic;                      // If it is a tx id (64 bytes), it mean that it a reply, else it is a title
    std::string nContentText;
    std::string sImgData;
    std::string sVoiceData;
    std::string sVideoData;
    //std::vector<char> nImgData;
    //std::vector<char> nVoiceData;
    //std::vector<char> nVideoData;
    std::string sExternalChainUrl;
    std::string sExternalChainHash;
    std::string nExternalChainData;
    unsigned int nLockTime;

    CBitChain()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nTime);
        READWRITE(nFromAddr);
        READWRITE(nToAddr);
        READWRITE(nChainType);
        READWRITE(nAllowReply);
        READWRITE(nNickName);
        READWRITE(nTopic);
        READWRITE(nContentText);
        READWRITE(sImgData);
        READWRITE(sVoiceData);
        READWRITE(sVideoData);
        READWRITE(sExternalChainUrl);
        READWRITE(sExternalChainHash);
        READWRITE(nExternalChainData);
        READWRITE(nLockTime);
    )

    void SetNull()
    {
        nVersion = 1;
        nTime = GetAdjustedTime();
        sImgData = "";     sVoiceData = "";     sVideoData = "";     nFromAddr = "";  //.clear();
	    nToAddr = "";     nChainType = 0;      nAllowReply = 1;
        nNickName = "";	    nTopic = "";     nContentText = "";
		sExternalChainUrl = "";     sExternalChainHash = "";     nExternalChainData = "";     nLockTime = 0;
    }

    bool IsNull() const
    {
        return ( nContentText.empty() && sExternalChainUrl.empty() && sImgData.empty() && sVoiceData.empty() && sVideoData.empty() && nExternalChainData.empty() );
    }

    //CBitChain(unsigned int nVersionIn = 1, string nFromAddrIn = "", string nToAddrIn = "", unsigned int nChainTypeIn, string nTopicIn, string nContentTextIn, string nContextUrlIn, char* pMediaIn, unsigned int nMediaLen, string nExternalChainDataIn = "")
    //CBitChain(int nChainTypeIn, string nNickNameIn, string nTopicIn, string nContentTextIn, string nContextUrlIn, char* pMediaIn, int nMediaLen, char nAllowReplyIn, string nFromAddrIn = "", string nToAddrIn = "", string nExternalChainDataIn = "")
    CBitChain(int nChainTypeIn, string nNickNameIn, string nTopicIn, string nContentTextIn, string nContextUrlIn, string sImgDataIn, string sVoiceDataIn, 
                     string sVideoDataIn, char nAllowReplyIn, string nFromAddrIn = "", string nToAddrIn = "", string nExternalChainDataIn = "")
    {
        SetNull();
        //nVersion = nVersionIn;
        nChainType = nChainTypeIn;
        nNickName = nNickNameIn;     nTopic = nTopicIn;     nContentText = nContentTextIn;
        sExternalChainUrl = nContextUrlIn;    	sImgData = sImgDataIn;     sVoiceData = sVoiceDataIn;     sVideoData = sVideoDataIn;
		nAllowReply = nAllowReplyIn;     nFromAddr = nFromAddrIn;     nToAddr = nToAddrIn;
        
		//printf("pMediaIn = %d, nMediaLen = %d \n", pMediaIn, nMediaLen);
        /*if( (pMediaIn != NULL) && (nMediaLen > 0) )
        {
            nImgData.resize(nMediaLen);
			memcpy((char*)&nImgData[0], pMediaIn, nMediaLen); 
            //nImgData.write(pMediaIn, nMediaLen);  //std::vector<char> nMediaData;
        }*/
        //std::vector<char> nVoiceData;
        //std::vector<char> nVideoData;
        nExternalChainData = nExternalChainDataIn;
    }

    void setMediaData(string sImgDataIn, string sVoiceDataIn, string sVideoDataIn)
    {
        sImgData = sImgDataIn;     sVoiceData = sVoiceDataIn;     sVideoData = sVideoDataIn;
    }
/*    void setMediaData(char* pImgIn, int nImgLen, char* pVoiceIn, int nVoiceLen, char* pVideoIn, int nVideoLen)
    {
        if( (pImgIn != NULL) && (nImgLen > 0) )
        {
            nImgData.resize(nImgLen);
			memcpy((char*)&nImgData[0], pImgIn, nImgLen); 
        }
        if( (pVoiceIn != NULL) && (nVoiceLen > 0) )
        {
            nVoiceData.resize(nVoiceLen);
			memcpy((char*)&nVoiceData[0], pVoiceIn, nVoiceLen); 
        }
        if( (pVideoIn != NULL) && (nVideoLen > 0) )
        {
            nVideoData.resize(nVideoLen);
			memcpy((char*)&nVideoData[0], pVideoIn, nVideoLen); 
        }
    } */
    /*CBitChain(unsigned int nVersionIn, string nFromAddrIn, string nToAddrIn, unsigned int nChainTypeIn, string nTopicIn, string nContentTextIn, string nContextUrlIn, char* pMediaIn, unsigned int nMediaLen, string nExternalChainDataIn)
    {
        CBitChain();
    }*/

    uint256 GetHash() const
    {
        return SerializeHash(*this);
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

class CBitChainProtocol
{
public:
	std::string nBitChainHeadStr;
    unsigned int nVersion;
    unsigned int nTime;
	std::string nFromAddr;
	CKeyID nFromAddrKey;
	std::string nToWho;      // NickName
    int nDatType;               // 0 = Chain Data, 1 = Msg Data
    unsigned char nAES;
    std::string nChainDat;  //std::vector<char> nBinData;
	std::string nSignTxt;
    uint256 bitChainHash;

    CBitChainProtocol()
    {
        SetNull();
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(nBitChainHeadStr);
        //nBitChainHeadStr = this->nBitChainHeadStr;
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nTime);
        READWRITE(nFromAddr);     READWRITE(nFromAddrKey);
        READWRITE(nToWho);
        READWRITE(nDatType);
        READWRITE(nAES);
        READWRITE(nChainDat);
        READWRITE(nSignTxt);     READWRITE(bitChainHash);
    )

    void SetNull()
    {
        nBitChainHeadStr = BitChain_Head;  //"BitChainP:";
        nVersion = 1;     nTime = GetAdjustedTime();     nChainDat = "";  //nBinData.clear();
	    nFromAddrKey = CKeyID(0);     nFromAddr = "";     nToWho = "";     nDatType = 0;
        nAES = 0;     nSignTxt = "";     bitChainHash = 0;
    }

    bool IsNull() const
    {
        return ( nChainDat.empty() );
    }

	void signChainData(const string strMessage)
	{
		if( strMessage.length() > 0 )
		{
			CBitcoinAddress address(sBitChainIdentAddress);
			if( address.IsValid() )
			{
				if( address.GetKeyID(nFromAddrKey) )  //nFromAddr = sBitChainIdentAddress
				{
					//nFromAddr = sBitChainIdentAddress;  	//int signMessageWithoutBase64(const string strAddress, const string strMessage, string& sRzt)
					signMessageWithoutBase64(sBitChainIdentAddress, strMessage, nSignTxt);
					//nSignTxt = signMessage(nFromAddr, strMessage);
				}else nChainDat.resize(0);
			}else nChainDat.resize(0);
		}
	}
    void setBinData(char* pDatIn, int nDatLen)
    {
        if( (pDatIn != NULL) && (nDatLen > 0) )
        {
            nChainDat.resize(nDatLen);
			memcpy((char*)nChainDat.c_str(), pDatIn, nDatLen);  //memcpy((char*)&nBinData[0], pDatIn, nDatLen); 
        }
    }
 
    CBitChainProtocol(char* pDatIn, int nDatLen)
    {
        SetNull();
        setBinData(pDatIn, nDatLen);
    }

    CBitChainProtocol(string sChainDat_In, int tabId, uint256 bitChainHash_In, int nAES_In=0)
    {
        SetNull();     nDatType = tabId;
		if( sChainDat_In.length() > 0 )
		{
        setBinData((char*)sChainDat_In.c_str(), sChainDat_In.length());
		signChainData( sChainDat_In );     bitChainHash = bitChainHash_In;
		if( (nAES_In > 0) && (sBitChainKeyAddress.length() > 31) )
		{
			int i = nChainDat.length();
			if( i > 32 )
			{
				char* pOrg = (char*)nChainDat.c_str();
				string sOrg(pOrg, 32);
				AES aes(Bits256, (unsigned char*)sBitChainKeyAddress.c_str());
				string sAES = "";
				int i = aes.Cipher(sOrg, sAES);
				if( i == 36 )
				{
					char* pAes = (char*)sAES.c_str();     memcpy(pOrg, pAes + 4, 32);     nAES = nAES_In;
				}
				sAES.resize(0);
			}
		}
		}
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }
};

#endif
