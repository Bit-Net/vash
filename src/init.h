// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_INIT_H
#define BITCOIN_INIT_H

#include "wallet.h"

extern unsigned int bFastsyncblockMode;
extern string s_BlockChain_Dir;
extern string s_fastSyncBlock_ini;
extern string s_bestHashForFastSync;
extern uint256 u256bestHashForFastSync;
extern uint256 preHashForFastSync;
extern unsigned int bFastsyncblockMode;
extern unsigned int bFastsyncBestHei;
extern uint64_t nFastStakeModifier;
extern unsigned int nFastStakeModifierChecksum;
extern unsigned int nFastPreStakeModifierChecksum;
extern int iFastsyncblockModeArg;
extern int iFastSyncBlockHeiOk;
extern int dw_Fast_Sync_Block_Active_Height;
extern const string sBitChain_ident;

extern CWallet* pwalletMain;
void StartShutdown();
void Shutdown(void* parg);
bool AppInit2();
std::string HelpMessage();

#endif
