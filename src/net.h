// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include <deque>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <openssl/rand.h>


#ifndef WIN32
#include <arpa/inet.h>
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef void* PVOID;
#endif

#include "mruset.h"
#include "netbase.h"
#include "protocol.h"
#include "addrman.h"

using namespace std;
using namespace boost;

class CRequestTracker;
class CNode;
class CBlockIndex;
extern int nBestHeight;
extern int BitNet_Version;
extern int BitNet_Network_id;
extern int fNetDbg;

#pragma pack (1)
struct Bitnet_node_struct
{
	DWORD v_Option;
	CNode *v_Node;
	DWORD v_IpAddr;
	DWORD v_LanId;
	DWORD v_RecvSize;
	DWORD v_isVpnServer;
	WORD  v_iVpnServicePort;
	DWORD v_bShowInOtherList;
	DWORD v_bShowWAddrInOtherNodes;
	WORD  v_iVpnServiceCtrlPort;
	int64_t v_iVpnServiceFee;
	DWORD v_iVpnServiceTryMinute;
	DWORD v_iTotalVpnConnects;
	DWORD v_iVpnSerCoinConfirms;
	WORD  v_P2P_proxy_port;
	std::string v_sDefWalletAddress;
	std::string v_Signature;	//v_sVpnWalletAddress
	std::string v_NicknamePack;
	std::string v_Nickname;
	std::string v_sVpnMemAndCpuInfo;
	DWORD v_iCanTalk;
	DWORD v_iVersion;
	DWORD v_Gui_Node_Index;
	DWORD v_Starting_recv;
	DWORD v_OpenSocketProxy;
	DWORD v_ProxyForThisNode;
	//-- Transaction file
	int64_t v_File_size;
	std::string v_File_Req;
	std::string v_File_Loc;
	std::string v_sTalkMsg;
	int64_t v_SyncNodeIpsTime;
	int64_t v_SendSyncNodeIpsTime;
	std::vector<char> v_RemoteFileBuf;
	std::vector<char> v_ProxyReqBuf;
	std::vector<char> v_ProxyAckBuf;
	std::vector<char> v_RecvIps;
	std::vector<char> v_ChatMsgPack;
	DWORD v_PingRTT;
	DWORD v_IsSendFile;
	DWORD v_ListItem;
	char  v_NickNamePackBuf[102];	//DWORD v_PNickNamePack;
	DWORD v_cSocketList;
	DWORD v_Network_id;
	WORD v_NodePort;
	WORD v_ListenPort;
	DWORD v_Rsv_ccc;
	char  v_Chat_FontSize;
	char  v_bCloseByRecvOverMaxRecvBufSz;
	DWORD v_LastSendTipTime;
	DWORD v_ChatWithAes;
	std::string v_AesKey;
	char  v_bSendingTip;
	char  v_bVoiceFlag;	// bit 1 = send voice to;	bit 2 = recv voice with
	char  v_bVideoFlag;	
	int64_t v_ConnectedTime;
	int v_Recv_ZeroBytes;
	DWORD v_VpnItem;
	DWORD v_BitNetMsgCount;
	int64_t v_LastPushMsgTime;
	char v_BitNetInfoReceived;
	char v_ReceivedMyBitNetInfo;
	unsigned char v_IsGuiNode;
	char v_OnlineRewardVpnTime;
	WORD v_OnlineXminReward_1Min;
	DWORD v_LastReqBitNetInfTime;
    DWORD v_LastReqNodesTime;
    DWORD v_LastGetReqNodesTime;
    DWORD v_LastGetRepNodesTime;
    DWORD v_LastBoost2OthersTime;
};
#pragma pack ()

void RelayShutDown();
void PushBoostMe(CNode* pNode);
void PushBoostMeToAll();
int BoostThisNodeToWorld(CNode* aNode);
void AddOrDelWalletIpPorts(CNode* pNode, int iOpt);
void AddIpPortToOneShot(unsigned int dIp, WORD wPort);
int ProcessIncomeIpPorts(std::vector<std::pair<unsigned int, WORD> > vNodeIPorts);
unsigned int getNeedSyncBlocks();
bool isBoostSyncBlockMode();

int BitNetBonusExist(string sBonusId);  // 2015.06.12 add
string GetBitNetBonusMsg(string sBonusId);
int AddBitNetBonus(string sBonusId, string sBonus);
int RelayBonusID(string sBonusId);


/** Time between pings automatically sent out for latency probing and keepalive (in seconds). */
static const int PING_INTERVAL = 2 * 60;
/** Time after which to disconnect, after waiting for a ping response (or inactivity). */
static const int TIMEOUT_INTERVAL = 20 * 60;

inline unsigned int ReceiveFloodSize() { return 1000*GetArg("-maxreceivebuffer", 5*1000); }
inline unsigned int SendBufferSize() { return 1000*GetArg("-maxsendbuffer", 1*1000); }

void AddOneShot(std::string strDest, int bPushFront = 0);	//void AddOneShot(std::string strDest);
bool RecvLine(SOCKET hSocket, std::string& strLine);
bool GetMyExternalIP(CNetAddr& ipRet);
void AddressCurrentlyConnected(const CService& addr);
CNode* FindNode(const CNetAddr& ip);
CNode* FindNode(const std::string& addrName);
CNode* FindNode(const CService& ip);
CNode* FindNode(const CNode* node);
CNode* ConnectNode(CAddress addrConnect, const char *strDest = NULL, int iPort = 0);
void MapPort();
unsigned short GetListenPort();
bool BindListenPort(const CService &bindAddr, std::string& strError=REF(std::string()));
void StartNode(void* parg);
bool StopNode();
void SocketSendData(CNode *pnode);
DWORD SyncNodeIpPort(DWORD ip, DWORD port);

enum
{
    LOCAL_NONE,   // unknown
    LOCAL_IF,     // address a local interface listens on
    LOCAL_BIND,   // address explicit bound to
    LOCAL_UPNP,   // address reported by UPnP
    LOCAL_IRC,    // address reported by IRC (deprecated)
    LOCAL_HTTP,   // address reported by whatismyip.com and similar
    LOCAL_MANUAL, // address explicitly specified (-externalip=)

    LOCAL_MAX
};

void SetLimited(enum Network net, bool fLimited = true);
bool IsLimited(enum Network net);
bool IsLimited(const CNetAddr& addr);
bool AddLocal(const CService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& addr, int nScore = LOCAL_NONE);
bool SeenLocal(const CService& addr);
bool IsLocal(const CService& addr);
bool GetLocal(CService &addr, const CNetAddr *paddrPeer = NULL);
bool IsReachable(const CNetAddr &addr);
void SetReachable(enum Network net, bool fFlag = true);
CAddress GetLocalAddress(const CNetAddr *paddrPeer = NULL);


enum
{
    MSG_TX = 1,
    MSG_BLOCK,
    MSG_BLKZP,
};

class CRequestTracker
{
public:
    void (*fn)(void*, CDataStream&);
    void* param1;

    explicit CRequestTracker(void (*fnIn)(void*, CDataStream&)=NULL, void* param1In=NULL)
    {
        fn = fnIn;
        param1 = param1In;
    }

    bool IsNull()
    {
        return fn == NULL;
    }
};


/** Thread types */
enum threadId
{
    THREAD_SOCKETHANDLER,
    THREAD_OPENCONNECTIONS,
    THREAD_MESSAGEHANDLER,
    THREAD_RPCLISTENER,
    THREAD_UPNP,
    THREAD_DNSSEED,
    THREAD_ADDEDCONNECTIONS,
    THREAD_DUMPADDRESS,
    THREAD_RPCHANDLER,
    THREAD_STAKE_MINER,
    THREAD_OPENOneShotNode,
    THREAD_MAX
};

extern bool fDiscover;
extern bool fUseUPnP;
extern uint64_t nLocalServices;
extern uint64_t nLocalHostNonce;
extern CAddress addrSeenByPeer;
extern boost::array<int, THREAD_MAX> vnThreadsRunning;
extern CAddrMan addrman;

extern std::vector<CNode*> vNodes;
extern CCriticalSection cs_vNodes;
extern std::map<CInv, CDataStream> mapRelay;
extern std::deque<std::pair<int64_t, CInv> > vRelayExpiration;
extern CCriticalSection cs_mapRelay;
extern std::map<CInv, int64_t> mapAlreadyAskedFor;
extern std::vector<std::pair<unsigned int, WORD> > vAllNodeIpPorts;	// 2015.06.06 add



class CNodeStats
{
public:
    uint64_t nServices;
    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    std::string addrName;
    int nVersion;
    std::string strSubVer;
    bool fInbound;
    int nStartingHeight;
    int nMisbehavior;
    double dPingTime;
    double dPingWait;
    char IsGuiNode;
};




class CNetMessage {
public:
    bool in_data;                   // parsing header (false) or data (true)

    CDataStream hdrbuf;             // partially received header
    CMessageHeader hdr;             // complete header
    unsigned int nHdrPos;

    CDataStream vRecv;              // received message data
    unsigned int nDataPos;

    int64_t nTime;                  // time (in microseconds) of message receipt.

    CNetMessage(int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), vRecv(nTypeIn, nVersionIn) {
        hdrbuf.resize(24);
        in_data = false;
        nHdrPos = 0;
        nDataPos = 0;
        nTime = 0;
    }

    bool complete() const
    {
        if (!in_data)
            return false;
        return (hdr.nMessageSize == nDataPos);
    }

    void SetVersion(int nVersionIn)
    {
        hdrbuf.SetVersion(nVersionIn);
        vRecv.SetVersion(nVersionIn);
    }

    int readHeader(const char *pch, unsigned int nBytes);
    int readData(const char *pch, unsigned int nBytes);
};





/** Information about a peer */
class CNode
{
public:
    // socket
    uint64_t nServices;
    SOCKET hSocket;
    CDataStream ssSend;
    size_t nSendSize; // total size of all vSendMsg entries
    size_t nSendOffset; // offset inside the first vSendMsg already sent
    std::deque<CSerializeData> vSendMsg;
    CCriticalSection cs_vSend;

    std::deque<CNetMessage> vRecvMsg;
    CCriticalSection cs_vRecvMsg;
    int nRecvVersion;

    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    CAddress addr;
    std::string addrName;
    CService addrLocal;
    int nVersion;
    std::string strSubVer;
    bool fOneShot;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    //bool fDisconnect;
    CSemaphoreGrant grantOutbound;
    int nRefCount;
	bool fDisconnect;
	
	/*DWORD sAddr;
	DWORD v_LanId;
	DWORD v_isVpnServer;
	WORD v_iVpnServicePort;
	DWORD v_bShowInOtherList;
	DWORD v_bShowWAddrInOtherNodes;
	WORD v_iVpnServiceCtrlPort;
	int64_t v_iVpnServiceFee;
	DWORD v_iVpnServiceTryMinute;
	DWORD v_iTotalVpnConnects;
	DWORD v_iVpnSerCoinConfirms;
	WORD v_P2P_proxy_port;
	std::string v_sDefWalletAddress;
	std::string v_sVpnWalletAddress;
	std::string v_NicknamePack;
	std::string v_Nickname;
	std::string v_sVpnMemAndCpuInfo;
	DWORD v_iCanTalk;
	DWORD v_iVersion;
	DWORD v_Starting_recv;
	DWORD v_OpenSocketProxy;
	DWORD v_Gui_Node_Index;
	//-- Transaction file
	int64_t v_File_size;
	std::string v_File_Req;
	std::string v_sTalkMsg;
	std::vector<char> v_RemoteFileBuf;
	std::string v_File_Loc;
	int64_t v_SyncNodeIpsTime;
	int64_t v_SendSyncNodeIpsTime;
	DWORD v_Network_id; */
	Bitnet_node_struct vBitNet;
    int zipblock;
protected:

    // Denial-of-service detection/prevention
    // Key is IP address, value is banned-until-time
    static std::map<CNetAddr, int64_t> setBanned;
    static CCriticalSection cs_setBanned;
    int nMisbehavior;

public:
    std::map<uint256, CRequestTracker> mapRequests;
    CCriticalSection cs_mapRequests;
    uint256 hashContinue;
    CBlockIndex* pindexLastGetBlocksBegin;
    uint256 hashLastGetBlocksEnd;
    int nStartingHeight;

    // flood relay
    std::vector<CAddress> vAddrToSend;
    mruset<CAddress> setAddrKnown;
    bool fGetAddr;
    std::set<uint256> setKnown;
    uint256 hashCheckpointKnown; // ppcoin: known sent sync-checkpoint

    // inventory based relay
    mruset<CInv> setInventoryKnown;
    std::vector<CInv> vInventoryToSend;
    CCriticalSection cs_inventory;
    std::multimap<int64_t, CInv> mapAskFor;

    // Ping time measurement:
    // The pong reply we're expecting, or 0 if no pong expected.
    uint64_t nPingNonceSent;
    // Time (in usec) the last ping was sent, or 0 if no ping was ever sent.
    int64_t nPingUsecStart;
    // Last measured round-trip time.
    int64_t nPingUsecTime;
    int64_t nGotPongTime;  // 2015.07.23 add
	unsigned char bSupportBoostSyncBlock;  // 2015.07.24 add
	unsigned int iSyncBlockIncremental;           // 2015.08.19 add
    // Whether a ping is requested.
    bool fPingQueued;

    CNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn=false) : ssSend(SER_NETWORK, INIT_PROTO_VERSION), setAddrKnown(5000)
    {
        nServices = 0;
        hSocket = hSocketIn;
        nRecvVersion = INIT_PROTO_VERSION;
        nLastSend = 0;
        nLastRecv = 0;
        nTimeConnected = GetTime();
        addr = addrIn;
        addrName = addrNameIn == "" ? addr.ToStringIPPort() : addrNameIn;
        nVersion = 0;
        strSubVer = "";
        fOneShot = false;
        fClient = false; // set by version message
        fInbound = fInboundIn;
        fNetworkNode = false;
        fSuccessfullyConnected = false;
        fDisconnect = false;
        nRefCount = 0;
        nSendSize = 0;
        nSendOffset = 0;
        hashContinue = 0;
        pindexLastGetBlocksBegin = 0;
        hashLastGetBlocksEnd = 0;
        nStartingHeight = -1;
        fGetAddr = false;
        nMisbehavior = 0;
        hashCheckpointKnown = 0;
        setInventoryKnown.max_size(SendBufferSize() / 1000);
        nPingNonceSent = 0;
        nPingUsecStart = 0;
        nPingUsecTime = 0;
		nGotPongTime = 0;
		bSupportBoostSyncBlock = 0;
		iSyncBlockIncremental = 0;
        fPingQueued = false;

		vBitNet.v_Option = 0;
		vBitNet.v_Node = this;
		vBitNet.v_IpAddr = 0;
		vBitNet.v_iVersion = 0;
		vBitNet.v_LanId = 0;
		vBitNet.v_RecvSize = 0;
		vBitNet.v_isVpnServer = 0;
		vBitNet.v_bShowInOtherList = 0;
		vBitNet.v_bShowWAddrInOtherNodes = 0;
		vBitNet.v_iVpnServicePort = 923;
		vBitNet.v_iVpnServiceCtrlPort = 922;
		vBitNet.v_iVpnServiceFee = 0;
		vBitNet.v_iVpnServiceTryMinute = 0;
		vBitNet.v_iTotalVpnConnects = 0;
		vBitNet.v_iVpnSerCoinConfirms = 0;
		vBitNet.v_Signature = "";	//v_sVpnWalletAddress = "";
		vBitNet.v_sDefWalletAddress = "";
		vBitNet.v_sTalkMsg = "";
		vBitNet.v_RemoteFileBuf.clear();
		vBitNet.v_ProxyReqBuf.clear();
		vBitNet.v_ProxyAckBuf.clear();
		vBitNet.v_RecvIps.clear();
		vBitNet.v_ChatMsgPack.clear();
		vBitNet.v_NicknamePack = "";
		vBitNet.v_Nickname = "";
		vBitNet.v_iCanTalk = 1;
		vBitNet.v_OpenSocketProxy = 0;
		vBitNet.v_ProxyForThisNode = 0;
		vBitNet.v_File_size = 0;
		vBitNet.v_Starting_recv = 0;
		vBitNet.v_File_Req = "";
		vBitNet.v_File_Loc = "";
		vBitNet.v_sVpnMemAndCpuInfo = "";
		vBitNet.v_SyncNodeIpsTime = 0;
		vBitNet.v_SendSyncNodeIpsTime = 0;
		vBitNet.v_P2P_proxy_port = 0;
		vBitNet.v_Gui_Node_Index = 0;
		vBitNet.v_ListItem = 0;
		vBitNet.v_PingRTT = 0xFFFFFFFF;
		vBitNet.v_IsSendFile = 0;
		memset(vBitNet.v_NickNamePackBuf, 0, sizeof(vBitNet.v_NickNamePackBuf));	//vBitNet.v_PNickNamePack = 0;
		vBitNet.v_cSocketList = 0;
		vBitNet.v_Network_id = 0;
		vBitNet.v_NodePort = 0;
		vBitNet.v_ListenPort = 0;	//920;	//GetListenPort();
		vBitNet.v_Rsv_ccc = 0;
		vBitNet.v_Chat_FontSize = 0;
		vBitNet.v_bCloseByRecvOverMaxRecvBufSz = 0;
		vBitNet.v_bSendingTip = 0;
		vBitNet.v_OnlineRewardVpnTime = 0;		// 2015.05.15 add
		vBitNet.v_OnlineXminReward_1Min = 0;
        
		vBitNet.v_bVoiceFlag = 0;	// 14.10.25 add,  bit 1 = send voice to;	bit 2 = recv voice with
        vBitNet.v_bVideoFlag = 0;		
		
		vBitNet.v_LastSendTipTime = 0;
		vBitNet.v_ChatWithAes = 0;
		vBitNet.v_AesKey = "";
		vBitNet.v_ConnectedTime = nTimeConnected;
		vBitNet.v_Recv_ZeroBytes = 0;
		vBitNet.v_VpnItem = 0;
		vBitNet.v_BitNetMsgCount = 0;
		vBitNet.v_LastPushMsgTime = 0;
		vBitNet.v_BitNetInfoReceived = 0;
		vBitNet.v_ReceivedMyBitNetInfo = 0;
		vBitNet.v_IsGuiNode = 0;
		vBitNet.v_LastReqNodesTime = 0;
		vBitNet.v_LastGetReqNodesTime = 0;
		vBitNet.v_LastGetRepNodesTime = 0;
		vBitNet.v_LastReqBitNetInfTime = 0;
		vBitNet.v_LastBoost2OthersTime = 0;
        zipblock = 0;

		//if( fDebug ){ printf("CNode Creater \n"); }

        // Be shy and don't send version until we hear
        if (hSocket != INVALID_SOCKET && !fInbound)
		{
            //if( fDebug ){ printf("CNode Creater PushVersion()\n"); }
			PushVersion();
		}
    }

    ~CNode()
    {
        if (hSocket != INVALID_SOCKET)
        {
            closesocket(hSocket);
            hSocket = INVALID_SOCKET;
        }
    }

private:
    CNode(const CNode&);
    void operator=(const CNode&);
public:


    int GetRefCount()
    {
        assert(nRefCount >= 0);
        return nRefCount;
    }

    // requires LOCK(cs_vRecvMsg)
    unsigned int GetTotalRecvSize()
    {
        unsigned int total = 0;
        BOOST_FOREACH(const CNetMessage &msg, vRecvMsg) 
            total += msg.vRecv.size() + 24;
        return total;
    }

    // requires LOCK(cs_vRecvMsg)
    bool ReceiveMsgBytes(const char *pch, unsigned int nBytes);

    // requires LOCK(cs_vRecvMsg)
    void SetRecvVersion(int nVersionIn)
    {
        nRecvVersion = nVersionIn;
        BOOST_FOREACH(CNetMessage &msg, vRecvMsg)
            msg.SetVersion(nVersionIn);
    }

    CNode* AddRef()
    {
        nRefCount++;
        return this;
    }

    void Release()
    {
        nRefCount--;
    }



    void AddAddressKnown(const CAddress& addr)
    {
        setAddrKnown.insert(addr);
    }

    void PushAddress(const CAddress& addr)
    {
        // Known checking here is only to save space from duplicates.
        // SendMessages will filter it again for knowns that were added
        // after addresses were pushed.
        if (addr.IsValid() && !setAddrKnown.count(addr))
            vAddrToSend.push_back(addr);
    }


    void AddInventoryKnown(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            setInventoryKnown.insert(inv);
        }
    }

    void PushInventory(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            if (!setInventoryKnown.count(inv))
                vInventoryToSend.push_back(inv);
        }
    }

    void AskFor(const CInv& inv, bool fImmediateRetry = false)  //void AskFor(const CInv& inv)
    {
        // We're using mapAskFor as a priority queue,
        // the key is the earliest time the request can be sent
        int64_t& nRequestTime = mapAlreadyAskedFor[inv];
        if (fDebugNet)
            printf("askfor %s   %"PRId64" (%s)\n", inv.ToString().c_str(), nRequestTime, DateTimeStrFormat("%H:%M:%S", nRequestTime/1000000).c_str());

        // Make sure not to reuse time indexes to keep things in the same order
        int64_t nNow = (GetTime() - 1) * 1000000;
        static int64_t nLastTime;
        ++nLastTime;
        nNow = std::max(nNow, nLastTime);
        nLastTime = nNow;

        // Each retry is 2 minutes after the last
        if (fImmediateRetry)
            nRequestTime = nNow;
        else
            nRequestTime = std::max(nRequestTime + 2 * 60 * 1000000, nNow);
        mapAskFor.insert(std::make_pair(nRequestTime, inv));
    }



    void BeginMessage(const char* pszCommand)
    {
        ENTER_CRITICAL_SECTION(cs_vSend);
        assert(ssSend.size() == 0);
        ssSend << CMessageHeader(pszCommand, 0);
        if( fNetDbg )
            printf("%I64u :: CNode::BeginMessage: [%s] to [%s] ", GetTime(), pszCommand, addr.ToString().c_str());
    }

    void AbortMessage()
    {
        ssSend.clear();

        LEAVE_CRITICAL_SECTION(cs_vSend);

        if (fDebug)
            printf("(aborted)\n");
    }

    void EndMessage()
    {
        if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
        {
            printf("dropmessages DROPPING SEND MESSAGE\n");
            AbortMessage();
            return;
        }

        if (ssSend.size() == 0)
            return;

        // Set the size
        unsigned int nSize = ssSend.size() - CMessageHeader::HEADER_SIZE;
        memcpy((char*)&ssSend[CMessageHeader::MESSAGE_SIZE_OFFSET], &nSize, sizeof(nSize));

        // Set the checksum
        uint256 hash = Hash(ssSend.begin() + CMessageHeader::HEADER_SIZE, ssSend.end());
        unsigned int nChecksum = 0;
        memcpy(&nChecksum, &hash, sizeof(nChecksum));
        assert(ssSend.size () >= CMessageHeader::CHECKSUM_OFFSET + sizeof(nChecksum));
        memcpy((char*)&ssSend[CMessageHeader::CHECKSUM_OFFSET], &nChecksum, sizeof(nChecksum));

        if( fNetDbg ){
            printf("%I64u :: CNode::EndMessage (%d bytes)\n", GetTime(), nSize);
        }

        std::deque<CSerializeData>::iterator it = vSendMsg.insert(vSendMsg.end(), CSerializeData());
		//std::deque<CSerializeData>::iterator it = vSendMsg.insert(vSendMsg.begin(), CSerializeData());
        ssSend.GetAndClear(*it);
        nSendSize += (*it).size();

        // If write queue empty, attempt "optimistic write"
        if (it == vSendMsg.begin())
            SocketSendData(this);

        vBitNet.v_LastPushMsgTime = GetTime();		
		LEAVE_CRITICAL_SECTION(cs_vSend);
    }

    void PushVersion();
	void PushBitNetInfo();
    void PushBonusID(string sBonusId);
    void PushBonusReq(string sBonusId, int iFlag);
    void PushBonusRep(string sBonusId, string sBonus);
    //void AddOrDelWalletIpPorts(CNode* pNode, int iOpt);
    void PushOneNode(unsigned int dIP, WORD wPort);
	//void PushBoostMe(CNode* pNode);
    //void PushBoostMeToAll();
    void PushWalletNodes();
	void PushVpnNode();
	void PushSyncBitNetNodeReq();
	void PushBitNetChat(std::string msg, DWORD fColor, int iFontSize, int iToAll, int bAes);
	
	void PushTransFileReq(char* pFile, int64_t fSize);
	void PushTransFileAck(char* pFile, DWORD iOk);
	void PushBinBuf(const char* pCmd, PVOID pBuf, int64_t bSz);
	//void PushTransFileBuf(PVOID pBuf, int64_t bSz);
	void PushSocketBuf(DWORD dOpt, PVOID pBuf, int64_t bSz);
	void PushBitNetCustomPak(char* pCmd, PVOID pBuf, int64_t bSz);
	void PushTransFileFinish(DWORD df, int64_t fSize);

    void PushMessage(const char* pszCommand)
    {
        try
        {
            BeginMessage(pszCommand);
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1>
    void PushMessage(const char* pszCommand, const T1& a1)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }

    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
    {
        try
        {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
            EndMessage();
        }
        catch (...)
        {
            AbortMessage();
            throw;
        }
    }


    void PushRequest(const char* pszCommand,
                     void (*fn)(void*, CDataStream&), void* param1)
    {
        uint256 hashReply;
        RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

        {
            LOCK(cs_mapRequests);
            mapRequests[hashReply] = CRequestTracker(fn, param1);
        }

        PushMessage(pszCommand, hashReply);
    }

    template<typename T1>
    void PushRequest(const char* pszCommand, const T1& a1,
                     void (*fn)(void*, CDataStream&), void* param1)
    {
        uint256 hashReply;
        RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

        {
            LOCK(cs_mapRequests);
            mapRequests[hashReply] = CRequestTracker(fn, param1);
        }

        PushMessage(pszCommand, hashReply, a1);
    }

    template<typename T1, typename T2>
    void PushRequest(const char* pszCommand, const T1& a1, const T2& a2,
                     void (*fn)(void*, CDataStream&), void* param1)
    {
        uint256 hashReply;
        RAND_bytes((unsigned char*)&hashReply, sizeof(hashReply));

        {
            LOCK(cs_mapRequests);
            mapRequests[hashReply] = CRequestTracker(fn, param1);
        }

        PushMessage(pszCommand, hashReply, a1, a2);
    }



    void PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd);
    bool IsSubscribed(unsigned int nChannel);
    void Subscribe(unsigned int nChannel, unsigned int nHops=0);
    void CancelSubscribe(unsigned int nChannel);
    void CloseSocketDisconnect();

    // Denial-of-service detection/prevention
    // The idea is to detect peers that are behaving
    // badly and disconnect/ban them, but do it in a
    // one-coding-mistake-won't-shatter-the-entire-network
    // way.
    // IMPORTANT:  There should be nothing I can give a
    // node that it will forward on that will make that
    // node's peers drop it. If there is, an attacker
    // can isolate a node and/or try to split the network.
    // Dropping a node for sending stuff that is invalid
    // now but might be valid in a later version is also
    // dangerous, because it can cause a network split
    // between nodes running old code and nodes running
    // new code.
    static void ClearBanned(); // needed for unit testing
    static bool IsBanned(CNetAddr ip);
    bool Misbehaving(int howmuch); // 1 == a little, 100 == a lot
    void copyStats(CNodeStats &stats);
};

inline void RelayInventory(const CInv& inv)
{
    // Put on lists to offer to the other nodes
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            pnode->PushInventory(inv);
    }
}

class CTransaction;
void RelayTransaction(const CTransaction& tx, const uint256& hash);
void RelayTransaction(const CTransaction& tx, const uint256& hash, const CDataStream& ss);


#endif
