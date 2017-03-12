// Copyright (c) 2017 The Shenzhen BitChain Technology Company
// Copyright (c) 2017 Vpncoin development team, Bit Lee
#include "zkpdialog.h"
#include "ui_zkpdialog.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "main.h"
#include <QApplication>
#include <QPixmap>
#include <QUrl>
#include <QFileDialog>
#include<QScrollArea>
#include <QToolBar>
#include <QDesktopServices>
#include <QClipboard>
#include <QWebPage>
#include <QWebFrame>
#include <qrencode.h>
#include "util.h"
#include "txdb.h"
#include "walletdb.h"
#include "init.h"
#include "lzma/LzmaLib.h"
#include "zero-knowledge-proof.h"
#include "base58.h"
#include "key.h"
#include <QStringList>
#include <QMetaType>
#include <QInputDialog>

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>
#include <QtWebKit/QWebSettings>

using namespace std;
using namespace boost;

extern int StreamToBuffer(CDataStream &ds, string& sRzt, int iSaveBufSize);
int CZeroKnowledgeProofToBuffer(CZeroKnowledgeProof *pb, string& sRzt)
{
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*pb);
	int bsz = StreamToBuffer(ssBlock, sRzt, 0);
	return bsz;
}
int CZeroKnowledgeProofFromBuffer(CZeroKnowledgeProof* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	ssBlock >> (*block);
	return i;
}

int CZeroKnowledgeProofMintKeyToBuffer(CZeroKnowledgeProofMintKey *pb, string& sRzt)
{
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*pb);
	int bsz = StreamToBuffer(ssBlock, sRzt, 0);
	return bsz;
}
int CZeroKnowledgeProofMintKeyFromBuffer(CZeroKnowledgeProofMintKey* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	ssBlock >> (*block);
	return i;
}

void ZkpForm::setMaxValues()
{
	int64_t i6Amount = 0;
	if( GetMaxZkpMintCoinAmount(nBestHeight, i6Amount) )
	{
		ui->spb_BurnAmount->setMaximum(i6Amount / COIN);
		ui->sb_gMintAmount->setMaximum(i6Amount / COIN);
	}
}

ZkpForm::ZkpForm(QWidget *parent) : QWidget(parent), ui(new Ui::ZkpForm), model(0)
{
    ui->setupUi(this);
	ui->label_16->setVisible(false);   ui->label_6->setVisible(false);   ui->le_m_toAddress->setVisible(false);   ui->label_17->setVisible(false);
	ui->label_7->setVisible(false);   ui->le_m_MintKey->setVisible(false);   ui->label_18->setVisible(false);   ui->label_8->setVisible(false);
	ui->sb_m_amount->setVisible(false);   ui->lab_err->clear(); //ui->label_16->setVisible(false);   ui->label_16->setVisible(false);   ui->label_16->setVisible(false);
    //ui->lab_setLoveImg->installEventFilter(this);
    //connect( ui->web_blessChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
}

ZkpForm::~ZkpForm()
{
    delete ui;
}

void ZkpForm::on_pb_newPubKey_clicked()
{
	string sAddr = getNewAddress("");
	ui->le_b_pubkey->setText( QString::fromStdString(sAddr) );
}

void ZkpForm::on_pb_g_new_clicked()
{
	string sAddr = getNewAddress("");
	ui->le_g_toAddress->setText( QString::fromStdString(sAddr) );
}

void ZkpForm::firstEnter()
{
	on_pb_newPubKey_clicked();      this->setMaxValues();
}
void ZkpForm::on_pb_burn_clicked()
{
	//if( IsZkpRuleActived(nBestHeight) )
	{
		int iBurnCoins = ui->spb_BurnAmount->value();
		if( iBurnCoins < (int)(Min_Burn_Coin_Amount / COIN) ){ ui->spb_BurnAmount->setFocus();  return; }
		string sPubKey = ui->le_b_pubkey->text().toStdString(), sZkpHash = "";
		if( IsValidBitcoinAddress(sPubKey) )
		{
			bool b = zkpBurn(iBurnCoins, sPubKey, sZkpHash);  //if( zkpBurn(iBurnCoins, sPubKey, sZkpHash) )
			{
				if( b ){ ui->le_burnProof->setText( QString::fromStdString(sZkpHash) );   ui->le_zkpForMintKey->setText( QString::fromStdString(sZkpHash) ); }
				else{ ui->le_burnProof->setText( tr(sZkpHash.c_str()) ); }
				ui->le_burnProof->setFocus();   ui->le_burnProof->selectAll();      if( b ){ ui->sb_gMintAmount->setValue(iBurnCoins);     ui->lab_err->clear(); }
			}
			on_pb_newPubKey_clicked();
			if( b ){ on_pb_genKey_clicked(); }
			else{  ui->lab_err->setText( tr(sZkpHash.c_str()) ); }
		}else{ ui->le_b_pubkey->setFocus(); }
		this->setMaxValues();
	}
}

int getZeroKnowledgeProofMintOutPack(const std::string betStr, ZeroKnowledgeProofMintOutPack& zmkp)
{
	int rzt = 0;   // ZKP-Mint | ZKP Hash | To Address | Amount | Mint Key (88 chars)
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
			else if( i == 3 ){ zmkp.sToAddress = pch; }
			else if( i == 4 ){ zmkp.nAmount = atoi(pch); }
			else if( i == 5 ){ zmkp.sMintKey = pch; }
			else if( i > 6 ){ break; }
			pch = strtok_r(NULL, delim, &reserve);
		}
		}catch (std::exception &e) {
			printf("getZeroKnowledgeProofMintOutPack:: err [%s]\n", e.what());
		}catch (...) {
            //PrintExceptionContinue(NULL, "getZeroKnowledgeProofMintOutPack()");
        }
		rzt = i;
	}
	return rzt;
}

void ZkpForm::on_pb_genKey_clicked()
{
	int iBurnCoins = ui->sb_gMintAmount->value();
	string sZkpHash = ui->le_zkpForMintKey->text().toStdString(), sToAddress = ui->le_g_toAddress->text().toStdString(), sRztMintKey="";
	if( IsValidBitcoinAddress(sToAddress) )	//if( sToAddress.length() >= 34 )
	{
		int64_t i6Amount = iBurnCoins * COIN;
		bool b = genZkpMintKey(sZkpHash, i6Amount, sToAddress, sRztMintKey);
		if( b ){
			string s = strprintf("%s|%s|%s|%s|%s", sZKPMint_Flags.c_str(), sZkpHash.c_str(), sToAddress.c_str(), inttostr(iBurnCoins).c_str(), sRztMintKey.c_str());
			sRztMintKey = s;      ui->le_m_zkpHash->setText( QString::fromStdString(sRztMintKey) );      ui->lab_err->clear();
		}
		ui->le_gMintKey->setText( QString::fromStdString(sRztMintKey) );   ui->le_gMintKey->setFocus();   ui->le_gMintKey->selectAll();
	}
}

void ZkpForm::on_pb_Mint_clicked()
{
	ZeroKnowledgeProofMintOutPack zmop{"", "",  0, ""};      string sMintOutKey = ui->le_m_zkpHash->text().toStdString();
	int k = getZeroKnowledgeProofMintOutPack(sMintOutKey, zmop);
	if( fDebug ){ printf("on_pb_Mint_clicked: k=%d, [%s] \n", k, sMintOutKey.c_str()); }
	if( k < 5 ){ ui->le_m_zkpHash->clear();   return; }
	//string sZkpHash = ui->le_m_zkpHash->text().toStdString(), sToAddress = ui->le_m_toAddress->text().toStdString(), sMintKey=ui->le_m_MintKey->text().toStdString();
	string sZkpHash = zmop.sZkpHash, sToAddress = zmop.sToAddress, sMintKey=zmop.sMintKey;      int iMintCoins = zmop.nAmount;  // ui->sb_m_amount->value(); 
	if( sToAddress.length() >= 34 )
	{
		int64_t nAmount = iMintCoins * COIN;
		CBitcoinAddress address(sToAddress);
		if( address.IsValid() )
		{
		//if( !isValidZkpMintKey(nAmount, sToAddress, const string& sProofStr, sMintKey) ){ return; }
		std::vector<std::pair<CScript, int64_t> > aVecSend;
		CScript scriptPubKey;
		scriptPubKey.SetDestination(address.Get());
		aVecSend.push_back( make_pair(scriptPubKey, nAmount) );
		string sRztTxHash = "";
		bool b = zkpmint(sZkpHash, aVecSend, sMintKey, sRztTxHash);
		if( b )
		{
			ui->le_m_zkpHash->clear();     ui->le_m_toAddress->clear();     ui->le_m_MintKey->clear();     ui->lab_err->clear();
		}else{ ui->lab_err->setText( tr(sRztTxHash.c_str()) ); }
		}
	}
}

void ZkpForm::on_le_zkpForMintKey_textChanged()
{
	bool b=false;    string sZkpHash = ui->le_zkpForMintKey->text().toStdString();      CZeroKnowledgeProof zkp;
	if( ReadZeroKnowledgeProof(sZkpHash, zkp) )
	{
		CBitcoinAddress address(zkp.nProofKeyID);
		if( address.IsValid() )
		{
			CTxDestination dest = address.Get();
			b = IsMine(*pwalletMain, dest);
		}
	}
	ui->pb_genKey->setEnabled(b);
}

bool ZkpForm::expZkpBurnTx(const CTransaction& tx, bool checkExists, QString& qRztZkpHash)
{
      string sRztZkpHash = "";     bool bExists=false;
	  bool rzt = IsZkpBurnTx(tx, checkExists, sRztZkpHash, bExists);
	  if( rzt )
	  {
			CZeroKnowledgeProof cZkp;      bool b = ReadZeroKnowledgeProof(sRztZkpHash, cZkp);
			qRztZkpHash = tr("Public Key:") + " " + QString::fromStdString(tx.vpndata) + ", " + tr("ZKP Hash:") + " " + QString::fromStdString(sRztZkpHash);
			if( b ){
				int64_t i6b=cZkp.GetBalance();      if( i6b > 0 ){ i6b = i6b / COIN; }
				QString qq = tr("Balance:") + QString::fromStdString( int64tostr(i6b) ) + "," + tr("Total:") + QString::fromStdString( int64tostr(cZkp.n6Coins / COIN) );
				qRztZkpHash = qq + ", " + qRztZkpHash;
			}
			if( bExists ) { }  //qRztZkpHash = qRztZkpHash + tr("Valid");
			else qRztZkpHash = qRztZkpHash + ", " + tr("Invalid");
	  }
	  return rzt;
}

bool ZkpForm::expZkpMintTx(const CTransaction& tx, QString& qRztZkpMintInfo)
{
	bool rzt=false;     qRztZkpMintInfo="";
	CZeroKnowledgeProofMintKey czmk;      CZeroKnowledgeProof czkp;
	int k = getCZKPAndCZKPMintKeyFromTx(tx, czkp, czmk);
	if( k > 0 )  // = 2
	{
		rzt=true;
		qRztZkpMintInfo = tr("ZKP Hash:") + " " + QString::fromStdString(czkp.GetHashString()) + ", " + tr("Public Key:") + " " + QString::fromStdString(czkp.GetProofKeys());
		if( k > 1 )
		{
			qRztZkpMintInfo = qRztZkpMintInfo + ", " + tr("Mint Key:") + " " + QString::fromStdString(czmk.nMintKeyStr);
		}else{ qRztZkpMintInfo = qRztZkpMintInfo + ", " + tr("Invalid mint key :("); }
	}
	if( fDebug ){  printf("expZkpMintTx(%s):: rzt=[%d], k=[%d], [%s] \n", tx.GetHash().ToString().c_str(), rzt, k, tx.vpndata.c_str()); }
	return rzt;
}	
