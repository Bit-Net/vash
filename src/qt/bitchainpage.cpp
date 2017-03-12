// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee
#include "bitchainpage.h"
#include "ui_bitchainpage.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "main.h"
#include "bitchain.h"
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
#include "mystyle.h"
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

#ifdef USE_MYSQL
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <boost/algorithm/string/replace.hpp>
#endif
#include <QMessageBox>
#include "qMsg.h"

using namespace std;
using namespace boost;

// BitChain-Service   VruEthNLt4ZkhfRPE8B7AHrnVxhMJvj63h
string sBitChain_Service_Addr = "";
extern std::string dumpPrivaKey(string strAddress);
extern QString lang_territory;
extern double DOB_DPI_RAT;
extern string sQkl_domain;  // www.qkl.io
extern int iBitChainService;
QWidget* BitChainheader = NULL;
extern QString qScrollBarStyleStr;
extern string sBitChainIdentAddress;
extern string buildSignTimeStr(int64_t tm);
extern void SetWebViewOpt(QWebView *aWebView, bool bNoContextMenu, bool bCookie);
#ifdef USE_MYSQL
extern QSqlDatabase mySqlDb;
extern bool mySqlOpened;
#endif
std::string sBitChainDecryptKey = "",  sBitChainDatDir = "";

extern unsigned int uint_256KB;  // = 256 * 1024;
extern string s_BlockChain_Dir;
extern string sBitChainKeyAddress;
extern std::string BitChain_Head;
extern int StreamToBuffer(CDataStream &ds, string& sRzt, int iSaveBufSize);
extern int writeBufToFile(char* pBuf, int bufLen, string fName);
extern int lzma_pack_buf(unsigned char* pBuf, int bufLen, string& sRzt, int iLevel, unsigned int iDictSize);
extern int lzma_depack_buf(unsigned char* pLzmaBuf, int bufLen, string& sRzt);
extern int getFileBinData(string sFile, string& sRzt);

CCriticalSection cs_bitchain;
int nAesBitChain = 0;
QString targetLoveWallet="", targetBabyWallet="", targetBlessWallet="", targetEventWallet="", targetProphesyWallet="", targetContractWallet="", targetMedicalWallet="", targetForumWallet ="";
int iRwdLoveRatio=0, iRwdBabyRatio=0, iRwdBlessRatio=0, iRwdEventRatio=0, iRwdProphesyRatio=0, iRwdContractRatio=0, iRwdMedicalRatio=0, iRwdForumRatio=0;

//BitchainPage *bitchainPage = NULL;
boost::signals2::signal<void (const CTransaction& tx, const string txMsg)> NotifyReceiveBitChainMsg;
boost::signals2::signal<void (const string txID)> NotifyReceiveBitChainMsgOne;
boost::signals2::signal<void (Widget* pMsg)> NotifyFreeWidget;

void resetBitChainRewardParam(int pid)
{
    if( pid == 1 ){ targetLoveWallet="";   iRwdLoveRatio=0; }
	else if( pid == 2 ){ targetBabyWallet="";   iRwdBabyRatio=0; }
	else if( pid == 3 ){ targetBlessWallet="";   iRwdBlessRatio=0; }
	else if( pid == 4 ){ targetEventWallet="";   iRwdEventRatio=0; }
	else if( pid == 5 ){ targetProphesyWallet="";   iRwdProphesyRatio=0; }
	else if( pid == 6 ){ targetContractWallet="";   iRwdContractRatio=0; }
	else if( pid == 7 ){ targetMedicalWallet="";   iRwdMedicalRatio=0; }
	else if( pid == 8 ){ targetForumWallet="";   iRwdForumRatio=0; }
}

void ShowModMsgBox(QString qTitle, QString qText)
{
    QMessageBox mbox;
    mbox.setWindowTitle( qTitle );   mbox.setText( qText );
    mbox.exec();  //Ä£Ì¬¶Ô»°¿ò
}

int CBitChainProtocolToBuffer(CBitChainProtocol *pb, string& sRzt)
{
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*pb);
	int bsz = StreamToBuffer(ssBlock, sRzt, 0);
	return bsz;
}
int CBitChainProtocolFromBuffer(CBitChainProtocol* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	ssBlock >> (*block);
	return i;
}

int CBitChainToBuffer(CBitChain *pb, string& sRzt)
{
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock << (*pb);
	int bsz = StreamToBuffer(ssBlock, sRzt, 0);
	return bsz;
}
int CBitChainFromBuffer(CBitChain* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	ssBlock >> (*block);
	return i;
}
/*int CBitChainFromBuffer(CBitChain* block, char* pBuf, int bufLen)
{
    //vector<char> v(bufLen);
	//memcpy((char*)&v[0], pBuf, bufLen);
	CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
	ssBlock.write(pBuf, bufLen);   int i = ssBlock.size();
	ssBlock >> (*block);
	return i;
}*/

int CBitChainFromLzmaBuffer(CBitChain* block, char* pBuf, int bufLen)
{
    int rzt = 0;
    string sDe;
	int lzRzt = lzma_depack_buf((unsigned char*)pBuf, bufLen, sDe);
    if( lzRzt > 0 )
	{
	    rzt = CBitChainFromBuffer(block, (char*)sDe.c_str(), lzRzt);
	}
    sDe.resize(0);	
	return rzt;
}


std::string sBitChainTeamAddress = "";
void initBitChain()  // call from init.cpp
{
	sBitChainTeamAddress = strprintf(">%s|%s|%s|", "VuwRCMUeDJ9XZTPfqbyudw13AKRscyrFBV", "VqhRfpL6ywhcDZDgN5jErPwhrKpFzEKaLd", "VcqAHYTTpicXZhQu4kPBJbDKbigRo6wDme");
	sBitChain_Service_Addr = GetArg("-bitchainservicewallet", "VruEthNLt4ZkhfRPE8B7AHrnVxhMJvj63h");     nAesBitChain = GetArg("-aesbitchain", 0);
    sBitChainDecryptKey = GetArg("-bitchaindecryptkey", "");     sBitChainDatDir = GetArg("-bitchaindatdir", "");
	if( sBitChainDecryptKey.length() < 32 ){ sBitChainDecryptKey = sBitChainKeyAddress; }
	if( sBitChainDatDir.length() < 4 ){  sBitChainDatDir = s_BlockChain_Dir; }
}

int bWebDevMode = 0;
#ifdef USE_WEBKIT
void loadWebSettings()
{
    //QSettings settings("bitnet_browser.ini",QSettings::IniFormat);
    //settings.beginGroup(QLatin1String("websettings"));

    QWebSettings *defaultSettings = QWebSettings::globalSettings();

    /*QString standardFontFamily = defaultSettings->fontFamily(QWebSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebSettings::DefaultFontSize);
    QFont standardFont = QFont(standardFontFamily, standardFontSize);
    standardFont = qvariant_cast<QFont>(settings.value(QLatin1String("standardFont"), standardFont));
    defaultSettings->setFontFamily(QWebSettings::StandardFont, standardFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFontSize, standardFont.pointSize());*/

    /*QString fixedFontFamily = defaultSettings->fontFamily(QWebSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebSettings::DefaultFixedFontSize);
    QFont fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedFont = qvariant_cast<QFont>(settings.value(QLatin1String("fixedFont"), fixedFont));
    defaultSettings->setFontFamily(QWebSettings::FixedFont, fixedFont.family());
    defaultSettings->setFontSize(QWebSettings::DefaultFixedFontSize, fixedFont.pointSize());*/
	
    defaultSettings->setAttribute(QWebSettings::JavascriptEnabled, true);
    defaultSettings->setAttribute(QWebSettings::PluginsEnabled, true);

    //QUrl url = settings.value(QLatin1String("userStyleSheet")).toUrl();
    //defaultSettings->setUserStyleSheetUrl(url);

    defaultSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    if( bWebDevMode > 0 ){defaultSettings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); }
	//else if( bNoContextMenu ){ aWebView->setContextMenuPolicy(Qt::NoContextMenu); }
    //settings.endGroup();
	
    //settings.beginGroup(QLatin1String("MainWindow"));
	//bDontAskCloseMoreTabsOnAppEnd = settings.value(QLatin1String("dontAskCloseTabsOnAppEnd"), true).toBool();	
    //settings.endGroup();	
}
#endif

BitchainPage* bitchainPG = NULL;
BitchainPage::BitchainPage(int activeTab, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BitchainPage),
    model(0),
    aTabId(activeTab)
{
    ui->setupUi(this);   //bitchainPage = this;
	qRegisterMetaType<std::string>();     //qRegisterMetaType<Widget *>('Widget');
	qRegisterMetaType<Widget*>("Widget*");
    subscribeToCoreSignals();

		char ba[9];
		ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'a'; ba[5] = 'd'; ba[6] = 'm'; ba[7] = 0;
		string s = ba;	//"-bewadm"
		bWebDevMode = GetArg(s.c_str(), 0);	//GetArg("-bewadm", 0);	

    ui->lab_setLoveImg->installEventFilter(this);
    ui->lab_setLoveMp3->installEventFilter(this);
    ui->lab_setKidImg->installEventFilter(this);
    ui->lab_setBlessImg->installEventFilter(this);
    ui->lab_setBlessMp3->installEventFilter(this);
    ui->lab_setEventImg->installEventFilter(this);
    ui->lab_setProphesyImg->installEventFilter(this);
    ui->lab_setProphesyMp3->installEventFilter(this);
    ui->lab_setContractImg->installEventFilter(this);
    ui->lab_setMedicalImg->installEventFilter(this);
    ui->lab_setForumImg->installEventFilter(this);
    ui->lab_setForumMp3->installEventFilter(this);

    iNeedPay = 1;     bitchainPG = this;
    //QString q = tr("If you type a parent topic id, mean that it is a reply, else it is new topic's title, it is optional");  //"Parent topic, it is optional");
    QString q = tr("New topic's title or parent topic id, it is optional");  //"Parent topic, it is optional");
    ui->lEdit_loveParentTopic->setPlaceholderText( q );
    ui->lEdit_kidsParentTopic->setPlaceholderText( q );
    ui->lEdit_blessParentTopic->setPlaceholderText( q );
    ui->lEdit_eventParentTopic->setPlaceholderText( q );
    ui->lEdit_prophesyParentTopic->setPlaceholderText( q );
    ui->lEdit_contractParentTopic->setPlaceholderText( q );
    ui->lEdit_medicalParentTopic->setPlaceholderText( q );
    ui->lEdit_forumParentTopic->setPlaceholderText( q );
    //q = tr("It is optional");
    //ui->lEdit_kidsUrl->setPlaceholderText( q );
    //ui->lEdit_eventLink->setPlaceholderText( q );   ui->lEdit_contractUrl->setPlaceholderText( q );   ui->lEdit_medicalUrl->setPlaceholderText( q );

    //ui->chain_tabWidget->tabBar()->hide();
    QTabBar *tabBar = ui->chain_tabWidget->findChild<QTabBar *>();
    if( tabBar ){ tabBar->hide(); }
    createActions();
    createToolBars();
    if( BitChainheader )
	{
        BitChainheader->installEventFilter(this);
	}
    connect(loveChainAction, SIGNAL(triggered()), this, SLOT(gotoLoveChain()));
    connect(kidsChainAction, SIGNAL(triggered()), this, SLOT(gotoKidsChain()));
    connect(blessChainAction, SIGNAL(triggered()), this, SLOT(gotoBlessChain()));
    connect(eventChainAction, SIGNAL(triggered()), this, SLOT(gotoEventChain()));
    connect(prophesyChainAction, SIGNAL(triggered()), this, SLOT(gotoProphesyChain()));
    connect(forumChainAction, SIGNAL(triggered()), this, SLOT(gotoForumChain()));
    connect(contractChainAction, SIGNAL(triggered()), this, SLOT(gotoContractChain()));
    connect(medicalChainAction, SIGNAL(triggered()), this, SLOT(gotoMedicalChain()));
    connect(regNickAction, SIGNAL(triggered()), this, SLOT(gotoRegNick()));
    connect(exportBitChainAction, SIGNAL(triggered()), this, SLOT(exportBitChain()));

    //ui->web_regNick->setZoomFactor( DOB_DPI_RAT );
    /*ui->web_loveChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_kidsChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_blessChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_eventChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_prophesyChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_contractChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_medicalChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
    ui->web_forumChain->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
*/

    loadWebSettings();
	SetWebViewOpt(ui->web_regNick, true, true);    SetWebViewOpt(ui->web_loveChain, true, true);
    SetWebViewOpt(ui->web_kidsChain, true, true);    SetWebViewOpt(ui->web_blessChain, true, true);
    SetWebViewOpt(ui->web_eventChain, true, true);    SetWebViewOpt(ui->web_prophesyChain, true, true);
    SetWebViewOpt(ui->web_contractChain, true, true);    SetWebViewOpt(ui->web_medicalChain, true, true);
    SetWebViewOpt(ui->web_forumChain, true, true);    //SetWebViewOpt(ui->web_regNick, true, true);

    //viewMenu->addAction(tr("Zoom &In"), this, SLOT(slotViewZoomIn()), QKeySequence(Qt::CTRL | Qt::Key_Plus));
    //viewMenu->addAction(tr("Zoom &Out"), this, SLOT(slotViewZoomOut()), QKeySequence(Qt::CTRL | Qt::Key_Minus));
	

    ui->web_regNick->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
    ui->web_loveChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_kidsChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_blessChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_eventChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_prophesyChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_contractChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_medicalChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->web_forumChain->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    //connect(this, SIGNAL(linkClicked(const QUrl &)), SLOT(linkIsClicked(const QUrl&)), Qt::DirectConnection);
    connect(ui->web_regNick, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_loveChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_kidsChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_blessChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_eventChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_prophesyChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_contractChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_medicalChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));
    connect(ui->web_forumChain, SIGNAL( linkClicked(const QUrl &) ), this, SLOT( handleWebviewClicked(QUrl) ));

    //connect( ui->web_regNick->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
	connect( ui->web_regNick->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared ()), this, SLOT(addJavaScriptObject()) );
    /* connect( ui->web_loveChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_kidsChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_blessChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_eventChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_prophesyChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_contractChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_medicalChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) );
    connect( ui->web_forumChain->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJavaScriptObject()) ); */
    //loveChainAction->trigger();
    //regNickAction->trigger();
    //ui->chain_tabWidget->setCurrentIndex(aTabId);
}

BitchainPage::~BitchainPage()
{
    unsubscribeFromCoreSignals();
    delete ui;
}

void BitchainPage::addJavaScriptObject(){
  //       ui->page()->mainFrame()->addToJavaScriptWindowObject(QString("screen"), interface);
   ui->web_regNick->page()->mainFrame()->addToJavaScriptWindowObject(QString("BitChain"), this);
}
void BitchainPage::bitGridClick()
{
    regNickAction->trigger();
}
void BitchainPage::buyDomain(QString sDomain, QString sAddress, QString sCoin)
{
    //vpncoin:VuwRCMUeDJ9XZTPfqbyudw13AKRscyrFBV?amount=10.00&label=BitChain-ident
	QString s = "BitNet:" + sAddress + "?amount=" + sCoin + "&label=BitChain-Domain&stype=2&message=" + sDomain;  //QString::number(iCoin, 10)
	QApplication::clipboard()->setText( s );
}
void BitchainPage::handleWebviewClicked(QUrl url)
{
    QString qOut = "out";     //url.host();
	QString qo = url.queryItemValue(qOut);     int bOut = qo.toInt();
	if( bOut > 0 )
	{
		url.removeQueryItem(qOut);     QDesktopServices::openUrl( url );
	}else{
		if(aTabId == 0 ){ ui->web_regNick->load(url); }
		else if(aTabId == 1 ){ ui->web_loveChain->load(url); }
		else if(aTabId == 2 ){ ui->web_kidsChain->load(url); }
		else if(aTabId == 3 ){ ui->web_blessChain->load(url); }
		else if(aTabId == 4 ){ ui->web_eventChain->load(url); }
		else if(aTabId == 5 ){ ui->web_prophesyChain->load(url); }
		else if(aTabId == 6 ){ ui->web_contractChain->load(url); }
		else if(aTabId == 7 ){ ui->web_medicalChain->load(url); }
		else if(aTabId == 8 ){ ui->web_forumChain->load(url); }
	}

/*     //if (url.ishtml()//isHtml does not exist actually you need to write something like it by yourself
    //     window->load (url);
    //else//non html (pdf) pages will be opened with default application
    QDesktopServices::openUrl( url ); */
}

void BitchainPage::slotViewZoomIn()
{
    //if (!currentTab()) return;
    //currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
}

void BitchainPage::slotViewZoomOut()
{
    //if (!currentTab()) return;
    //currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
}

int GetTransactionByTxID(const string& txID, CTransaction& tx, int iCheckHashBlock = 1)
{
    int rzt = 0;
	if( txID.length() < 34 ){ return rzt; }
	
    uint256 hash;
    hash.SetHex(txID);	//params[0].get_str());

    //CTransaction tx;
    uint256 hashBlock = 0;
    if( GetTransaction(hash, tx, hashBlock) )
    {
        if( iCheckHashBlock > 0 ){ if( hashBlock > 0 ){ rzt++; } }
		else{ rzt++; }
    }
    return rzt;
}

void BitchainPage::updateBitChainMsg(const CTransaction& tx, const string txMsg)
{
    LOCK(cs_bitchain);
}

extern bool verifyMessage(const string strAddress, const string strSign, const string strMessage);
//std::string getBitChainProtoDescribeStr(std::string sBcp, qint64 nTime, uint256 hash)
std::string getBitChainProtoDescribeStr(std::string sBcp)
{
//printf("--> getBitChainProtoDescribeStr:: [%s] [%I64u] [%s] \n", sBcp.c_str(), nTime, hash.ToString().c_str());
	string rzt = "";
	if( GetArg("-explainbitchaintx", 1) == 0 ){ return rzt; }
	//if( nTime < 1460605747 ){ return rzt; }  //1460937615
	CBitChainProtocol bcp;   char* pBuf = (char*)sBcp.c_str();   
	int k = sizeof(CBitChainProtocol);     int bufLen = sBcp.length(),     j = sBcp.find(BitChain_Head);
	if( (bufLen >= k) && (j == 1) )
	{
		//writeBufToFile(pBuf, bufLen, sBitChainDatDir + txID + ".bcp");
		//rzt.resize(0);     
		int i = CBitChainProtocolFromBuffer(&bcp, pBuf, bufLen);
//printf("getBitChainProtoDescribeStr:: [%d :: %d] [%d  :: %d] [%d :: %s]\n", bufLen, i, j, k, bcp.nFromAddr.length(), bcp.nFromAddr.c_str());
//printf("getBitChainProtoDescribeStr:: [%d :: %d] [%d  :: %d] \n", bufLen, i, j, k);
		try{
			if( i >= k )
			{
				if( bcp.nFromAddr.size() == 34 ){ rzt = "BitChain earlier version";  return rzt; }  // old version;
				//printf("--> getBitChainProtoDescribeStr:: [%s] [%I64u] [%s] \n", sBcp.c_str(), nTime, hash.ToString().c_str());
				CBitcoinAddress address;
			bool bAddrOk = address.Set(bcp.nFromAddrKey);
			string sFromAddr = "";     int iAES = bcp.nAES;
			if( bAddrOk ){ sFromAddr = address.ToString(); }
			//printf("getBitChainProtoDescribeStr:: [%s] \n", sFromAddr.c_str());
			string sChainDatType = "";
			if( bcp.nDatType == 1 ){ sChainDatType = "Love"; }
			else if( bcp.nDatType == 2 ){ sChainDatType = "Baby"; }
			else if( bcp.nDatType == 3 ){ sChainDatType = "Bless"; }
			else if( bcp.nDatType == 4 ){ sChainDatType = "Event"; }
			else if( bcp.nDatType == 5 ){ sChainDatType = "Prophesy"; }
			else if( bcp.nDatType == 6 ){ sChainDatType = "Contract"; }
			else if( bcp.nDatType == 7 ){ sChainDatType = "Medical"; }
			else if( bcp.nDatType == 8 ){ sChainDatType = "Forum"; }
			  //printf("getBitChainProtoDescribeStr:: [%s] \n", sChainDatType.c_str());
				if( iAES > 0 )
				{
					char* pOrg = (char*)bcp.nChainDat.c_str();
					string sAES;   sAES.resize(36);   char* pAes = (char*)sAES.c_str();
					*(int *)pAes = 32;     memcpy(pAes + 4, pOrg, 32);
					unsigned char* pDecryptKey = (unsigned char*)sBitChainDecryptKey.c_str();
					if( sBitChainIdentAddress == sFromAddr ){ pDecryptKey = (unsigned char*)sBitChainKeyAddress.c_str(); }  // is encrypt by self
					AES aes(Bits256, pDecryptKey);
					string sOrg = "";
					int ii = aes.InvCipher(sAES, sOrg);
					if( ii == 32 )
					{
						pAes = (char*)sOrg.c_str();     memcpy(pOrg, pAes, 32); 
					}
					sAES.resize(0);     sOrg.resize(0);
				}
				//if( (bcp.nChainDat.size() >0) && (sFromAddr.size() > 32) && (bcp.nSignTxt.size() > 34) )
				string e64 = EncodeBase64(bcp.nSignTxt);
				k = bcp.nChainDat.length();     bool bSign = verifyMessage(sFromAddr, e64, bcp.nChainDat);
				//printf("getBitChainProtoDescribeStr:: [%d :: %d] [%s] \n", k, bSign, e64.c_str());
				
				rzt = strprintf("BitChain Protocol:  Ver [%d], Create time [%d], AES [%d], Data type [%s],\n", bcp.nVersion, bcp.nTime, iAES, sChainDatType.c_str());
				rzt = strprintf("%s From Address [%s, valid %s],\n", rzt.c_str(), sFromAddr.c_str(), bAddrOk ? "true" : "false");
				rzt = strprintf("%s Sign [%s, valid %s],\n", rzt.c_str(), e64.c_str(), bSign ? "true" : "false");
				if( bSign )
				{
					char *pLzma = (char*)bcp.nChainDat.c_str();
					CBitChain bitChain;
					int iDz = CBitChainFromLzmaBuffer(&bitChain, pLzma, k);
					//printf("getBitChainProtoDescribeStr::  iDz [%d :: %d] [%s] [%s]\n", iDz, k, bitChain.GetHash().ToString().c_str(), bcp.bitChainHash.ToString().c_str());
					if( bitChain.GetHash() == bcp.bitChainHash )
					{
						rzt = strprintf("%s Name [%s], Topic [%s], Have Image [%s], Have Voice [%s], \n", rzt.c_str(), bitChain.nNickName.c_str(), bitChain.nTopic.c_str(), bitChain.sImgData.length() ? "yes" : "no", bitChain.sVoiceData.length() ? "yes" : "no");
						//rzt = strprintf("%s Have Image [%s], Have Voice [%s], \n", rzt.c_str(), 
						rzt = strprintf("%s [%s] \n External Url [%s]", rzt.c_str(), bitChain.nContentText.c_str(), bitChain.sExternalChainUrl.c_str());
					}
				}
				//rzt = strprintf("BitChain Protocol: \n Ver [%d], Create time [%I64u], AES [%d], Hash [%s], \n From Address [%s, valid %s],  Data type [%s], Data size [%d], \n Sign [%s, valid %s]", 
			    //                    bcp.nVersion, bcp.nTime, bcp.nAES, bcp.GetHash().ToString().c_str(), 
				//					sFromAddr.c_str(), bAddrOk ? "true" : "false", sChainDatType.c_str(), k, 
				//					e64.c_str(), b ? "true" : "false");

			}else rzt = "Invalid BitChain Protocol :(";
		}catch (runtime_error& exc) {
			//string se = string( exc.what() );
			//printf("getBitChainProtoDescribeStr runtime_error [%s]\n", se.c_str()); 		
		}
		catch (std::exception& e) {
			//string str = string( e.what() );
			//printf("getBitChainProtoDescribeStr exception [%s]\n", str.c_str()); 
		}
		catch (...)
		{
			// someone threw something undecypherable
		}
	}
//printf("<-- getBitChainProtoDescribeStr:: [%s] \n", sBcp.c_str());
	return rzt;
}

extern int iShowBitChainTipMsg;
string sNewLine = "\r\n\r\n";
int exportBitChainTx(std::string txID, string sAesKey, int iCheckHashBlock, int txMust2MeOrFromTeam, int saveBcpFile, int saveTxtFile, int saveImgFile, int saveMp3File, int showTipBox, int saveToMysql, int mkDir, BitchainPage* bp)
{
	int rzt = 0;
	CTransaction tx;
	if( GetTransactionByTxID(txID, tx, iCheckHashBlock) > 0 )
	{
		try{
			//ShowModMsgBox("updateBitChainMsgOne", "GetTransactionByTxID > 0");
			char* pBuf = (char*)tx.vpndata.c_str();   int bufLen = tx.vpndata.length();
			if( bufLen >= sizeof(CBitChainProtocol) )
			{
			    string saveInDir = sBitChainDatDir + txID;
				if( mkDir > 0 ){   boost::filesystem::create_directory(saveInDir);   saveInDir = saveInDir + "/BitChain"; }
				if( saveBcpFile > 0 ){ writeBufToFile(pBuf, bufLen, saveInDir + ".bcp"); }
			    CBitChainProtocol bcp;     int i = CBitChainProtocolFromBuffer(&bcp, pBuf, bufLen);
			    int iAES = bcp.nAES;
			    CBitcoinAddress address;
			    bool bAddrOk = address.Set(bcp.nFromAddrKey);
			    string sFromAddr = "";
			    if( bAddrOk ){ sFromAddr = address.ToString(); }
			    if( fDebug ){ printf("--> CBitChainProtocolFromBuffer:: i = [%d], bufLen = [%d], AES = [%d], AddrOk =[%d] from [%s] \n", i, bufLen, iAES, bAddrOk, sFromAddr.c_str()); }  //--> CBitChainProtocolFromBuffer:: i = [4635], bufLen = [4635] 
			    if( (i > 34) && (bcp.nChainDat.size() >0) && (sFromAddr.size() > 32) && (bcp.nSignTxt.size() > 34) )
			{
				bool bNeedProcess = true;
				if( txMust2MeOrFromTeam > 0 )
				{
				    bool bMyTx = pwalletMain->IsMine(tx);
					//bool bFromBitChainTeamAddr = (sFromAddr == "VuwRCMUeDJ9XZTPfqbyudw13AKRscyrFBV") || (sFromAddr == "VqhRfpL6ywhcDZDgN5jErPwhrKpFzEKaLd") || (sFromAddr == "VcqAHYTTpicXZhQu4kPBJbDKbigRo6wDme");
				    bool bFromBitChainTeamAddr = sBitChainTeamAddress.find(sFromAddr) > 0;
				    bNeedProcess = bMyTx || bFromBitChainTeamAddr;
				}
				if( bNeedProcess )
				{
				    if( iAES > 0 )
				    {
					    char* pOrg = (char*)bcp.nChainDat.c_str();
					    string sAES;   sAES.resize(36);   char* pAes = (char*)sAES.c_str();
					    *(int *)pAes = 32;     memcpy(pAes + 4, pOrg, 32);
					    unsigned char* pDecryptKey = (unsigned char*)sAesKey.c_str();   //sBitChainDecryptKey.c_str();
					    if( sBitChainIdentAddress == sFromAddr ){ pDecryptKey = (unsigned char*)sBitChainKeyAddress.c_str(); }  // is encrypt by self
					    AES aes(Bits256, pDecryptKey);
					    string sOrg = "";
					    int i = aes.InvCipher(sAES, sOrg);
					    if( i == 32 )
					    {
						    pAes = (char*)sOrg.c_str();     memcpy(pOrg, pAes, 32); 
					    }
					    sAES.resize(0);     sOrg.resize(0);
				    }
				    string e64 = EncodeBase64(bcp.nSignTxt);
				    int k = bcp.nChainDat.length();     char *pLzma = (char*)bcp.nChainDat.c_str();
				    bool b = verifyMessage(sFromAddr, e64, bcp.nChainDat);
				    //writeBufToFile(pLzma, k, saveInDir + ".7zp");
				    //CBitChainProtocol data size = [4506], hash = [5b29168dc7aa95150d92c957cc6ee526b89f7be00f1793b221205a45fbdead4f], b =[1], wallet = [VcqAHYTTpicXZhQu4kPBJbDKbigRo6wDme], signTxt = [INPF0dpTFV8zvpfNECyfD6g+pkf6zLjDdLmh4BbqomZC7PHBbCdcj++W0o21s5pc69qgKRE2x8SO0CyOsluqdc4=] 
				    if( fDebug ){ printf("CBitChainProtocol data size = [%d], hash = [%s], b =[%d], wallet = [%s], signTxt = [%s] \n", k, bcp.GetHash().ToString().c_str(), b, sFromAddr.c_str(), e64.c_str()); }
				    if( b )
				    {
					    CBitChain bitChain;
					    int iDz = CBitChainFromLzmaBuffer(&bitChain, pLzma, k);
					    //CBitChainFromLzmaBuffer:: iDz  = 1153, Hash = [952a29291bc3f489b1a949190ff33194ee4044c44f73c3530e224e1b08fb7d97] 
				        if( fDebug ){ printf("CBitChainFromLzmaBuffer:: iDz  = %d, Hash = [%s :: %s] [%d : %d] \n", iDz, bcp.bitChainHash.ToString().c_str(), bitChain.GetHash().ToString().c_str(), bitChain.sImgData.length(), bitChain.sVoiceData.length()); }
				        //string sBitChain = "";
				        //int iBitChainSz = CBitChainToBuffer(&bitChain, sBitChain);
				        //writeBufToFile((char*)sBitChain.c_str(), sBitChain.size(), saveInDir + ".bitchain");
						QString qTx = QString::fromStdString( txID );
#ifdef QT_GUI
					    if( showTipBox > 0  )
					    {
						    QString tt = QString::fromStdString( bitChain.nTopic ), ct = QString::fromStdString( bitChain.nContentText );
						    Widget* qMsg = new Widget;     qMsg->setAttribute(Qt::WA_DeleteOnClose, true);
						    qMsg->setMsg(tt, ct, qTx, bp, bitChain.nChainType);     qMsg->showAsQQ();
					    }
#endif
				        string sImg="", sMp3 = "";     char* p = NULL;     int i = 0;
						if( saveTxtFile > 0 )
						{
							sImg = bitChain.nNickName + sNewLine + bitChain.nTopic + sNewLine + bitChain.nContentText + sNewLine + bitChain.sExternalChainUrl;
							i = sImg.length();   writeBufToFile((char*)sImg.c_str(), i, saveInDir + ".txt");
							sImg.resize(0);   sImg = "";
						}
					    if( saveImgFile > 0 )
					    {
				            i = bitChain.sImgData.length();
				            if( i > 10 )
				            {
					            p = (char*)bitChain.sImgData.c_str();
					            string sn = "";
					            if( (p[0] == 'B') && (p[1] == 'M') ){ sn = ".bmp"; }
					            else if( (p[0] == 'G') && (p[1] == 'I') ){ sn = ".gif"; }
					            else if( (p[1] == 'P') && (p[2] == 'N') ){ sn = ".png"; }
					            else{ sn = ".jpg"; }
					            sImg = sn;
					            writeBufToFile(p, i, saveInDir + sn);
				            }
					    }
					    if( saveMp3File > 0 )
					    {
					        p = NULL;     i = bitChain.sVoiceData.length();
					        if( i > 10 )
					        {
						        p = (char*)bitChain.sVoiceData.c_str();
						        sMp3 = ".mp3";
						        writeBufToFile(p, i, saveInDir + sMp3);
					        }
					    }
#ifdef USE_MYSQL
					    //if( saveToMysql > 0 ){ submitBitChainToMySql(qTx, bitChain, sImg, sMp3, sFromAddr); }
#endif
                        rzt++;
				    }
			    }
			}
		  }
		}catch (std::exception& e) {
			//PrintException(&e, "ThreadVPN_Chat2()");
			string str = string( e.what() );
			printf("exportBitChainTx except [%s]\n", str.c_str()); 
		}
		catch (...)
		{
			// someone threw something undecypherable
		}
	}//else if( fDebug ){ printf("<--exportBitChainTx:: [%s] not valid :(\n", txID.c_str()); }
	return rzt;
}

void BitchainPage::updateBitChainMsgOne(std::string txID)
{
    //LOCK(cs_bitchain);   
	//if( fDebug ){ printf("-->updateBitChainMsgOne:: [%s] [%d] \n", txID.c_str(), sizeof(CBitChainProtocol)); }  // sizeof(CBitChainProtocol) = 88
	bool bNeedProcess = iBitChainService > 0;
    if( (!bNeedProcess) && (iShowBitChainTipMsg == 0) ){ return; }

	int iCheckHashBlock = 1, txMust2MeOrFromTeam = 1, saveBcpFile = 0, saveTxtFile = 0, saveImgFile = 0, saveMp3File = 0, showTipBox = 1, saveToMysql = 0, mkDir = 0;
#ifdef USE_MYSQL
    showTipBox = 0;   iCheckHashBlock = 0;   saveBcpFile++;   saveImgFile++;   saveMp3File++;   saveToMysql++;
#endif
	int i = exportBitChainTx(txID, sBitChainDecryptKey, iCheckHashBlock, txMust2MeOrFromTeam, saveBcpFile, saveTxtFile, saveImgFile, saveMp3File, showTipBox, saveToMysql, mkDir, this);
	//printf( "exportBitChainTx rzt = [%d] \n", i);
}

static void NotifyReceiveBitChainMsg2(BitchainPage *bitchainPage, const CTransaction& tx, const string txMsg)
{
    //OutputDebugStringF("NotifyTransactionChanged %s status=%i\n", hash.GetHex().c_str(), status);
	//if( bitchainPage )
	{
		QMetaObject::invokeMethod(bitchainPage, "updateBitChainMsg", Qt::QueuedConnection, 
                              Q_ARG(const CTransaction&, tx), Q_ARG(const string, txMsg));
	}
}

// resubmitbitchaintx c0b6c942e09548b92530df9ba8528732fb6cb245f6b89dafb9b0ca173ecce9eb
static void NotifyReceiveBitChainMsgOne_(BitchainPage *bitchainPage, const string txID)
{
	//printf("NotifyReceiveBitChainMsgOne_  [%s]  [%u]\n", txID.c_str(), bitchainPage);
	if( !QMetaObject::invokeMethod(bitchainPage, "updateBitChainMsgOne", Qt::QueuedConnection, Q_ARG(std::string, txID)) )
	{
		//ShowModMsgBox("NotifyReceiveBitChainMsgOne_", "invokeMethod faile :( ");
		//printf("NotifyReceiveBitChainMsgOne_  invokeMethod faile :(\n");
	}
}

int exportBitChainTx(QString qTx)
{
    int rzt = 0;
    if( qTx.length() < 34 ){ return rzt; }
	string txID = qTx.toStdString();
    QString qKey = qTx;
	int iCheckHashBlock = 1, txMust2MeOrFromTeam = 0, saveBcpFile = 0, saveTxtFile = 1, saveImgFile = 1, saveMp3File = 1, showTipBox = 1, saveToMysql = 0, mkDir = 1;
#ifdef USE_MYSQL
    //showTipBox = 0;   iCheckHashBlock = 0;   saveBcpFile++;   saveImgFile++;   saveMp3File++;   saveToMysql++;
#endif
	int i = exportBitChainTx(txID, sBitChainDecryptKey, iCheckHashBlock, txMust2MeOrFromTeam, saveBcpFile, saveTxtFile, saveImgFile, saveMp3File, showTipBox, saveToMysql, mkDir, bitchainPG);
	if( fDebug ){ printf( "exportBitChainTx rzt = [%d] \n", i); }
	//qKey = tr("Export") + " ";
	if( i <= 0 ) //qKey = qKey + tr("success");
	{
	    qKey = bitchainPG->tr("Export") + " " + bitchainPG->tr("fail");
	    QMessageBox::information(bitchainPG, bitchainPG->tr("Export BitChain"), qKey);
	}else{
	    rzt = i;   txID = sBitChainDatDir + txID;     qKey = QString::fromStdString(txID);
		QDesktopServices::openUrl( QUrl::fromLocalFile(qKey) );
	}
    return rzt;
}

void BitchainPage::exportBitChain()
{
    bool isOK = false;
    QString qKey = QInputDialog::getText(NULL, tr("Export BitChain"), tr("Please input BitChain tx id") + "                    ", QLineEdit::Normal, "", &isOK); 
	if( isOK && (qKey.length() > 34) )
	{
		exportBitChainTx(qKey);
	}
}

void BitchainPage::freeBitChainMsgDlg(Widget* pDlg)
{
    //printf("NotifyFreeBitChainQMsgDlg [%u]  [%s]\n", pDlg, pDlg->url.toStdString().c_str());
	if( pDlg != NULL )
	{
	    //Widget* p = (Widget*)pDlg;   
		delete pDlg;
	}
}

static void NotifyFreeQMsgDlg_(BitchainPage *bitchainPage, Widget* pDlg)
{
	//printf("NotifyFreeBitChainQMsgDlg_  [%u]  [%u]\n", pDlg, bitchainPage);
	if( !QMetaObject::invokeMethod(bitchainPage, "freeBitChainMsgDlg", Qt::QueuedConnection, Q_ARG(Widget*, pDlg)) )
	{
		//ShowModMsgBox("NotifyReceiveBitChainMsgOne_", "invokeMethod faile :( ");
		//printf("NotifyReceiveBitChainMsgOne_  invokeMethod faile :(\n");
	}
}

void BitchainPage::subscribeToCoreSignals()
{
    // Connect signals to bitchain
    NotifyFreeWidget.connect(boost::bind(NotifyFreeQMsgDlg_, this, _1));
    NotifyReceiveBitChainMsgOne.connect(boost::bind(NotifyReceiveBitChainMsgOne_, this, _1));
    //NotifyReceiveBitChainMsg.connect(boost::bind(NotifyReceiveBitChainMsg2, this, _1, _2));
	//boost::signals2::signal<void (Widget* pMsg)> NotifyFreeWidget;
}

void BitchainPage::unsubscribeFromCoreSignals()
{
    // Disconnect signals from bitchain
    NotifyFreeWidget.disconnect(boost::bind(NotifyFreeQMsgDlg_, this, _1));
    NotifyReceiveBitChainMsgOne.disconnect(boost::bind(NotifyReceiveBitChainMsgOne_, this, _1));
    //NotifyReceiveBitChainMsg.disconnect(boost::bind(NotifyReceiveBitChainMsg2, this, _1, _2));
}

void BitchainPage::freshPage(int pageId)
{
    if( pageId == aTabId ){ gotoPage(pageId); }
}

void BitchainPage::setCurrentPage(int iPage)
{
    ui->chain_tabWidget->setCurrentIndex(iPage);
    if( iPage == 1 ){ loveChainAction->setChecked(true); }
	else if( iPage == 2 ){ kidsChainAction->setChecked(true); }
	else if( iPage == 3 ){ blessChainAction->setChecked(true); }
	else if( iPage == 4 ){ eventChainAction->setChecked(true); }
	else if( iPage == 5 ){ prophesyChainAction->setChecked(true); }
	else if( iPage == 6 ){ contractChainAction->setChecked(true); }
	else if( iPage == 7 ){ medicalChainAction->setChecked(true); }
	else if( iPage == 8 ){ forumChainAction->setChecked(true); }
}

extern std::string int64tostr(int64_t n);   //int bSetZoomed=0;
void BitchainPage::gotoPage(int pageId, bool setChecked, QString qTxid)
{
//printf("BitchainPage::gotoPage [%d] \n", pageId);
    int64_t tm = GetTime();
    string sSig = buildSignTimeStr( tm );
    string sEn = lang_territory.toLower().toStdString();
    string sTopic = "";     if( qTxid.length() ){ sTopic = qTxid.toStdString(); }
    //if( lang_territory.indexOf("zh_CN") == -1 ){ bEn=1; }
//printf("BitchainPage::gotoPage tm = ""%" PRId64 "\n", tm);
//printf("BitchainPage::gotoPage sBitChainIdentAddress = [%s] [%s]\n", sBitChainIdentAddress.c_str(), sEn.c_str());

    string sUrl = strprintf( "http://%s/?pid=%d&tm=%s&addr=%s&zm=%.2f&lang=%s&ttx=%s&sign=%s", sQkl_domain.c_str(), pageId, int64tostr(tm).c_str(), sBitChainIdentAddress.c_str(), DOB_DPI_RAT, sEn.c_str(), sTopic.c_str(), sSig.c_str());

    QString q = QString::fromStdString( sUrl );  //QString::fromStdString(sQkl_domain);  // www.qkl.im
	ui->chain_tabWidget->setCurrentIndex(pageId);
    aTabId = pageId;
	//if( setChecked ){ setCurrentPage(pageId); }
    if( pageId == 1 ){ ui->web_loveChain->load(QUrl(q));   ui->ptEdit_loveWords->setFocus(); }
    else if( pageId == 2 ){ ui->web_kidsChain->load(QUrl(q));   ui->ptEdit_kidInfo->setFocus(); }
    else if( pageId == 3 ){ ui->web_blessChain->load(QUrl(q));   ui->ptEdit_blessWords->setFocus(); }
    else if( pageId == 4 ){ ui->web_eventChain->load(QUrl(q));   ui->ptEdit_eventTxt->setFocus(); }
    else if( pageId == 5 ){ ui->web_prophesyChain->load(QUrl(q));   ui->ptEdit_prophesyTxt->setFocus(); }
    else if( pageId == 6 ){ ui->web_contractChain->load(QUrl(q));   ui->ptEdit_contractInfo->setFocus(); }
    else if( pageId == 7 ){ ui->web_medicalChain->load(QUrl(q));   ui->ptEdit_medicalInfo->setFocus(); }
    else if( pageId == 8 ){ ui->web_forumChain->load(QUrl(q));   ui->ptEdit_forumTxt->setFocus(); }
    else if( pageId == 0 )
	{
		//QDesktopServices::openUrl( QUrl(q) );
		ui->web_regNick->load(QUrl(q));   ui->web_regNick->setFocus();
	}
}

void BitchainPage::gotoLoveChain()
{
    gotoPage( 1 );
}
void BitchainPage::gotoKidsChain()
{
    gotoPage( 2 );
}
void BitchainPage::gotoBlessChain()
{
    gotoPage( 3 );
}
void BitchainPage::gotoEventChain()
{
    gotoPage( 4 );
}
void BitchainPage::gotoProphesyChain()
{
    gotoPage( 5 );
}
void BitchainPage::gotoContractChain()
{
    gotoPage( 6 );
}
void BitchainPage::gotoMedicalChain()
{
    gotoPage( 7 );
}
void BitchainPage::gotoForumChain()
{
    gotoPage( 8 );
}
void BitchainPage::gotoRegNick()
{
    gotoPage( 0 );
}

bool fUseBlackTheme = false;
static QWidget* makeToolBarSpacer()
{
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    spacer->setStyleSheet(fUseBlackTheme ? "QWidget { background: rgb(30,32,36); }" : "QWidget { background: none; }");
    return spacer;
}

void BitchainPage::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    loveChainAction = new QAction(QIcon(":/icons/tb_love"), tr("Love Wall"), this);
    loveChainAction->setToolTip(tr("Record your love in the blockchain"));
    loveChainAction->setCheckable(true);
    //loveChainAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(loveChainAction);

    kidsChainAction = new QAction(QIcon(":/icons/tb_kids"), tr("Baby Home"), this);
	kidsChainAction->setToolTip(tr("Finding lost babys through blockchain"));
    kidsChainAction->setCheckable(true);
    tabGroup->addAction(kidsChainAction);

    blessChainAction = new QAction(QIcon(":/icons/tb_blessTree"), tr("Bless Tree"), this);
	blessChainAction->setToolTip(tr("Make a beautiful wish, and fight for it"));
    blessChainAction->setCheckable(true);
    tabGroup->addAction(blessChainAction);

    eventChainAction = new QAction(QIcon(":/icons/tb_event"), tr("Big Event"), this);
    eventChainAction->setToolTip(tr("Record emergencies, record the changes of the times"));
    eventChainAction->setCheckable(true);
    tabGroup->addAction(eventChainAction);

    prophesyChainAction = new QAction(QIcon(":/icons/tb_prophesy"), tr("Prophet Chain"), this);
    prophesyChainAction->setCheckable(true);
    tabGroup->addAction(prophesyChainAction);

    forumChainAction = new QAction(QIcon(":/icons/tb_forum"), tr("Forum in Chain"), this);
    forumChainAction->setToolTip(tr("Based on Blockchain, fully distributed and decentralize forum"));
    forumChainAction->setCheckable(true);
    tabGroup->addAction(forumChainAction);	

    contractChainAction = new QAction(QIcon(":/icons/tb_contract"), tr("Certificate & Contract"), this);
	contractChainAction->setToolTip(tr("Record certificates and contracts, in order to prevent forgery"));
    contractChainAction->setCheckable(true);
    tabGroup->addAction(contractChainAction);	
	
    medicalChainAction = new QAction(QIcon(":/icons/tb_medical"), tr("Medical Chain"), this);
	//medicalChainAction->setToolTip(tr("Record certificates and contracts, in order to prevent forgery"));
    medicalChainAction->setCheckable(true);
    tabGroup->addAction(medicalChainAction);	

    regNickAction = new QAction(QIcon(":/icons/tb_reg"), tr("Bit Grid"), this);
    regNickAction->setToolTip(tr("Based on blockchain, dynamic domain name resolution (like oray)"));
    regNickAction->setCheckable(true);
    tabGroup->addAction(regNickAction);

    blockChainAASAction = new QAction(QIcon(":/icons/tb_baas"), tr("Blockchain As A Service"), this);
    blockChainAASAction->setCheckable(false);
    tabGroup->addAction(blockChainAASAction);

    exportBitChainAction = new QAction(QIcon(":/icons/filesave"), tr("Export BitChain"), this);
    //exportBitChainAction->setCheckable(true);
    tabGroup->addAction(exportBitChainAction);
}

void BitchainPage::createToolBars()
{
    toolbar = new QToolBar(tr("Tabs toolbar"));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	MyStyle* mStyle = new MyStyle;
	int ww = 40;     toolbar->setStyle(mStyle);
    if( DOB_DPI_RAT > 1 ){ ww = ww * DOB_DPI_RAT; }
    toolbar->setIconSize(QSize(ww, ww));
    //QScrollArea* scrollArea = new QScrollArea(this);
    //scrollArea->addScrollBarWidget(toolbar, Qt::AlignCenter);

    //if (fUseBlackTheme)
    {
        BitChainheader = new QWidget();
		int w = 150, h = 116;
		if( DOB_DPI_RAT > 1 ){ w = w * DOB_DPI_RAT;   h = h * DOB_DPI_RAT; }
        BitChainheader->setMinimumSize(w, h);
        BitChainheader->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        BitChainheader->setCursor(Qt::OpenHandCursor);
        //header->setStyleSheet("QWidget { background-color: rgb(24,26,30); background-repeat: no-repeat; background-image: url(:/images/header); background-position: top center; }");
        BitChainheader->setStyleSheet("QWidget { background:transparent; border-image: url(:/icons/tb_header) 0 0 0 0 stretch stretch; }");
        toolbar->addWidget(BitChainheader);
        toolbar->addWidget(makeToolBarSpacer());
    }

    toolbar->addAction(loveChainAction);
    toolbar->addAction(kidsChainAction);
    toolbar->addAction(blessChainAction);
    toolbar->addAction(contractChainAction);
    toolbar->addAction(medicalChainAction);
    toolbar->addAction(prophesyChainAction);
    toolbar->addAction(eventChainAction);
    toolbar->addAction(forumChainAction);
    //toolbar->addAction(regNickAction);
    //toolbar->addAction(blockChainAASAction);

    //toolbar->addWidget(makeToolBarSpacer());

    QToolBar* toolbarBaaS = new QToolBar(tr("BAAS"));
    toolbarBaaS->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbarBaaS->setContextMenuPolicy(Qt::PreventContextMenu);
	toolbarBaaS->addAction(regNickAction);

    /*QToolButton *tbApps= new QToolButton;
	tbApps->addAction(blockChainAASAction);
	tbApps->addAction(exportBitChainAction);
	tbApps->setDefaultAction(blockChainAASAction);
	tbApps->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    tbApps->setPopupMode( QToolButton::InstantPopup );   // MenuButtonPopup

    toolbarBaaS->addWidget(tbApps); */
	toolbarBaaS->addAction(blockChainAASAction);
    toolbarBaaS->setIconSize(QSize(ww, ww));     toolbarBaaS->setStyle(mStyle);


	/*if( GetArg("lotterybtn", 1) )
	{
		QToolBar *toolbarLot = addMyToolBar(tr("Lottery"));
		toolbarLot->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		//toolbarLot->addAction(lotteryAction);		
		toolbarLot->setIconSize(QSize(dIconSz, dIconSz));

		QToolButton *tbApps= new QToolButton;
		tbApps->addAction(lotteryAction);
		//tbApps->addSeparator();
		tbApps->addAction(chequeAction);
		tbApps->addAction(bonusAction);
		//tbApps->addAction(bitnetBroAction);
		tbApps->setDefaultAction(lotteryAction);
		tbApps->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
		tbApps->setPopupMode( QToolButton::MenuButtonPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
		toolbarLot->addWidget(tbApps);
	}*/
	
    //QWidget* wi = makeToolBarSpacer();
    /* QLabel *label = new QLabel(wi);
    QVBoxLayout *layout = new QVBoxLayout();
    label->setText(tr("Blockchain As A Service"));
    layout->addWidget(label);
    wi->setLayout(layout);
    wi->setMaximumSize(160, 116); */

    QString sMainStyle = "QTabBar::tab:first{margin-left: 30px;}"
                                       "QTabBar::tab { background: transparent; border: 1px solid #333333; border-bottom: none; color: black; padding-left: 5px; padding-right: 10px;"
                                       "padding-top: 3px; padding-bottom: 3px; min-width: 99px;} "
                                       "QTabBar::tab:hover { background-color: #333333; border: 1px solid #444444; border-bottom: none; } "
                                       "QTabBar::tab:selected { background-color: #111111; border: 1px solid #333333; border-top: 1px solid yellow;"
                                       "border-bottom: none; font-weight :bold; color: green} "
                                       "QTabBar::tab:selected, QTabBar::tab:hover { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);} "
                                       "QTabBar::tab:selected { border-color: #9B9B9B; border-bottom-color: #C2C7CB; } "
                                       "QTabBar::tab:!selected { margin-top: 2px; } "
									   //"QToolBar::handle { min-height: TbSeparatorHei; }"
									   //"QToolBar::separator { min-height: TbSeparatorHei; }"
                                       "QPlainTextEdit,#lab_setContractImg,#lab_setMedicalImg,#lab_setLoveImg,#lab_setKidImg,#lab_setBlessImg,#lab_setEventImg,#lab_setProphesyImg,#lab_setLoveMp3,#lab_setBlessMp3,#lab_setProphesyMp3,#lab_setForumMp3,#lab_setForumImg"
                                       "{ border: 1px solid #B7B7B7; border-radius: 5px; padding-left:3px; padding-right:3px; margin:0px; min-width: PlainTextEditHei; max-height: PlainTextEditHei; height: PlainTextEditHei; } ";

    QString sStyle = "QToolButton{ height: ToolButtonHei; background: rgb(20,150,50); color: rgb(222,222,222); border: none; border-left-color: rgb(30,32,36); border-left-style: solid; border-left-width: ChkWidth; margin-top: 8px; margin-bottom: 8px; }"
                      "QToolButton:checked { color: #ffbb00; border: none; border-left-color: red; border-left-style: solid; border-left-width: ChkWidth; }"
					  "QToolButton:hover { color: #ffbb00; }";
    QString sNewBtnHei = "40px", sNewPlainTextEditHei = "66px", sTbSeparatorHei = "10px";
	QString sChkWidth = "6px";
	if( DOB_DPI_RAT > 1 )
    {
	    sNewBtnHei = QString::number(DOB_DPI_RAT * 40, 'f', 0) + "px";
		sNewPlainTextEditHei = QString::number(DOB_DPI_RAT * 66, 'f', 0) + "px";
		sChkWidth = QString::number(DOB_DPI_RAT * 6, 'f', 0) + "px";
		//sTbSeparatorHei = QString::number(DOB_DPI_RAT * 10, 'f', 0) + "px";
	}
	sStyle.replace("ToolButtonHei", sNewBtnHei);   sStyle.replace("ChkWidth", sChkWidth);
	sMainStyle.replace("PlainTextEditHei", sNewPlainTextEditHei);  // + qScrollBarStyleStr;
	//sMainStyle.replace("TbSeparatorHei", sTbSeparatorHei);
    toolbar->setStyleSheet(sStyle);   toolbarBaaS->setStyleSheet(sStyle);   this->setStyleSheet(sMainStyle); 

    toolbar->setOrientation(Qt::Vertical);   toolbar->setMovable(false);
    toolbarBaaS->setOrientation(Qt::Vertical);   toolbarBaaS->setMovable(false);
    addToolBar(Qt::LeftToolBarArea, toolbar);   addToolBar(Qt::LeftToolBarArea, toolbarBaaS);

    int w = 180;
    if( lang_territory.indexOf("zh_CN") == -1 ){ w = w + 60; }
    if( DOB_DPI_RAT > 1 ){ w = w * DOB_DPI_RAT; }
	toolbarBaaS->widgetForAction(regNickAction)->setFixedWidth(w);

//tbApps->setFixedWidth(w);   tbApps->setFixedHeight(34 * DOB_DPI_RAT);
	toolbarBaaS->widgetForAction(blockChainAASAction)->setFixedWidth(w);
	toolbarBaaS->widgetForAction(blockChainAASAction)->setFixedHeight(32 * DOB_DPI_RAT);
	//toolbarBaaS->widgetForAction(exportBitChainAction)->setFixedWidth(w);

    /*foreach(QAction *action, toolbar->actions()) {
        w = std::max(w, toolbar->widgetForAction(action)->width());
    }
    w = 234;  //w + 10; */

    foreach(QAction *action, toolbar->actions()) {
        toolbar->widgetForAction(action)->setFixedWidth(w);
		/*QString t = action->text();
		if( t.length() < 2 ){
            int h = DOB_DPI_RAT * 32;
			QPixmap orig(":/icons/tb_arrow");
            //return orig.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			action->setIcon(QIcon( orig.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation) )); 
		}*/
    }
}

void BitchainPage::setModel(WalletModel *model)
{
    this->model = model;

    // update the display unit, to not use the default ("BTC")
    //updateDisplayUnit();
}

void BitchainPage::on_chain_tabWidget_currentChanged(int index)
{
    aTabId = index;
    updatePayForChain( aTabId );
}

void BitchainPage::on_ptEdit_loveWords_textChanged()
{
    updatePayForChain( 1 );
}

void BitchainPage::on_ptEdit_kidInfo_textChanged()
{
    updatePayForChain( 2 );
}

void BitchainPage::on_ptEdit_blessWords_textChanged()
{
    updatePayForChain( 3 );
}

void BitchainPage::on_ptEdit_eventTxt_textChanged()
{
    updatePayForChain( 4 );
}
void BitchainPage::on_ptEdit_prophesyTxt_textChanged()
{
    updatePayForChain( 5 );
}
void BitchainPage::on_ptEdit_contractInfo_textChanged()
{
    updatePayForChain( 6 );
}
void BitchainPage::on_ptEdit_medicalInfo_textChanged()
{
    updatePayForChain( 7 );
}
void BitchainPage::on_ptEdit_forumTxt_textChanged()
{
    updatePayForChain( 8 );
}
int getFileSize(QString fn)
{
	int size = 0;
	if( fn.length() <= 0 ){ return size; }
	QFile myFile(fn);  //"C:\\file.txt");
	if (myFile.open(QIODevice::ReadOnly)){
		size = myFile.size();  //when file does open.
		myFile.close();
	}
	return size;
}

int BitchainPage::getSendToChainDatSize(int tabId, int& imgSz, int& mp3Sz, int addNameSz)
{
    int i = 0;     imgSz = 0;     mp3Sz = 0;
	QString t = "";  //txtMessage->toPlainText();
    if( tabId == 0 ){ return 0; }
    else if( tabId == 1 ){ 
		t = ui->ptEdit_loveWords->toPlainText() + ui->lEdit_loveParentTopic->text();
		if( addNameSz > 0 ){ t = ui->lEdit_LoveName->text() + t; }
		imgSz = getFileSize(ui->lab_setLoveImg->toolTip());     mp3Sz = getFileSize(ui->lab_setLoveMp3->toolTip());
	}
    else if( tabId == 2 ){
		t = ui->lEdit_kidsParentTopic->text() + ui->ptEdit_kidInfo->toPlainText() + ui->lEdit_kidsUrl->text();
		if( addNameSz > 0 ){ t = ui->lEdit_BabyName->text() + t; }
		imgSz = getFileSize(ui->lab_setKidImg->toolTip());
	}
    else if( tabId == 3 ){
		t = ui->lEdit_blessParentTopic->text() + ui->ptEdit_blessWords->toPlainText();
		if( addNameSz > 0 ){ t = ui->lEdit_BlessName->text() + t; }
		mp3Sz = getFileSize(ui->lab_setBlessMp3->toolTip());     imgSz = getFileSize(ui->lab_setBlessImg->toolTip());
	}
    else if( tabId == 4 ){
		t = ui->lEdit_eventParentTopic->text() + ui->ptEdit_eventTxt->toPlainText() + ui->lEdit_eventLink->text();
		if( addNameSz > 0 ){ t = ui->lEdit_EventName->text() + t; }
		imgSz = getFileSize(ui->lab_setEventImg->toolTip());
	}
    else if( tabId == 5 ){
		t = ui->lEdit_prophesyParentTopic->text() + ui->ptEdit_prophesyTxt->toPlainText();
		if( addNameSz > 0 ){ t = ui->lEdit_ProphesyName->text() + t; }
		mp3Sz = getFileSize(ui->lab_setProphesyMp3->toolTip());     imgSz = getFileSize(ui->lab_setProphesyImg->toolTip());
	}
    else if( tabId == 6 ){
		t = ui->lEdit_contractParentTopic->text() + ui->ptEdit_contractInfo->toPlainText() + ui->lEdit_contractUrl->text();
		if( addNameSz > 0 ){ t = ui->lEdit_ContractName->text() + t; }
		imgSz = getFileSize(ui->lab_setContractImg->toolTip());
	}
    else if( tabId == 7 ){
		t = ui->lEdit_medicalParentTopic->text() + ui->ptEdit_medicalInfo->toPlainText() + ui->lEdit_medicalUrl->text();
		if( addNameSz > 0 ){ t = ui->lEdit_MedicalName->text() + t; }
		imgSz = getFileSize(ui->lab_setMedicalImg->toolTip());
	}
    else if( tabId == 8 ){
		t = ui->lEdit_forumParentTopic->text() + ui->ptEdit_forumTxt->toPlainText();
		if( addNameSz > 0 ){ t = ui->lEdit_ForumName->text() + t; }
		mp3Sz = getFileSize(ui->lab_setForumMp3->toolTip());    imgSz = getFileSize(ui->lab_setForumImg->toolTip());
	}
    return t.length() + imgSz + mp3Sz;
}
void BitchainPage::updatePayForChain(int tabId)
{
	int c = 1, imgSz= 0, mp3Sz = 0, ic=0, mc=0;
    int i = getSendToChainDatSize(tabId, imgSz, mp3Sz, c);
	int txtSz = i - imgSz - mp3Sz;
	if( txtSz > 0 )
	{
		 c = txtSz / 1000;
	}
    if( c < 1 ){ c = 1; }
	if( imgSz > 0 ){
		ic = imgSz / (1000 * 2);     if( ic < 1 ){ ic=1; }
		c = c + ic;
	}
	if( mp3Sz > 0 ){
		mc = mp3Sz / (1000 * 10);     if( mc < 1 ){ mc=1; }
		c = c + mc;
	}

	//printf("updatePayForChain:: c = [%d], ic = [%d], mc = [%d], imgSz = [%d], mp3Sz = [%d] \n", c, ic, mc, imgSz, mp3Sz);
	iNeedPay = c;
	QString t = QString("%1").arg(c) + " VPN";  //QString s = QString::number(a, 10);      // s == "63"
    if( tabId == 0 ){  }
    else if( tabId == 1 ){ ui->lab_payForLove->setText( t ); }
    else if( tabId == 2 ){ ui->lab_payForKid->setText( t ); }
    else if( tabId == 3 ){ ui->lab_payForBless->setText( t ); }
    else if( tabId == 4 ){ ui->lab_payForEvent->setText( t ); }
    else if( tabId == 5 ){ ui->lab_payForProphesy->setText( t ); }
    else if( tabId == 6 ){ ui->lab_payForContract->setText( t ); }
    else if( tabId == 7 ){ ui->lab_payForMedical->setText( t ); }
    else if( tabId == 8 ){ ui->lab_payForForum->setText( t ); }
}

void BitchainPage::on_pBtn_regNick_clicked()
{
    submit_data_toChain( 0 );
}
void BitchainPage::on_pBtn_subLoveToChain_clicked()
{
    submit_data_toChain( 1 );
}
void BitchainPage::on_pBtn_subKidsInfoToChain_clicked()
{
    submit_data_toChain( 2 );
}
void BitchainPage::on_pBtn_subBlessToChain_clicked()
{
    submit_data_toChain( 3 );
}
void BitchainPage::on_pBtn_subEventToChain_clicked()
{
    submit_data_toChain( 4 );
}
void BitchainPage::on_pBtn_subProphesyToChain_clicked()
{
    submit_data_toChain( 5 );
}
void BitchainPage::on_pBtn_subContractToChain_clicked()
{
    submit_data_toChain( 6 );
}
void BitchainPage::on_pBtn_subMedicalToChain_clicked()
{
    submit_data_toChain( 7 );
}
void BitchainPage::on_pBtn_subForumToChain_clicked()
{
    submit_data_toChain( 8 );
}

/*
        CScript scriptPubKey;
        scriptPubKey.SetDestination(address.Get());
        int64_t nAmount = AmountFromValue(s.value_);
        totalAmount += nAmount;
        vecSend.push_back(make_pair(scriptPubKey, nAmount));
		 bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, stxData, iJiami);  // iJiami = 1 = encryption
	*/
int getBitChainRewardParam(int pid, string& rwdAddress)
{
	int rzt = 0;   rwdAddress = "";
    if( pid == 1 ){
		if( (iRwdLoveRatio >= 2) && (targetLoveWallet.length() > 30) ){ rwdAddress = targetLoveWallet.toStdString();   rzt = iRwdLoveRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 2 ){
		if( (iRwdBabyRatio >= 2) && (targetBabyWallet.length() > 30) ){ rwdAddress = targetBabyWallet.toStdString();   rzt = iRwdBabyRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 3 ){
		if( (iRwdBlessRatio >= 2) && (targetBlessWallet.length() > 30) ){ rwdAddress = targetBlessWallet.toStdString();   rzt = iRwdBlessRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 4 ){
		if( (iRwdEventRatio >= 2) && (targetEventWallet.length() > 30) ){ rwdAddress = targetEventWallet.toStdString();   rzt = iRwdEventRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 5){
		if( (iRwdProphesyRatio >= 2) && (targetProphesyWallet.length() > 30) ){ rwdAddress = targetProphesyWallet.toStdString();   rzt = iRwdProphesyRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 6 ){
		if( (iRwdContractRatio >= 2) && (targetContractWallet.length() > 30) ){ rwdAddress = targetContractWallet.toStdString();   rzt = iRwdContractRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 7 ){
		if( (iRwdMedicalRatio >= 2) && (targetMedicalWallet.length() > 30) ){ rwdAddress = targetMedicalWallet.toStdString();   rzt = iRwdMedicalRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
    else if( pid == 8 ){
		if( (iRwdForumRatio >= 2) && (targetForumWallet.length() > 30) ){ rwdAddress = targetForumWallet.toStdString();   rzt = iRwdForumRatio; }
		CBitcoinAddress ad(rwdAddress);     if( !ad.IsValid() ){ rzt = 0;     rwdAddress = ""; }
	}
	return rzt;
}
int sendBitChainMsgToBlockChain(int tabId, const string sToAddr, CBitChainProtocol& bcp, int64_t i6Coin)
{
	int rzt = 0;
	if( (sToAddr.length() != 34) || (i6Coin <= 0) ){ return rzt; }
    string sRwdAddr = "";     int ii = getBitChainRewardParam(tabId, sRwdAddr);
    vector<pair<CScript, int64_t> > vecSend;
    CScript scriptPubKey;
    scriptPubKey.SetDestination(CBitcoinAddress(sToAddr).Get());
	if( ii < 2 )
	{
		vecSend.push_back(make_pair(scriptPubKey, i6Coin));
	}else{
		int64_t i62 = i6Coin / ii;
		CScript rwdPubKey;
		rwdPubKey.SetDestination(CBitcoinAddress(sRwdAddr).Get());
		vecSend.push_back(make_pair(scriptPubKey, i6Coin - i62));
		vecSend.push_back(make_pair(rwdPubKey, i62));
	}
	CWalletTx wtxNew;   string sChainData="";
	//CBitChainProtocol* pbp = (CBitChainProtocol*)&bcp;
	int iDz = CBitChainProtocolToBuffer(&bcp, sChainData);
	//string fName = strprintf("%s%I64u_%u.bcp", sBitChainDatDir.c_str(), GetTime(), iDz);
	//writeBufToFile((char*)sChainData.c_str(), iDz, fName);

	if( pwalletMain->SendMoney(vecSend, i6Coin, wtxNew, sChainData, 0, true) == "" ){ rzt++; }
/*
        std::vector<std::pair<CScript, int64_t> > vecSend;
        //foreach(const SendCoinsRecipient &rcp, recipients)
        {
            CScript scriptPubKey;
            scriptPubKey.SetDestination(CBitcoinAddress(sToAddr).Get());
            vecSend.push_back( make_pair(scriptPubKey, i6Coin) );
        }

        CWalletTx wtx;
		//wtx.strFromAccount = strAccount;
        CReserveKey keyChange(pwalletMain);
        int64_t nFeeRequired = 0;   CBitChainProtocol* pbp = (CBitChainProtocol*)&bcp;
	string sChainData="";   CBitChainProtocolToBuffer(pbp, sChainData);
//bool CreateTransaction(CScript scriptPubKey, int64_t nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string stxData, int bEncrypt, const CCoinControl *coinControl=NULL);		
		bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, sChainData, 0, NULL);
		if (!fCreated)
		{
			ShowModMsgBox("Error...", "Create Transaction failed");  //if( fDebug ){ printf( "Create Transaction failed \n"); }
			return rzt;
		}
		if (!pwalletMain->CommitTransaction(wtx, keyChange))
		{
			ShowModMsgBox("Error...", "Commit Transaction failed");  //if( fDebug ){ printf( "Transaction commit failed \n"); }
			//return rzt;
		}else rzt++; */
		return rzt;
}
void BitchainPage::submit_data_toChain(int tabId)
{
    //updatePayForChain( aTabId );
	if( tabId > 0 )  // not reg nickname
	{
		int imgSz = 0,   mp3Sz = 0,     i = getSendToChainDatSize(tabId, imgSz, mp3Sz, 0);
		if( i <= 0 ){ return; }
		//CBitChain(unsigned int nChainTypeIn, string nTopicIn, string nContentTextIn, string nContextUrlIn, char* pMediaIn, unsigned int nMediaLen, string nFromAddrIn = "", string nToAddrIn = "", string nExternalChainDataIn = "")
		string nTopic = "", nContentTextIn = "", nContextUrlIn = "", nFromAddrIn = "", nToAddrIn = "", nExternalChainDataIn = "", nNickName = "";
		int nImgLen = 0;     int64_t i6Coin = 0;
		char* pImgIn = NULL;     char nAllowReply = 1;     QString sPay = "";
		string  nImgFile = "", nMp3File = "";   //widget->toolTip();
		if( tabId == 1 )
		{
			nTopic = ui->lEdit_loveParentTopic->text().toStdString();     nNickName = ui->lEdit_LoveName->text().toStdString();
			nContentTextIn = ui->ptEdit_loveWords->toPlainText().toStdString();
			nImgFile = ui->lab_setLoveImg->toolTip().toStdString();     nMp3File = ui->lab_setLoveMp3->toolTip().toStdString();
			sPay = ui->lab_payForLove->text();
		}
		else if( tabId == 2 )
		{
			nTopic = ui->lEdit_kidsParentTopic->text().toStdString();     nNickName = ui->lEdit_BabyName->text().toStdString();
			nContentTextIn = ui->ptEdit_kidInfo->toPlainText().toStdString();     nContextUrlIn = ui->lEdit_kidsUrl->text().toStdString();
			nImgFile = ui->lab_setKidImg->toolTip().toStdString();     //nMp3File = ui->lab_setLoveMp3->toolTip().toStdString();
			sPay = ui->lab_payForKid->text();
		}
		else if( tabId == 3 )
		{
			nTopic = ui->lEdit_blessParentTopic->text().toStdString();     nNickName = ui->lEdit_BlessName->text().toStdString();
			nContentTextIn = ui->ptEdit_blessWords->toPlainText().toStdString();
			nImgFile = ui->lab_setBlessImg->toolTip().toStdString();     nMp3File = ui->lab_setBlessMp3->toolTip().toStdString();
			sPay = ui->lab_payForBless->text();
		}
		else if( tabId == 4 )
		{
			nTopic = ui->lEdit_eventParentTopic->text().toStdString();     nNickName = ui->lEdit_EventName->text().toStdString();
			nContentTextIn = ui->ptEdit_eventTxt->toPlainText().toStdString();     nContextUrlIn = ui->lEdit_eventLink->text().toStdString();
			nImgFile = ui->lab_setEventImg->toolTip().toStdString();     //nMp3File = ui->lab_setBlessMp3->toolTip().toStdString();
			sPay = ui->lab_payForEvent->text();
		}
		else if( tabId == 5 )
		{
			nTopic = ui->lEdit_prophesyParentTopic->text().toStdString();     nNickName = ui->lEdit_ProphesyName->text().toStdString();
			nContentTextIn = ui->ptEdit_prophesyTxt->toPlainText().toStdString();
			nImgFile = ui->lab_setProphesyImg->toolTip().toStdString();     nMp3File = ui->lab_setProphesyMp3->toolTip().toStdString();
			sPay = ui->lab_payForProphesy->text();
		}
		else if( tabId == 6 )
		{
			nTopic = ui->lEdit_contractParentTopic->text().toStdString();     nNickName = ui->lEdit_ContractName->text().toStdString();
			nContentTextIn = ui->ptEdit_contractInfo->toPlainText().toStdString();     nContextUrlIn = ui->lEdit_contractUrl->text().toStdString();
			nImgFile = ui->lab_setContractImg->toolTip().toStdString();     //nMp3File = ui->lab_setProphesyMp3->toolTip().toStdString();
			sPay = ui->lab_payForContract->text();
		}
		else if( tabId == 7 )
		{
			nTopic = ui->lEdit_medicalParentTopic->text().toStdString();     nNickName = ui->lEdit_MedicalName->text().toStdString();
			nContentTextIn = ui->ptEdit_medicalInfo->toPlainText().toStdString();     nContextUrlIn = ui->lEdit_medicalUrl->text().toStdString();
			nImgFile = ui->lab_setMedicalImg->toolTip().toStdString();     //nMp3File = ui->lab_setProphesyMp3->toolTip().toStdString();
			sPay = ui->lab_payForMedical->text();
		}
		else if( tabId == 8 )
		{
			nTopic = ui->lEdit_forumParentTopic->text().toStdString();     nNickName = ui->lEdit_ForumName->text().toStdString();
			nContentTextIn = ui->ptEdit_forumTxt->toPlainText().toStdString();     //nContextUrlIn = ui->lEdit_medicalUrl->text().toStdString();
			nImgFile = ui->lab_setForumImg->toolTip().toStdString();     nMp3File = ui->lab_setForumMp3->toolTip().toStdString();
			sPay = ui->lab_payForForum->text();
		}
		int jj = sPay.split(" ")[0].toInt();
		if( jj <= 0 ){ jj = 1; }
		i6Coin = jj * COIN;
		string sImgBuf = "", sMp3Buf = "";
		if( nImgFile.length() > 3 )
		{
			nImgLen = getFileBinData( nImgFile, sImgBuf);
			//if( nImgLen > 0 ){ pImgIn = (char*)sImgBuf.c_str(); }
		}
		if( nMp3File.length() > 3 )
		{
			int nVoiceLen = getFileBinData( nMp3File, sMp3Buf);
			/*if( nVoiceLen > 0 )
			{
				char* pVoiceIn = (char*)sMp3Buf.c_str();
				bitChain.setMediaData(NULL, 0, pVoiceIn, nVoiceLen, NULL, 0);
				sMp3Buf.resize(0);
			}*/
		}
		//CBitChain bitChain(tabId, nNickName, nTopic, nContentTextIn, nContextUrlIn, pImgIn, nImgLen, nAllowReply, nFromAddrIn, nToAddrIn, nExternalChainDataIn);
		CBitChain bitChain(tabId, nNickName, nTopic, nContentTextIn, nContextUrlIn, sImgBuf, sMp3Buf, "", nAllowReply, nFromAddrIn, nToAddrIn, nExternalChainDataIn);
		/*if( nMp3File.length() > 3 )
		{
			int nVoiceLen = getFileBinData( nMp3File, sMp3Buf);
			if( nVoiceLen > 0 )
			{
				char* pVoiceIn = (char*)sMp3Buf.c_str();
				bitChain.setMediaData(NULL, 0, pVoiceIn, nVoiceLen, NULL, 0);
				sMp3Buf.resize(0);
			}
		}*/
		//sImgBuf.resize(0);     sMp3Buf.resize(0);

		uint256 bitHash = bitChain.GetHash();
		//string sHash = bitHash.ToString();
		string sBitChain = "";
		int iBitChainSz = CBitChainToBuffer(&bitChain, sBitChain);
		//string fName = strprintf("%s%I64u_%u.bitChain", sBitChainDatDir.c_str(), GetTime(), iBitChainSz);
		//writeBufToFile((char*)sBitChain.c_str(), iBitChainSz, fName);
		string sLzma = "";
        int lzmaSz = lzma_pack_buf((unsigned char *)sBitChain.c_str(), iBitChainSz, sLzma, 9, uint_256KB); 
		if( lzmaSz > 0 )
		{
			sLzma.resize(lzmaSz);
			/* string sPack = "";    // BitChainData:
			int iPackSz = lzmaSz + 14;
			//printf("sLzma Length [%d :: %d] \n", lzmaSz, sLzma.length());
			sPack.resize(iPackSz);
			char* p = (char*)sPack.c_str();
			memcpy(p, "BitChainData:", 13);   *(unsigned short *)(p + 13) = tabId;
			p = p + 13 + sizeof(unsigned short);
			memcpy(p, (char*)sLzma.c_str(), lzmaSz);
			fName = strprintf("%s%I64u_%u.7zChain", sBitChainDatDir.c_str(), GetTime(), lzmaSz);
			writeBufToFile((char*)sPack.c_str(), iPackSz, fName);
			sPack.resize(0); */

			//CBitChain b2;
			//int iDz = CBitChainFromLzmaBuffer(&b2, (char*)sLzma.c_str(), lzmaSz);
		  //CBitChainFromLzmaBuffer:: iDz  = 1153, Hash = [952a29291bc3f489b1a949190ff33194ee4044c44f73c3530e224e1b08fb7d97] 
			//printf("CBitChainFromLzmaBuffer:: iDz  = %d, Hash = [%s] \n", iDz, b2.GetHash().ToString().c_str());
			//printf("b2:: len  = [%d : %d : %d], [%u] \n", b2.sImgData.length(), b2.sVoiceData.length(), b2.sVideoData.length(), (int)(i6Coin / COIN));
			
			//CBitChainProtocol bcp((char*)sLzma.c_str(), lzmaSz);   sLzma.resize(0);
			CBitChainProtocol bcp( sLzma, tabId, bitHash, nAesBitChain );   sLzma.resize(0);
			//int iDz = CBitChainProtocolToBuffer(&bcp, sLzma);
			//string fName = strprintf("%s%I64u_%s_%u.bcp", sBitChainDatDir.c_str(), GetTime(), bitHash.ToString().c_str(), lzmaSz);
			//writeBufToFile((char*)sLzma.c_str(), iDz, fName);   sLzma.resize(0);
			if( !bcp.IsNull() )
			{
				if( sendBitChainMsgToBlockChain(tabId, sBitChain_Service_Addr, bcp, i6Coin) > 0 )
				{
					clearBtnClicked( tabId );     resetBitChainRewardParam(tabId);     //remove(fName);
				}
			}
		}
//submit_data_toChain:: ChainType = [0], Hash = [952a29291bc3f489b1a949190ff33194ee4044c44f73c3530e224e1b08fb7d97], save to file [h:\coin\vpncoin-master\release\BlockChain\1457071680_656.7zChain], 7Zip size [656] 
		//printf("submit_data_toChain:: ChainType = [%d], Hash = [%s], save to file [%s], 7Zip size [%d] \n", tabId, sHash.c_str(), fName.c_str(), lzmaSz);
	}
	else{
	
	}
}

bool BitchainPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        if( obj == ui->lab_setLoveImg )
        {
            select_photo_toChain( 1 );
			/*const QPixmap *pm = ui->label_qrcode->pixmap();
            if( pm != NULL )
            {
                QApplication::clipboard()->setPixmap( *pm );
                ovShowMsg();
            }*/
        }
        else if( obj == ui->lab_setKidImg ){ select_photo_toChain( 2 ); }
        else if( obj == ui->lab_setBlessImg ){ select_photo_toChain( 3 ); }
        else if( obj == ui->lab_setEventImg ){ select_photo_toChain( 4 ); }
        else if( obj == ui->lab_setProphesyImg ){ select_photo_toChain( 5 ); }
        else if( obj == ui->lab_setContractImg ){ select_photo_toChain( 6 ); }
        else if( obj == ui->lab_setMedicalImg ){ select_photo_toChain( 7 ); }
        else if( obj == ui->lab_setForumImg ){ select_photo_toChain( 8 ); }
        else if( obj == ui->lab_setLoveMp3 ){ select_photo_toChain( 1, 1 ); }
        else if( obj == ui->lab_setBlessMp3 ){ select_photo_toChain( 3, 3 ); }
        else if( obj == ui->lab_setProphesyMp3 ){ select_photo_toChain( 5, 5 ); }
        else if( obj == ui->lab_setForumMp3 ){ select_photo_toChain( 8, 8 ); }
        else if( obj == BitChainheader )
		{
		    QString sUrl = "http://www.bitchaing.com";
		    QDesktopServices::openUrl(QUrl(sUrl));
		}
    }
    /*else if (ev->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(ev);
        long a = keyEvent->key();
        QString s = QString::number(a, 10);   ShowToastMsg(s);
    }*/
    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

void BitchainPage::select_photo_toChain(int imgId, int mp3Id)
{
    int tabId = imgId;
	if( mp3Id >= 1 )
    {
        QString filename = QFileDialog::getOpenFileName(this, tr("Choose Sound File"), ".", tr("Sound file (*.mp3)"));
        if(!filename.isEmpty())
        {
			if( mp3Id == 1 ){ ui->lab_setLoveMp3->setText(filename);   ui->lab_setLoveMp3->setToolTip(filename); }
			else if( mp3Id == 3 ){ ui->lab_setBlessMp3->setText(filename);   ui->lab_setBlessMp3->setToolTip(filename); }
			else if( mp3Id == 5 ){ ui->lab_setProphesyMp3->setText(filename);   ui->lab_setProphesyMp3->setToolTip(filename); }
			else if( mp3Id == 8 ){ ui->lab_setForumMp3->setText(filename);   ui->lab_setForumMp3->setToolTip(filename); }
		}
    }else{
        QString filename = QFileDialog::getOpenFileName(this, tr("Choose Picture File"), ".", tr("Picture file (*.png *.bmp *.jpg)"));
        if(!filename.isEmpty())
        {
        QPixmap pic = QPixmap(filename);
        QPixmap pic2 = pic.scaled(66, 66,  Qt::KeepAspectRatio);
        if( tabId == 1 ){ ui->lab_setLoveImg->setPixmap( pic2 ); ui->lab_setLoveImg->setToolTip(filename); }
        else if( tabId == 2 ){ ui->lab_setKidImg->setPixmap( pic2 ); ui->lab_setKidImg->setToolTip(filename); }
        else if( tabId == 3 ){ ui->lab_setBlessImg->setPixmap( pic2 ); ui->lab_setBlessImg->setToolTip(filename); }
        else if( tabId == 4 ){ ui->lab_setEventImg->setPixmap( pic2 ); ui->lab_setEventImg->setToolTip(filename); }
        else if( tabId == 5 ){ ui->lab_setProphesyImg->setPixmap( pic2 ); ui->lab_setProphesyImg->setToolTip(filename); }
        else if( tabId == 6 ){ ui->lab_setContractImg->setPixmap( pic2 ); ui->lab_setContractImg->setToolTip(filename); }
        else if( tabId == 7 ){ ui->lab_setMedicalImg->setPixmap( pic2 ); ui->lab_setMedicalImg->setToolTip(filename); }
        else if( tabId == 8 ){ ui->lab_setForumImg->setPixmap( pic2 ); ui->lab_setForumImg->setToolTip(filename); }
        }
    }
	this->updatePayForChain(tabId);
}

void BitchainPage::on_pBtn_clearLove_clicked()
{
    clearBtnClicked( 1 );
}
void BitchainPage::on_pBtn_clearKids_clicked()
{
    clearBtnClicked( 2 );
}
void BitchainPage::on_pBtn_clearBless_clicked()
{
    clearBtnClicked( 3 );
}
void BitchainPage::on_pBtn_clearEvent_clicked()
{
    clearBtnClicked( 4 );
}
void BitchainPage::on_pBtn_clearProphesy_clicked()
{
    clearBtnClicked( 5 );
}
void BitchainPage::on_pBtn_clearContract_clicked()
{
    clearBtnClicked( 6 );
}
void BitchainPage::on_pBtn_clearMedical_clicked()
{
    clearBtnClicked( 7 );
}
void BitchainPage::on_pBtn_clearForum_clicked()
{
    clearBtnClicked(8);
}
void BitchainPage::clearBtnClicked(int tabId)
{
    QString q = tr("Click to select photo");
    QString m = tr("Click to select music");
	if( tabId == 1 )
    {
        ui->lEdit_loveParentTopic->clear();
        ui->ptEdit_loveWords->clear();
        ui->lab_setLoveImg->setPixmap( QPixmap() );
        ui->lab_setLoveImg->setToolTip( QString() );
        ui->lab_setLoveImg->setText(q);
        ui->lab_setLoveMp3->setToolTip( QString() );
        ui->lab_setLoveMp3->setText(m);
		ui->ptEdit_loveWords->setFocus(); 
	}
    else if( tabId == 2 )
    {
        ui->lEdit_kidsParentTopic->clear();
        ui->ptEdit_kidInfo->clear();
        ui->lEdit_kidsUrl->clear();
        ui->lab_setKidImg->setPixmap( QPixmap() );
        ui->lab_setKidImg->setToolTip( QString() );
        ui->lab_setKidImg->setText(q);
		ui->ptEdit_kidInfo->setFocus(); 
	}
    else if( tabId == 3 )
    {
        ui->lEdit_blessParentTopic->clear();
        ui->ptEdit_blessWords->clear();
        ui->lab_setBlessImg->setPixmap( QPixmap() );
        ui->lab_setBlessImg->setToolTip( QString() );
        ui->lab_setBlessImg->setText(q);
        ui->lab_setBlessMp3->setToolTip( QString() );
        ui->lab_setBlessMp3->setText(m);
		ui->ptEdit_blessWords->setFocus(); 
	}
    else if( tabId == 4 )
    {
        ui->lEdit_eventParentTopic->clear();
        ui->ptEdit_eventTxt->clear();
        ui->lEdit_eventLink->clear();
        ui->lab_setEventImg->setPixmap( QPixmap() );
        ui->lab_setEventImg->setToolTip( QString() );
        ui->lab_setEventImg->setText(q);
		ui->ptEdit_eventTxt->setFocus(); 
	}
    else if( tabId == 5 )
    {
        ui->lEdit_prophesyParentTopic->clear();
        ui->ptEdit_prophesyTxt->clear();
        ui->lEdit_eventLink->clear();
		//ui->lab_payForProphecy->setText("1 VPN");
        ui->lab_setProphesyImg->setPixmap( QPixmap() );
        ui->lab_setProphesyImg->setToolTip( QString() );
        ui->lab_setProphesyImg->setText(q);
        ui->lab_setProphesyMp3->setToolTip( QString() );
        ui->lab_setProphesyMp3->setText(m);
		ui->ptEdit_prophesyTxt->setFocus(); 
	}
    else if( tabId == 6 )
    {
        ui->lEdit_contractParentTopic->clear();
        ui->ptEdit_contractInfo->clear();
        ui->lEdit_contractUrl->clear();
        ui->lab_setContractImg->setPixmap( QPixmap() );
        ui->lab_setContractImg->setToolTip( QString() );
        ui->lab_setContractImg->setText(q);
		ui->ptEdit_contractInfo->setFocus(); 
	}
    else if( tabId == 7 )
    {
        ui->lEdit_medicalParentTopic->clear();
        ui->ptEdit_medicalInfo->clear();
        ui->lEdit_medicalUrl->clear();
        ui->lab_setMedicalImg->setPixmap( QPixmap() );
        ui->lab_setMedicalImg->setToolTip( QString() );
        ui->lab_setMedicalImg->setText(q);
		ui->ptEdit_medicalInfo->setFocus(); 
	}
    else if( tabId == 8 )
    {
		ui->lEdit_forumParentTopic->clear();
		ui->ptEdit_forumTxt->clear();
        ui->lab_setForumImg->setPixmap( QPixmap() );
        ui->lab_setForumImg->setToolTip( QString() );
        ui->lab_setForumImg->setText(q);
        ui->lab_setForumMp3->setToolTip( QString() );
        ui->lab_setForumMp3->setText(m);
		ui->ptEdit_forumTxt->setFocus();
	}
    this->updatePayForChain(tabId);
}

int BitchainPage::doBitChainCmd(QString sCmd)
{
	int rzt = 0;
	//BitChainRep:/1/transaction id/nickname
	QStringList lst = sCmd.split("/");  //[0].toInt();
	int ic = lst.count();
	if( ic > 3 )
	{
		int iPage = lst[1].toInt();     if( iPage < 9 ){ rzt++; }     resetBitChainRewardParam(iPage);
		QString sOpt = lst[0], tx = lst[2],  nickName = lst[3],  tgWallet="";
		int iCoinSplit=0;
		if( ic > 4 ){ tgWallet = lst[4]; }     if( ic > 5 ){ iCoinSplit = lst[5].toInt(); }
		if( sOpt == "BitChainRep:" )
		{
			ui->chain_tabWidget->setCurrentIndex(iPage);
			if( iPage == 1 )
			{
				ui->lEdit_LoveName->setText(nickName);     ui->lEdit_loveParentTopic->setText(tx);
				loveChainAction->setChecked(true);     ui->ptEdit_loveWords->setFocus();
				targetLoveWallet = tgWallet;   iRwdLoveRatio = iCoinSplit;
			}
			else if( iPage == 2 )
			{
				ui->lEdit_kidsParentTopic->setText(tx);     ui->lEdit_BabyName->setText(nickName);
				kidsChainAction->setChecked(true);     ui->ptEdit_kidInfo->setFocus();
				targetBabyWallet = tgWallet;   iRwdBabyRatio = iCoinSplit;
			}
			else if( iPage == 3 )
			{
				ui->lEdit_BlessName->setText(nickName);     ui->lEdit_blessParentTopic->setText(tx);
				blessChainAction->setChecked(true);     ui->ptEdit_blessWords->setFocus();
				targetBlessWallet = tgWallet;   iRwdBlessRatio = iCoinSplit;
			}
			else if( iPage == 4 )
			{
				ui->lEdit_eventParentTopic->setText(tx);     ui->lEdit_EventName->setText(nickName);
				eventChainAction->setChecked(true);     ui->ptEdit_eventTxt->setFocus();
				targetEventWallet = tgWallet;   iRwdEventRatio = iCoinSplit;
			}
			else if( iPage == 5 )
			{
				ui->lEdit_ProphesyName->setText(nickName);     ui->lEdit_prophesyParentTopic->setText(tx);
				prophesyChainAction->setChecked(true);     ui->ptEdit_prophesyTxt->setFocus();
				targetProphesyWallet = tgWallet;   iRwdProphesyRatio = iCoinSplit;
			}
			else if( iPage == 6 )
			{
				ui->lEdit_contractParentTopic->setText(tx);     ui->lEdit_ContractName->setText(nickName);
				contractChainAction->setChecked(true);     ui->ptEdit_contractInfo->setFocus();
				targetContractWallet = tgWallet;   iRwdContractRatio = iCoinSplit;
			}
			else if( iPage == 7 )
			{
				ui->lEdit_medicalParentTopic->setText(tx);     ui->lEdit_MedicalName->setText(nickName);
				medicalChainAction->setChecked(true);     ui->ptEdit_medicalInfo->setFocus();
				targetMedicalWallet = tgWallet;   iRwdMedicalRatio = iCoinSplit;
			}
			else if( iPage == 8 )
			{
				ui->lEdit_forumParentTopic->setText(tx);     ui->lEdit_ForumName->setText(nickName);
				forumChainAction->setChecked(true);     ui->ptEdit_forumTxt->setFocus();
				targetForumWallet = tgWallet;   iRwdForumRatio = iCoinSplit;
			}
		}
	}
	return rzt;
}
