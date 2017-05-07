// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "txdb.h"
#include "walletdb.h"
#include "bitcoinrpc.h"
#include "net.h"
#include "init.h"
#include "util.h"
#include "ui_interface.h"
#include "checkpoints.h"
#include "zerocoin/ZeroTest.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <openssl/crypto.h>

#ifdef USE_SQLITE
#include <sqlite3.h>
#endif

#ifdef QT_GUI
#include <QSettings>
#endif

#ifndef WIN32
#include <signal.h>
#endif


using namespace std;
using namespace boost;

CWallet* pwalletMain;
CClientUIInterface uiInterface;
bool fConfChange;
unsigned int nNodeLifespan;
unsigned int nDerivationMethodIndex;
unsigned int nMinerSleep;
bool fUseFastIndex;
enum Checkpoints::CPMode CheckpointsMode;
extern int64_t S_To_64(const char *s);
extern void initZkpForVer1703();

#ifdef WIN32
extern void LoadIniCfg( DWORD bStart, DWORD dRelay );
#endif
extern string GetAddressesByAccount(const string strAccount, bool fCreateNewIfNotExists = false);
extern string getAccountAddress(const string strAccount, bool bForceNew=false);
extern int fNetDbg;
extern int fBindPort920;
extern int fFixedChangeAddress;
extern int fNewSocketThread;
extern string s_BlockChain_AdBonus_Dir;


string s_BlockChain_Dir = "";
string s_fastSyncBlock_ini = "";
string s_bestHashForFastSync = "";
string sBitChainIdentAddress = "";
string sBitChainKeyAddress = "";
const string sWallet_restore = "wallet.restore";
const string sBitChain_ident = "BitChain-ident";
const string sBitChain_key = "BitChain-key";
uint256 u256bestHashForFastSync = 0;
uint256 preHashForFastSync = 0;
unsigned int bFastsyncblockMode = 0;
unsigned int bFastsyncBestHei = 0;
uint64_t nFastStakeModifier = 0;
unsigned int nFastStakeModifierChecksum = 0;
unsigned int nFastPreStakeModifierChecksum = 0;
int iFastsyncblockModeArg = 0;
int iFastSyncBlockHeiOk = 0;
int dw_Fast_Sync_Block_Active_Height = 400000;
int dw_zip_block = 0;
int dw_zip_limit_size = 0;
int dw_zip_txdb = 0;
extern void initBitChain();

#ifdef USE_SQLITE
   sqlite3 *dbAllAddress = NULL;
#endif
//////////////////////////////////////////////////////////////////////////////
//
// Shutdown
//

void ExitTimeout(void* parg)
{
#ifdef WIN32
    MilliSleep(5000);
    ExitProcess(0);
#endif
}

void StartShutdown()
{
#ifdef QT_GUI
    // ensure we leave the Qt main loop for a clean GUI exit (Shutdown() is called in bitcoin.cpp afterwards)
    uiInterface.QueueShutdown();
#else
    // Without UI, Shutdown() can simply be started in a new thread
    NewThread(Shutdown, NULL);
#endif
}

void Shutdown(void* parg)
{
    static CCriticalSection cs_Shutdown;
    static bool fTaken;

    // Make this thread recognisable as the shutdown thread
    RenameThread("vpncoin-shutoff");

    bool fFirstThread = false;
    {
        TRY_LOCK(cs_Shutdown, lockShutdown);
        if (lockShutdown)
        {
            fFirstThread = !fTaken;
            fTaken = true;
        }
    }
    static bool fExit;
    if (fFirstThread)
    {
        fShutdown = true;
        nTransactionsUpdated++;
//        CTxDB().Close();
        bitdb.Flush(false);
        StopNode();
        bitdb.Flush(true);
        boost::filesystem::remove(GetPidFile());
        UnregisterWallet(pwalletMain);
        delete pwalletMain;
#ifdef USE_SQLITE
        if( dbAllAddress ) sqlite3_close(dbAllAddress);
#endif
        NewThread(ExitTimeout, NULL);
        MilliSleep(50);
        printf("VpnCoin exited\n\n");
        fExit = true;
#ifndef QT_GUI
        // ensure non-UI client gets exited here, but let Bitcoin-Qt reach 'return 0;' in bitcoin.cpp
        exit(0);
#endif
    }
    else
    {
        while (!fExit)
            MilliSleep(500);
        MilliSleep(100);
        ExitThread(0);
    }
}

void HandleSIGTERM(int)
{
    fRequestShutdown = true;
}

void HandleSIGHUP(int)
{
    fReopenDebugLog = true;
}





//////////////////////////////////////////////////////////////////////////////
//
// Start
//
#if !defined(QT_GUI)
bool AppInit(int argc, char* argv[])
{
    bool fRet = false;
    try
    {
        //
        // Parameters
        //
        // If Qt is used, parameters/bitcoin.conf are parsed in qt/bitcoin.cpp's main()
        ParseParameters(argc, argv);
        if (!boost::filesystem::is_directory(GetDataDir(false)))
        {
            fprintf(stderr, "Error: Specified directory does not exist\n");
            Shutdown(NULL);
        }
        ReadConfigFile(mapArgs, mapMultiArgs);

        if (mapArgs.count("-?") || mapArgs.count("--help"))
        {
            // First part of help message is specific to bitcoind / RPC client
            std::string strUsage = _("VpnCoin version") + " " + FormatFullVersion() + "\n\n" +
                _("Usage:") + "\n" +
                  "  vpncoind [options]                     " + "\n" +
                  "  vpncoind [options] <command> [params]  " + _("Send command to -server or vpncoind") + "\n" +
                  "  vpncoind [options] help                " + _("List commands") + "\n" +
                  "  vpncoind [options] help <command>      " + _("Get help for a command") + "\n";

            strUsage += "\n" + HelpMessage();

            fprintf(stdout, "%s", strUsage.c_str());
            return false;
        }

        // Command-line RPC
        for (int i = 1; i < argc; i++)
            if (!IsSwitchChar(argv[i][0]) && !boost::algorithm::istarts_with(argv[i], "vpncoin:"))
                fCommandLine = true;

        if (fCommandLine)
        {
            int ret = CommandLineRPC(argc, argv);
            exit(ret);
        }

        fRet = AppInit2();
    }
    catch (std::exception& e) {
        PrintException(&e, "AppInit()");
    } catch (...) {
        PrintException(NULL, "AppInit()");
    }
    if (!fRet)
        Shutdown(NULL);
    return fRet;
}

extern void noui_connect();
int main(int argc, char* argv[])
{
    bool fRet = false;

    // Connect bitcoind signal handlers
    noui_connect();

    fRet = AppInit(argc, argv);

    if (fRet && fDaemon)
        return 0;

    return 1;
}
#endif

bool static InitError(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, _("VpnCoin"), CClientUIInterface::OK | CClientUIInterface::MODAL);
    return false;
}

bool static InitWarning(const std::string &str)
{
    uiInterface.ThreadSafeMessageBox(str, _("VpnCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
    return true;
}


bool static Bind(const CService &addr, bool fError = true) {
    if (IsLimited(addr))
        return false;
    std::string strError;
    if (!BindListenPort(addr, strError)) {
        if (fError)
            return InitError(strError);
        return false;
    }
    return true;
}

// Core-specific options shared between UI and daemon
std::string HelpMessage()
{
    string strUsage = _("Options:") + "\n" +
        "  -?                     " + _("This help message") + "\n" +
        "  -conf=<file>           " + _("Specify configuration file (default: vpncoin.conf)") + "\n" +
        "  -pid=<file>            " + _("Specify pid file (default: vpncoind.pid)") + "\n" +
        "  -datadir=<dir>         " + _("Specify data directory") + "\n" +
        "  -wallet=<dir>          " + _("Specify wallet file (within data directory)") + "\n" +
        "  -dbcache=<n>           " + _("Set database cache size in megabytes (default: 25)") + "\n" +
        "  -dblogsize=<n>         " + _("Set database disk log size in megabytes (default: 100)") + "\n" +
        "  -timeout=<n>           " + _("Specify connection timeout in milliseconds (default: 5000)") + "\n" +
        "  -proxy=<ip:port>       " + _("Connect through socks proxy") + "\n" +
        "  -socks=<n>             " + _("Select the version of socks proxy to use (4-5, default: 5)") + "\n" +
        "  -tor=<ip:port>         " + _("Use proxy to reach tor hidden services (default: same as -proxy)") + "\n"
        "  -dns                   " + _("Allow DNS lookups for -addnode, -seednode and -connect") + "\n" +
        "  -port=<port>           " + _("Listen for connections on <port> (default: 1920 or testnet: 2920)") + "\n" +
        "  -maxconnections=<n>    " + _("Maintain at most <n> connections to peers (default: 125)") + "\n" +
        "  -addnode=<ip>          " + _("Add a node to connect to and attempt to keep the connection open") + "\n" +
        "  -connect=<ip>          " + _("Connect only to the specified node(s)") + "\n" +
        "  -seednode=<ip>         " + _("Connect to a node to retrieve peer addresses, and disconnect") + "\n" +
        "  -externalip=<ip>       " + _("Specify your own public address") + "\n" +
        "  -onlynet=<net>         " + _("Only connect to nodes in network <net> (IPv4, IPv6 or Tor)") + "\n" +
        "  -discover              " + _("Discover own IP address (default: 1 when listening and no -externalip)") + "\n" +
        "  -irc                   " + _("Find peers using internet relay chat (default: 0)") + "\n" +
        "  -listen                " + _("Accept connections from outside (default: 1 if no -proxy or -connect)") + "\n" +
        "  -bind=<addr>           " + _("Bind to given address. Use [host]:port notation for IPv6") + "\n" +
        "  -dnsseed               " + _("Query for peer addresses via DNS lookup, if low on addresses (default: 1 unless -connect)") + "\n" +
        "  -forcednsseed          " + _("Always query for peer addresses via DNS lookup (default: 0)") + "\n" +
        "  -synctime              " + _("Sync time with other nodes. Disable if time on your system is precise e.g. syncing with NTP (default: 1)") + "\n" +
        "  -cppolicy              " + _("Sync checkpoints policy (default: strict)") + "\n" +
        "  -banscore=<n>          " + _("Threshold for disconnecting misbehaving peers (default: 100)") + "\n" +
        "  -bantime=<n>           " + _("Number of seconds to keep misbehaving peers from reconnecting (default: 86400)") + "\n" +
        "  -maxreceivebuffer=<n>  " + _("Maximum per-connection receive buffer, <n>*1000 bytes (default: 5000)") + "\n" +
        "  -maxsendbuffer=<n>     " + _("Maximum per-connection send buffer, <n>*1000 bytes (default: 1000)") + "\n" +
#ifdef USE_UPNP
#if USE_UPNP
        "  -upnp                  " + _("Use UPnP to map the listening port (default: 1 when listening)") + "\n" +
#else
        "  -upnp                  " + _("Use UPnP to map the listening port (default: 0)") + "\n" +
#endif
#endif
        "  -paytxfee=<amt>        " + _("Fee per KB to add to transactions you send") + "\n" +
        "  -mininput=<amt>        " + _("When creating transactions, ignore inputs with value less than this (default: 0.01)") + "\n" +
#ifdef QT_GUI
        "  -server                " + _("Accept command line and JSON-RPC commands") + "\n" +
#endif
#if !defined(WIN32) && !defined(QT_GUI)
        "  -daemon                " + _("Run in the background as a daemon and accept commands") + "\n" +
#endif
        "  -testnet               " + _("Use the test network") + "\n" +
        "  -debug                 " + _("Output extra debugging information. Implies all other -debug* options") + "\n" +
        "  -debugnet              " + _("Output extra network debugging information") + "\n" +
        "  -logtimestamps         " + _("Prepend debug output with timestamp") + "\n" +
        "  -shrinkdebugfile       " + _("Shrink debug.log file on client startup (default: 1 when no -debug)") + "\n" +
        "  -printtoconsole        " + _("Send trace/debug info to console instead of debug.log file") + "\n" +
#ifdef WIN32
        "  -printtodebugger       " + _("Send trace/debug info to debugger") + "\n" +
#endif
        "  -rpcuser=<user>        " + _("Username for JSON-RPC connections") + "\n" +
        "  -rpcpassword=<pw>      " + _("Password for JSON-RPC connections") + "\n" +
        "  -rpcport=<port>        " + _("Listen for JSON-RPC connections on <port> (default: 1921 or testnet: 2921)") + "\n" +
        "  -rpcallowip=<ip>       " + _("Allow JSON-RPC connections from specified IP address") + "\n" +
        "  -rpcconnect=<ip>       " + _("Send commands to node running on <ip> (default: 127.0.0.1)") + "\n" +
        "  -blocknotify=<cmd>     " + _("Execute command when the best block changes (%s in cmd is replaced by block hash)") + "\n" +
        "  -walletnotify=<cmd>    " + _("Execute command when a wallet transaction changes (%s in cmd is replaced by TxID)") + "\n" +
        "  -confchange            " + _("Require a confirmations for change (default: 0)") + "\n" +
        "  -alertnotify=<cmd>     " + _("Execute command when a relevant alert is received (%s in cmd is replaced by message)") + "\n" +
        "  -upgradewallet         " + _("Upgrade wallet to latest format") + "\n" +
        "  -keypool=<n>           " + _("Set key pool size to <n> (default: 100)") + "\n" +
        "  -rescan                " + _("Rescan the block chain for missing wallet transactions") + "\n" +
        "  -salvagewallet         " + _("Attempt to recover private keys from a corrupt wallet.dat") + "\n" +
        "  -fixedchangeaddress=<n>        " + _("Use wallet default key as Change Address (default: 1)") + "\n" +
        "  -fastsyncblockmode=<n>       " + _("Fast sync block mode (default: 1)") + "\n" +
        "  -checkblocks=<n>       " + _("How many blocks to check at startup (default: 2500, 0 = all)") + "\n" +
        "  -checklevel=<n>        " + _("How thorough the block verification is (0-6, default: 1)") + "\n" +
        "  -loadblock=<file>      " + _("Imports blocks from external blk000?.dat file") + "\n" +

        "\n" + _("Block creation options:") + "\n" +
        "  -blockminsize=<n>      "   + _("Set minimum block size in bytes (default: 0)") + "\n" +
        "  -blockmaxsize=<n>      "   + _("Set maximum block size in bytes (default: 250000)") + "\n" +
        "  -blockprioritysize=<n> "   + _("Set maximum size of high-priority/low-fee transactions in bytes (default: 27000)") + "\n" +

        "\n" + _("SSL options: (see the Bitcoin Wiki for SSL setup instructions)") + "\n" +
        "  -rpcssl                                  " + _("Use OpenSSL (https) for JSON-RPC connections") + "\n" +
        "  -rpcsslcertificatechainfile=<file.cert>  " + _("Server certificate file (default: server.cert)") + "\n" +
        "  -rpcsslprivatekeyfile=<file.pem>         " + _("Server private key (default: server.pem)") + "\n" +
        "  -rpcsslciphers=<ciphers>                 " + _("Acceptable ciphers (default: TLSv1+HIGH:!SSLv2:!aNULL:!eNULL:!AH:!3DES:@STRENGTH)") + "\n";

    return strUsage;
}

/** Sanity checks
 *  Ensure that Bitcoin is running in a usable environment with all
 *  necessary library support.
 */
bool InitSanityCheck(void)
{
    if(!ECC_InitSanityCheck()) {
        InitError("OpenSSL appears to lack support for elliptic curve cryptography. For more "
                  "information, visit https://en.bitcoin.it/wiki/OpenSSL_and_EC_Libraries");
        return false;
    }

    // TODO: remaining sanity checks, see #4081

    return true;
}

int ReadBlockIndexFromFile(CBlockIndex* block, int iHi)
{
	int rzt = 0;
	string sFile = strprintf("%s%d.idx", s_BlockChain_Dir.c_str(), iHi);
	printf("ReadBlockIndexFromFile %s\n", sFile.c_str());
	
	ifstream is;
	//is(sFile.c_str(), ios::binary);
	is.open( sFile.c_str(), ios::binary);
	//if( !is.is_open() ){ return rzt; }
	if( is.is_open() )
	{
		//printf("ReadBlockIndexFromFile open ok [%s] \n", block.ToString().c_str());
		//OutputDebugStringA("Open ok");
		is.read((char*)block, sizeof(CBlockIndex));
		//OutputDebugStringA("Read ok");
		//printf("ReadBlockIndexFromFile %s\n", block->ToString().c_str());
		is.close();   rzt++;
	}
	return rzt;
}

int ReadBlockFromFile(CBlock& block, int iHi)
{
	int rzt = 0;
	string sFile = strprintf("%s%d.block", s_BlockChain_Dir.c_str(), iHi);
	//if( fDebug ) printf("ReadBlockFromFile %s\n", sFile.c_str());
	FILE* file = fopen(sFile.c_str(), "rb");
	if (!file){ return rzt; }
	CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
	if( !filein ){ return rzt; }

	//filein.nType |= SER_BLOCKHEADERONLY;
    try{
		filein >> block;   rzt++;
    }
    catch (std::exception &e) {
        //return error("%s() : deserialize or I/O error", __PRETTY_FUNCTION__);
    }
	filein.fclose();
	return rzt;
}

bool readAndInstallBlock(int nHei)
{
	bool rzt = false;
	CBlock block;
	if( ReadBlockFromFile(block, nHei) > 0 )
	{
		bFastsyncBestHei = nHei;
		block.print();
		block.CheckBlock(true, true, false);
		
		unsigned int nFile;
		unsigned int nBlockPos;
		if( block.WriteToDisk(nFile, nBlockPos, true) )
		{
			if (!block.AddToBlockIndex(nFile, nBlockPos, u256bestHashForFastSync))
			{
				if( fDebug ) printf("readAndInstallBlock:: AddToBlockIndex false :( \n");
			}else rzt = true;
		}
	}
	return rzt;
}

#ifdef QT_GUI
void doFastSyncBlock()
{
	int iFastsyncblockMode = iFastsyncblockModeArg;  //iFastsyncblockModeArg = GetArg("-fastsyncblockmode", 1);
	if( iFastsyncblockMode > 0 )
	{
        if( fDebug ) printf("FastsyncblockMode [%s] \n", s_fastSyncBlock_ini.c_str());
		QSettings *ConfigIni = new QSettings(QString::fromUtf8(s_fastSyncBlock_ini.c_str()), QSettings::IniFormat, 0);		
		int nHei = ConfigIni->value("/bnt/bestHei", "0").toInt(); //Checkpoints::GetTotalBlocksEstimate();
		if( nHei > 0 )
		{
		  if( nBestHeight < nHei )
		  {
			s_bestHashForFastSync = ConfigIni->value("/bnt/bestHash", "0").toString().toStdString();
			uint256 i6Hash(s_bestHashForFastSync);  u256bestHashForFastSync = i6Hash;
			if( fDebug ) printf("\n\n bFastsyncblockMode nHei = [%d] [%s] \n", nHei, u256bestHashForFastSync.ToString().c_str());

			s_bestHashForFastSync = ConfigIni->value("/bnt/preHash", "0").toString().toStdString();
			uint256 iPreHash(s_bestHashForFastSync); 
			if( iPreHash > 0 )
			{
				//CBlockIndex pBlockIndex;
				preHashForFastSync = iPreHash;
				CBlockIndex* preBlockIndex = new CBlockIndex();
				if( ReadBlockIndexFromFile(preBlockIndex, (nHei - 1)) > 0 )
				{
					preBlockIndex->phashBlock = &preHashForFastSync;
					preBlockIndex->pprev = NULL;   //BLOCK_STAKE_MODIFIER = (1 << 2)
					preBlockIndex->pnext = NULL;
					std::string str = ConfigIni->value("/bnt/preMerkleroot", "0").toString().toStdString();
					uint256 vhashMerkleRoot(str);
					preBlockIndex->hashMerkleRoot = vhashMerkleRoot;
					if( fDebug ) printf("mapBlockIndex.size() = %"PRIszu", %d : %d %d \n",   mapBlockIndex.size(), preBlockIndex->nHeight, preBlockIndex->nFlags, (1 << 2));
					mapBlockIndex.insert(make_pair(iPreHash, preBlockIndex));
					
					// Write to disk block index
					CTxDB txdb;
					if ( txdb.TxnBegin() )
					{
						CDiskBlockIndex cId(preBlockIndex);
						if( fDebug ) printf("WriteBlockIndex cid hei = [%d] [%d] \n", cId.nHeight, preBlockIndex->nHeight);
						str = ConfigIni->value("/bnt/ppreHash", "0").toString().toStdString();
						uint256 vPrePreHash(str);
						cId.hashPrev   = vPrePreHash;  //preBlockIndex->hashPrev;
						cId.hashMerkleRoot = preBlockIndex->hashMerkleRoot;
						cId.nFile          = preBlockIndex->nFile;
						cId.nBlockPos      = preBlockIndex->nBlockPos;
						cId.nHeight        = preBlockIndex->nHeight;
						cId.nMint          = preBlockIndex->nMint;
						cId.nMoneySupply   = preBlockIndex->nMoneySupply;
						cId.nFlags         = preBlockIndex->nFlags;
						cId.nStakeModifier = preBlockIndex->nStakeModifier;
						cId.prevoutStake   = preBlockIndex->prevoutStake;
						cId.nStakeTime     = preBlockIndex->nStakeTime;
						cId.hashProof      = preBlockIndex->hashProof;
						cId.nVersion       = preBlockIndex->nVersion;
						cId.hashMerkleRoot = preBlockIndex->hashMerkleRoot;
						cId.nTime          = preBlockIndex->nTime;
						cId.nBits          = preBlockIndex->nBits;
						cId.nNonce         = preBlockIndex->nNonce;
						txdb.WriteBlockIndex(cId);  //txdb.WriteBlockIndex(CDiskBlockIndex(preBlockIndex));
						if (!txdb.TxnCommit()){ printf("TxnCommit false :( \n"); }
					}

					
					string sss = ConfigIni->value("/bnt/StakeModifier", "0").toString().toStdString();
					uint256 i256(sss); 
					nFastStakeModifier = i256.Get64();   //(uint64_t)S_To_64(sss.c_str());
					nFastStakeModifierChecksum = ConfigIni->value("/bnt/modifierchecksum", "0").toInt();
					if( fDebug ) printf("(%8x), (%"PRIx64") %s\n",  nFastStakeModifierChecksum, nFastStakeModifier, sss.c_str());
					//printf("insert preHash [%s :: %s] \n", iPreHash.ToString().c_str(), pBlockIndex.ToString().c_str());

					bFastsyncblockMode++; 
					readAndInstallBlock(nHei - 3);
					readAndInstallBlock(nHei - 2);
					readAndInstallBlock(nHei - 1);
					CBlock block;
					if( ReadBlockFromFile(block, nHei) > 0 )  //if( readAndInstallBlock(nHei) )
					{
						//bFastsyncblockMode++;   
						bFastsyncBestHei = nHei;
						block.print();						
						/*unsigned int nFile;
						unsigned int nBlockPos;
						if( block.WriteToDisk(nFile, nBlockPos, true) )
						{
							if (!block.AddToBlockIndex(nFile, nBlockPos, u256bestHashForFastSync))
							{
								if( fDebug ) printf("AddToBlockIndex false :( \n");
							}
						}*/
	
						//printf("block: \n %s \n", block.ToString().c_str());
						if( ProcessBlock(NULL, &block) ){ 
							iFastSyncBlockHeiOk = bFastsyncBestHei;   
							ConfigIni->setValue("/ok/bestHei", iFastSyncBlockHeiOk);
						}
						else{ if( fDebug ) printf("doFastSyncBlock:: ProcessBlock false \n"); }
					}else{ if( fDebug ) printf("doFastSyncBlock:: ReadBlockFromFile false \n"); }
					bFastsyncblockMode = 0;
				}else{
					if( fDebug ) printf("doFastSyncBlock:: ReadBlockIndexFromFile false \n");
				}
			}else{
				if( fDebug ) printf("iPreHash = 0 \n");
			}
		  }
		}else{ printf("nHei = 0 \n"); }
		delete ConfigIni;
		if( fDebug ) printf("bFastsyncblockMode [%d] end \n\n\n", nHei);
		/*uint256 nHash = Checkpoints::GetTotalBlocksEstimateHash();
		int nHei = Checkpoints::GetTotalBlocksEstimate();
		if( (nHei > 0) && (nHash > 0) )
		{
			hashGenesisBlock = nHash;
			CTxDB txdb;
			txdb.WriteHashBestChain(nHash);
			if (!txdb.TxnCommit()){ printf("SetBestChain() : TxnCommit failed \n"); }
			else pindexGenesisBlock = pindexNew;
		}*/
	}
}
#endif

bool restoreWalletByFile(const string wltFile)
{
    // Copy wallet.dat
    filesystem::path pathSrc( wltFile );
    filesystem::path pathDest = GetDataDir() / sWallet_restore;
    try {
#if BOOST_VERSION >= 104000
            filesystem::copy_file(pathSrc, pathDest, filesystem::copy_option::overwrite_if_exists);
#else
            filesystem::copy_file(pathSrc, pathDest);
#endif
            //printf("copied wallet.dat to %s\n", pathDest.string().c_str());
            return true;
        } catch(const filesystem::filesystem_error &e) {
            printf("error copying wallet.dat to %s - %s\n", pathDest.string().c_str(), e.what());
            return false;
        }
}

int isBlockChainCompressed(std::string strDataDir, int iDefault)
{
	int rzt = iDefault;
#ifdef WIN32 
	string sFile = strDataDir + "\\" + "blk0001.dat";   //string sFile = strprintf("%s\\blk0001.dat", strDataDir.c_str());
#else
	string sFile = strDataDir + "/" + "blk0001.dat";
#endif
	//printf("isBitChainCompressed %s\n", sFile.c_str());
	
	ifstream is;     is.open( sFile.c_str(), ios::binary);
	if( is.is_open() )
	{
		std::string s;     s.resize(512);     char* p = (char*)s.c_str();
		is.read(p, 512);     is.close();
		unsigned int i = *( (unsigned int *)&p[8] );
		if( i == 0x01 ){ rzt = 0; }
		else if( i == 0xB7 ){ rzt = 1; }
		s.resize(0);
	}
	return rzt;
}

#ifdef USE_SQLITE
static int selectCountCallback(void *data, int argc, char **argv, char **azColName)
{
   //fprintf(stderr, "%s: ", (const char*)data);
   if( (data!=NULL) && (argc > 0) )
   {
      *(int64_t *)data = atoi64(argv[0]);
   }
   return 0;
}

static int selectCallback(void *data, int argc, char **argv, char **azColName)
{
   //fprintf(stderr, "%s: ", (const char*)data);
   //(int *)data^ = 
   int i;
   for(i=0; i<argc; i++){
      printf("%s :: %s = %s\n", (const char*)data, azColName[i], argv[i] ? argv[i] : "NULL");   // Callback function called :: Count(*) = 10
   }
   //printf("\n");
   return 0;
}

static int sqliteCallback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("sqlite callback %s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int openSqliteDb()
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int  rc;
   char *sql;
   rc = sqlite3_threadsafe();
printf("---> openSqliteDb, thread safe = [%d] \n", rc);
   /* Open database */
   rc = sqlite3_open("alladdress.db", &dbAllAddress);  //World.db3  alladdress.db
   
   
   /* Create SQL statement */
   sql = "SELECT Count(*) from Addresss where id < 10";  //"SELECT * from Keys";
   /* Execute SQL statement */
   int64_t icnt = 0;
   rc = sqlite3_exec(dbAllAddress, sql, selectCountCallback, (void*)&icnt, &zErrMsg);
   if( rc != SQLITE_OK ){
      printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      printf("Query done, count = [%I64u]\n", icnt);
   }

   const char* data = "Callback function called";
   //sql = "SELECT Count(*) from City where ID < 11";  //"SELECT * from Keys";
   sql = "SELECT * from City where ID < 6";  //"SELECT * from Keys";
   /* Execute SQL statement */
   rc = sqlite3_exec(dbAllAddress, sql, selectCallback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      printf("Operation done successfully\n");
   }
   
   
   rc = sqlite3_open("test.db", &db);
   if( rc ){
      printf("Can't open database: %s\n", sqlite3_errmsg(db));
      return(0);
   }else{
      printf("Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "CREATE TABLE COMPANY("  \
         "ID INT PRIMARY KEY     NOT NULL," \
         "NAME           TEXT    NOT NULL," \
         "AGE            INT     NOT NULL," \
         "ADDRESS        CHAR(50)," \
         "SALARY         REAL );";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, sqliteCallback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   printf("SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      printf("Table created successfully\n");
   }
   sqlite3_close(db);
printf("<--- openSqliteDb \n");
   return 0;
}

struct dbCallbackPack
{
   int vType;
   int vID;
   int64_t i6Rzt;
   string sRzt;
};
static int getCoinCallback(void *data, int argc, char **argv, char **azColName)
{
   if( data != NULL )  //if( (data!=NULL) && (argc > 2) )
   {
      dbCallbackPack* p = (dbCallbackPack *)data;
	  if( argc > p->vID )
	  {
		  if( p->vType == 1 ){ p->i6Rzt = atoi64(argv[p->vID]); }
		  else if( p->vType == 2 ){ p->sRzt = argv[p->vID]; }
		  //*(int64_t *)data = atoi64(argv[2]);
	  }
   }
   return 0;
}
int updateAllAdressDB(const string sAddr, int64_t nValue, bool bAdd)
{
   int rzt = 0;
   string sql = "SELECT Count(*) from Addresss where addr = '" + sAddr + "'";  //"SELECT * from Keys";
   if( fDebug ) printf("updateAllAdressDB:: bAdd=[%d], sql [%s] \n", bAdd, sql.c_str());
   int64_t icnt = 0;
   int rc = sqlite3_exec(dbAllAddress, sql.c_str(), selectCountCallback, (void*)&icnt, NULL);
   if( (icnt == 0) || (rc != SQLITE_OK) ){ // not exist, 
      //printf("SQL error: %s\n", zErrMsg);
      //sqlite3_free(zErrMsg);
	  if( bAdd )  
	  {
         //sql = "SELECT Count(*) from Addresss where addr = '" + sAddr + "'";  //"SELECT * from Keys";
         sql = "INSERT INTO Addresss (addr, coin) VALUES ('" + sAddr + "', " + strprintf("%" PRId64 ");", nValue); 
         rc = sqlite3_exec(dbAllAddress, sql.c_str(), NULL, NULL, NULL);
	     if( fDebug ) printf("updateAllAdressDB:: sql [%s], rc = [%d]\n", sql.c_str(), rc);
		 if( rc == SQLITE_OK ){ rzt++; }
	  }else{
	     printf("updateAllAdressDB:: bAdd = [%d], nValue[%I64u], Address [%s] not exist :( eeeeeerror\n", bAdd, nValue, sAddr.c_str());
	  }
   }else if( icnt > 0 ){
      //printf("Query done, count = [%I64u]\n", icnt);
	  sql = "SELECT * from Addresss where addr = '" + sAddr + "'";
	  icnt = 0;
	  dbCallbackPack pack={1, 2, 0, ""};
      rc = sqlite3_exec(dbAllAddress, sql.c_str(), getCoinCallback, (void*)&pack, NULL);
	  icnt = pack.i6Rzt;
	  if( fDebug ) printf("updateAllAdressDB:: getCoin [%I64u] rc=[%d] nValue=[%I64u]\n", icnt, rc, nValue);
      if( rc == SQLITE_OK )
	  {
         if( bAdd )
		 {
		    int64_t t = icnt;
			icnt = icnt + nValue;
			if( icnt < t ){ printf("updateAllAdressDB:: add overflow [%I64u] < [%I64u] eeeeeerror\n", icnt, t); }
		 }else{
		    if( icnt >= nValue ){ icnt = icnt - nValue; }
		 }
		 sql = "UPDATE Addresss set coin = " + strprintf("%" PRId64, icnt) + " where addr='" + sAddr + "'";
		 rc = sqlite3_exec(dbAllAddress, sql.c_str(), NULL, NULL, NULL);
		 if( fDebug ) printf("updateAllAdressDB:: bAdd=[%d], sql [%s], rc=[%d], icnt=[%I64u] \n", bAdd, sql.c_str(), rc, icnt);
		 if( rc == SQLITE_OK ){ rzt++; }
	  }
   }
   return rzt;
}

void updateTotalCoin(int64_t nValue)
{
   string sql = "SELECT * from total";
   int64_t icnt = 0;
   dbCallbackPack pack={1, 0, 0, ""};
   int rc = sqlite3_exec(dbAllAddress, sql.c_str(), getCoinCallback, (void*)&pack, NULL);
   icnt = pack.i6Rzt;
   if( fDebug ) printf("updateTotalCoin:: Cur Total Coin [%I64u] rc=[%d] nValue=[%I64u]\n", icnt, rc, nValue);
   if( rc == SQLITE_OK )
   {
	   icnt = icnt + nValue;
	   sql = "UPDATE total set totalcoin = " + strprintf("%" PRId64, icnt);
	   rc = sqlite3_exec(dbAllAddress, sql.c_str(), 0, 0, NULL);
	   if( fDebug ) printf("updateTotalCoin:: sql [%s], rc=[%d], [%I64u] \n", sql.c_str(), rc, icnt);
	   //if( rc == SQLITE_OK ){ rzt++; }
   }
}

int isBlockProcessed(int nHi)
{
   int rzt = 0;
   string sql = "SELECT Count(*) from processedblock where num = " + strprintf("%d", nHi);
   if( fDebug ) printf("isBlockProcessed:: nHi=[%d], sql [%s] \n", nHi, sql.c_str());
   int64_t icnt = 0;
   int rc = sqlite3_exec(dbAllAddress, sql.c_str(), selectCountCallback, (void*)&icnt, NULL);
   if( icnt  > 0 ){ rzt++; }
   return rzt;
}

void insertProcessedBlock(int nHi, string sHash)
{
   string sql = "INSERT INTO processedblock (hash, num) VALUES ('" + sHash + "', " + strprintf("%d);", nHi); 
   if( fDebug ) printf("insertProcessedBlock:: sql [%s] \n", sql.c_str());
   sqlite3_exec(dbAllAddress, sql.c_str(), NULL, NULL, NULL);
}
#endif

/** Initialize bitcoin.
 *  @pre Parameters should be parsed and config file should be read.
 */
bool AppInit2()
{
    // ********************************************************* Step 1: setup
#ifdef _MSC_VER
    // Turn off Microsoft heap dump noise
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, CreateFileA("NUL", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, 0));
#endif
#if _MSC_VER >= 1400
    // Disable confusing "helpful" text message on abort, Ctrl-C
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

/* #ifdef WIN32
    // Enable Data Execution Prevention (DEP)
    // Minimum supported OS versions: WinXP SP3, WinVista >= SP1, Win Server 2008
    // A failure is non-critical and needs no further attention!
#ifndef PROCESS_DEP_ENABLE
// We define this here, because GCCs winbase.h limits this to _WIN32_WINNT >= 0x0601 (Windows 7),
// which is not correct. Can be removed, when GCCs winbase.h is fixed!
#define PROCESS_DEP_ENABLE 0x00000001
#endif
    typedef BOOL (WINAPI *PSETPROCDEPPOL)(DWORD);
    PSETPROCDEPPOL setProcDEPPol = (PSETPROCDEPPOL)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetProcessDEPPolicy");
    if (setProcDEPPol != NULL) setProcDEPPol(PROCESS_DEP_ENABLE);
#endif */

#ifndef WIN32
    umask(077);

    // Clean shutdown on SIGTERM
    struct sigaction sa;
    sa.sa_handler = HandleSIGTERM;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    // Reopen debug.log on SIGHUP
    struct sigaction sa_hup;
    sa_hup.sa_handler = HandleSIGHUP;
    sigemptyset(&sa_hup.sa_mask);
    sa_hup.sa_flags = 0;
    sigaction(SIGHUP, &sa_hup, NULL);
#endif

    // ********************************************************* Step 2: parameter interactions

    nNodeLifespan = GetArg("-addrlifespan", 7);
    fUseFastIndex = GetBoolArg("-fastindex", true);
    nMinerSleep = GetArg("-minersleep", 500);
#ifdef QT_GUI
    fNodebuglog = GetArg("-nodebuglog", 1);
    dw_zip_block = GetArg("-zipblock", 1);
#else
    fNodebuglog = GetArg("-nodebuglog", 0);
    dw_zip_block = GetArg("-zipblock", 0);
#endif
    dw_zip_limit_size = GetArg("-ziplimitsize", 64);
    dw_zip_txdb = GetArg("-ziptxdb", 0);
    if( dw_zip_block > 1 ){ dw_zip_block = 1; }
    else if( dw_zip_block == 0 ){ dw_zip_txdb = 0; }
	
	fNetDbg = GetArg("-netdbg", 0);
	fNewSocketThread = GetArg("-newp2pservicethread", 0);
#ifdef WIN32
	fFixedChangeAddress = GetArg("-fixedchangeaddress", 1);
#else
	fFixedChangeAddress = GetArg("-fixedchangeaddress", 0);
#endif

    CheckpointsMode = Checkpoints::STRICT;
    std::string strCpMode = GetArg("-cppolicy", "strict");

    if(strCpMode == "strict")
        CheckpointsMode = Checkpoints::STRICT;

    if(strCpMode == "advisory")
        CheckpointsMode = Checkpoints::ADVISORY;

    if(strCpMode == "permissive")
        CheckpointsMode = Checkpoints::PERMISSIVE;

    nDerivationMethodIndex = 0;

    fTestNet = GetBoolArg("-testnet");
    if (fTestNet) {
        SoftSetBoolArg("-irc", true);
    }

    if (mapArgs.count("-bind")) {
        // when specifying an explicit binding address, you want to listen on it
        // even when -connect or -proxy is specified
        SoftSetBoolArg("-listen", true);
    }

    if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0) {
        // when only connecting to trusted nodes, do not seed via DNS, or listen by default
        SoftSetBoolArg("-dnsseed", false);
        SoftSetBoolArg("-listen", false);
    }

    if (mapArgs.count("-proxy")) {
        // to protect privacy, do not listen by default if a proxy server is specified
        SoftSetBoolArg("-listen", false);
    }

    if (!GetBoolArg("-listen", true)) {
        // do not map ports or try to retrieve public IP when not listening (pointless)
        SoftSetBoolArg("-upnp", false);
        SoftSetBoolArg("-discover", false);
    }

    if (mapArgs.count("-externalip")) {
        // if an explicit public IP is specified, do not try to find others
        SoftSetBoolArg("-discover", false);
    }

    if (GetBoolArg("-salvagewallet")) {
        // Rewrite just private keys: rescan to find transactions
        SoftSetBoolArg("-rescan", true);
    }

    // ********************************************************* Step 3: parameter-to-internal-flags

    fDebug = GetBoolArg("-debug");

    // -debug implies fDebug*
    if (fDebug)
        fDebugNet = true;
    else
        fDebugNet = GetBoolArg("-debugnet");

#if !defined(WIN32) && !defined(QT_GUI)
    fDaemon = GetBoolArg("-daemon");
#else
    fDaemon = false;
#endif

    if (fDaemon)
        fServer = true;
    else
        fServer = GetBoolArg("-server");

    /* force fServer when running without GUI */
#if !defined(QT_GUI)
    fServer = true;
#endif
    fPrintToConsole = GetBoolArg("-printtoconsole");
    fPrintToDebugger = GetBoolArg("-printtodebugger");
    fLogTimestamps = GetBoolArg("-logtimestamps");

    if (mapArgs.count("-timeout"))
    {
        int nNewTimeout = GetArg("-timeout", 5000);
        if (nNewTimeout > 0 && nNewTimeout < 600000)
            nConnectTimeout = nNewTimeout;
    }

    if (mapArgs.count("-paytxfee"))
    {
        if (!ParseMoney(mapArgs["-paytxfee"], nTransactionFee))
            return InitError(strprintf(_("Invalid amount for -paytxfee=<amount>: '%s'"), mapArgs["-paytxfee"].c_str()));
        if (nTransactionFee > 0.25 * COIN)
            InitWarning(_("Warning: -paytxfee is set very high! This is the transaction fee you will pay if you send a transaction."));
    }

    fConfChange = GetBoolArg("-confchange", false);

    if (mapArgs.count("-mininput"))
    {
        if (!ParseMoney(mapArgs["-mininput"], nMinimumInputValue))
            return InitError(strprintf(_("Invalid amount for -mininput=<amount>: '%s'"), mapArgs["-mininput"].c_str()));
    }

    // ********************************************************* Step 4: application initialization: dir lock, daemonize, pidfile, debug log
    // Sanity check
    if (!InitSanityCheck())
        return InitError(_("Initialization sanity check failed. VpnCoin is shutting down."));

    std::string strDataDir = GetDataDir().string();
    std::string strWalletFileName = GetArg("-wallet", "wallet.dat");

dw_zip_block = isBlockChainCompressed(strDataDir, dw_zip_block);
#ifndef QT_GUI
    printf("BlockChain compress flag = [%d] \n", dw_zip_block);
#endif

    filesystem::path pathWalletBack = GetDataDir() / sWallet_restore;
    if (filesystem::exists(pathWalletBack))
	{
            filesystem::path pathWalletDat = GetDataDir() / "wallet.dat";
			if (filesystem::exists(pathWalletDat))
			{
				filesystem::path pathWalletRename = GetDataDir() / strprintf("wallet_%I64u.dat", GetTime());
				RenameOver(pathWalletDat, pathWalletRename);
			}
            RenameOver(pathWalletBack, pathWalletDat);
			strWalletFileName = pathWalletDat.string();
    }


    // strWalletFileName must be a plain filename without a directory
    //if (strWalletFileName != boost::filesystem::basename(strWalletFileName) + boost::filesystem::extension(strWalletFileName))
    //    return InitError(strprintf(_("Wallet %s resides outside data directory %s."), strWalletFileName.c_str(), strDataDir.c_str()));

    boost::filesystem::path pcu = boost::filesystem::current_path();
#ifndef WIN32
	s_BlockChain_AdBonus_Dir = strDataDir + "/";
	s_BlockChain_Dir = s_BlockChain_AdBonus_Dir;
    //printf("Current dir is [%s] [%s] \n", sCurDir.c_str(), s_BlockChain_Dir.c_str());
#else
	s_BlockChain_AdBonus_Dir = pcu.string().c_str(); // + "\\BlockChain\\AdBonus\\";
	s_BlockChain_Dir = s_BlockChain_AdBonus_Dir + "/BlockChain/";
	boost::filesystem::create_directory(s_BlockChain_Dir);
	//std::string sBlock = s_BlockChain_Dir + "block//";
	//boost::filesystem::create_directory(sBlock);
	s_fastSyncBlock_ini = s_BlockChain_Dir + "fastsyncblock.ini";
#endif
	s_BlockChain_AdBonus_Dir = s_BlockChain_Dir;  // + "AdBonus//";

	// Make sure only a single Bitcoin process is using the data directory.
    boost::filesystem::path pathLockFile = GetDataDir() / ".lock";
    FILE* file = fopen(pathLockFile.string().c_str(), "a"); // empty lock file; created if it doesn't exist.
    if (file) fclose(file);
    static boost::interprocess::file_lock lock(pathLockFile.string().c_str());
    if (!lock.try_lock())
        return InitError(strprintf(_("Cannot obtain a lock on data directory %s.  VpnCoin is probably already running."), strDataDir.c_str()));

#if !defined(WIN32) && !defined(QT_GUI)
    if (fDaemon)
    {
        // Daemonize
        pid_t pid = fork();
        if (pid < 0)
        {
            fprintf(stderr, "Error: fork() returned %d errno %d\n", pid, errno);
            return false;
        }
        if (pid > 0)
        {
            CreatePidFile(GetPidFile(), pid);
            return true;
        }

        pid_t sid = setsid();
        if (sid < 0)
            fprintf(stderr, "Error: setsid() returned %d errno %d\n", sid, errno);
    }
#endif

    if (GetBoolArg("-shrinkdebugfile", !fDebug))
        ShrinkDebugFile();
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("VpnCoin version %s (%s)\n", FormatFullVersion().c_str(), CLIENT_DATE.c_str());
    printf("Using OpenSSL version %s\n", SSLeay_version(SSLEAY_VERSION));
    if (!fLogTimestamps)
        printf("Startup time: %s\n", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
    printf("Default data directory %s\n", GetDefaultDataDir().string().c_str());
    printf("Used data directory %s\n", strDataDir.c_str());
	//printf("BlockChain directory %s\n", s_BlockChain_Dir.c_str());
	printf("Ad Bonus directory %s\n", s_BlockChain_AdBonus_Dir.c_str());
    std::ostringstream strErrors;

    if (fDaemon)
        fprintf(stdout, "VpnCoin server starting\n");

    int64_t nStart;

    // ********************************************************* Step 5: verify database integrity

    uiInterface.InitMessage(_("Verifying database integrity..."));

    if (!bitdb.Open(GetDataDir()))
    {
        string msg = strprintf(_("Error initializing database environment %s!"
                                 " To recover, BACKUP THAT DIRECTORY, then remove"
                                 " everything from it except for wallet.dat."), strDataDir.c_str());
        return InitError(msg);
    }

    if (GetBoolArg("-salvagewallet"))
    {
        // Recover readable keypairs:
        if (!CWalletDB::Recover(bitdb, strWalletFileName, true))
            return false;
    }

	boost::filesystem::path sf = GetDataDir() / strWalletFileName;
	if( strWalletFileName.length() > 2 )
	{
		const char* pW = strWalletFileName.c_str();
		if( pW[1] == ':' ) sf = strWalletFileName;
	}
	
    if( filesystem::exists(sf) )    //if( filesystem::exists(GetDataDir() / strWalletFileName) )
    {
        CDBEnv::VerifyResult r = bitdb.Verify(strWalletFileName, CWalletDB::Recover);
        if (r == CDBEnv::RECOVER_OK)
        {
            string msg = strprintf(_("Warning: wallet.dat corrupt, data salvaged!"
                                     " Original wallet.dat saved as wallet.{timestamp}.bak in %s; if"
                                     " your balance or transactions are incorrect you should"
                                     " restore from a backup."), strDataDir.c_str());
            uiInterface.ThreadSafeMessageBox(msg, _("VpnCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
        }
        if (r == CDBEnv::RECOVER_FAIL)
            return InitError(_("wallet.dat corrupt, salvage failed"));
    }

    // ********************************************************* Step 6: network initialization

    int nSocksVersion = GetArg("-socks", 5);

    if (nSocksVersion != 4 && nSocksVersion != 5)
        return InitError(strprintf(_("Unknown -socks proxy version requested: %i"), nSocksVersion));

    if (mapArgs.count("-onlynet")) {
        std::set<enum Network> nets;
        BOOST_FOREACH(std::string snet, mapMultiArgs["-onlynet"]) {
            enum Network net = ParseNetwork(snet);
            if (net == NET_UNROUTABLE)
                return InitError(strprintf(_("Unknown network specified in -onlynet: '%s'"), snet.c_str()));
            nets.insert(net);
        }
        for (int n = 0; n < NET_MAX; n++) {
            enum Network net = (enum Network)n;
            if (!nets.count(net))
                SetLimited(net);
        }
    }

    CService addrProxy;
    bool fProxy = false;
    if (mapArgs.count("-proxy")) {
        addrProxy = CService(mapArgs["-proxy"], 9050);
        if (!addrProxy.IsValid())
            return InitError(strprintf(_("Invalid -proxy address: '%s'"), mapArgs["-proxy"].c_str()));

        if (!IsLimited(NET_IPV4))
            SetProxy(NET_IPV4, addrProxy, nSocksVersion);
        if (nSocksVersion > 4) {
            if (!IsLimited(NET_IPV6))
                SetProxy(NET_IPV6, addrProxy, nSocksVersion);
            SetNameProxy(addrProxy, nSocksVersion);
        }
        fProxy = true;
    }

    // -tor can override normal proxy, -notor disables tor entirely
    if (!(mapArgs.count("-tor") && mapArgs["-tor"] == "0") && (fProxy || mapArgs.count("-tor"))) {
        CService addrOnion;
        if (!mapArgs.count("-tor"))
            addrOnion = addrProxy;
        else
            addrOnion = CService(mapArgs["-tor"], 9050);
        if (!addrOnion.IsValid())
            return InitError(strprintf(_("Invalid -tor address: '%s'"), mapArgs["-tor"].c_str()));
        SetProxy(NET_TOR, addrOnion, 5);
        SetReachable(NET_TOR);
    }

    // see Step 2: parameter interactions for more information about these
    fNoListen = !GetBoolArg("-listen", true);
    fDiscover = GetBoolArg("-discover", true);
    fNameLookup = GetBoolArg("-dns", true);
#ifdef USE_UPNP
    fUseUPnP = GetBoolArg("-upnp", USE_UPNP);
	fBindPort920 = GetArg("-bind920", 0);
#endif

    bool fBound = false;
    if (!fNoListen)
    {
        std::string strError;
        if (mapArgs.count("-bind")) {
            BOOST_FOREACH(std::string strBind, mapMultiArgs["-bind"]) {
                CService addrBind;
                if (!Lookup(strBind.c_str(), addrBind, GetListenPort(), false))
                    return InitError(strprintf(_("Cannot resolve -bind address: '%s'"), strBind.c_str()));
                fBound |= Bind(addrBind);
            }
        } else {
            struct in_addr inaddr_any;
            inaddr_any.s_addr = INADDR_ANY;
            if (!IsLimited(NET_IPV6))
                fBound |= Bind(CService(in6addr_any, GetListenPort()), false);
            if (!IsLimited(NET_IPV4))
                fBound |= Bind(CService(inaddr_any, GetListenPort()), !fBound);
#ifdef WIN32
           if( fBindPort920 > 0 ){ Bind(CService(inaddr_any, 920), !fBound); }
#endif
        }
        if (!fBound)
            return InitError(_("Failed to listen on any port. Use -listen=0 if you want this."));
    }

    if (mapArgs.count("-externalip"))
    {
        BOOST_FOREACH(string strAddr, mapMultiArgs["-externalip"]) {
            CService addrLocal(strAddr, GetListenPort(), fNameLookup);
            if (!addrLocal.IsValid())
                return InitError(strprintf(_("Cannot resolve -externalip address: '%s'"), strAddr.c_str()));
            AddLocal(CService(strAddr, GetListenPort(), fNameLookup), LOCAL_MANUAL);
        }
    }

    if (mapArgs.count("-reservebalance")) // ppcoin: reserve balance amount
    {
        if (!ParseMoney(mapArgs["-reservebalance"], nReserveBalance))
        {
            InitError(_("Invalid amount for -reservebalance=<amount>"));
            return false;
        }
    }

    if (mapArgs.count("-checkpointkey")) // ppcoin: checkpoint master priv key
    {
        if (!Checkpoints::SetCheckpointPrivKey(GetArg("-checkpointkey", "")))
            InitError(_("Unable to sign checkpoint, wrong checkpointkey?\n"));
    }

    BOOST_FOREACH(string strDest, mapMultiArgs["-seednode"])
        AddOneShot(strDest);

    // ********************************************************* Step 7: load blockchain

    if (!bitdb.Open(GetDataDir()))
    {
        string msg = strprintf(_("Error initializing database environment %s!"
                                 " To recover, BACKUP THAT DIRECTORY, then remove"
                                 " everything from it except for wallet.dat."), strDataDir.c_str());
        return InitError(msg);
    }

    if (GetBoolArg("-loadblockindextest"))
    {
        CTxDB txdb("r");
        txdb.LoadBlockIndex();
        PrintBlockTree();
        return false;
    }

#ifdef QT_GUI
	iFastsyncblockModeArg = GetArg("-fastsyncblockmode", 0);
	if( iFastsyncblockModeArg > 0 )
	{
		QSettings *ConfigIni = new QSettings(QString::fromUtf8(s_fastSyncBlock_ini.c_str()), QSettings::IniFormat, 0);		
		iFastSyncBlockHeiOk = ConfigIni->value("/ok/bestHei", "0").toInt();
		nFastPreStakeModifierChecksum = ConfigIni->value("/bnt/premodifierchecksum", "0").toInt();
		delete ConfigIni;
		if( fDebug ) printf("FastsyncblockMode [%s] [%d] [%d]\n", s_fastSyncBlock_ini.c_str(), iFastSyncBlockHeiOk, nFastPreStakeModifierChecksum);
	}
#endif

    uiInterface.InitMessage(_("Loading block index..."));
    printf("Loading block index...\n");
    nStart = GetTimeMillis();
    if (!LoadBlockIndex())
        return InitError(_("Error loading blkindex.dat"));


    // as LoadBlockIndex can take several minutes, it's possible the user
    // requested to kill bitcoin-qt during the last operation. If so, exit.
    // As the program has not fully started yet, Shutdown() is possibly overkill.
    if (fRequestShutdown)
    {
        printf("Shutdown requested. Exiting.\n");
        return false;
    }
    printf(" block index %15"PRId64"ms\n", GetTimeMillis() - nStart);

    if (GetBoolArg("-printblockindex") || GetBoolArg("-printblocktree"))
    {
        PrintBlockTree();
        return false;
    }

    if (mapArgs.count("-printblock"))
    {
        string strMatch = mapArgs["-printblock"];
        int nFound = 0;
        for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
        {
            uint256 hash = (*mi).first;
            if (strncmp(hash.ToString().c_str(), strMatch.c_str(), strMatch.size()) == 0)
            {
                CBlockIndex* pindex = (*mi).second;
                CBlock block;
                block.ReadFromDisk(pindex);
                block.BuildMerkleTree();
                block.print();
                printf("\n");
                nFound++;
            }
        }
        if (nFound == 0)
            printf("No blocks matching %s were found\n", strMatch.c_str());
        return false;
    }

    // ********************************************************* Testing Zerocoin


    if (GetBoolArg("-zerotest", false))
    {
        printf("\n=== ZeroCoin tests start ===\n");
        Test_RunAllTests();
        printf("=== ZeroCoin tests end ===\n\n");
    }

    // ********************************************************* Step 8: load wallet

    uiInterface.InitMessage(_("Loading wallet..."));
    printf("Loading wallet...\n");
    nStart = GetTimeMillis();
    bool fFirstRun = true;
    pwalletMain = new CWallet(strWalletFileName);
    DBErrors nLoadWalletRet = pwalletMain->LoadWallet(fFirstRun);
    if (nLoadWalletRet != DB_LOAD_OK)
    {
        if (nLoadWalletRet == DB_CORRUPT)
            strErrors << _("Error loading wallet.dat: Wallet corrupted") << "\n";
        else if (nLoadWalletRet == DB_NONCRITICAL_ERROR)
        {
            string msg(_("Warning: error reading wallet.dat! All keys read correctly, but transaction data"
                         " or address book entries might be missing or incorrect."));
            uiInterface.ThreadSafeMessageBox(msg, _("VpnCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
        }
        else if (nLoadWalletRet == DB_TOO_NEW)
            strErrors << _("Error loading wallet.dat: Wallet requires newer version of VpnCoin") << "\n";
        else if (nLoadWalletRet == DB_NEED_REWRITE)
        {
            strErrors << _("Wallet needed to be rewritten: restart VpnCoin to complete") << "\n";
            printf("%s", strErrors.str().c_str());
            return InitError(strErrors.str());
        }
        else
            strErrors << _("Error loading wallet.dat") << "\n";
    }

    if (GetBoolArg("-upgradewallet", fFirstRun))
    {
        int nMaxVersion = GetArg("-upgradewallet", 0);
        if (nMaxVersion == 0) // the -upgradewallet without argument case
        {
            printf("Performing wallet upgrade to %i\n", FEATURE_LATEST);
            nMaxVersion = CLIENT_VERSION;
            pwalletMain->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
        }
        else
            printf("Allowing wallet upgrade up to %i\n", nMaxVersion);
        if (nMaxVersion < pwalletMain->GetVersion())
            strErrors << _("Cannot downgrade wallet") << "\n";
        pwalletMain->SetMaxVersion(nMaxVersion);
    }

    if (fFirstRun)
    {
        // Create new keyUser and set as default key
        RandAddSeedPerfmon();

        CPubKey newDefaultKey;
        if (pwalletMain->GetKeyFromPool(newDefaultKey, false)) {
            pwalletMain->SetDefaultKey(newDefaultKey);
            if (!pwalletMain->SetAddressBookName(pwalletMain->vchDefaultKey.GetID(), ""))
                strErrors << _("Cannot write default address") << "\n";
        }
    }

    printf("%s", strErrors.str().c_str());
    printf(" wallet      %15"PRId64"ms\n", GetTimeMillis() - nStart);

    RegisterWallet(pwalletMain);

    CBlockIndex *pindexRescan = pindexBest;
    if (GetBoolArg("-rescan"))
        pindexRescan = pindexGenesisBlock;
    else
    {
        CWalletDB walletdb(strWalletFileName);
        CBlockLocator locator;
        if (walletdb.ReadBestBlock(locator))
            pindexRescan = locator.GetBlockIndex();
    }
    if (pindexBest != pindexRescan && pindexBest && pindexRescan && pindexBest->nHeight > pindexRescan->nHeight)
    {
        uiInterface.InitMessage(_("Rescanning..."));
        printf("Rescanning last %i blocks (from block %i)...\n", pindexBest->nHeight - pindexRescan->nHeight, pindexRescan->nHeight);
        nStart = GetTimeMillis();
        pwalletMain->ScanForWalletTransactions(pindexRescan, true);
        printf(" rescan      %15"PRId64"ms\n", GetTimeMillis() - nStart);
    }

    // ********************************************************* Step 9: import blocks

    if (mapArgs.count("-loadblock"))
    {
        uiInterface.InitMessage(_("Importing blockchain data file."));

        BOOST_FOREACH(string strFile, mapMultiArgs["-loadblock"])
        {
            FILE *file = fopen(strFile.c_str(), "rb");
            if (file)
                LoadExternalBlockFile(file);
        }
        exit(0);
    }
    //if( filesystem::exists(pcu / "VpnDial.dat") ){ bVpnDialFileExist++; }
    sBitChainIdentAddress = GetAddressesByAccount(sBitChain_ident, true);  //"BitChain-ident"
    sBitChainKeyAddress = GetArg("-bitchainkey", "");
    if( sBitChainKeyAddress.length() < 32 ){ sBitChainKeyAddress = GetAddressesByAccount(sBitChain_key, true); }  //"BitChain-key"
    //if( sBitChainAddr.length() < 30 )
    {
        //sBitChainAddr = getAccountAddress("BitChain"); 
        if( fDebug )printf("BitChain ident address [%s], Key [%s] \n", sBitChainIdentAddress.c_str(), sBitChainKeyAddress.c_str());
    }

    filesystem::path pathBootstrap = GetDataDir() / "bootstrap.dat";
    if (filesystem::exists(pathBootstrap)) {
        uiInterface.InitMessage(_("Importing bootstrap blockchain data file."));

        FILE *file = fopen(pathBootstrap.string().c_str(), "rb");
        if (file) {
            filesystem::path pathBootstrapOld = GetDataDir() / "bootstrap.dat.old";
            LoadExternalBlockFile(file);
            RenameOver(pathBootstrap, pathBootstrapOld);
        }
    }

    // ********************************************************* Step 10: load peers

    uiInterface.InitMessage(_("Loading addresses..."));
    printf("Loading addresses...\n");
    nStart = GetTimeMillis();

    {
        CAddrDB adb;
        if (!adb.Read(addrman))
            printf("Invalid or missing peers.dat; recreating\n");
    }

    printf("Loaded %i addresses from peers.dat  %"PRId64"ms\n",
           addrman.size(), GetTimeMillis() - nStart);

    // ********************************************************* Step 11: start node

    if (!CheckDiskSpace())
        return false;

    RandAddSeedPerfmon();
#ifndef WIN32
    sDefWalletAddress = CBitcoinAddress(pwalletMain->vchDefaultKey.GetID()).ToString(); 
#endif

#ifdef QT_GUI
	doFastSyncBlock();
#endif

    //// debug print
    printf("mapBlockIndex.size() = %"PRIszu"\n",   mapBlockIndex.size());
    printf("nBestHeight = %d\n",            nBestHeight);
    printf("setKeyPool.size() = %"PRIszu"\n",      pwalletMain->setKeyPool.size());
    printf("mapWallet.size() = %"PRIszu"\n",       pwalletMain->mapWallet.size());
    printf("mapAddressBook.size() = %"PRIszu"\n",  pwalletMain->mapAddressBook.size());
    initZkpForVer1703();   // 2017.04.17

#ifdef USE_BITNET
	LoadIniCfg(1, 0);
#endif	
#ifdef QT_GUI
    initBitChain();
#endif

    if (!NewThread(StartNode, NULL))
        InitError(_("Error: could not start node"));

    if (fServer)
        NewThread(ThreadRPCServer, NULL);

    // ********************************************************* Step 12: finished

    uiInterface.InitMessage(_("Done loading"));
    printf("Done loading\n");

#ifdef USE_SQLITE
   openSqliteDb();
#endif

    if (!strErrors.str().empty())
        return InitError(strErrors.str());

     // Add wallet transactions that aren't already in a block to mapTransactions
    pwalletMain->ReacceptWalletTransactions();

#if !defined(QT_GUI)
    // Loop until process is exit()ed from shutdown() function,
    // called from ThreadRPCServer thread when a "stop" command is received.
    while (1)
        MilliSleep(5000);
#endif

    return true;
}
