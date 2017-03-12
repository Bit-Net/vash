#ifndef BITNET_H
#define BITNET_H
#ifdef USE_BITNET

#ifdef WIN32
#include <windows.h>
#endif
#include <stdint.h>
#include <string>
#include "net.h"
//#include "ui_interface.h"
//#include "init.h"
//#include "bitcoingui.h"
//#include "bitcoinrpc.h"
#include "notificator.h"
#include "sendcoinsentry.h"

using namespace std;
using namespace boost;

#ifdef WIN32
typedef DWORD (WINAPI *lpSetProcessDpiAwareness) (DWORD);
typedef DWORD (WINAPI *lpSyncTransaction) (PCHAR pTxid, PCHAR pTxMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime, int64_t txNow);
typedef DWORD (WINAPI *lpOpenBitNetCenter) (DWORD, DWORD, const char* wAddr);
typedef DWORD (WINAPI *lpStart_Vpn_Client) (DWORD);
typedef DWORD (WINAPI *lpStart_Vpn_Server) (DWORD);
typedef DWORD (WINAPI *lpReadIni) (char* pSec, char* pv, char* pr);
typedef DWORD (WINAPI *lpSendBitNetWebChatMsg) (const char* pMsg, int WebId, int EventId);
typedef DWORD (WINAPI *lpDropEvent) (const char* pMsg);
typedef DWORD (WINAPI *lpInitVpnDll) (PVOID pGetBuildinVpnFunc, HWND x, HWND vParentHwnd, int64_t* y, const char* DefWAddr, int wPort, int bitNetVer, int bitNetId, HWND vWebViewHwnd);
typedef DWORD (WINAPI *lpInitVpnParam) (PDWORD pUseChat, PDWORD pChat_Act, char* pChatStr, PDWORD pCurNode, PDWORD pShowInOtherList, 
			   PDWORD pIsVpnServer, PDWORD pVpnServicePort, PDWORD pVpnServiceCtrlPort, PDWORD pVpnServiceTryMinute, 
			   PINT64 pVpnServiceFee, char* pVpnWalletAddress, PDWORD pVpnNeedSendTo, PINT64 pSendCoins, char* pSendToVpnWalletAddress);

typedef DWORD (WINAPI *lpSyncNode) (DWORD dOption, DWORD dRecvSize, CNode* node, Bitnet_node_struct* vNode);
/*typedef DWORD (WINAPI *lpSyncNode) (DWORD bAdd, CNode* node, DWORD dRecvSize, DWORD dNodeIp, DWORD dNodePort, DWORD isServer, DWORD dCtrlPort, DWORD dPort, DWORD dTryMinute, DWORD dCanTalk, 
									int64_t i6Fee, int64_t i6Fsz, const char* defWAddr, const char* wAddr, const char* NickName, const char* sTalkMsg, const char* pRmtFile, 
									char* pMemAndCpu, DWORD ddSerCoinConfirms, DWORD dTotalVpnConnects);
*/

extern Notificator *vpnNotificator;
extern SendCoinsEntry *vpnSendCoinsEntry;
extern int dStartVpnClient;
extern std::string sVpnIni;
extern std::string sVpnDll;
extern HWND gCoinGuiHwnd;
extern HWND gBitNetPageHwnd;	// 2014.11.05 add
extern HWND gTrayIconHwnd;
extern std::string sSysLang;
extern DWORD d_Vpn_LanID;
//extern DWORD d_iVersion;

//-- VPN service var
extern DWORD bShowInOtherList;
extern DWORD bShowWAddrInOtherNodes;
extern DWORD dStartVpnServer;
extern DWORD isVpnServer;
extern DWORD iOpenSocketProxy;
extern WORD  iVpnServicePort;
extern WORD  iVpnServiceCtrlPort;
extern DWORD d_Open_P2P_Proxy;
extern WORD  d_P2P_proxy_port;
extern DWORD d_P2P_Proxy_Port_mapped;
extern int64_t iVpnServiceFee;
extern DWORD iVpnServiceTryMinute;
extern DWORD iVpnSerCoinConfirms;
extern DWORD iTotalVpnConnects;
extern std::string sDefWalletAddress;
extern std::string sVpnWalletAddress;
extern std::string sVpnNicknamePack;
//extern char* pVpnWalletAddress;
extern std::string sVpnMemAndCpuInfo;
extern char b_OnlineRewardVpnTime;
extern WORD w_OnlineXminReward_1Min;

extern DWORD iVpnSendToSpecifiedFlag;
extern DWORD iVpnSendToSpecifiedType;
extern int64_t iVpnSendToSpecifiedCoins;
extern std::string iVpnSendToSpecifiedAddressTo;
extern std::string iVpnSendToSpecifiedAddressFrom; 

//-- VPN chat
extern DWORD dUseChat;
//extern DWORD g_chat_thread_start;
//extern DWORD g_chat_act;	// hava a msg need send to
//extern char* g_chat_str;
//extern CNode* g_cur_chat_node;

extern HINSTANCE hBitNetDll;
//HMODULE hBitNetDll = 0;
extern lpOpenBitNetCenter OpenBitNetCenter;
extern lpStart_Vpn_Client pStart_Vpn_Client;
extern lpStart_Vpn_Server pStart_Vpn_Server;

DWORD GetHostPort(char* sHost);	//--2014.11.13 add
void GetDefaultWalletAddress();
void LoadIniCfg( DWORD bStart, DWORD dRelay );
HMODULE LoadBitNetDll();
int InitBitNetDll();
FARPROC LoadBitNetFunc(LPCSTR pFunName);
DWORD ShowBitNetCenterGui(DWORD bShow, DWORD bEndApp);
int Do_Start_Vpn_Client(int bStart);
int Do_Start_Vpn_Server(int bStart);
int start_vpn_client();
int stop_vpn_client();
int start_vpn_server(int i);
int stop_vpn_server();
int RelayBitNetInfo();
void Start_VPN_chat_Thread();
void ThreadVPN_Chat2(void* parg);
void ThreadVPN_Chat(void* parg);
DWORD WINAPI SendVpnChatMsg(CNode* node, PCHAR msg);
DWORD WINAPI GetBuildinVpnFunc(int iFunc);
DWORD SynNodeToBitNetGui(CNode* node, DWORD bAdd, DWORD dRecvSize, const char* pTalk);
DWORD WINAPI GetBuildinVpnFunc(int iFunc);
std::string getAddressesbyaccount(const std::string strAccount);
DWORD SyncTransactionToGui(std::string& txId, std::string& txMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime);
#endif
#endif // BITNET_H
#endif