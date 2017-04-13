// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "irc.h"
#include "db.h"
#include "net.h"
#include "init.h"
#include "strlcpy.h"
#include "addrman.h"
#include "ui_interface.h"
#include <iostream>
#include <fstream> 

#ifdef USE_BITNET
#include <string.h>
#include "bitnet.h"
#endif

#ifdef USE_UPNP
#include <miniupnpc/miniwget.h>
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
#endif

using namespace std;
using namespace boost;

static const int MAX_OUTBOUND_CONNECTIONS = 16;

void ThreadMessageHandler2(void* parg);
void ThreadSocketHandler2(void* parg);
void ThreadOpenConnections2(void* parg);
void ThreadOpenAddedConnections2(void* parg);
#ifdef USE_UPNP
void ThreadMapPort2(void* parg);
#endif
void ThreadDNSAddressSeed2(void* parg);
bool OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound = NULL, const char *strDest = NULL, bool fOneShot = false);
DWORD AddNodeIp(DWORD dAddr, int bAdd);
//int AddOrRemoveNodeIpAndPort(CNode* node, unsigned short sPort, int bAdd);
DWORD SyncNodeIps(vector<int> &vIps);


struct LocalServiceInfo {
    int nScore;
    int nPort;
};

//
// Global state variables
//
bool fDiscover = true;
bool fUseUPnP = false;

int fNetDbg = 0, fNewSocketThread = 0, fMaxOutbound = 16, fBindPort920 = 0;
string s_BlockChain_AdBonus_Dir = "";
string s_Dont_Relay_AdBonus = "";
int fFixedChangeAddress = 0;  // 2015.07.01 add
uint64_t nLocalServices = NODE_NETWORK;
static CCriticalSection cs_mapLocalHost;
static map<CNetAddr, LocalServiceInfo> mapLocalHost;
static bool vfReachable[NET_MAX] = {};
static bool vfLimited[NET_MAX] = {};
static CNode* pnodeLocalHost = NULL;
CAddress addrSeenByPeer(CService("0.0.0.0", 0), nLocalServices);
uint64_t nLocalHostNonce = 0;
boost::array<int, THREAD_MAX> vnThreadsRunning;
static std::vector<SOCKET> vhListenSocket;
CAddrMan addrman;

vector<CNode*> vNodes;
CCriticalSection cs_vNodes;
vector<int> vAllNodeIps;
CCriticalSection cs_vWalletNodes;	// 2015.06.06 add
std::vector<std::pair<unsigned int, WORD> > vAllNodeIpPorts;	// 2015.06.06 add
CCriticalSection cs_vAllNodesIp;
map<CInv, CDataStream> mapRelay;
deque<pair<int64_t, CInv> > vRelayExpiration;
CCriticalSection cs_mapRelay;
map<CInv, int64_t> mapAlreadyAskedFor;

static deque<string> vOneShots;
CCriticalSection cs_vOneShots;

static deque<string> vOneShots2;  // 2015.06.06 add
std::vector<std::pair<string, int> > vOneShots3;	// 2015.06.26 add
CCriticalSection cs_vOneShots2;

std::vector<std::pair<string, int64_t> > vBitNetBonus;	// 2015.06.11 add
CCriticalSection cs_vBitNetBonus;

set<CNetAddr> setservAddNodeAddresses;
CCriticalSection cs_setservAddNodeAddresses;

static CSemaphore *semOutbound = NULL;

double fVpn_Cny_price = 0.0;
double fVpn_Btc_price = 0.0;
int bUpdate_price_now = 0;

std::string sQkl_domain = "www.qkl.im";  // www.qkl.io
int BitNet_Version = 1702;
int BitNet_Network_id = 1;  // VpnCoin = 1


int GetTotalConnects()
{
	LOCK(cs_vAllNodesIp);
	return vAllNodeIps.size();
}

DWORD GetHostPort(char* sHost)	//--2014.11.13 add
{
    DWORD rzt = GetDefaultPort();
	try{
	char *ver = strstr(sHost, ":");
    if (ver != NULL){
        ver[0] = 0;	  ver++;
		if( ver[0] > 0 ){ rzt = atoi(ver); }
	}
    }catch (std::exception& e) {
		string strE = string(e.what()); 
		printf("GetHostPort: except [%s]\n", strE.c_str());	
        //PrintException(&e, "GetHostPort()");
    } catch (...) {
        PrintException(NULL, "GetHostPort()");
    }	
	return rzt;
}

//int AddOrRemoveNodeIpAndPort(CNode* node, unsigned short sPort, int bAdd)
DWORD AddNodeIp(DWORD dAddr, int bAdd)
{
  DWORD rzt = 0;
  if( GetBoolArg("-autosyncnode", true) )
  {
	//DWORD dAddr = node->vBitNet.v_IpAddr;
	//unsigned short iPort = sPort;
	//if( iPort == 0 ){ iPort = node->addr.GetPort(); }
	
	if( (dAddr != 0) && (dAddr != 0xFFFFFFFF) )	//if( pnode && (pnode->sAddr) )
	{
		LOCK(cs_vAllNodesIp);
		if( vAllNodeIps.size() )
		{
			DWORD dFind = 0;
			//vector<DWORD>::iterator itr = vAllNodeIps.begin();
			//while (itr != vAllNodeIps.end())
			
			//vector<DWORD>::iterator itr = vAllNodeIps.find();
			for (vector<int>::iterator itr = vAllNodeIps.begin(); itr != vAllNodeIps.end();) 
			{
				if( *itr == dAddr )
				{
					dFind++;
					if( !bAdd ){	// =0, del
						itr = vAllNodeIps.erase(itr);	// del ip
						//itr = vAllNodeIps.erase(itr);	// del port
						rzt++;
					}
					break;
				}
				else itr++;
			}
			if( bAdd == 1 ){	// =1, add
				if( !dFind ) { 
					vAllNodeIps.push_back(dAddr);
					//vAllNodeIps.push_back(iPort);	
					rzt++; 
				}
			}
			else if( bAdd == 2 ){ rzt = dFind; }
   
		
			/*BOOST_FOREACH(DWORD dIp, vAllNodeIps)
			{
				if( dIp == pnode->sAddr )
				{ 
					if( !bAdd ){ vAllNodeIps.erase(dIp); }
					dFind++; 
					break; 
				}
			}
			if( bAdd )
			{ 
				if(!bFind){ vAllNodeIps.push_back(pnode->sAddr); }
			}
			else if( bFind ){  } */
		}else if( bAdd == 1 ){ vAllNodeIps.push_back(dAddr); rzt++; }
		if( fDebug ){ printf("AddNodeIp:: Opt=%d, ip=%X, NodeCount=%d, rzt=%d\n", bAdd, dAddr, vAllNodeIps.size(), rzt); }
	}
  }
  return rzt;
}

std::string dwIpToStr(const DWORD nValue)   
{     
    //char strTemp[20];   
    //sprintf( strTemp,"%d.%d.%d.%d", (nValue&0xff000000)>>24, (nValue&0x00ff0000)>>16, (nValue&0x0000ff00)>>8, (nValue&0x000000ff) );
	std::string sIp = strprintf("%d.%d.%d.%d", (nValue&0x000000ff), (nValue&0x0000ff00)>>8, (nValue&0x00ff0000)>>16, (nValue&0xff000000)>>24);
    return sIp;	//string(strTemp);   
} 

void AddDisconnectNode(CNode *pnode)
{
#ifdef USE_BITNET
	if( (GetArg("-reconsockerrnode", 1)) && (pnode != NULL) )
	{
		//if( pnode->hSocket != INVALID_SOCKET )
		{
			string sHostPort;
			DWORD dPort = pnode->vBitNet.v_ListenPort;
			if( dPort > 10 )
			{
				sHostPort = strprintf("+%s:%d", pnode->addr.ToStringIP().c_str(), dPort); 
			}else{
				sHostPort = strprintf("+%s", pnode->addr.ToString().c_str()); 
			}
			AddOneShot(sHostPort, 0);
			//if( fDebug ){ printf("AddDisconnectNode [%s]\n", sHostPort.c_str()); }
		}
	}
#endif
}

DWORD ConnectToIp(DWORD dIp, DWORD dPort)
{
    DWORD rzt = 0;
	//if(fDebug){ printf("ConnectToIp 1: %X, : %d\n", dIp, dPort); }
    try {
		if( dIp > 0 )
		{
			string strAddr;
			string sHostPort;
			DWORD dCmd = dPort & 0xFFFC0000;
			if( (dPort & 0x10000) > 0 ){ 
			    //dPort = dPort - 0x10000; 
				strAddr = (char*)dIp;
			}else{ strAddr = dwIpToStr(dIp); }
			dPort = dPort & 0x0000FFFF;
			
			if( dPort == 0 ){ dPort = 920; }	//GetDefaultPort(); }
			sHostPort = strprintf("+%s:%d", strAddr.c_str(), dPort); 
			if( fNetDbg ){ printf("ConnectToIp [%s] : %X, ips= %s : %d, rzt=%d\n", sHostPort.c_str(), dIp, strAddr.c_str(), dPort, rzt); }
			//AddOneShot(sHostPort);

			if( (dCmd & 0x00080000) > 0 )	// direct connect
			{
				CAddress addr;
				//if(fDebug){ printf("ConnectToIp 2: %X, ips= %s : %d, %s\n", dIp, sHostPort.c_str(), dPort, (char*)dIp); }
				CNode* pnode = ConnectNode(addr, strAddr.c_str(), dPort);
				/*
                vector<CAddress> vAdd;
                int nOneDay = 24*3600;
                addr.nTime = GetTime() - 3*nOneDay - GetRand(4*nOneDay); // use a random age between 3 and 7 days old
                vAdd.push_back(addr);
                addrman.Add(vAdd, CNetAddr(strAddr.c_str(), true));
				*/	
			}else{ AddOneShot(sHostPort); }
			rzt++;
			
			//rzt = OpenNetworkConnection(addr, NULL, strAddr.c_str());

                /* int nOneDay = 24*3600;	
                char *pIpPort = (char *)sHostPort.c_str();
                if( pIpPort[0] == '+' ){ pIpPort++; }				
                CAddress addr = CAddress(CService(pIpPort));
                addr.nTime = GetTime() - 3*nOneDay - GetRand(4*nOneDay); // use a random age between 3 and 7 days old
                addrman.Add(addr, CNetAddr(strAddr, false));	*/
				
			//if(fDebug){ printf("ConnectToIp [%s] : %X, ips= %s : %d, rzt=%d\n", pIpPort, dIp, strAddr.c_str(), dPort, rzt); }
		}
    }
    catch (std::exception &e) {
		//string strE = string(e.what()); 
		//printf("ConnectToIp: except [%s]\n", strE.c_str());
		PrintException(&e, "ConnectToIp()");
    } catch (...) {
        PrintException(NULL, "ConnectToIp()");
    }
	return rzt;
}
#pragma pack (1)
struct IpAndPort_struct
{
	DWORD v_Ip;
	unsigned short v_Port;
};

struct IpAndPort_Array
{
	DWORD v_Count;
	IpAndPort_struct IpPorts[1];
};
#pragma pack ()
	
	
/*                CAddress addr;
                OpenNetworkConnection(addr, NULL, strAddr.c_str());
*/
				
/* void static ThreadConnectToIpAndPort(void* parg)
{
    if(fDebug){ printf("ThreadConnectToIpAndPort started %X\n", parg); }
    try
    {	
		if( parg != NULL )
		{
			IpAndPort_Array* IpAr = (IpAndPort_Array*)parg;
			//delete (IpAndPort_struct*)parg;	//pIpPort;
			int i, j;
			j = IpAr->v_Count;
			if(fDebug){ printf("ThreadConnectToIpAndPort, IpPortPak =%X, Count =%d\n", IpAr, j); }
			if( j > 0 )
			{
				for(i = 0; i < j; i++)
				{
					DWORD dIp, dPort, dRzt;
					dIp = IpAr->IpPorts[i].v_Ip;
					dPort = IpAr->IpPorts[i].v_Port;
					if( dIp ){ 
						if(fDebug){ printf("ThreadConnectToIpAndPort %X, i =%d, %X : %u\n", IpAr, i, dIp, dPort); }
						dRzt = ConnectToIp(dIp, dPort); 
						if(fDebug){ printf("ThreadConnectToIpAndPort %X, i =%d, %X : %u, Rzt =%u\n", IpAr, i, dIp, dPort, dRzt); }
						
						for (int i2 = 0; i2 < 10; i2++)
						{
							MilliSleep(500);
							if (fShutdown) break;
						}
					}
					if (fShutdown) break;
				}
			}
		}
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadConnectToIpAndPort()");
    } catch (...) {
        PrintException(NULL, "ThreadConnectToIpAndPort()");
    }
	delete[] (char*)parg;	//pIpPort;
    if(fDebug){ printf("ThreadConnectToIpAndPort exiting %X\n", parg); }
} */

/* DWORD SyncNodeIpPort(DWORD ip, DWORD port)
{
	DWORD rzt = 0;
	if( ip > 0 ){
		if(fDebug){ printf("SyncNodeIpPort IpPortPak =%X, sz=%u\n", ip, port); }
		char* p = new char[port];
		if( p ){
		if(fDebug){ printf("SyncNodeIpPort IpPortPak =%X, sz=%u\n", p, port); }
		CopyMemory(&p[0], (char*)ip, port);
		if (!NewThread(ThreadConnectToIpAndPort, p)){
			printf("Error: NewThread(ThreadConnectToIpAndPort) failed\n"); }
		else{ rzt++; }}
	}
	return rzt;
} */

DWORD SyncNodeIps(vector<int> &vIps)
{
	DWORD rzt = 0;
	int iTt = GetArg("-maxconnections", 125);
	int iTc = GetTotalConnects();

	if( (iTc < iTt) && (vIps.size() > 0) )
	{
        BOOST_FOREACH(int hIp, vIps) {
            printf("SyncNodeIps: %X\n", hIp);
			if( AddNodeIp(hIp, 2) == 0 )	// not exists
			{
				if( ConnectToIp(hIp, 0) ){ rzt++; }
				MilliSleep(20);
			}
        }
	}
	return rzt;
}

void AddOneShot(string strDest, int bPushFront)
{
    LOCK(cs_vOneShots);
	try{
	int bAdd = 1;
	if( GetArg("-checkoneshot", 0) )
	{
		deque<string>::iterator itString = find( vOneShots.begin(), vOneShots.end(), strDest );
		if( itString != vOneShots.end() ){ bAdd = 0; }
	}
	if( bAdd )
	{
		if( bPushFront == 0 ){ vOneShots.push_back(strDest); }
		else{ vOneShots.push_front(strDest); }
		if( GetArg("-debug2", 0) ){ printf("AddOneShot [%s], count = %d\n", strDest.c_str(), vOneShots.size()); }
	}
	//else if( fDebug ){ printf("AddOneShot [%s] exists, count = %d\n", strDest.c_str(), vOneShots.size()); }
    }
    catch (std::exception &e) {
		string strE = string(e.what()); 
		printf("AddOneShot: except [%s]\n", strE.c_str());
		//PrintException(&e, "AddOneShot()");
    } catch (...) {
        PrintException(NULL, "AddOneShot()");
    }	
}

void AddOneShot2(string strDest, int bPushFront)
{
    LOCK(cs_vOneShots2);
	try{
	int bAdd = 1;
	if( GetArg("-checkoneshot", 0) )
	{
		deque<string>::iterator itString = find( vOneShots2.begin(), vOneShots2.end(), strDest );
		if( itString != vOneShots2.end() ){ bAdd = 0; }
	}
	if( bAdd )
	{
		if( bPushFront == 0 ){ vOneShots2.push_back(strDest); }
		else{ vOneShots2.push_front(strDest); }
		if( fNetDbg ){ printf("AddOneShot2 [%s], count = %d\n", strDest.c_str(), vOneShots2.size()); }
	}
	//else if( fDebug ){ printf("AddOneShot2 [%s] exists, count = %d\n", strDest.c_str(), vOneShots2.size()); }
    }
    catch (std::exception &e) {
		string strE = string(e.what()); 
		printf("AddOneShot2: except [%s]\n", strE.c_str());
    } catch (...) {
        PrintException(NULL, "AddOneShot2()");
    }	
}

unsigned short GetListenPort()
{
    return (unsigned short)(GetArg("-port", GetDefaultPort()));
}

void CNode::PushGetBlocks(CBlockIndex* pindexBegin, uint256 hashEnd)
{
    // Filter out duplicate requests
    if (pindexBegin == pindexLastGetBlocksBegin && hashEnd == hashLastGetBlocksEnd)
        return;
    pindexLastGetBlocksBegin = pindexBegin;
    hashLastGetBlocksEnd = hashEnd;

    PushMessage("getblocks", CBlockLocator(pindexBegin), hashEnd);
}

// find 'best' local address for a particular peer
bool GetLocal(CService& addr, const CNetAddr *paddrPeer)
{
    if (fNoListen)
        return false;

    int nBestScore = -1;
    int nBestReachability = -1;
    {
        LOCK(cs_mapLocalHost);
        for (map<CNetAddr, LocalServiceInfo>::iterator it = mapLocalHost.begin(); it != mapLocalHost.end(); it++)
        {
            int nScore = (*it).second.nScore;
            int nReachability = (*it).first.GetReachabilityFrom(paddrPeer);
            if (nReachability > nBestReachability || (nReachability == nBestReachability && nScore > nBestScore))
            {
                addr = CService((*it).first, (*it).second.nPort);
                nBestReachability = nReachability;
                nBestScore = nScore;
            }
        }
    }
    return nBestScore >= 0;
}

// get best local address for a particular peer as a CAddress
CAddress GetLocalAddress(const CNetAddr *paddrPeer)
{
    CAddress ret(CService("0.0.0.0",0),0);
    CService addr;
    if (GetLocal(addr, paddrPeer))
    {
        ret = CAddress(addr);
        ret.nServices = nLocalServices;
        ret.nTime = GetAdjustedTime();
    }
    return ret;
}

bool RecvLine(SOCKET hSocket, string& strLine)
{
    strLine = "";
    while (true)
    {
        char c;
        int nBytes = recv(hSocket, &c, 1, 0);
        if (nBytes > 0)
        {
            if (c == '\n')
                continue;
            if (c == '\r')
                return true;
            strLine += c;
            if (strLine.size() >= 9000)
                return true;
        }
        else if (nBytes <= 0)
        {
            if (fShutdown)
                return false;
            if (nBytes < 0)
            {
                int nErr = WSAGetLastError();
                if (nErr == WSAEMSGSIZE)
                    continue;
                if (nErr == WSAEWOULDBLOCK || nErr == WSAEINTR || nErr == WSAEINPROGRESS)
                {
                    MilliSleep(10);
                    continue;
                }
            }
            if (!strLine.empty())
                return true;
            if (nBytes == 0)
            {
                // socket closed
                printf("RecvLine: socket closed\n");
                return false;
            }
            else
            {
                // socket error
                int nErr = WSAGetLastError();
                printf("recv failed: %d\n", nErr);
                return false;
            }
        }
    }
}

// used when scores of local addresses may have changed
// pushes better local address to peers
void static AdvertizeLocal()
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if (pnode->fSuccessfullyConnected)
        {
            CAddress addrLocal = GetLocalAddress(&pnode->addr);
            if (addrLocal.IsRoutable() && (CService)addrLocal != (CService)pnode->addrLocal)
            {
                pnode->PushAddress(addrLocal);
                pnode->addrLocal = addrLocal;
            }
        }
    }
}

void SetReachable(enum Network net, bool fFlag)
{
    LOCK(cs_mapLocalHost);
    vfReachable[net] = fFlag;
    if (net == NET_IPV6 && fFlag)
        vfReachable[NET_IPV4] = true;
}

// learn a new local address
bool AddLocal(const CService& addr, int nScore)
{
    if (!addr.IsRoutable())
        return false;

    if (!fDiscover && nScore < LOCAL_MANUAL)
        return false;

    if (IsLimited(addr))
        return false;

    printf("AddLocal(%s,%i)\n", addr.ToString().c_str(), nScore);

    {
        LOCK(cs_mapLocalHost);
        bool fAlready = mapLocalHost.count(addr) > 0;
        LocalServiceInfo &info = mapLocalHost[addr];
        if (!fAlready || nScore >= info.nScore) {
            info.nScore = nScore + (fAlready ? 1 : 0);
            info.nPort = addr.GetPort();
        }
        SetReachable(addr.GetNetwork());
    }

    AdvertizeLocal();

    return true;
}

bool AddLocal(const CNetAddr &addr, int nScore)
{
    return AddLocal(CService(addr, GetListenPort()), nScore);
}

/** Make a particular network entirely off-limits (no automatic connects to it) */
void SetLimited(enum Network net, bool fLimited)
{
    if (net == NET_UNROUTABLE)
        return;
    LOCK(cs_mapLocalHost);
    vfLimited[net] = fLimited;
}

bool IsLimited(enum Network net)
{
    LOCK(cs_mapLocalHost);
    return vfLimited[net];
}

bool IsLimited(const CNetAddr &addr)
{
    return IsLimited(addr.GetNetwork());
}

/** vote for a local address */
bool SeenLocal(const CService& addr)
{
    {
        LOCK(cs_mapLocalHost);
        if (mapLocalHost.count(addr) == 0)
            return false;
        mapLocalHost[addr].nScore++;
    }

    AdvertizeLocal();

    return true;
}

/** check whether a given address is potentially local */
bool IsLocal(const CService& addr)
{
    LOCK(cs_mapLocalHost);
    return mapLocalHost.count(addr) > 0;
}

/** check whether a given address is in a network we can probably connect to */
bool IsReachable(const CNetAddr& addr)
{
    LOCK(cs_mapLocalHost);
    enum Network net = addr.GetNetwork();
    return vfReachable[net] && !vfLimited[net];
}

bool GetMyExternalIP2(const CService& addrConnect, const char* pszGet, const char* pszKeyword, CNetAddr& ipRet)
{
    SOCKET hSocket;
    if (!ConnectSocket(addrConnect, hSocket))
        return error("GetMyExternalIP() : connection to %s failed", addrConnect.ToString().c_str());

    send(hSocket, pszGet, strlen(pszGet), MSG_NOSIGNAL);

    string strLine;
    while (RecvLine(hSocket, strLine))
    {
        if (strLine.empty()) // HTTP response is separated from headers by blank line
        {
            while (true)
            {
                if (!RecvLine(hSocket, strLine))
                {
                    closesocket(hSocket);
                    return false;
                }
                if (pszKeyword == NULL)
                    break;
                if (strLine.find(pszKeyword) != string::npos)
                {
                    strLine = strLine.substr(strLine.find(pszKeyword) + strlen(pszKeyword));
                    break;
                }
            }
            closesocket(hSocket);
            if (strLine.find("<") != string::npos)
                strLine = strLine.substr(0, strLine.find("<"));
            strLine = strLine.substr(strspn(strLine.c_str(), " \t\n\r"));
            while (strLine.size() > 0 && isspace(strLine[strLine.size()-1]))
                strLine.resize(strLine.size()-1);
            CService addr(strLine,0,true);
            printf("GetMyExternalIP() received [%s] %s\n", strLine.c_str(), addr.ToString().c_str());
            if (!addr.IsValid() || !addr.IsRoutable())
                return false;
            ipRet.SetIP(addr);
            return true;
        }
    }
    closesocket(hSocket);
    return error("GetMyExternalIP() : connection closed");
}

// We now get our external IP from the IRC server first and only use this as a backup
bool GetMyExternalIP(CNetAddr& ipRet)
{
    CService addrConnect;
    const char* pszGet;
    const char* pszKeyword;

    for (int nLookup = 0; nLookup <= 1; nLookup++)
    for (int nHost = 1; nHost <= 2; nHost++)
    {
        // We should be phasing out our use of sites like these.  If we need
        // replacements, we should ask for volunteers to put this simple
        // php file on their web server that prints the client IP:
        //  <?php echo $_SERVER["REMOTE_ADDR"]; ?>
        if (nHost == 1)
        {
            addrConnect = CService("91.198.22.70",80); // checkip.dyndns.org

            if (nLookup == 1)
            {
                CService addrIP("checkip.dyndns.org", 80, true);
                if (addrIP.IsValid())
                    addrConnect = addrIP;
            }

            pszGet = "GET / HTTP/1.1\r\n"
                     "Host: checkip.dyndns.org\r\n"
                     "User-Agent: VpnCoin\r\n"
                     "Connection: close\r\n"
                     "\r\n";

            pszKeyword = "Address:";
        }
        else if (nHost == 2)
        {
            addrConnect = CService("74.208.43.192", 80); // www.showmyip.com

            if (nLookup == 1)
            {
                CService addrIP("www.showmyip.com", 80, true);
                if (addrIP.IsValid())
                    addrConnect = addrIP;
            }

            pszGet = "GET /simple/ HTTP/1.1\r\n"
                     "Host: www.showmyip.com\r\n"
                     "User-Agent: VpnCoin\r\n"
                     "Connection: close\r\n"
                     "\r\n";

            pszKeyword = NULL; // Returns just IP address
        }

        if (GetMyExternalIP2(addrConnect, pszGet, pszKeyword, ipRet))
            return true;
    }

    return false;
}

void ThreadGetMyExternalIP(void* parg)
{
    // Make this thread recognisable as the external IP detection thread
    RenameThread("vpncoin-ext-ip");

    CNetAddr addrLocalHost;
    if (GetMyExternalIP(addrLocalHost))
    {
        printf("GetMyExternalIP() returned %s\n", addrLocalHost.ToStringIP().c_str());
        AddLocal(addrLocalHost, LOCAL_HTTP);
    }
}

bool GetStrFromUrl(const string sHost, int port, const string sUrl, string& sRzt)
{
    SOCKET hSocket;
    try{
    CService addrConnect(sHost, port, true);  //"www.payhub.xyz", 80
    if( !addrConnect.IsValid() ) return false;

	string sGet = "GET " + sUrl + " HTTP/1.1\r\n" +  // /
                     "Host: " + sHost + "\r\n" +     //"Host: www.showmyip.com\r\n"
                     "User-Agent: VpnCoin\r\n" +
                     "Connection: close\r\n" +
                     "\r\n";

    const char* pszGet = sGet.c_str();
	const char* pszKeyword = NULL; // Returns just IP address
			
    //SOCKET hSocket;
    if (!ConnectSocket(addrConnect, hSocket))
        return error("GetStrFromUrl() : connection to %s failed", addrConnect.ToString().c_str());

    send(hSocket, pszGet, sGet.length(), MSG_NOSIGNAL);  //send(hSocket, pszGet, strlen(pszGet), MSG_NOSIGNAL);

    string strLine;
    while (RecvLine(hSocket, strLine))
    {
        if (strLine.empty()) // HTTP response is separated from headers by blank line
        {
            while (true)
            {
                if( fShutdown ) break;
				if (!RecvLine(hSocket, strLine))
                {
                    closesocket(hSocket);
                    return false;
                }
                if (pszKeyword == NULL)
                    break;
                if (strLine.find(pszKeyword) != string::npos)
                {
                    strLine = strLine.substr(strLine.find(pszKeyword) + strlen(pszKeyword));
                    break;
                }
            }
            closesocket(hSocket);   sRzt = strLine;

            //strLine = strLine.substr(strspn(strLine.c_str(), " \t\n\r"));
            //while (strLine.size() > 0 && isspace(strLine[strLine.size()-1]))
            //    strLine.resize(strLine.size()-1);

            return true;
        }
    }
    closesocket(hSocket);
    return error("GetStrFromUrl() : connection closed");
    }
    catch (std::exception& e) {
        closesocket(hSocket);   return false;   //PrintException(&e, "GetStrFromUrl()");
    } catch (...) {
        closesocket(hSocket);   return false;
    }
}

//http://www.payhub.xyz/vpn2cny.php
//extern void showAndroidToastMsg(string msg);
void ThreadGetVpnPrice(void* parg)
{
    // Make this thread recognisable as the external IP detection thread
    RenameThread("vpncoin-price");

	int64_t i61 = 0;
	try{
		while( !fShutdown )
		{
			int64_t i62 = GetTime();
			if( (bUpdate_price_now > 0) || ((i62 - i61) > 180) )   // 3 minutes sync once
			{
				string sRzt = "";
				if( bUpdate_price_now > 0 ){ bUpdate_price_now = 0; }
				string sTm = strprintf("%" PRId64, i62);
				string sUrl = "/vpn2cny.php?tm=" + sTm;
				if( GetStrFromUrl("www.payhub.xyz", 80, sUrl, sRzt) )
				{
					//showAndroidToastMsg(sRzt);
					if( sRzt.length() > 0 )
					{
						fVpn_Cny_price = atof( sRzt.c_str() );
						sRzt.resize(0);
					}
				}
				i61 = i62;
			}
			MilliSleep(1000);
		}
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadPrice()");
    } catch (...) {
        throw; // support pthread_cancel()
    }
} 

extern string signMessage(const string strAddress, const string strMessage);
extern string sBitChainIdentAddress;

unsigned char ToHex(unsigned char x)   
{   
    return  x > 9 ? x + 55 : x + 48;   
} 
std::string UrlEncode(const std::string& str)  
{  
    std::string strTemp = "";  
    size_t length = str.length();  
    for (size_t i = 0; i < length; i++)  
    {  
        if (isalnum((unsigned char)str[i]) ||   
            (str[i] == '-') ||  
            (str[i] == '_') ||   
            (str[i] == '.') ||   
            (str[i] == '~'))  
            strTemp += str[i];  
        else if (str[i] == ' ')  
            strTemp += "+";  
        else  
        {  
            strTemp += '%';  
            strTemp += ToHex((unsigned char)str[i] >> 4);  
            strTemp += ToHex((unsigned char)str[i] % 16);  
        }  
    }  
    return strTemp;  
}

string buildSignTimeStr(int64_t tm)
{
    string rzt = "", sTm = strprintf("%" PRId64, tm);  //GetTime());
    rzt = signMessage(sBitChainIdentAddress, sTm);
    return rzt;
}

void ThreadWalletLoginDns(void* parg)
{
    // Make this thread recognisable as the external IP detection thread
    RenameThread("vpncoin-dns");
//#ifdef QT_GUI
	int64_t i61 = 0;
	int iFreq = GetArg("-loginbitchainfreq", 60);
	try{
		while( !fShutdown )
		{
			int64_t i62 = GetTime();
			if( ((i62 - i61) > iFreq) )   // 1 minutes sync once
			{
				string sTm = strprintf("%" PRId64, i62);
				string sRzt = "", sUrl = "/wallogin.php?addr=" + sBitChainIdentAddress + "&tm=" + sTm+ "&sign=" + buildSignTimeStr(i62);
				//printf("Wallogin url = [%s] \n", sUrl.c_str());
				if( GetStrFromUrl(sQkl_domain, 80, sUrl, sRzt) )
				{
					//printf("Wallogin Rzt = [%s] \n", sRzt.c_str());  //showAndroidToastMsg(sRzt);
				}
				i61 = i62;
			}
			MilliSleep(1000);
		}
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadWalletLoginDns()");
    } catch (...) {
        throw; // support pthread_cancel()
    }
//#endif
} 


void AddressCurrentlyConnected(const CService& addr)
{
    addrman.Connected(addr);
}


//CNode* g_fastNode = NULL;
CNode* getMinPingValueNode()
{
    CNode* rzt = NULL;
    int64_t tm = 0x1000000000000000;  //0xFFFFFFFFFFFFFFFF;
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pNode, vNodes) 
	{
        if( fDebug ){
			string s = strprintf("Node [%s], GotPongTime [%I64u], PingUsecTime [%I64u], BoostSyncBlock [%u]", pNode->addr.ToString().c_str(), pNode->nGotPongTime, pNode->nPingUsecTime, pNode->bSupportBoostSyncBlock);
			printf("%s\n", s.c_str());
		}
		if( (pNode->nGotPongTime > 0) && (pNode->bSupportBoostSyncBlock > 0) )
		{
			if( pNode->nPingUsecTime < tm )
			{
				tm = pNode->nPingUsecTime;
				rzt = pNode;
			}
		}
    }
	return rzt;
}

CNode* FindNode(const CNetAddr& ip)
{
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            if ((CNetAddr)pnode->addr == ip)
                return (pnode);
    }
    return NULL;
}

CNode* FindNode(const std::string& addrName)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        if (pnode->addrName == addrName)
            return (pnode);
    return NULL;
}

CNode* FindNode(const CService& addr)
{
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            if ((CService)pnode->addr == addr)
                return (pnode);
    }
    return NULL;
}

CNode* FindNode(const CNode* node)
{
    try {
        //LOCK(cs_vNodes);
        vector<CNode*> vNodesCopy = vNodes;
		BOOST_FOREACH(CNode* pnode, vNodesCopy)
            if (pnode == node)
                return (pnode);
    }catch (std::exception &e) {
		string strE = string(e.what()); 
		printf("FindNode: except [%s]\n", strE.c_str());
    }	
    return NULL;
}

CNode* FindNode(const char *pIpPort)
{
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
	{
        string sIp = pnode->addr.ToString();
		char *p1 = (char*)(sIp.c_str());
		if( strcmp(p1, pIpPort) == 0 ){ 
			if( GetArg("-debug2", 0) ){ printf(" FindNode(char *) [%s] = true\n", pIpPort); }
			return (pnode); 
		}
	}
    return NULL;
}

CNode* FindNode(unsigned int dIp, WORD wPort)
{
    if( dIp > 0 )
	{
		LOCK(cs_vNodes);
		BOOST_FOREACH(CNode* pnode, vNodes)
		{
			unsigned int iIp = pnode->vBitNet.v_IpAddr;
			WORD iPort = pnode->vBitNet.v_ListenPort;
			if( (dIp == iIp) && (iPort == wPort) )
			{
				return (pnode); 
			}
		}
	}
    return NULL;
}

CNode* ConnectNode(CAddress addrConnect, const char *pszDest, int iPort)	//CNode* ConnectNode(CAddress addrConnect, const char *pszDest)
{
    if (pszDest == NULL) {
        if (IsLocal(addrConnect))
            return NULL;

        // Look for an existing connection
        CNode* pnode = FindNode((CService)addrConnect);
        if (pnode)
        {
            pnode->AddRef();
            return pnode;
        }
    }
/*	else{
		CNode* pnode = FindNode(pszDest);
		if( pnode )
		{ 
#ifdef USE_BITNET
		    if( pnode->vBitNet.v_ListItem ){ 
			pnode->AddRef();   return pnode; 
			}
#else
			pnode->AddRef();   return pnode; 
#endif
		}
	} */


    /// debug print
    if( fNetDbg ) printf("trying connection [%s] : [%d] lastseen=%.1fhrs\n",
        pszDest ? pszDest : addrConnect.ToString().c_str(), iPort,
        pszDest ? 0 : (double)(GetAdjustedTime() - addrConnect.nTime)/3600.0);

    // Connect
    SOCKET hSocket;
	int nTimeout = GetArg("-contimeout", 5000);	//-- 2014.11.30 add
	int iDefPort = iPort;
	if( iDefPort == 0 ){ iDefPort = GetDefaultPort(); }
    if (pszDest ? ConnectSocketByName(addrConnect, hSocket, pszDest, iDefPort, nTimeout) : ConnectSocket(addrConnect, hSocket, nTimeout))	//if (pszDest ? ConnectSocketByName(addrConnect, hSocket, pszDest, GetDefaultPort()) : ConnectSocket(addrConnect, hSocket))
    {
        addrman.Attempt(addrConnect);

        /// debug print
        if( fNetDbg ) printf("%" PRId64 " :: connected %s\n", GetTime(), pszDest ? pszDest : addrConnect.ToString().c_str());

        // Set to non-blocking
#ifdef WIN32
        u_long nOne = 1;
        if (ioctlsocket(hSocket, FIONBIO, &nOne) == SOCKET_ERROR)
            if( fNetDbg ) printf("ConnectSocket() : ioctlsocket non-blocking setting failed, error %d\n", WSAGetLastError());
#else
        if (fcntl(hSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
            if( fNetDbg ) printf("ConnectSocket() : fcntl non-blocking setting failed, error %d\n", errno);
#endif

        // Add node
        CNode* pnode = new CNode(hSocket, addrConnect, pszDest ? pszDest : "", false);
        pnode->AddRef();

        {
            LOCK(cs_vNodes);
            vNodes.push_back(pnode);
        }

        pnode->nTimeConnected = GetTime();
        return pnode;
    }
    else
    {
        return NULL;
    }
}

void CNode::CloseSocketDisconnect()
{
	//if( fDebug ){ printf("CloseSocketDisconnect Socket =%u, fDisconnect =%u, %X:%d\n", hSocket, fDisconnect, vBitNet.v_IpAddr, addr.GetPort()); }
	//if( !fDisconnect )
	try{
    fDisconnect = true;
    if (hSocket != INVALID_SOCKET)
    {
		if( fNetDbg ){ printf("%" PRId64 " :: CloseSocketDisconnect node %s\n", GetTime(), addrName.c_str()); }
		closesocket(hSocket);
        hSocket = INVALID_SOCKET;
		//AddOrDelWalletIpPorts(this, 0);  // 2015.06.06 add, del ip port
		
        //AddNodeIp(this->vBitNet.v_IpAddr, 0);
#ifdef USE_BITNET
		//if( dUseChat || dStartVpnClient || dStartVpnServer )
		{
			if( vBitNet.v_ListItem > 0 ){ SynNodeToBitNetGui(this, 0, 0, NULL); }
		}
#endif
		
        // in case this fails, we'll empty the recv buffer when the CNode is deleted
        TRY_LOCK(cs_vRecvMsg, lockRecv);
        if (lockRecv)
            vRecvMsg.clear();
    }
	} catch (std::exception& e)
	{
		string strE = string(e.what()); 
		printf("CloseSocketDisconnect: except [%s]\n", strE.c_str());
	}
}

extern int dw_zip_block;
void CNode::PushVersion()
{
    /// when NTP implemented, change to just nTime = GetAdjustedTime()
    int64_t nTime = (fInbound ? GetAdjustedTime() : GetTime());
    CAddress addrYou = (addr.IsRoutable() && !IsProxy(addr) ? addr : CAddress(CService("0.0.0.0",0)));
    CAddress addrMe = GetLocalAddress(&addr);
	/*
	int iVer = BitNet_Version;
    RAND_bytes((unsigned char*)&nLocalHostNonce, sizeof(nLocalHostNonce));
    printf("send version message: version %d, blocks=%d, us=%s, them=%s, peer=%s\n", PROTOCOL_VERSION, nBestHeight, addrMe.ToString().c_str(), addrYou.ToString().c_str(), addr.ToString().c_str());
    PushMessage("version", PROTOCOL_VERSION, nLocalServices, nTime, addrYou, addrMe,
                nLocalHostNonce, FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<string>()), nBestHeight, iVer);
	*/			
	this->BeginMessage("version");
	this->ssSend << PROTOCOL_VERSION << nLocalServices << nTime << addrYou << addrMe;
    this->ssSend << nLocalHostNonce << FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<string>()) << nBestHeight << BitNet_Version;
	this->ssSend << BitNet_Network_id;
	unsigned short wPort = GetListenPort();
	unsigned char bIsGui = 0;	// 2014.12.18 add
	
/* #ifdef USE_BITNET
	wPort = GetListenPort();
#endif */

#ifdef QT_GUI
    bIsGui++;
#endif	
	this->ssSend << wPort;	//this->ssSend << BitNet_Network_id << wPort;
	this->ssSend << bIsGui;
	
//2014.12.28 begin
	unsigned short wCtPort = 0;
	unsigned short wPrPort = 0;	
#ifdef USE_BITNET
	wCtPort = iVpnServiceCtrlPort;
	wPrPort = d_P2P_proxy_port;
#endif
	this->ssSend << wCtPort;
	this->ssSend << wPrPort;
//2014.12.28 end	

//--2014.11.10 begin	
#ifdef USE_BITNET
    if( GetArg("-pushbitnetinver", 0) )
	{
	    unsigned char bIncludeBitNet = 1;
		this->ssSend << bIncludeBitNet;	//--2014.12.21 add
		
		std::string sMemCpu = "";
		DWORD iTtVpnConn = 0, iSerCCF = 0;
		if( dStartVpnServer )
		{	 
			iTtVpnConn = iTotalVpnConnects;
			iSerCCF = iVpnSerCoinConfirms;
			sMemCpu = std::string( sVpnMemAndCpuInfo.c_str() ); 
		}
		this->ssSend << bShowInOtherList << isVpnServer << iVpnServiceCtrlPort << iVpnServicePort << iVpnServiceFee << iVpnServiceTryMinute << sDefWalletAddress << sVpnWalletAddress << sVpnNicknamePack << sMemCpu << iSerCCF << iTtVpnConn << d_Vpn_LanID << iOpenSocketProxy;
		this->ssSend << d_P2P_proxy_port ;	// << wPort;	// << BitNet_Version;
		this->ssSend<< b_OnlineRewardVpnTime << w_OnlineXminReward_1Min;	// 2015.05.15 add
	}
#endif
//--2014.11.10 end

	unsigned char iSupportBoostSyncBlock = 1;   // 2015.07.24 add, iSupportBoostSyncBlock
	this->ssSend << iSupportBoostSyncBlock;
    this->ssSend << dw_zip_block;                      // 2016.05.18 add
	
    this->EndMessage();
}

string DigitIpToString(unsigned int dIP)
{
	string rzt;
	if( dIP > 0 )
	{
		unsigned char* p = (unsigned char*)&dIP;
		//rzt = strprintf("%d.%d.%d.%d", atoi(p[0]), atoi(p[1]), atoi(p[2]), atoi(p[3])); 
		//string s1 = p[0], s2 = p[1], s3 = p[2], s4 = p[3];
		rzt = strprintf("%d.%d.%d.%d", p[0], p[1], p[2], p[3]); 
	}
	return rzt;
}

void AddOrDelWalletIpPorts(CNode* pNode, int iOpt)
{
	LOCK(cs_vWalletNodes);
	unsigned int dIp = pNode->vBitNet.v_IpAddr;
	WORD wPort = pNode->vBitNet.v_ListenPort;
	//printf("AddOrDelWalletIpPorts [%s]:[%d], Opt = %d\n", DigitIpToString(dIp).c_str(), wPort, iOpt);
	//if( (wPort == 0) && (dIp > 0) ){ wPort = GetListenPort(); }
	if( dIp > 0 )  //if( (dIp > 0) && (wPort > 0) )
	{
		int iGet = 0;
		BOOST_FOREACH(const PAIRTYPE(unsigned int, WORD)& item, vAllNodeIpPorts)
		{
			unsigned int iIp = item.first;
			WORD iPort = item.second;
			if( (dIp == iIp)  && (wPort == iPort) )
			{
				iGet++;  
				if( iOpt == 0 )  // Del
				{
                    // remove from vAllNodeIpPorts
                    vAllNodeIpPorts.erase( remove(vAllNodeIpPorts.begin(), vAllNodeIpPorts.end(), item), vAllNodeIpPorts.end() );
				}
				break;
			}
		}
		if( (iOpt == 1) && (iGet == 0) )  // Add
		{
			vAllNodeIpPorts.push_back(make_pair(dIp, wPort));
			if( fDebug ) printf("AddOrDelWalletIpPorts 1,  [%s]:[%d],  count = %d\n", DigitIpToString(dIp).c_str(), wPort, vAllNodeIpPorts.size());
		}
	}
}


int OneShot2Exist(string sHost)
{
	int rzt = 0;
	if( sHost.length() > 6 )
	{
		LOCK(cs_vOneShots2);
		BOOST_FOREACH(const PAIRTYPE(string, int)& item, vOneShots3)
		{
			if( item.first == sHost )
			{
				rzt++;  break;
			}
		}
	}
	return rzt;
}

/* int DelOneShots2(const PAIRTYPE(string, int)& itm)
{
	LOCK(cs_vOneShots2);
	BOOST_FOREACH(const PAIRTYPE(string, int)& item, vOneShots3)
	{
		if( item == itm )
		{
			vOneShots3.erase( remove(vOneShots3.begin(), vOneShots3.end(), item), vOneShots3.end() );
			break;
		}
	}
	return vOneShots3.size();
}*/

int AddOneShot22(string sHost, int ic)
{
	int rzt = 0;
	if( sHost.length() > 6 )
	{
		int i = OneShot2Exist(sHost);
		if( i == 0 )
		{
			LOCK(cs_vOneShots2);
			vOneShots3.push_back(make_pair(sHost, ic));
			rzt++;
		}
	}
	return rzt;
}

void AddIpPortToOneShot(unsigned int dIp, WORD wPort)
{
	string sHostPort;
	if( wPort == 0 ){ wPort = 920; }
	//if( fNetDbg ){ printf("AddIpPortToOneShot [%X] : [%d] \n", dIp, wPort); }
	if( dIp > 0 )
	{
		if( FindNode(dIp, wPort) == NULL )
		{
			sHostPort = strprintf("-%s:%d", DigitIpToString(dIp).c_str(), wPort); 
			if( fNetDbg ){ printf("AddIpPortToOneShot [%s]\n", sHostPort.c_str()); }
			int icc = GetArg("-reconnoneshot", 1);
			AddOneShot22(sHostPort, icc);  //AddOneShot2(sHostPort, 1);
		}
	}
}

int ProcessIncomeIpPorts(std::vector<std::pair<unsigned int, WORD> > vNodeIPorts)
{
	int rzt = 0;
    //if( fDebug ){ printf("--> ProcessIncomeIpPorts vNodeIPorts.size() = [%d]\n", vNodeIPorts.size()); }
	BOOST_FOREACH(const PAIRTYPE(unsigned int, WORD)& item, vNodeIPorts)
	{
		//unsigned int iIp = item.first;
		//WORD iPort = item.second;
		if( fNetDbg ){ printf("ProcessIncomeIpPorts [%x]:[%d]\n", item.first, item.second); }
		AddIpPortToOneShot(item.first,  item.second);
		rzt++;
	}
    if( fNetDbg ){ printf("<-- ProcessIncomeIpPorts vNodeIPorts.size() = [%d], [%d]\n", vNodeIPorts.size(), rzt); }
	return rzt;
}

int BoostThisNodeToWorld(CNode* aNode)
{
    int rzt = 0;
    if( aNode != NULL )
	{
        unsigned int dIp = aNode->vBitNet.v_IpAddr;
		WORD wPort = aNode->vBitNet.v_ListenPort;
		if( dIp > 0 )
		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if( aNode != pnode ){ pnode->PushOneNode(dIp, wPort);  rzt++; }
			}
		}
    }
	return rzt;
}

int SaveBonusMsgToFile(string sBonusId, string sBonus)
{
	int rzt = 0;
	if( (sBonusId.length() < 10) || (sBonus.length() < 20) ){ return rzt; }
	
	string sDir = GetDataDir().string() + "\\" + sBonusId;
	ofstream myfile; 
	myfile.open (sDir.c_str());
	if( myfile.is_open() )
	{
		myfile << sBonus;
		myfile.close(); 
		rzt++; 
	}
	return rzt;
}

int BitNetBonusFileExist(string sBonusId)
{
	int rzt = 0;
	if( sBonusId.length() > 10 )
	{
		string sFile = s_BlockChain_AdBonus_Dir + sBonusId;
        ifstream t;
		t.open( sFile.c_str() ); 
		if( t.is_open() )
		{
			t.close();   rzt++;
		}
	}
	return rzt;
}

int BitNetBonusExist(string sBonusId)
{
	int rzt = 0;
	if( sBonusId.length() > 10 )
	{
		LOCK(cs_vBitNetBonus);
		//BOOST_FOREACH(string sBns, vBitNetBonus)
		BOOST_FOREACH(const PAIRTYPE(string, int64_t)& item, vBitNetBonus)
		{
			string sBns = item.first;
			int dEc = sBns.find(":");
			if (dEc != string::npos)
			{
				string sId = sBns.substr(0, dEc);
				//WORD iPort = item.second;
				if( sBonusId == sId )
				{
					rzt++;  break;
				}
			}
		}
	}
	if( rzt == 0 )
	{
		rzt = BitNetBonusFileExist(sBonusId);
	}
	return rzt;
}

string GetBitNetBonusMsgFromFile(string sBonusId)
{
	string rzt = "";
	if( sBonusId.length() > 10 )
	{
		string sFile = s_BlockChain_AdBonus_Dir + sBonusId;
        ifstream t;
		t.open( sFile.c_str() ); 
		if( t.is_open() )  //if( t.good() )
		{
			std::stringstream buffer;  
			buffer << t.rdbuf();
			//std::string contents(buffer.str());
			t.close();    rzt = buffer.str();
		}
	}
	return rzt;
}

string GetBitNetBonusMsg(string sBonusId)
{
	string rzt = "";
	if( sBonusId.length() > 10 )
	{
		LOCK(cs_vBitNetBonus);
		BOOST_FOREACH( PAIRTYPE(string, int64_t)& item, vBitNetBonus )  //BOOST_FOREACH(const PAIRTYPE(string, string)& item, vBitNetBonus)
		{
			string sBns = item.first;
			int dEc = sBns.find(":");
			if (dEc != string::npos)
			{
				string sId = sBns.substr(0, dEc);
				if( sBonusId == sId )
				{
					rzt = sBns.substr(dEc + 1);  //item.second;
					item.second = GetTime();
					break;
				}
			}
		}
	}
	if( rzt.length() < 10 )
	{
		rzt = GetBitNetBonusMsgFromFile(sBonusId);
	}
	return rzt;
}

int AddBitNetBonus(string sBonusId, string sBonus)
{
	int rzt = 0;
	if( (sBonusId.length() > 10) && (sBonus.length() > 20) )
	{
		int i = BitNetBonusExist(sBonusId);
		if( i == 0 )
		{
			LOCK(cs_vBitNetBonus);
			vBitNetBonus.push_back(make_pair(sBonusId + ":" + sBonus, GetTime()));    //vBitNetBonus.push_back(sBonusId);
			//if( SaveBonusMsgToFile(sBonusId, sBonus) ){ rzt++; }
			if( fNetDbg ){ printf("AddBitNetBonus [%s]  [%s]\n", sBonusId.c_str(), sBonus.c_str()); }
			rzt++;
		}
	}
	return rzt;
}

int DelExpiredBonus(int iMax)
{
	int rzt = 0;
	int64_t tm = GetTime();
	LOCK(cs_vBitNetBonus);
	BOOST_FOREACH(const PAIRTYPE(string, int64_t)& item, vBitNetBonus)
	{
		int64_t t2 = item.second;
		if( (tm > t2) && ((tm - t2) >= 900) )
		{
			vBitNetBonus.erase( remove(vBitNetBonus.begin(), vBitNetBonus.end(), item), vBitNetBonus.end() );
			rzt++;
			if( rzt >= iMax ){ break; }
		}
	}
	return rzt;
}

int RelayBonusID(string sBonusId)
{
	int rzt = 0;
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, vNodes)
	{
		if( pnode != NULL ){ pnode->PushBonusID(sBonusId);   rzt++; }
	}
	return rzt;
}

int RelayBonusReq(string sBonusId)
{
	int rzt = 0;
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, vNodes)
	{
		if( pnode != NULL ){ pnode->PushBonusReq(sBonusId, 1);   rzt++; }
	}
	return rzt;
}

void CNode::PushBonusReq(string sBonusId, int iFlag)
{
	if( (sBonusId.length() > 10) && (this->vBitNet.v_iVersion > 1128) )
	{
		//string sBns = pBonusId;
		this->BeginMessage("BonusReq");
		this->ssSend << sBonusId << iFlag;
		this->EndMessage();
	}
}

void CNode::PushBonusRep(string sBonusId, string sBonus)
{
	if( (sBonusId.length() > 10)  && (sBonus.length() > 20) )
	{
		//string sBns = pBonusId;
		this->BeginMessage("BonusRep");
		this->ssSend << sBonusId << sBonus;
		this->EndMessage();
	}
}

void CNode::PushBonusID(string sBonusId)
{
	if( sBonusId.length() > 10 )
	{
		//string sBns = pBonusId;
		this->BeginMessage("BonusTx");  // "BonusID"
		this->ssSend << BitNet_Network_id << sBonusId;
		this->EndMessage();
	}
}

void CNode::PushOneNode(unsigned int dIP, WORD wPort)
{
	if( dIP > 0 )  //if( (dIP > 0) && (this->vBitNet.v_iVersion > 1128) )
	{
		this->BeginMessage("ConnectTo");
		this->ssSend << dIP << wPort;
		this->EndMessage();
	}
}

void PushBoostMe(CNode* pNode)
{
	if( pNode != NULL ){ 
	    if( pNode->vBitNet.v_iVersion > 1128 ){ pNode->PushMessage("BoostMe"); }
	}
}

void PushBoostMeToAll()
{
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, vNodes)
	{
		PushBoostMe(pnode);
	}
}

void CNode::PushWalletNodes()
{
	LOCK(cs_vWalletNodes);
	if( !vAllNodeIpPorts.empty() )
	{
		this->BeginMessage("RepNodes");
		this->ssSend << vAllNodeIpPorts;
		this->EndMessage();
	}
}


void CNode::PushBitNetInfo()
{
#ifdef USE_BITNET	
/*
int bShowInOtherList = 0;
int isVpnServer = 0;
int iVpnServicePort = 923;
int iVpnServiceCtrlPort = 922;
int64_t iVpnServiceFee = 0;
int iVpnServiceTryMinute = 0;
std::string sVpnWalletAddress = ""; */

	/*std::string sDefWAddr;
	if( bShowWAddrInOtherNodes )
	{
		sDefWAddr = sDefWalletAddress;
	}*/
	//if( dStartVpnServer ){ sWAddr = std::string(sVpnWalletAddress.c_str()); }
	//else { sWAddr = std::string(sDefWalletAddress.c_str()); } */
	//PushMessage("vpn-1", bShowInOtherList, isVpnServer, iVpnServiceCtrlPort, iVpnServicePort, iVpnServiceFee, iVpnServiceTryMinute, sDefWalletAddress, sVpnWalletAddress, sVpnNickname);
	
	std::string sMemCpu = "";
	DWORD iTtVpnConn = 0, iSerCCF = 0;

	if( dStartVpnServer )
	{ 
		iTtVpnConn = iTotalVpnConnects;
		iSerCCF = iVpnSerCoinConfirms;
		sMemCpu = std::string( sVpnMemAndCpuInfo.c_str() ); 
	}

	//if( fDebug ){ printf("PushBitNetInfo::iTotalVpnConnects=%u, iVpnSerCoinConfirms=%u, %s, %u\n", iTotalVpnConnects, iVpnSerCoinConfirms, sVpnMemAndCpuInfo.c_str(), GetListenPort()); }
	//PushMessage("vpn-1", bShowInOtherList, isVpnServer, iVpnServiceCtrlPort, iVpnServicePort, iVpnServiceFee, iVpnServiceTryMinute, sDefWalletAddress, sVpnWalletAddress, sVpnNickname);
    if( vBitNet.v_iVersion < 1115 ){ this->BeginMessage("vpn-1"); }
	else{ this->BeginMessage("BitNet-1"); }
    //this->ssSend << bShowInOtherList << isVpnServer << iVpnServiceCtrlPort << iVpnServicePort << iVpnServiceFee << iVpnServiceTryMinute << sDefWAddr << sVpnWalletAddress << sVpnNickname << sMemCpu << iSerCCF << iTtVpnConn << d_Vpn_LanID;
	this->ssSend << bShowInOtherList << isVpnServer << iVpnServiceCtrlPort << iVpnServicePort << iVpnServiceFee << iVpnServiceTryMinute << sDefWalletAddress << sVpnWalletAddress << sVpnNicknamePack << sMemCpu << iSerCCF << iTtVpnConn << d_Vpn_LanID << iOpenSocketProxy;
	//unsigned short wPort = GetListenPort();
	this->ssSend << d_P2P_proxy_port;	// << wPort;	// << BitNet_Version;
	this->ssSend<< b_OnlineRewardVpnTime << w_OnlineXminReward_1Min;	// 2015.05.15 add
    this->EndMessage();
	//if( fDebug ){ printf("PushBitNetInfo to [%s], iVer =%d\n", this->addr.ToString().c_str(), this->vBitNet.v_iVersion); }
#endif
}

void CNode::PushBitNetChat(std::string msg, DWORD fColor, int iFontSize, int iToAll, int bAes)
{
	//if( vBitNet.v_iVersion < 1117 ){ PushMessage("vpn-c", msg); }
	//else{ 
 	  //WORD fSize = iFontSize;
	  PushMessage("BitNet-c", msg); //}	//PushMessage("BitNet-c", msg, fSize, fColor, iToAll, bAes); }
}

void CNode::PushTransFileReq(char* pFile, int64_t fSize)
{
	// Side A
	this->vBitNet.v_File_Loc = std::string( pFile );
	this->vBitNet.v_File_size = fSize;
	if( vBitNet.v_iVersion < 1115 ){ PushMessage("vpn-fr", this->vBitNet.v_File_Loc, fSize); }
	else{ PushMessage("BitNet-fr", this->vBitNet.v_File_Loc, fSize); }
}

void CNode::PushTransFileAck(char* pFile, DWORD iOk)
{
	if( pFile ){ this->vBitNet.v_File_Loc = std::string( pFile ); }
	else{ this->vBitNet.v_File_Loc.clear(); }
	if( vBitNet.v_iVersion < 1115 ){ PushMessage("vpn-fa", iOk); }
	else{ PushMessage("BitNet-fa", iOk); }
}

void CNode::PushBinBuf(const char* pCmd, PVOID pBuf, int64_t bSz)
{
	if( pBuf )
	{
        if( this->vBitNet.v_iVersion > 1112 ){
		try {
            vector<char> v(bSz);
			//copy(pBuf, pBuf + bSz, v.begin());
			memcpy((char*)&v[0], pBuf, bSz);
			PushMessage(pCmd, v);	//filein >> *this;
        }
        catch (std::exception &e) {
            //return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
        }}
	}
}

void CNode::PushBitNetCustomPak(char* pCmd, PVOID pBuf, int64_t bSz)
{
	PushBinBuf(pCmd, pBuf, bSz);
}

/* void CNode::PushTransFileBuf(PVOID pBuf, int64_t bSz)
{
	if( pBuf )
	{
        PushBinBuf("vpn-fb", pBuf, bSz);
		try {
            vector<char> v(bSz);
			//copy(pBuf, pBuf + bSz, v.begin());
			memcpy((char*)&v[0], pBuf, bSz);
			PushMessage("vpn-fb", v);	//filein >> *this;
        }
        catch (std::exception &e) {
            //return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
        }
	}
} */

void CNode::PushSocketBuf(DWORD dOpt, PVOID pBuf, int64_t bSz)
{
	if( pBuf )
	{
        if( dOpt == 0 ){ 
			if( vBitNet.v_iVersion < 1115 ){ PushBinBuf("vpn-fb", pBuf, bSz); }
			else{ PushBinBuf("BitNet-fb", pBuf, bSz); }
		}
		if( dOpt == 1 ){ 
			if( vBitNet.v_iVersion < 1115 ){ PushBinBuf("vpn-pR", pBuf, bSz); }
			else{ PushBinBuf("BitNet-pR", pBuf, bSz); }
		}
		else if( dOpt == 2 ){ 
			if( vBitNet.v_iVersion < 1115 ){ PushBinBuf("vpn-pA", pBuf, bSz); }
			else{ PushBinBuf("BitNet-pA", pBuf, bSz); }
		}
		else if( dOpt == 3 ){ 
			if( vBitNet.v_iVersion < 1115 ){ PushBinBuf("vpn-a", pBuf, bSz); }
			else{ PushBinBuf("BitNet-AA", pBuf, bSz); }
		}
	}
}

void CNode::PushTransFileFinish(DWORD df, int64_t fSize)
{
	if( this->vBitNet.v_File_size > 0 )
	{
		if( vBitNet.v_iVersion < 1115 ){ PushMessage("vpn-fc", fSize); }
		else{ PushMessage("BitNet-fc", fSize); }
	}
	this->vBitNet.v_File_size = 0;
	this->vBitNet.v_Starting_recv = 0;
	this->vBitNet.v_RemoteFileBuf.clear();
	this->vBitNet.v_File_Loc.clear();
	this->vBitNet.v_File_Req.clear();
	
}

void CNode::PushSyncBitNetNodeReq()
{
	PushMessage("BitNet-AR", BitNet_Network_id, BitNet_Version);
}

void CNode::PushVpnNode()
{
	int64_t nTime = GetTime();
	int bNeedSync = 0;
	if( vBitNet.v_SendSyncNodeIpsTime == 0 ){ bNeedSync++; }
	else if( (nTime > vBitNet.v_SendSyncNodeIpsTime) && ( (nTime - vBitNet.v_SendSyncNodeIpsTime) >= 360 ) ){ bNeedSync++; }
	
	if( bNeedSync )
	{
	  LOCK(cs_vAllNodesIp);
	  vector<int> vCopyIps = vAllNodeIps;
	  if( vCopyIps.size() > 1 )
	  {
		/* for (vector<int>::iterator itr = vCopyIps.begin(); itr != vCopyIps.end();) 
		{
			if( *itr == vBitNet.v_IpAddr )
			{
				itr = vCopyIps.erase(itr);
			}
			else itr++;
		} */
		if( vCopyIps.size() > 0 ){
		try {
			PushMessage("BitNet-A", vCopyIps);
			vBitNet.v_SendSyncNodeIpsTime = nTime;
        }
        catch (std::exception &e) {
            //return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
        }}
	  }
	}
}

std::map<CNetAddr, int64_t> CNode::setBanned;
CCriticalSection CNode::cs_setBanned;

void CNode::ClearBanned()
{
    setBanned.clear();
}

bool CNode::IsBanned(CNetAddr ip)
{
    bool fResult = false;
    {
        LOCK(cs_setBanned);
        std::map<CNetAddr, int64_t>::iterator i = setBanned.find(ip);
        if (i != setBanned.end())
        {
            int64_t t = (*i).second;
            if (GetTime() < t)
                fResult = true;
        }
    }
    return fResult;
}

bool CNode::Misbehaving(int howmuch)
{
    if (addr.IsLocal())
    {
        printf("Warning: Local node %s misbehaving (delta: %d)!\n", addrName.c_str(), howmuch);
        return false;
    }

    nMisbehavior += howmuch;
    if (nMisbehavior >= GetArg("-banscore", 100))
    {
        int64_t banTime = GetTime()+GetArg("-bantime", 60*60*24);  // Default 24-hour ban
        printf("Misbehaving: %s (%d -> %d) DISCONNECTING\n", addr.ToString().c_str(), nMisbehavior-howmuch, nMisbehavior);
        {
            LOCK(cs_setBanned);
            if (setBanned[addr] < banTime)
                setBanned[addr] = banTime;
        }
        if( fDebug ){ printf("[%s] Misbehaving %d\n", addrName.c_str(), howmuch); }
		CloseSocketDisconnect();
        return true;
    } else
        printf("Misbehaving: %s (%d -> %d)\n", addr.ToString().c_str(), nMisbehavior-howmuch, nMisbehavior);
    return false;
}

#undef X
#define X(name) stats.name = name
void CNode::copyStats(CNodeStats &stats)
{
    X(nServices);
    X(nLastSend);
    X(nLastRecv);
    X(nTimeConnected);
    X(addrName);
    X(nVersion);
    X(strSubVer);
    X(fInbound);
    X(nStartingHeight);
    X(nMisbehavior);

    // It is common for nodes with good ping times to suddenly become lagged,
    // due to a new block arriving or other large transfer.
    // Merely reporting pingtime might fool the caller into thinking the node was still responsive,
    // since pingtime does not update until the ping is complete, which might take a while.
    // So, if a ping is taking an unusually long time in flight,
    // the caller can immediately detect that this is happening.
    int64_t nPingUsecWait = 0;
    if ((0 != nPingNonceSent) && (0 != nPingUsecStart)) {
        nPingUsecWait = GetTimeMicros() - nPingUsecStart;
    }

    // Raw ping time is in microseconds, but show it to user as whole seconds (Bitcoin users should be well used to small numbers with many decimal places by now :)
    stats.dPingTime = (((double)nPingUsecTime) / 1e6);
    stats.dPingWait = (((double)nPingUsecWait) / 1e6);
    stats.IsGuiNode = vBitNet.v_IsGuiNode;
}
#undef X

// requires LOCK(cs_vRecvMsg)
bool CNode::ReceiveMsgBytes(const char *pch, unsigned int nBytes)
{
    while (nBytes > 0) {

        // get current incomplete message, or create a new one
        if (vRecvMsg.empty() ||
            vRecvMsg.back().complete())
            vRecvMsg.push_back(CNetMessage(SER_NETWORK, nRecvVersion));

        CNetMessage& msg = vRecvMsg.back();

        // absorb network data
        int handled;
        if (!msg.in_data)
            handled = msg.readHeader(pch, nBytes);
        else
            handled = msg.readData(pch, nBytes);

        if (handled < 0)
                return false;

        pch += handled;
        nBytes -= handled;

        if (msg.complete())
            msg.nTime = GetTimeMicros();
    }

    return true;
}

int CNetMessage::readHeader(const char *pch, unsigned int nBytes)
{
    // copy data to temporary parsing buffer
    unsigned int nRemaining = 24 - nHdrPos;
    unsigned int nCopy = std::min(nRemaining, nBytes);

    memcpy(&hdrbuf[nHdrPos], pch, nCopy);
    nHdrPos += nCopy;

    // if header incomplete, exit
    if (nHdrPos < 24)
        return nCopy;

    // deserialize to CMessageHeader
    try {
        hdrbuf >> hdr;
    }
    catch (std::exception &e) {
        return -1;
    }

    // reject messages larger than MAX_SIZE
    if (hdr.nMessageSize > MAX_SIZE)
            return -1;

    // switch state to reading message data
    in_data = true;

    return nCopy;
}

int CNetMessage::readData(const char *pch, unsigned int nBytes)
{
    unsigned int nRemaining = hdr.nMessageSize - nDataPos;
    unsigned int nCopy = std::min(nRemaining, nBytes);

    if (vRecv.size() < nDataPos + nCopy) {
        // Allocate up to 256 KiB ahead, but never more than the total message size.
        vRecv.resize(std::min(hdr.nMessageSize, nDataPos + nCopy + 256 * 1024));
    }

    memcpy(&vRecv[nDataPos], pch, nCopy);
    nDataPos += nCopy;

    return nCopy;
}







// requires LOCK(cs_vSend)
void SocketSendData(CNode *pnode)
{
    std::deque<CSerializeData>::iterator it = pnode->vSendMsg.begin();

    while (it != pnode->vSendMsg.end()) {
        const CSerializeData &data = *it;
		int dsz = data.size();
		if( fNetDbg ){ printf("SocketSendData to [%s], data.size = %d, nSendOffset = %d\n", pnode->addrName.c_str(), dsz, pnode->nSendOffset); }
        assert(dsz > pnode->nSendOffset);	//assert(data.size() > pnode->nSendOffset);
        int nBytes = send(pnode->hSocket, &data[pnode->nSendOffset], data.size() - pnode->nSendOffset, MSG_NOSIGNAL | MSG_DONTWAIT);
        if( fNetDbg ){ printf("SocketSendData to [%s], data.size = %d, Send size = %d\n", pnode->addrName.c_str(), dsz, nBytes); }
		if (nBytes > 0) {
            pnode->nLastSend = GetTime();
            pnode->nSendOffset += nBytes;
			if( fNetDbg ){ printf("SocketSendData to [%s], nSendOffset = %d, data.size = %d, Send size = %d\n", pnode->addrName.c_str(), pnode->nSendOffset, data.size(), nBytes); }
            if (pnode->nSendOffset == data.size()) {
                pnode->nSendOffset = 0;
                pnode->nSendSize -= data.size();
                it++;
            } else {
                // could not send full message; stop sending more
                break;
            }
        } else {
            if (nBytes < 0) {
                // error
                int nErr = WSAGetLastError();
                if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
                {
                    //if( nErr != 10053 )
					{
					AddDisconnectNode(pnode);	//-- 2014.11.30 add
					
					printf("[%s] socket send error %d\n", pnode->addrName.c_str(), nErr);
                    pnode->CloseSocketDisconnect();
					}
                }
            }
            // couldn't send anything at all
            break;
        }
    }

    if (it == pnode->vSendMsg.end()) {
        assert(pnode->nSendOffset == 0);
        assert(pnode->nSendSize == 0);
    }
    pnode->vSendMsg.erase(pnode->vSendMsg.begin(), it);
}

void ThreadSocketHandler(void* parg)
{
    // Make this thread recognisable as the networking thread
    RenameThread("vpncoin-net");

    try
    {
        vnThreadsRunning[THREAD_SOCKETHANDLER]++;
        ThreadSocketHandler2(parg);
        vnThreadsRunning[THREAD_SOCKETHANDLER]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_SOCKETHANDLER]--;
        PrintException(&e, "ThreadSocketHandler()");
    } catch (...) {
        vnThreadsRunning[THREAD_SOCKETHANDLER]--;
        throw; // support pthread_cancel()
    }
    printf("ThreadSocketHandler exited\n");
}

void ThreadSocketHandler2(void* parg)
{
    printf("ThreadSocketHandler started\n");
    list<CNode*> vNodesDisconnected;
    unsigned int nPrevNodeCount = 0;

    while (true)
    {
        //
        // Disconnect nodes
        //
        {
            LOCK(cs_vNodes);
            // Disconnect unused nodes
            vector<CNode*> vNodesCopy = vNodes;
            BOOST_FOREACH(CNode* pnode, vNodesCopy)
            {
                if (pnode->fDisconnect ||
                    (pnode->GetRefCount() <= 0 && pnode->vRecvMsg.empty() && pnode->nSendSize == 0 && pnode->ssSend.empty()))
                {
                    // remove from vNodes
                    vNodes.erase(remove(vNodes.begin(), vNodes.end(), pnode), vNodes.end());

                    // release outbound grant (if any)
                    pnode->grantOutbound.Release();

                    // close socket and cleanup
					//if( fDebug ){ printf("ThreadSocketHandler2 [%s] CloseSocketDisconnect, fDisconnect =%d, GetRefCount() =%d\n", pnode->addrName.c_str(), pnode->fDisconnect, pnode->GetRefCount()); }
                    pnode->CloseSocketDisconnect();

                    // hold in disconnected pool until all refs are released
                    if (pnode->fNetworkNode || pnode->fInbound)
                        pnode->Release();
                    vNodesDisconnected.push_back(pnode);
                }
            }

            // Delete disconnected nodes
            list<CNode*> vNodesDisconnectedCopy = vNodesDisconnected;
            BOOST_FOREACH(CNode* pnode, vNodesDisconnectedCopy)
            {
                // wait until threads are done using it
                if (pnode->GetRefCount() <= 0)
                {
                    bool fDelete = false;
                    {
                        TRY_LOCK(pnode->cs_vSend, lockSend);
                        if (lockSend)
                        {
                            TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
                            if (lockRecv)
                            {
                                TRY_LOCK(pnode->cs_mapRequests, lockReq);
                                if (lockReq)
                                {
                                    TRY_LOCK(pnode->cs_inventory, lockInv);
                                    if (lockInv)
                                        fDelete = true;
                                }
                            }
                        }
                    }
                    if (fDelete)
                    {
                        vNodesDisconnected.remove(pnode);
                        delete pnode;
                    }
                }
            }
        }
        if (vNodes.size() != nPrevNodeCount)
        {
            nPrevNodeCount = vNodes.size();
            uiInterface.NotifyNumConnectionsChanged(vNodes.size());
        }


        //
        // Find which sockets have data to receive
        //
        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = GetArg("-tv_usec", 50000); //50000; // frequency to poll pnode->vSend

        fd_set fdsetRecv;
        fd_set fdsetSend;
        fd_set fdsetError;
        FD_ZERO(&fdsetRecv);
        FD_ZERO(&fdsetSend);
        FD_ZERO(&fdsetError);
        SOCKET hSocketMax = 0;
        bool have_fds = false;

        if( fNewSocketThread == 0 )
		{
			BOOST_FOREACH(SOCKET hListenSocket, vhListenSocket) {
				FD_SET(hListenSocket, &fdsetRecv);
				hSocketMax = max(hSocketMax, hListenSocket);
				have_fds = true;
			}
		}
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
            {
                if (pnode->hSocket == INVALID_SOCKET)
                    continue;
                {
                    TRY_LOCK(pnode->cs_vSend, lockSend);
                    if (lockSend) {
                        // do not read, if draining write queue
                        if (!pnode->vSendMsg.empty())
                            FD_SET(pnode->hSocket, &fdsetSend);
                        else
                            FD_SET(pnode->hSocket, &fdsetRecv);
                        FD_SET(pnode->hSocket, &fdsetError);
                        hSocketMax = max(hSocketMax, pnode->hSocket);
                        have_fds = true;
                    }
                }
            }
        }

		if (fShutdown){ return; }
        if( fNewSocketThread == 0 )
		{
			vnThreadsRunning[THREAD_SOCKETHANDLER]--;
			int nSelect = select(have_fds ? hSocketMax + 1 : 0,
                             &fdsetRecv, &fdsetSend, &fdsetError, &timeout);
			vnThreadsRunning[THREAD_SOCKETHANDLER]++;
			if (fShutdown){ return; }
			if (nSelect == SOCKET_ERROR)
			{
				if (have_fds)
				{
					int nErr = WSAGetLastError();
					printf("socket select error %d\n", nErr);
					for (unsigned int i = 0; i <= hSocketMax; i++)
						FD_SET(i, &fdsetRecv);
				}
				FD_ZERO(&fdsetSend);
				FD_ZERO(&fdsetError);
				MilliSleep(timeout.tv_usec/1000);
			}


        //
        // Accept new connections
        //
        BOOST_FOREACH(SOCKET hListenSocket, vhListenSocket)
        if (hListenSocket != INVALID_SOCKET && FD_ISSET(hListenSocket, &fdsetRecv))
        {
            struct sockaddr_storage sockaddr;
            socklen_t len = sizeof(sockaddr);
            SOCKET hSocket = accept(hListenSocket, (struct sockaddr*)&sockaddr, &len);
            CAddress addr;
            int nInbound = 0;

            if (hSocket != INVALID_SOCKET)
                if (!addr.SetSockAddr((const struct sockaddr*)&sockaddr))
                    printf("Warning: Unknown socket family\n");

            {
                LOCK(cs_vNodes);
                BOOST_FOREACH(CNode* pnode, vNodes)
                    if (pnode->fInbound)
                        nInbound++;
            }

            if (hSocket == INVALID_SOCKET)
            {
                int nErr = WSAGetLastError();
                if (nErr != WSAEWOULDBLOCK)
                    printf("socket error accept failed: %d\n", nErr);
            }
            else if (nInbound >= GetArg("-maxconnections", 5000) ) //- MAX_OUTBOUND_CONNECTIONS)
            {
                closesocket(hSocket);
            }
            else if (CNode::IsBanned(addr))
            {
                printf("connection from %s dropped (banned)\n", addr.ToString().c_str());
                closesocket(hSocket);
            }
            else
            {
				struct in_addr ip;
				addr.GetInAddr(&ip);
								
				if( ip.s_addr == 0x100007f )	// 127.0.0.1
				{ 
					printf("connection from %s dropped (banned)\n", addr.ToString().c_str());
					closesocket(hSocket);				
				}else{
					printf("accepted connection %s\n", addr.ToString().c_str());	//printf("accepted connection %s, %x\n", addr.ToString().c_str(), ip.s_addr);
					CNode* pnode = new CNode(hSocket, addr, "", true);
					pnode->AddRef();
					{
						LOCK(cs_vNodes);
						vNodes.push_back(pnode);
					}
				}
            }
        }
		}


        //
        // Service each socket
        //
        vector<CNode*> vNodesCopy;
        {
            LOCK(cs_vNodes);
            vNodesCopy = vNodes;
            BOOST_FOREACH(CNode* pnode, vNodesCopy)
                pnode->AddRef();
        }
        BOOST_FOREACH(CNode* pnode, vNodesCopy)
        {
            if (fShutdown)
                return;

            //
            // Receive
            //
            if (pnode->hSocket == INVALID_SOCKET)
                continue;
            if (FD_ISSET(pnode->hSocket, &fdsetRecv) || FD_ISSET(pnode->hSocket, &fdsetError))
            {
                TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
                if (lockRecv)
                {
                    unsigned int iTrs, iRfs = 1000 * GetArg("-maxreceivebuffer", 5*1000);
					if( pnode->vBitNet.v_Starting_recv ){ iTrs = 0; }
					else{ iTrs = pnode->GetTotalRecvSize(); }
					if (iTrs > iRfs) {	//if (pnode->GetTotalRecvSize() > ReceiveFloodSize()) {
                        //if (!pnode->fDisconnect)
                            //printf("socket recv flood control disconnect (%u bytes)\n", pnode->GetTotalRecvSize());
							printf("[%s] socket recv flood control disconnect (%u bytes), %u\n", pnode->addr.ToString().c_str(), iTrs, iRfs);
                        pnode->vBitNet.v_bCloseByRecvOverMaxRecvBufSz = 1;
						pnode->CloseSocketDisconnect();
                    }
                    else {
                        // typical socket buffer is 8K-64K
                        char pchBuf[0x10000];
                        int nBytes = recv(pnode->hSocket, pchBuf, sizeof(pchBuf), MSG_DONTWAIT);
                        if (nBytes > 0)
                        {
                            //if( fDebug ){ printf("[%s] recv %d bytes\n", pnode->addr.ToString().c_str(), nBytes); }
							if( pnode->vBitNet.v_Recv_ZeroBytes > 0 ){ pnode->vBitNet.v_Recv_ZeroBytes = 0; }	// BitNet Add
							
							if (!pnode->ReceiveMsgBytes(pchBuf, nBytes))
							{
                                if( fDebug ){ printf("ReceiveMsgBytes faile, [%s] CloseSocketDisconnect\n", pnode->addrName.c_str()); }
								AddDisconnectNode(pnode);	//-- 2014.11.30 add
								pnode->CloseSocketDisconnect();
							}
                            pnode->nLastRecv = GetTime();
                        }
                        else if (nBytes == 0)
                        {
                            // socket closed gracefully
                            pnode->vBitNet.v_Recv_ZeroBytes++;	// BitNet Add			
							if( pnode->nLastRecv == 0 ){ pnode->PushMessage("getver"); }
							else if( pnode->nLastSend == 0 ){ pnode->PushVersion(); }							
							else{ pnode->fDisconnect = true; }
							
							//if (!pnode->fDisconnect)
							{
							
								/*int64_t i6Now = GetTime();
								if( ((i6Now - pnode->nLastSend) >= 60) or ((i6Now - pnode->nLastRecv) >= 60) )
								{
									pnode->PushMessage("getver");
								}*/
								//AddDisconnectNode(pnode);	//-- 2014.11.30 add								
								//pnode->CloseSocketDisconnect();							
							}
							
							//if( (i6Now > pnode->nTimeConnected) && ((i6Now - pnode->nTimeConnected) >= 5) )
							{
								//if (!pnode->fDisconnect)
									//printf("socket closed (nBytes = 0), %I64u : %u, [%s]\n", pnode->nTimeConnected, pnode->vBitNet.v_Recv_ZeroBytes, pnode->addr.ToString().c_str());
								//if( pnode->vBitNet.v_Recv_ZeroBytes > 60 )
									//AddDisconnectNode(pnode);	//-- 2014.11.30 add
									//pnode->CloseSocketDisconnect();
							}
                        }
                        else if (nBytes < 0)
                        {
                            // error
                            int nErr = WSAGetLastError();
                            if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
                            {
                                //if( nErr != 10053 )	//if( nErr != 10054 )
								{
									if (!pnode->fDisconnect)
									{
										if( fNetDbg ) printf("socket recv error %d, [%s]\n", nErr, pnode->addr.ToString().c_str());
										AddDisconnectNode(pnode);	//-- 2014.11.30 add								
									}
									pnode->CloseSocketDisconnect();
								}
                            }
                        }
                    }
                }
            }

            //
            // Send
            //
            if (pnode->hSocket == INVALID_SOCKET)
                continue;
            if (FD_ISSET(pnode->hSocket, &fdsetSend))
            {
                TRY_LOCK(pnode->cs_vSend, lockSend);
                if (lockSend)
                    SocketSendData(pnode);
            }

            //
            // Inactivity checking
            //
            int64_t nTime = GetTime();
            if( (nTime - pnode->nTimeConnected) > 66 )  // 60
            {
                if( pnode->nLastSend == 0 ){ pnode->PushVersion(); }
				else if ( pnode->nLastRecv == 0 )	//if (pnode->nLastRecv == 0 || pnode->nLastSend == 0)
                {
					if( pnode->vBitNet.v_IsGuiNode > 0 ){ pnode->PushMessage("getver"); }
					else{  
						if( !pnode->fDisconnect )
						{
							AddDisconnectNode(pnode);	//-- 2014.11.30 add								
							pnode->CloseSocketDisconnect();	//pnode->fDisconnect = true; //}
						}					
					}
					//if( fDebug ){ printf("socket (%s) no message in first 360 seconds, %I64u %I64u\n", pnode->addr.ToString().c_str(), pnode->nLastRecv, pnode->nLastSend); }
					//if( pnode->vBitNet.v_BitNetMsgCount == 0 )
					{ 
						//if( (nTime > pnode->vBitNet.v_LastPushMsgTime) && (nTime - pnode->vBitNet.v_LastPushMsgTime) > 60 ){ pnode->PushMessage("getver"); }
					}
                }
                else if (nTime - pnode->nLastSend > TIMEOUT_INTERVAL)
                {
                    printf("socket sending timeout: %"PRId64"s\n", nTime - pnode->nLastSend);
                    pnode->fDisconnect = true;
                }
                else if (nTime - pnode->nLastRecv > (pnode->nVersion > BIP0031_VERSION ? TIMEOUT_INTERVAL : 90*60))
                {
                    printf("socket receive timeout: %"PRId64"s\n", nTime - pnode->nLastRecv);
                    pnode->fDisconnect = true;
                }
                else if (pnode->nPingNonceSent && pnode->nPingUsecStart + TIMEOUT_INTERVAL * 1000000 < GetTimeMicros())
                {
                    printf("ping timeout: %fs\n", 0.000001 * (GetTimeMicros() - pnode->nPingUsecStart));
                    pnode->fDisconnect = true;
                }
            }/* else{
				if( pnode->nLastSend == 0 )
				{ 
					pnode->PushVersion();
					printf("pnode (%s) nLastSend = 0, PushVersion()\n", pnode->addr.ToString().c_str());
				}
			} */
        }
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodesCopy)
			{
                //if( pnode->vBitNet.v_ListItem > 0 ){ SynNodeToBitNetGui(pnode, 0, 0, NULL); }	//--2014.12.06 add
				pnode->Release();
			}
        }

        MilliSleep(10);
    }
}	//ThreadSocketHandler2


void SocketServiceThread(void* parg)
{
    RenameThread("SocketServiceThread");
	printf("SocketServiceThread started\n");
	
	int fDbg = GetArg("-ssthrdbg", 0);
	int uSec = GetArg("-tv_usec", 50000); //50000; // frequency to poll pnode->vSend
    try
    {
        //vnThreadsRunning[THREAD_UPNP]++;
        //ThreadMapPort2(parg);
        //vnThreadsRunning[THREAD_UPNP]--;
		while( fShutdown == false )
		{

        struct timeval timeout;
        timeout.tv_sec  = 0;
        timeout.tv_usec = uSec;

        fd_set fdsetRecv;
        fd_set fdsetSend;
        fd_set fdsetError;
        FD_ZERO(&fdsetRecv);
        FD_ZERO(&fdsetSend);
        FD_ZERO(&fdsetError);
        SOCKET hSocketMax = 0;
        bool have_fds = false;

        BOOST_FOREACH(SOCKET hListenSocket, vhListenSocket) {
            FD_SET(hListenSocket, &fdsetRecv);
            hSocketMax = max(hSocketMax, hListenSocket);
            have_fds = true;
        }

        //vnThreadsRunning[THREAD_SOCKETHANDLER]--;
        int nSelect = select(have_fds ? hSocketMax + 1 : 0,
                             &fdsetRecv, &fdsetSend, &fdsetError, &timeout);
        //vnThreadsRunning[THREAD_SOCKETHANDLER]++;
        if (fShutdown){ return; }
        if (nSelect == SOCKET_ERROR)
        {
            if (have_fds)
            {
                int nErr = WSAGetLastError();
                printf("socket select error %d\n", nErr);
                for (unsigned int i = 0; i <= hSocketMax; i++)
                    FD_SET(i, &fdsetRecv);
            }
            FD_ZERO(&fdsetSend);
            FD_ZERO(&fdsetError);
            MilliSleep(timeout.tv_usec/1000);
        }


        //
        // Accept new connections
        //
          BOOST_FOREACH(SOCKET hListenSocket, vhListenSocket)
          if (hListenSocket != INVALID_SOCKET && FD_ISSET(hListenSocket, &fdsetRecv))
          {
            struct sockaddr_storage sockaddr;
            socklen_t len = sizeof(sockaddr);
            SOCKET hSocket = accept(hListenSocket, (struct sockaddr*)&sockaddr, &len);
            CAddress addr;
            int nInbound = 0;

            if (hSocket != INVALID_SOCKET)
                if (!addr.SetSockAddr((const struct sockaddr*)&sockaddr))
                    printf("Warning: Unknown socket family\n");

            /*{
                LOCK(cs_vNodes);
                BOOST_FOREACH(CNode* pnode, vNodes)
                    if (pnode->fInbound)
                        nInbound++;
            }*/

            if (hSocket == INVALID_SOCKET)
            {
                int nErr = WSAGetLastError();
                if (nErr != WSAEWOULDBLOCK)
                    printf("socket error accept failed: %d\n", nErr);
            }
            /*else if (nInbound >= GetArg("-maxconnections", 5000) - MAX_OUTBOUND_CONNECTIONS)
            {
                closesocket(hSocket);
            }*/
            else if (CNode::IsBanned(addr))
            {
                if( fDbg ){ printf("connection from %s dropped (banned)\n", addr.ToString().c_str()); }
                closesocket(hSocket);
            }
            else
            {
				struct in_addr ip;
				addr.GetInAddr(&ip);
								
				if( ip.s_addr == 0x100007f )	// 127.0.0.1
				{ 
					if( fDbg ){ printf("connection from %s dropped (banned)\n", addr.ToString().c_str()); }
					closesocket(hSocket);				
				}else{
					if( fDbg ){ printf("accepted connection %s\n", addr.ToString().c_str());	}  //printf("accepted connection %s, %x\n", addr.ToString().c_str(), ip.s_addr);
					CNode* pnode = new CNode(hSocket, addr, "", true);
					pnode->AddRef();
					{
						LOCK(cs_vNodes);
						vNodes.push_back(pnode);
					}
				}
            }
          }
		  MilliSleep(10);
		}
    }
    catch (std::exception& e) {
        //vnThreadsRunning[THREAD_UPNP]--;
        PrintException(&e, "SocketServiceThread()");
    } catch (...) {
        //vnThreadsRunning[THREAD_UPNP]--;
        PrintException(NULL, "SocketServiceThread()");
    }
    printf("SocketServiceThread exited\n");
}








#ifdef USE_UPNP
void ThreadMapPort(void* parg)
{
    // Make this thread recognisable as the UPnP thread
    RenameThread("vpncoin-UPnP");

    try
    {
        vnThreadsRunning[THREAD_UPNP]++;
        ThreadMapPort2(parg);
        vnThreadsRunning[THREAD_UPNP]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_UPNP]--;
        PrintException(&e, "ThreadMapPort()");
    } catch (...) {
        vnThreadsRunning[THREAD_UPNP]--;
        PrintException(NULL, "ThreadMapPort()");
    }
    printf("ThreadMapPort exited\n");
}

void ThreadMapPort2(void* parg)
{
    printf("ThreadMapPort started\n");

    std::string port = strprintf("%u", GetListenPort());
    std::string port920 = "920";
    const char * multicastif = 0;
    const char * minissdpdpath = 0;
    struct UPNPDev * devlist = 0;
    char lanaddr[64];

#ifdef USE_BITNET
	int i_Open_P2P_Proxy = d_Open_P2P_Proxy;
	int i_P2P_proxy_port = d_P2P_proxy_port;
	std::string P2p_port = strprintf("%u", i_P2P_proxy_port);
#endif

#ifndef UPNPDISCOVER_SUCCESS
    /* miniupnpc 1.5 */
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0);
#else
    /* miniupnpc 1.6 */
    int error = 0;
    devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0, 0, &error);
#endif

    struct UPNPUrls urls;
    struct IGDdatas data;
    int r, r2, r920;

    r = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr));
    if (r == 1)
    {
        if (fDiscover) {
            char externalIPAddress[40];
            r = UPNP_GetExternalIPAddress(urls.controlURL, data.first.servicetype, externalIPAddress);
            if(r != UPNPCOMMAND_SUCCESS)
                printf("UPnP: GetExternalIPAddress() returned %d\n", r);
            else
            {
                if(externalIPAddress[0])
                {
                    printf("UPnP: ExternalIPAddress = %s\n", externalIPAddress);
                    AddLocal(CNetAddr(externalIPAddress), LOCAL_UPNP);
                }
                else
                    printf("UPnP: GetExternalIPAddress failed.\n");
            }
        }

        string strDesc = "VpnCoin " + FormatFullVersion();
		string strDesc920 = strDesc + "_" + port920;
#ifdef USE_BITNET		
		string strP2pDesc = "BitNet P2P Proxy " + P2p_port;
#endif
		
#ifndef UPNPDISCOVER_SUCCESS
        /* miniupnpc 1.5 */
        r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0);
        if( fBindPort920 > 0 ){ r920 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port920.c_str(), port920.c_str(), lanaddr, strDesc920.c_str(), "TCP", 0); }
#ifdef USE_BITNET
		if( i_Open_P2P_Proxy )
		  r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0);
#endif

#else
        /* miniupnpc 1.6 */
        r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0, "0");
        if( fBindPort920 > 0 ){ r920= UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port920.c_str(), port920.c_str(), lanaddr, strDesc920.c_str(), "TCP", 0, "0"); }
#ifdef USE_BITNET
		if( i_Open_P2P_Proxy )
		  r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0, "0");
#endif

#endif

        if(r!=UPNPCOMMAND_SUCCESS)
            printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",  port.c_str(), port.c_str(), lanaddr, r, strupnperror(r));
        else
            printf("UPnP Port Mapping successful. %u\n", r);

#ifdef USE_BITNET
		if(r2 != UPNPCOMMAND_SUCCESS){
            d_P2P_Proxy_Port_mapped = 0;
			if( fDebug ) printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n", P2p_port.c_str(), P2p_port.c_str(), lanaddr, r2, strupnperror(r2));
		}
		else{
            d_P2P_Proxy_Port_mapped++;
			if( fDebug ) printf("UPnP Port Mapping for P2P Proxy successful. %u\n", r2);
		}
#endif
			
        int i = 1;
		int i_P2P_Proxy_Port_mapped = 0;
        while (true)
        {			
#ifdef USE_BITNET
			if( i_P2P_proxy_port != d_P2P_proxy_port )
			{
				printf("UPNP i_P2P_proxy_port= %u : %d\n", i_P2P_proxy_port, d_P2P_proxy_port);
				r2 = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), "TCP", 0);
				printf("UPNP_DeletePortMapping() Proxy =%d\n", r2);
				d_P2P_Proxy_Port_mapped = 0;
				i_P2P_Proxy_Port_mapped = 0;
				
				i_P2P_proxy_port = d_P2P_proxy_port;
				P2p_port.clear();
				P2p_port = strprintf("%u", i_P2P_proxy_port);
				strP2pDesc.clear();
				strP2pDesc = "BitNet P2P Proxy " + P2p_port;
#ifndef UPNPDISCOVER_SUCCESS
        /* miniupnpc 1.5 */
		if( i_Open_P2P_Proxy )
		{
			r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0);
			if( r2 == UPNPCOMMAND_SUCCESS ) d_P2P_Proxy_Port_mapped = 1;
		}
#else
        /* miniupnpc 1.6 */
		if( i_Open_P2P_Proxy )
		{
			r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0, "0");
			if( r2 == UPNPCOMMAND_SUCCESS ) d_P2P_Proxy_Port_mapped = 1;
		}
#endif
			}
			//if( i_Open_P2P_Proxy != d_Open_P2P_Proxy )
			{
				i_Open_P2P_Proxy = d_Open_P2P_Proxy;
			}		
			if( i_P2P_Proxy_Port_mapped != d_P2P_Proxy_Port_mapped )
			{
				printf("UPNP i_P2P_Proxy_Port_mapped %u : %d\n", i_P2P_Proxy_Port_mapped, d_P2P_Proxy_Port_mapped);
				i_P2P_Proxy_Port_mapped = d_P2P_Proxy_Port_mapped;
				LoadIniCfg( 0, 1 );
			}
#endif

		
			if (fShutdown || !fUseUPnP)
            {
                r  = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port.c_str(), "TCP", 0);
                if( fBindPort920 > 0 ){ r920  = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, port920.c_str(), "TCP", 0); }
#ifdef USE_BITNET
				r2 = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), "TCP", 0);
                printf("UPNP_DeletePortMapping() returned : %d\n", r);
				if( fDebug ){ printf("UPNP_DeletePortMapping() P2P Proxy returned : %d\n", r2); }
#endif				
                freeUPNPDevlist(devlist); devlist = 0;
                FreeUPNPUrls(&urls);
                return;
            }
            if (i % 600 == 0) // Refresh every 20 minutes
            {
#ifndef UPNPDISCOVER_SUCCESS
                /* miniupnpc 1.5 */
                r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0);
                if( fBindPort920 > 0 ){ r920 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port920.c_str(), port920.c_str(), lanaddr, strDesc920.c_str(), "TCP", 0); }
#ifdef USE_BITNET
				if( i_Open_P2P_Proxy )
				{
				  r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0); 
				  if( r2 == UPNPCOMMAND_SUCCESS ) d_P2P_Proxy_Port_mapped = 1;
				}	else{
				  r2 = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), "TCP", 0);
				  d_P2P_Proxy_Port_mapped = 0;
				}
#endif
#else
                /* miniupnpc 1.6 */
                r  = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port.c_str(), port.c_str(), lanaddr, strDesc.c_str(), "TCP", 0, "0");
                if( fBindPort920 > 0 ){ r920  = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, port920.c_str(), port920.c_str(), lanaddr, strDesc920.c_str(), "TCP", 0, "0"); }
#ifdef USE_BITNET
				if( i_Open_P2P_Proxy )
				{
				  r2 = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), P2p_port.c_str(), lanaddr, strP2pDesc.c_str(), "TCP", 0, "0");
				  if( r2 == UPNPCOMMAND_SUCCESS ) d_P2P_Proxy_Port_mapped = 1;
				}	else{
				  r2 = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, P2p_port.c_str(), "TCP", 0);
				  d_P2P_Proxy_Port_mapped = 0;
				}
#endif
#endif

                if(r != UPNPCOMMAND_SUCCESS)
                    printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n", port.c_str(), port.c_str(), lanaddr, r, strupnperror(r));
                else
                    printf("UPnP Port Mapping successful.\n");
					
#ifdef USE_BITNET					
                if( fDebug ){
					if(r2 != UPNPCOMMAND_SUCCESS)
					  printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n", P2p_port.c_str(), P2p_port.c_str(), lanaddr, r2, strupnperror(r2));
					else
					  printf("UPnP Port Mapping for P2P Proxy successful.\n");
				}
#endif
            }
            MilliSleep(2000);
            i++;
        }
    } else {
        printf("No valid UPnP IGDs found\n");
        freeUPNPDevlist(devlist); devlist = 0;
        if (r != 0)
            FreeUPNPUrls(&urls);
        while (true)
        {
            if (fShutdown || !fUseUPnP)
                return;
            MilliSleep(2000);
        }
    }
}

void MapPort()
{
    if (fUseUPnP && vnThreadsRunning[THREAD_UPNP] < 1)
    {
        if (!NewThread(ThreadMapPort, NULL))
            printf("Error: ThreadMapPort(ThreadMapPort) failed\n");
    }
}
#else
void MapPort()
{
    // Intentionally left blank.
}
#endif









// DNS seeds
// Each pair gives a source name and a seed name.
// The first name is used as information source for addrman.
// The second name should resolve to a list of seed addresses.
static const char *strDNSSeed[][2] = {
    {"s1.vpncoin.org", "seed.vpncoin.org"},
    {"s2.vpncoin.org", "node.vpncoin.org"},
    {"s3.vpncoin.org", "pool.vpncoin.org"},
	{"s4.vpncoin.org", "s4.vpncoin.org"},
	{"s5.vpncoin.org", "s5.vpncoin.org"},
	{"s6.vpncoin.org", "s6.vpncoin.org"},
	{"s7.vpncoin.org", "s7.vpncoin.org"},
	{"s8.vpncoin.org", "s8.vpncoin.org"},
	{"s9.vpncoin.org", "s9.vpncoin.org"},	
	{"sa.vpncoin.org", "abe.vpncoin.org"},
	{"sf.vpncoin.org", "faucet.vpncoin.org"},
    {"s1.bitnet.cc", "s1.bitnet.cc"},
    {"s2.bitnet.cc", "s2.bitnet.cc"},
    {"s3.bitnet.cc", "s3.bitnet.cc"},
	{"s4.bitnet.cc", "s4.bitnet.cc"},
	{"s5.bitnet.cc", "s5.bitnet.cc"},
	{"s6.bitnet.cc", "s6.bitnet.cc"},
	{"s7.bitnet.cc", "s7.bitnet.cc"},
	{"s8.bitnet.cc", "s8.bitnet.cc"},
	{"s9.bitnet.cc", "s9.bitnet.cc"},
	{"www.bmarket.cc", "www.bmarket.cc"},
};

void ThreadDNSAddressSeed(void* parg)
{
    // Make this thread recognisable as the DNS seeding thread
    RenameThread("vpncoin-dnsseed");

    try
    {
        vnThreadsRunning[THREAD_DNSSEED]++;
        ThreadDNSAddressSeed2(parg);
        vnThreadsRunning[THREAD_DNSSEED]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_DNSSEED]--;
        PrintException(&e, "ThreadDNSAddressSeed()");
    } catch (...) {
        vnThreadsRunning[THREAD_DNSSEED]--;
        throw; // support pthread_cancel()
    }
    printf("ThreadDNSAddressSeed exited\n");
}

void ThreadDNSAddressSeed2(void* parg)
{
    printf("ThreadDNSAddressSeed started\n");

    if( parg == NULL )
	{
		// goal: only query DNS seeds if address need is acute
	  if ((addrman.size() > 0) &&
        (!GetBoolArg("-forcednsseed", false))) {
        MilliSleep(11 * 1000);

        LOCK(cs_vNodes);
        if (vNodes.size() >= 2) {
            printf("P2P peers available. Skipped DNS seeding.\n");
            return;
        }
      }
	}
    int found = 0;

    if (!fTestNet)
    {
        printf("Loading addresses from DNS seeds (could take a while)\n");
		char buf[512];
        for (unsigned int seed_idx = 0; seed_idx < ARRAYLEN(strDNSSeed); seed_idx++) {
            if (HaveNameProxy()) {
                AddOneShot(strDNSSeed[seed_idx][1]);
            } else {
                vector<CNetAddr> vaddr;
                vector<CAddress> vAdd;

				std::string sHost = strDNSSeed[seed_idx][1];
				unsigned int wPort = 0;	//GetDefaultPort();
				//if( fDebug ){ printf("Host [%s] Port [%d]\n", sHost.c_str(), wPort); }				

				
				memset(buf, 0, sizeof(buf)); 
				memcpy(buf, sHost.c_str(), sHost.length());
				char* pHost = (char*)&buf[0];
				wPort = GetHostPort(pHost);

				//if( fDebug ){ printf("Host [%s] Port [%d]\n", pHost, wPort); }
                if (LookupHost(pHost, vaddr))	 
				//if (LookupHost(strDNSSeed[seed_idx][1], vaddr))
                {
                    BOOST_FOREACH(CNetAddr& ip, vaddr)
                    {
                        int nOneDay = 24*3600;
                        CAddress addr = CAddress(CService(ip, wPort));
						//CAddress addr = CAddress(CService(ip, GetDefaultPort()));

                        /*{
                            CSemaphoreGrant grant(*semOutbound);
                            OpenNetworkConnection(addr, &grant);
                            MilliSleep(500);
                            if( fShutdown ) return;
                        }*/

                        addr.nTime = GetTime() - 3*nOneDay - GetRand(4*nOneDay); // use a random age between 3 and 7 days old
                        vAdd.push_back(addr);
						AddOneShot( addr.ToString() );
						if( fNetDbg ){ printf("ThreadDNSAddressSeed [%s] [%s]:[%d]\n", pHost, addr.ToString().c_str(), wPort); }
                        found++;
                    }
                }
                addrman.Add(vAdd, CNetAddr(pHost, true));	
				//addrman.Add(vAdd, CNetAddr(strDNSSeed[seed_idx][0], true));
            }
        }
    }

    printf("%d addresses found from DNS seeds\n", found);
}

DWORD SyncNodeIpPort(DWORD ip, DWORD port)
{
	DWORD rzt = 0;
	//if(fDebug){ printf("SyncNodeIpPort %X : %d\n", ip, port); }
	if( ip == 1 )
	{
		/*rzt = NewThread(ThreadDNSAddressSeed, (char*)ip);
		if( !rzt )
            printf("Error: NewThread(ThreadDNSAddressSeed) failed\n");
			*/
	}
	else{ rzt = ConnectToIp(ip, port); }
	//if(fDebug){ printf("SyncNodeIpPort %X : %d rzt %d\n", ip, port, rzt); }
	return rzt;
}





unsigned int pnSeed[] =
{

};

void DumpAddresses()
{
    int64_t nStart = GetTimeMillis();

    CAddrDB adb;
    adb.Write(addrman);

    printf("Flushed %d addresses to peers.dat  %"PRId64"ms\n",
           addrman.size(), GetTimeMillis() - nStart);
}

void ThreadDumpAddress2(void* parg)
{
    vnThreadsRunning[THREAD_DUMPADDRESS]++;
    while (!fShutdown)
    {
        DumpAddresses();
        vnThreadsRunning[THREAD_DUMPADDRESS]--;
        MilliSleep(600000);
        vnThreadsRunning[THREAD_DUMPADDRESS]++;
    }
    vnThreadsRunning[THREAD_DUMPADDRESS]--;
}

void ThreadDumpAddress(void* parg)
{
    // Make this thread recognisable as the address dumping thread
    RenameThread("vpncoin-adrdump");

    try
    {
        ThreadDumpAddress2(parg);
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadDumpAddress()");
    }
    printf("ThreadDumpAddress exited\n");
}

void ThreadOpenConnections(void* parg)
{
    // Make this thread recognisable as the connection opening thread
    RenameThread("vpncoin-opencon");

    try
    {
        vnThreadsRunning[THREAD_OPENCONNECTIONS]++;
        ThreadOpenConnections2(parg);
        vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
        PrintException(&e, "ThreadOpenConnections()");
    } catch (...) {
        vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
        PrintException(NULL, "ThreadOpenConnections()");
    }
    printf("ThreadOpenConnections exited\n");
}

void static ProcessOneShot()
{
    string strDest;
	int ic;
    {
        LOCK(cs_vOneShots);
        if (vOneShots.empty())
            return;
        ic = vOneShots.size();
		strDest = vOneShots.front();
        vOneShots.pop_front();
    }
	char *pHost = (char *)strDest.c_str();
    CAddress addr;
    CSemaphoreGrant grant(*semOutbound, true);
    if (grant) {
	    bool fOneShot = true;
		bool bAdd = true;
		//char *pHost = (char *)strDest.c_str();
		if( pHost[0] == '+' )
		{
			fOneShot = false;   pHost++;
			if( fNetDbg ){ printf("ProcessOneShot + [%s] [%d]\n", pHost, ic); }
		}
		else if( pHost[0] == '-' )
		{
			fOneShot = false;   pHost++;  bAdd = false;
			if( fNetDbg ){ printf("ProcessOneShot - [%s] [%d]\n", pHost, ic); }
		}
        if (!OpenNetworkConnection(addr, &grant, pHost, fOneShot))	//if (!OpenNetworkConnection(addr, &grant, strDest.c_str(), true))
		{
            //if( fDebug ){ printf("OpenNetworkConnection [%s] faile :(\n", pHost); }
			if( bAdd )  //if( GetArg("-autosyncnode", 1) == 0 ) 	//--2014.11.15 add
			{
			    AddOneShot(strDest); 
			}
		}
    }
	//else if( fDebug ){ printf("ProcessOneShot [%s] grant = NULL :( %d\n", pHost, ic); }
}


int GetParamCountInConf(string sFile, string sParam)  //int GetParamCountInConf(string sFile, "addnode=")
{
    int rzt = 0;
	if( (sFile.length() == 0) || (sParam.length() == 0) ){ return rzt; }

	ifstream infile;
    infile.open(sFile.c_str());
	if( infile.is_open() )
	{
		string line;
		while (getline(infile, line, '\n'))
		{
			if( line.find(sParam) != string::npos ){ rzt++; }
		}
		infile.close();
	}
    return rzt;
}

int NodeAddedInConf(string sNode)
{
	int rzt = 0;
	if( sNode.length() > 0 )
	{
		string s_conf_file = GetConfigFile().string(); 
		//std::ifstream t(s_conf_file);
        ifstream t;  //( s_conf_file );
		t.open( s_conf_file.c_str() ); 
		if( t.is_open() )  //if( t.good() )
		{
			std::stringstream buffer;  
			buffer << t.rdbuf();
			std::string contents(buffer.str());
			t.close(); 
			string s2 = "addnode=" + sNode;
			unsigned int dEc = contents.find( s2 ); 
			if (dEc != string::npos){ rzt = dEc + 1; }
		}
	}
	return rzt;
}

int NodeAddedInConf(char* pNode)
{
	int rzt = 0;
	if( pNode != NULL )
	{
		string sNode = pNode;
		rzt = NodeAddedInConf(sNode);
	}
	return rzt;
}

//׷\BCӷ\BDʽд\CEļ\FE  
void MyWritefile(string sFile, string sBuf)  
{  
    fstream f;  //( sFile, ios::out|ios::app );
	f.open(sFile.c_str(), ios::out|ios::app );
    if( f.is_open() )  //if( !f ){ return; }
	{
		f << sBuf;
		f.close(); 
	}
}

int AddNodeToConf(string sNode)
{
	int rzt = 0;
	if( sNode.length() > 0 )
	{
		string s_conf_file = GetConfigFile().string(); 
		string s2 = "\naddnode=" + sNode;
		MyWritefile(s_conf_file, s2);
		rzt++;
	}
	return rzt;
}

int AddNodeToConf(char* pNode)
{
	int rzt = 0;
	if( pNode != NULL )
	{
		string sNode = pNode;
		rzt = AddNodeToConf(sNode);
	}
	return rzt;
}


int GetOneShots2Count()
{
	LOCK(cs_vOneShots2);
	return vOneShots3.size();
}

//typedef std::pair<string, int> pair_OneShotItem;
 int GetOneShots2Item(string &sHost)
{
	int rzt = 0;
	sHost = "";
	LOCK(cs_vOneShots2);
	if( vOneShots3.size() > 0 )
	{
		BOOST_FOREACH(const PAIRTYPE(string, int)& item, vOneShots3)
		{
			sHost = item.first.c_str();
			rzt = item.second;
			vOneShots3.erase( remove(vOneShots3.begin(), vOneShots3.end(), item), vOneShots3.end() );
			break;
		}
	}
	return rzt;
}

int iAddNodeCount = 0;
void static ProcessOneShot2(int64_t iTm)
{
	int ic = GetOneShots2Count();
	if( ic == 0 ){ 
		//if( fNetDbg ){ printf("%I64u :: ProcessOneShot2 Count = 0 :(\n", iTm); }
		return; 
	}
    string strDest = "";
	int icc = GetOneShots2Item(strDest);
	if( icc == 0 ){ 
		//if( fNetDbg ){ printf("%I64u :: ProcessOneShot2 Item = 0 :(\n", iTm); }
		return; 
	}
	if( fNetDbg ){ printf("%" PRId64 " :: ProcessOneShot2 Count = [%d], icc = [%d] \n", iTm, ic, icc); }
    /*{
        LOCK(cs_vOneShots2);
        if (vOneShots2.empty()){ return; }
        ic = vOneShots2.size();
		strDest = vOneShots2.front();
        vOneShots2.pop_front();
    }*/
	
	char* pHost = (char *)strDest.c_str();
	CAddress addr;
	CSemaphoreGrant grant(*semOutbound, true);
	if (grant) {
		bool fOneShot = true;
		bool bAdd = true;
		//char *pHost = (char *)strDest.c_str();
		if( pHost[0] == '+' )
		{
			fOneShot = false;   pHost++;
			if( fNetDbg ){ printf("%" PRId64 " :: ProcessOneShot2 + [%s] [%d]\n", iTm, pHost, ic); }
		}
		else if( pHost[0] == '-' )
		{
			fOneShot = false;   pHost++;  bAdd = false;
			if( fNetDbg ){ printf("%" PRId64 " :: ProcessOneShot2 - [%s] [%d : %d]\n", iTm, pHost, ic); }
		}
		
        if( GetArg("-addnodetoconf", 0) && (GetArg("-isseednode", 0) == 0) )
        {
            int iMaxNodeToConf = GetArg("-maxnodetoconf", 5000);  // 2015.10.17 add
            if( iAddNodeCount < iMaxNodeToConf ){ iAddNodeCount = GetParamCountInConf(GetConfigFile().string(), "addnode="); }			
			if( (iAddNodeCount < iMaxNodeToConf) && (NodeAddedInConf(pHost) == 0) )
			{
				if( !fTestNet ){ AddNodeToConf(pHost); }
			}
        }

		if (!OpenNetworkConnection(addr, &grant, pHost, fOneShot))	//if (!OpenNetworkConnection(addr, &grant, strDest.c_str(), true))
		{
			if( icc > 0 ){ icc--; }
			if( fNetDbg ){ printf("%" PRId64 " :: ProcessOneShot2 OpenNetworkConnection [%s] faile, [%d : %d] :(\n", iTm, pHost, ic, icc); }
			//if( fNetDbg ){ printf("%I64u :: ProcessOneShot2 icc = [%d] [%d] [%s] \n", iTm, icc, ic, strDest.c_str()); }
			if( icc > 0 )  //if( bAdd ) 
			{
				AddOneShot22(strDest, icc); 
			}
		}
		//else if( fNetDbg ){ printf("%I64u :: ProcessOneShot2 OpenNetworkConnection [%s] = true, %d %d\n", iTm, pHost, ic, icc); }
	}
	else {
		AddOneShot22(strDest, icc);
		if( fNetDbg ){ printf("%" PRId64 " :: ProcessOneShot2 [%s] grant = NULL, %d %d :( \n", iTm, pHost, ic, icc); }
	}
}

void ThreadOpenOneShotNode(void* parg)
{
    // Make this thread recognisable as the connection opening thread
    RenameThread("vpncoin-openone");
	if( fNetDbg ){ printf("ThreadOpenOneShotNode start\n"); }
    try
    {		
		int64_t iTm;
		while ( !fShutdown )
		{
			vnThreadsRunning[THREAD_OPENOneShotNode]++;
			iTm = GetTime();   ProcessOneShot2(iTm); 
			vnThreadsRunning[THREAD_OPENOneShotNode]--;
			MilliSleep(500);
			//vnThreadsRunning[THREAD_OPENOneShotNode]++;
			if (fShutdown) return;
			MilliSleep(100);
		}		
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_OPENOneShotNode]--;
        PrintException(&e, "ThreadOpenOneShotNode()");
    } catch (...) {
        vnThreadsRunning[THREAD_OPENOneShotNode]--;
        PrintException(NULL, "ThreadOpenOneShotNode()");
    }
    if( fDebug ){ printf("ThreadOpenOneShotNode exited\n"); }
}

void static ThreadStakeMiner(void* parg)
{
    printf("ThreadStakeMiner started\n");
    CWallet* pwallet = (CWallet*)parg;
    try
    {
        vnThreadsRunning[THREAD_STAKE_MINER]++;
        StakeMiner(pwallet);
        vnThreadsRunning[THREAD_STAKE_MINER]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_STAKE_MINER]--;
        PrintException(&e, "ThreadStakeMiner()");
    } catch (...) {
        vnThreadsRunning[THREAD_STAKE_MINER]--;
        PrintException(NULL, "ThreadStakeMiner()");
    }
    printf("ThreadStakeMiner exiting, %d threads remaining\n", vnThreadsRunning[THREAD_STAKE_MINER]);
}

void ThreadOpenConnections2(void* parg)
{
    printf("ThreadOpenConnections started\n");

    // Connect to specific addresses
    if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0)
    {
        for (int64_t nLoop = 0;; nLoop++)
        {
            ProcessOneShot();
            BOOST_FOREACH(string strAddr, mapMultiArgs["-connect"])
            {
                CAddress addr;
                OpenNetworkConnection(addr, NULL, strAddr.c_str());
                for (int i = 0; i < 10 && i < nLoop; i++)
                {
                    MilliSleep(500);
                    if (fShutdown)
                        return;
                }
            }
            MilliSleep(500);
        }
    }

    printf("ThreadOpenConnections started 2\n");
    /*for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++)  // 2015.10.14 add
    {
        struct in_addr ip;
        memcpy(&ip, &pnSeed[i], sizeof(ip));
        CAddress addr(CService(ip, GetDefaultPort()));
        CSemaphoreGrant grant(*semOutbound);
        OpenNetworkConnection(addr, &grant);
        MilliSleep(500);
        if( fShutdown ) return;
    }*/

    printf("ThreadOpenConnections started 3\n");

    // Initiate network connections
    int64_t nStart = GetTime();
    while (true)
    {
        ProcessOneShot();

        vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
        MilliSleep(500);
        vnThreadsRunning[THREAD_OPENCONNECTIONS]++;
        if (fShutdown)
            return;


        vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
        CSemaphoreGrant grant(*semOutbound);
        vnThreadsRunning[THREAD_OPENCONNECTIONS]++;
        if (fShutdown)
            return;

        // Add seed nodes if IRC isn't working
        if (addrman.size()==0 && (GetTime() - nStart > 60) && !fTestNet)
        {
            std::vector<CAddress> vAdd;
            for (unsigned int i = 0; i < ARRAYLEN(pnSeed); i++)
            {
                // It'll only connect to one or two seed nodes because once it connects,
                // it'll get a pile of addresses with newer timestamps.
                // Seed nodes are given a random 'last seen time' of between one and two
                // weeks ago.
                const int64_t nOneWeek = 7*24*60*60;
                struct in_addr ip;
                memcpy(&ip, &pnSeed[i], sizeof(ip));
                CAddress addr(CService(ip, GetDefaultPort()));
                addr.nTime = GetTime()-GetRand(nOneWeek)-nOneWeek;
                vAdd.push_back(addr);
            }
            addrman.Add(vAdd, CNetAddr("127.0.0.1"));
        }

        //
        // Choose an address to connect to based on most recently seen
        //
        CAddress addrConnect;

        // Only connect out to one peer per network group (/16 for IPv4).
        // Do this here so we don't have to critsect vNodes inside mapAddresses critsect.
        int nOutbound = 0;
        set<vector<unsigned char> > setConnected;
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes) {
                if (!pnode->fInbound) {
                    setConnected.insert(pnode->addr.GetGroup());
                    nOutbound++;
                }
            }
        }

        int64_t nANow = GetAdjustedTime();

        int nTries = 0;
        while (true)
        {
            // use an nUnkBias between 10 (no outgoing connections) and 90 (8 outgoing connections)
            CAddress addr = addrman.Select(10 + min(nOutbound,8)*10);

            // if we selected an invalid address, restart
            if (!addr.IsValid() || setConnected.count(addr.GetGroup()) || IsLocal(addr))
                break;

            // If we didn't find an appropriate destination after trying 100 addresses fetched from addrman,
            // stop this loop, and let the outer loop run again (which sleeps, adds seed nodes, recalculates
            // already-connected network ranges, ...) before trying new addrman addresses.
            nTries++;
            if (nTries > 100)
                break;

            if (IsLimited(addr))
                continue;

            // only consider very recently tried nodes after 30 failed attempts
            if (nANow - addr.nLastTry < 600 && nTries < 30)
                continue;

            // do not allow non-default ports, unless after 50 invalid addresses selected already
            if (addr.GetPort() != GetDefaultPort() && nTries < 50)
                continue;

            addrConnect = addr;
            break;
        }

        if (addrConnect.IsValid())
            OpenNetworkConnection(addrConnect, &grant);
    }
}

void ThreadOpenAddedConnections(void* parg)
{
    // Make this thread recognisable as the connection opening thread
    RenameThread("vpncoin-opencon");

    try
    {
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]++;
        ThreadOpenAddedConnections2(parg);
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]--;
        PrintException(&e, "ThreadOpenAddedConnections()");
    } catch (...) {
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]--;
        PrintException(NULL, "ThreadOpenAddedConnections()");
    }
    printf("ThreadOpenAddedConnections exited\n");
}

void ThreadOpenAddedConnections2(void* parg)
{
	printf("ThreadOpenAddedConnections started\n");

    if (mapArgs.count("-addnode") == 0)
        return;

    if (HaveNameProxy()) {
        while(!fShutdown) {
            BOOST_FOREACH(string& strAddNode, mapMultiArgs["-addnode"]) {
                CAddress addr;
                CSemaphoreGrant grant(*semOutbound);
                OpenNetworkConnection(addr, &grant, strAddNode.c_str());
                MilliSleep(500);
            }
            vnThreadsRunning[THREAD_ADDEDCONNECTIONS]--;
            MilliSleep(120000); // Retry every 2 minutes
            vnThreadsRunning[THREAD_ADDEDCONNECTIONS]++;
        }
        return;
    }

    vector<vector<CService> > vservAddressesToAdd(0);
    BOOST_FOREACH(string& strAddNode, mapMultiArgs["-addnode"])
    {
        vector<CService> vservNode(0);
		
		/*char buf[512];
		memset(buf, 0, sizeof(buf)); 
		memcpy(buf, strAddNode.c_str(), strAddNode.length());
		char *pHost = &buf[0];
		unsigned int wPort = GetHostPort(pHost); */
		
        //if(Lookup(pHost, vservNode, wPort, fNameLookup, 0))	
		if(Lookup(strAddNode.c_str(), vservNode, GetDefaultPort(), fNameLookup, 0))
        {
            vservAddressesToAdd.push_back(vservNode);
            {
                LOCK(cs_setservAddNodeAddresses);
                BOOST_FOREACH(CService& serv, vservNode)
                    setservAddNodeAddresses.insert(serv);
            }
        }
    }
    while (true)
    {
        vector<vector<CService> > vservConnectAddresses = vservAddressesToAdd;
        // Attempt to connect to each IP for each addnode entry until at least one is successful per addnode entry
        // (keeping in mind that addnode entries can have many IPs if fNameLookup)
        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodes)
                for (vector<vector<CService> >::iterator it = vservConnectAddresses.begin(); it != vservConnectAddresses.end(); it++)
                    BOOST_FOREACH(CService& addrNode, *(it))
                        if (pnode->addr == addrNode)
                        {
                            it = vservConnectAddresses.erase(it);
                            it--;
                            break;
                        }
        }
        BOOST_FOREACH(vector<CService>& vserv, vservConnectAddresses)
        {
            CSemaphoreGrant grant(*semOutbound);
            OpenNetworkConnection(CAddress(*(vserv.begin())), &grant);
            MilliSleep(500);
            if (fShutdown)
                return;
        }
        if (fShutdown)
            return;
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]--;
        MilliSleep(120000); // Retry every 2 minutes
        vnThreadsRunning[THREAD_ADDEDCONNECTIONS]++;
        if (fShutdown)
            return;
    }
}

		
// if successful, this moves the passed grant to the constructed node
bool OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot)
{
    //
    // Initiate outbound network connection
    //
    if (fShutdown)
        return false;
    if (!strDest)
        if (IsLocal(addrConnect) ||
            FindNode((CNetAddr)addrConnect) || CNode::IsBanned(addrConnect) ||
            FindNode(addrConnect.ToStringIPPort().c_str()))
            return false;
    /*if (strDest && FindNode(strDest))
	{
        //if( fDebug ){ printf("Node [%s] exists\n", strDest); }
		return false;
	}*/

    vnThreadsRunning[THREAD_OPENCONNECTIONS]--;
	
	CNode* pnode = ConnectNode(addrConnect, strDest);
    vnThreadsRunning[THREAD_OPENCONNECTIONS]++;
    if (fShutdown)
        return false;
    if (!pnode)
        return false;
    if (grantOutbound)
        grantOutbound->MoveTo(pnode->grantOutbound);
    pnode->fNetworkNode = true;
    if (fOneShot)
        pnode->fOneShot = true;

    return true;
}








void ThreadMessageHandler(void* parg)
{
    // Make this thread recognisable as the message handling thread
    RenameThread("vpncoin-msghand");

    try
    {
        vnThreadsRunning[THREAD_MESSAGEHANDLER]++;
        ThreadMessageHandler2(parg);
        vnThreadsRunning[THREAD_MESSAGEHANDLER]--;
    }
    catch (std::exception& e) {
        vnThreadsRunning[THREAD_MESSAGEHANDLER]--;
        PrintException(&e, "ThreadMessageHandler()");
    } catch (...) {
        vnThreadsRunning[THREAD_MESSAGEHANDLER]--;
        PrintException(NULL, "ThreadMessageHandler()");
    }
    printf("ThreadMessageHandler exited\n");
}

//bool bBoostSyncBlockMode = false;
unsigned int getNeedSyncBlocks()
{
    unsigned int rzt = 0;
	unsigned int newNumBlocksOfPeers = GetNumBlocksOfPeers();
	//if( fDebug){ printf("getNeedSyncBlocks  nBestHeight = [%d] : [%d] \n", nBestHeight, newNumBlocksOfPeers); }
	if( newNumBlocksOfPeers > nBestHeight ){ rzt = newNumBlocksOfPeers - nBestHeight; }
	return rzt;
}

bool isBoostSyncBlockMode()
{
	bool rzt = false;
	/* int newNumBlocksOfPeers = GetNumBlocksOfPeers();
	if( fDebug){ printf("boostSyncBlockMode  nBestHeight = [%d] : [%d] \n", nBestHeight, newNumBlocksOfPeers); }
	if( (newNumBlocksOfPeers > nBestHeight) && ((newNumBlocksOfPeers - nBestHeight) > 100) ){ rzt = true; } */
	if( getNeedSyncBlocks() > 100 ){ rzt = true; }
	return rzt;
}

/*static deque<CBlock> vBlocks;
CCriticalSection cs_vBlocks;
int AddMyBlock(CBlock& pblock)
{
	LOCK(cs_vBlocks);
	try{
		vBlocks.push_back( pblock );
		return 1;
	}catch (std::exception& e) {
		string str = string( e.what() );
		printf("AddMyBlock except [%s]\n", str.c_str()); 
	}
	catch (...)
	{
		// someone threw something undecypherable
	}
	return 0;
}
void ImportBlock()
{
    LOCK(cs_vBlocks);
    if (vBlocks.empty())
        return;
    //ic = vBlocks.size();
	CBlock* b = &vBlocks.front();
	ProcessBlock(NULL, b);
    vBlocks.pop_front();
}*/

extern unsigned int bServiceMode;
void ThreadMessageHandler2(void* parg)
{
    printf("ThreadMessageHandler started\n");
    SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	int64_t nLastSyncBlockTime = 0;
    while (!fShutdown)
    {
		//vector<CNode*> vNodesSync;

		vector<CNode*> vNodesCopy;
        {
            LOCK(cs_vNodes);
            vNodesCopy = vNodes;
            BOOST_FOREACH(CNode* pnode, vNodesCopy)
			{
                pnode->AddRef();
				
				/*if( bNeedFastSync )
				{
				    if( (pnode->nGotPongTime > 0) || (pnode->bSupportBoostSyncBlock > 0) )
				    {
						vNodesSync.push_back(pnode);
				    }
				}*/
			}
		
			/*if( bNeedFastSync )
			{
				int iNodeCount = vNodesSync.size();
				if( iNodeCount > 0 )
				{
					int jjj = iNeedBlocks / iNodeCount;
					if( jjj > 100 ){ jjj = 100; }
					int iHi = nBestHeight + jjj;
					BOOST_FOREACH(CNode* pnode, vNodesSync)
					{
						pnode->PushMessage("getblockv2", iHi, jjj);
						if( fDebug ){ printf("[%d] Send get block v2 to [%s] [%d :: %d] \n", iNodeCount, pnode->addr.ToString().c_str(), iHi, jjj); }
						iHi = iHi + jjj;
					}
					vNodesSync.clear();
					//vector<CNode*>(vNodesSync).swap(vNodesSync);
					nLastSyncBlockTime = GetTime();
				}
			}*/
		}
		
	
        // Poll the connected nodes for messages
        CNode* pnodeTrickle = NULL;
        if (!vNodesCopy.empty())
            pnodeTrickle = vNodesCopy[GetRand(vNodesCopy.size())];
        BOOST_FOREACH(CNode* pnode, vNodesCopy)
        {
            if (pnode->fDisconnect)
                continue;

            if( pnode->vBitNet.v_IpAddr == 0 )
			{
				struct in_addr ip;
				pnode->addr.GetInAddr(&ip);
				pnode->vBitNet.v_IpAddr = ip.s_addr;
				pnode->vBitNet.v_NodePort = pnode->addr.GetPort();
				//printf("ThreadMessageHandler2: ip=%X\n", pnode->sAddr);
			}
			
            // Receive messages
            {
                TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
                if (lockRecv)
                    if (!ProcessMessages(pnode))
					{
                        //if( fDebug ){ printf("ProcessMessages faile, [%s] CloseSocketDisconnect\n", pnode->addrName.c_str()); }
						pnode->CloseSocketDisconnect();
					}
            }
            if (fShutdown)
                return;

            // Send messages
            {
                /*
				bool bNeedFastSync = false;
				if( bServiceMode == 0 )
				{
					unsigned int iNeedBlocks = getNeedSyncBlocks();
					int64_t nCurTm = GetTime();
					if( (nCurTm > nLastSyncBlockTime) && ((nCurTm - nLastSyncBlockTime) > 15) )
					{
						if( iNeedBlocks > 100 )
						{
							int iIncremental = 100;
							BOOST_FOREACH(CNode* pnode, vNodesCopy)
							{
								if( (pnode->nGotPongTime > 0) || (pnode->bSupportBoostSyncBlock > 0) )
								{
									pnode->iSyncBlockIncremental = nBestHeight + iIncremental;
									iIncremental = iIncremental + 100;
								}
							}
						}
					}
				} */

				TRY_LOCK(pnode->cs_vSend, lockSend);
                if (lockSend){
                    //SocketSendData(pnode);	// 2014.11.12 add
					SendMessages(pnode, pnode == pnodeTrickle);
				}
            }
            if (fShutdown)
                return;
        }

        {
            LOCK(cs_vNodes);
            BOOST_FOREACH(CNode* pnode, vNodesCopy)
			{
                //if( pnode->vBitNet.v_ListItem > 0 ){ SynNodeToBitNetGui(pnode, 0, 0, NULL); }	//-- 2014.12.06 add
				pnode->Release();
			}
        }

        // Wait and allow messages to bunch up.
        // Reduce vnThreadsRunning so StopNode has permission to exit while
        // we're sleeping, but we must always check fShutdown after doing this.
        vnThreadsRunning[THREAD_MESSAGEHANDLER]--;
        MilliSleep(100);
        if (fRequestShutdown)
            StartShutdown();
        vnThreadsRunning[THREAD_MESSAGEHANDLER]++;
        if (fShutdown)
            return;
    }
}	//ThreadMessageHandler2






bool BindListenPort(const CService &addrBind, string& strError)
{
    strError = "";
    int nOne = 1;

#ifdef WIN32
    // Initialize Windows Sockets
    WSADATA wsadata;
    int ret = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (ret != NO_ERROR)
    {
        strError = strprintf("Error: TCP/IP socket library failed to start (WSAStartup returned error %d)", ret);
        printf("%s\n", strError.c_str());
        return false;
    }
#endif

    // Create socket for listening for incoming connections
    struct sockaddr_storage sockaddr;
    socklen_t len = sizeof(sockaddr);
    if (!addrBind.GetSockAddr((struct sockaddr*)&sockaddr, &len))
    {
        strError = strprintf("Error: bind address family for %s not supported", addrBind.ToString().c_str());
        printf("%s\n", strError.c_str());
        return false;
    }

    SOCKET hListenSocket = socket(((struct sockaddr*)&sockaddr)->sa_family, SOCK_STREAM, IPPROTO_TCP);
    if (hListenSocket == INVALID_SOCKET)
    {
        strError = strprintf("Error: Couldn't open socket for incoming connections (socket returned error %d)", WSAGetLastError());
        printf("%s\n", strError.c_str());
        return false;
    }

#ifdef SO_NOSIGPIPE
    // Different way of disabling SIGPIPE on BSD
    setsockopt(hListenSocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&nOne, sizeof(int));
#endif

#ifndef WIN32
    // Allow binding if the port is still in TIME_WAIT state after
    // the program was closed and restarted.  Not an issue on windows.
    setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&nOne, sizeof(int));
#endif


#ifdef WIN32
    // Set to non-blocking, incoming connections will also inherit this
    if (ioctlsocket(hListenSocket, FIONBIO, (u_long*)&nOne) == SOCKET_ERROR)
#else
    if (fcntl(hListenSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
#endif
    {
        strError = strprintf("Error: Couldn't set properties on socket for incoming connections (error %d)", WSAGetLastError());
        printf("%s\n", strError.c_str());
        return false;
    }

    // some systems don't have IPV6_V6ONLY but are always v6only; others do have the option
    // and enable it by default or not. Try to enable it, if possible.
    if (addrBind.IsIPv6()) {
#ifdef IPV6_V6ONLY
#ifdef WIN32
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&nOne, sizeof(int));
#else
        setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&nOne, sizeof(int));
#endif
#endif
#ifdef WIN32
        int nProtLevel = 10 /* PROTECTION_LEVEL_UNRESTRICTED */;
        int nParameterId = 23 /* IPV6_PROTECTION_LEVEl */;
        // this call is allowed to fail
        setsockopt(hListenSocket, IPPROTO_IPV6, nParameterId, (const char*)&nProtLevel, sizeof(int));
#endif
    }

    if (::bind(hListenSocket, (struct sockaddr*)&sockaddr, len) == SOCKET_ERROR)
    {
        int nErr = WSAGetLastError();
        if (nErr == WSAEADDRINUSE)
            strError = strprintf(_("Unable to bind to %s on this computer. VpnCoin is probably already running."), addrBind.ToString().c_str());
        else
            strError = strprintf(_("Unable to bind to %s on this computer (bind returned error %d, %s)"), addrBind.ToString().c_str(), nErr, strerror(nErr));
        printf("%s\n", strError.c_str());
        return false;
    }
    printf("Bound to %s\n", addrBind.ToString().c_str());

    // Listen for incoming connections
    if (listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        strError = strprintf("Error: Listening for incoming connections failed (listen returned error %d)", WSAGetLastError());
        printf("%s\n", strError.c_str());
        return false;
    }

    vhListenSocket.push_back(hListenSocket);

    if (addrBind.IsRoutable() && fDiscover)
        AddLocal(addrBind, LOCAL_BIND);

    return true;
}

int fGetMyExternalIPByBitNet = 0;
void static Discover()
{
    if (!fDiscover)
        return;

#ifdef WIN32
    // Get local host IP
    char pszHostName[1000] = "";
    if (gethostname(pszHostName, sizeof(pszHostName)) != SOCKET_ERROR)
    {
        vector<CNetAddr> vaddr;
        if (LookupHost(pszHostName, vaddr))
        {
            BOOST_FOREACH (const CNetAddr &addr, vaddr)
            {
                AddLocal(addr, LOCAL_IF);
            }
        }
    }
#else
    // Get local host ip
    struct ifaddrs* myaddrs;
    if (getifaddrs(&myaddrs) == 0)
    {
        for (struct ifaddrs* ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL) continue;
            if ((ifa->ifa_flags & IFF_UP) == 0) continue;
            if (strcmp(ifa->ifa_name, "lo") == 0) continue;
            if (strcmp(ifa->ifa_name, "lo0") == 0) continue;
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                struct sockaddr_in* s4 = (struct sockaddr_in*)(ifa->ifa_addr);
                CNetAddr addr(s4->sin_addr);
                if (AddLocal(addr, LOCAL_IF))
                    printf("IPv4 %s: %s\n", ifa->ifa_name, addr.ToString().c_str());
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            {
                struct sockaddr_in6* s6 = (struct sockaddr_in6*)(ifa->ifa_addr);
                CNetAddr addr(s6->sin6_addr);
                if (AddLocal(addr, LOCAL_IF))
                    printf("IPv6 %s: %s\n", ifa->ifa_name, addr.ToString().c_str());
            }
        }
        freeifaddrs(myaddrs);
    }
#endif

    // Don't use external IPv4 discovery, when -onlynet="IPv6"
    if ( (!IsLimited(NET_IPV4)) && (fGetMyExternalIPByBitNet == 0) )
        NewThread(ThreadGetMyExternalIP, NULL);
}

void StartNode(void* parg)
{
    // Make this thread recognisable as the startup thread
    RenameThread("vpncoin-start");
	//fNetDbg = GetArg("-netdbg", 0);
	//fFixedChangeAddress = GetArg("-fixedchangeaddress", 1);
	fMaxOutbound = GetArg("-maxoutconnections", 5000);

    if (semOutbound == NULL) {
        // initialize semaphore
        int nMaxOutbound = fMaxOutbound;  //max(MAX_OUTBOUND_CONNECTIONS, (int)GetArg("-maxconnections", 5000));  // min
        semOutbound = new CSemaphore(nMaxOutbound);
    }

    if (pnodeLocalHost == NULL)
        pnodeLocalHost = new CNode(INVALID_SOCKET, CAddress(CService("127.0.0.1", 0), nLocalServices));

    Discover();

    //
    // Start threads
    //

    if (!GetBoolArg("-dnsseed", true))
        printf("DNS seeding disabled\n");
    else
        if (!NewThread(ThreadDNSAddressSeed, NULL))
            printf("Error: NewThread(ThreadDNSAddressSeed) failed\n");

    // Map ports with UPnP
    if (fUseUPnP)
        MapPort();

    // Get addresses from IRC and advertise ours
    //if (!NewThread(ThreadIRCSeed, NULL))
    //    printf("Error: NewThread(ThreadIRCSeed) failed\n");

    // Send and receive from sockets, accept connections
    if (!NewThread(ThreadSocketHandler, NULL))
        printf("Error: NewThread(ThreadSocketHandler) failed\n");
#ifdef QT_GUI
    if( GetBoolArg("-getprice", true) ){ NewThread(ThreadGetVpnPrice, NULL); }  // 2016.02.03 add
    if( GetBoolArg("-loginbitchain", true) ){ NewThread(ThreadWalletLoginDns, NULL); }  // 2016.04.02 add
#endif

    if( fNewSocketThread > 0 )
    {
	if (!NewThread(SocketServiceThread, NULL))
	printf("Error: NewThread(SocketServiceThread) failed\n");
    }

    // Initiate outbound connections from -addnode
    if (!NewThread(ThreadOpenAddedConnections, NULL))
        printf("Error: NewThread(ThreadOpenAddedConnections) failed\n");

    // Initiate outbound connections
    if (!NewThread(ThreadOpenConnections, NULL))
        printf("Error: NewThread(ThreadOpenConnections) failed\n");
		
    // Initiate outbound connections
    if (!NewThread(ThreadOpenOneShotNode, NULL))	//--2014.11.29 add
        printf("Error: NewThread(ThreadOpenOneShotNode) failed\n");		

    // Process messages
    if (!NewThread(ThreadMessageHandler, NULL))
        printf("Error: NewThread(ThreadMessageHandler) failed\n");

    // Dump network addresses
    if (!NewThread(ThreadDumpAddress, NULL))
        printf("Error; NewThread(ThreadDumpAddress) failed\n");

    // Mine proof-of-stake blocks in the background
#ifdef QT_GUI
    if (!GetBoolArg("-staking", true))
#else
    if (!GetBoolArg("-staking", false))
#endif
        printf("Staking disabled\n");
    else
        if (!NewThread(ThreadStakeMiner, pwalletMain))
            printf("Error: NewThread(ThreadStakeMiner) failed\n");
}

void RelayShutDown()
{
  try{
	vector<CNode*> vNodesCopy;
    LOCK(cs_vNodes);
    vNodesCopy = vNodes;
	BOOST_FOREACH(CNode* pnode, vNodesCopy)
	{
		pnode->PushMessage("ShutDown"); 
	}
  }
  catch (std::exception& e) {
        PrintException(&e, "RelayShutDown()");
  } catch (...) {
        PrintException(NULL, "RelayShutDown()");
  }	
}

bool StopNode()
{
    printf("StopNode()\n");
    fShutdown = true;
	//RelayShutDown();	// BitNet Add, 2015.02.01: will stop at here, so disabled
//if( fDebug ){ printf("StopNode() -> RelayShutDown()\n"); }
    nTransactionsUpdated++;
    int64_t nStart = GetTime();
	int iOutConn = fMaxOutbound;
    if (semOutbound)
        for (int i=0; i<iOutConn; i++)  //MAX_OUTBOUND_CONNECTIONS
            semOutbound->post();
    do
    {
        int nThreadsRunning = 0;
        for (int n = 0; n < THREAD_MAX; n++)
            nThreadsRunning += vnThreadsRunning[n];
        if (nThreadsRunning == 0)
            break;
        if (GetTime() - nStart > 20)
            break;
        MilliSleep(20);
    } while(true);
if( fDebug ){ printf("StopNode() -> while loop()\n"); }
    if (vnThreadsRunning[THREAD_SOCKETHANDLER] > 0) printf("ThreadSocketHandler still running\n");
    if (vnThreadsRunning[THREAD_OPENCONNECTIONS] > 0) printf("ThreadOpenConnections still running\n");
    if (vnThreadsRunning[THREAD_MESSAGEHANDLER] > 0) printf("ThreadMessageHandler still running\n");
    if (vnThreadsRunning[THREAD_RPCLISTENER] > 0) printf("ThreadRPCListener still running\n");
    if (vnThreadsRunning[THREAD_RPCHANDLER] > 0) printf("ThreadsRPCServer still running\n");
    if (vnThreadsRunning[THREAD_OPENOneShotNode] > 0) printf("ThreadOpenOneShotNode still running\n");
#ifdef USE_UPNP
    if (vnThreadsRunning[THREAD_UPNP] > 0) printf("ThreadMapPort still running\n");
#endif
    if (vnThreadsRunning[THREAD_DNSSEED] > 0) printf("ThreadDNSAddressSeed still running\n");
    if (vnThreadsRunning[THREAD_ADDEDCONNECTIONS] > 0) printf("ThreadOpenAddedConnections still running\n");
    if (vnThreadsRunning[THREAD_DUMPADDRESS] > 0) printf("ThreadDumpAddresses still running\n");
    if (vnThreadsRunning[THREAD_STAKE_MINER] > 0) printf("ThreadStakeMiner still running\n");
    nStart = GetTime();
	while (vnThreadsRunning[THREAD_MESSAGEHANDLER] > 0 || vnThreadsRunning[THREAD_RPCHANDLER] > 0){
        MilliSleep(20);
		//if( fDebug ){ printf("vnThreads still running\n"); }
		if (GetTime() - nStart > 20){ break; }
	}
    MilliSleep(50);
if( fDebug ){ printf("StopNode() -> DumpAddresses()\n"); }
    DumpAddresses();
if( fDebug ){ printf("<- StopNode()\n"); }
    return true;
}

class CNetCleanup
{
public:
    CNetCleanup()
    {
    }
    ~CNetCleanup()
    {
        // Close sockets
        BOOST_FOREACH(CNode* pnode, vNodes)
            if (pnode->hSocket != INVALID_SOCKET)
                closesocket(pnode->hSocket);
        BOOST_FOREACH(SOCKET hListenSocket, vhListenSocket)
            if (hListenSocket != INVALID_SOCKET)
                if (closesocket(hListenSocket) == SOCKET_ERROR)
                    printf("closesocket(hListenSocket) failed with error %d\n", WSAGetLastError());

#ifdef WIN32
        // Shutdown Windows Sockets
        WSACleanup();
#endif
    }
}
instance_of_cnetcleanup;


void RelayTransaction(const CTransaction& tx, const uint256& hash)
{
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss.reserve(10000);
    ss << tx;
    RelayTransaction(tx, hash, ss);
}

void RelayTransaction(const CTransaction& tx, const uint256& hash, const CDataStream& ss)
{
    CInv inv(MSG_TX, hash);
    {
        LOCK(cs_mapRelay);
        // Expire old relay messages
        while (!vRelayExpiration.empty() && vRelayExpiration.front().first < GetTime())
        {
            mapRelay.erase(vRelayExpiration.front().second);
            vRelayExpiration.pop_front();
        }

        // Save original serialized message so newer versions are preserved
        mapRelay.insert(std::make_pair(inv, ss));
        vRelayExpiration.push_back(std::make_pair(GetTime() + 15 * 60, inv));
    }

    RelayInventory(inv);
}
