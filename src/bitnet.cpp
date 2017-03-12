// Copyright (c) 2014 BitNet Team
//#ifdef USE_BITNET
#include "bitnet.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <stdint.h>
#include <string>
#include "ui_interface.h"
#include "util.h"
#include "init.h"
#include "bitcoinrpc.h"
#include "bitcoingui.h"
#include "txdb.h"
#include "walletdb.h"
#include "wallet.h"
#include "net.h"
#include <QLocale>
#include <QSettings>
#include <QTextCodec>
#include "notificator.h"
#include "sendcoinsentry.h"
#include "walletmodel.h"
#include "overviewpage.h"
#include "clientmodel.h"

#include "MemoryModule.h"
#include "json\json_spirit_reader.h"

#ifdef USE_WEBKIT
#include <QWebPage>
#include <QWebView>
#include <QWebFrame>
//#include <QUrl>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include "webPage.h"
#include "qwinwidget.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QMovie>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>
#include <QStyle>
#include <QSettings>
#include <QDesktopWidget>
#include <QListWidget>
#include <iostream>
#include <qsystemtrayicon>
#include <QToolButton>
#include "bitcoinunits.h"
#include "coincontrol.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

CCriticalSection cs_vBitNet;
//DWORD d_iVersion = 1113;
Notificator *vpnNotificator = NULL;
SendCoinsEntry *vpnSendCoinsEntry = NULL;
BitcoinGUI *vpnBitcoinGUI = NULL;

#ifdef USE_WEBKIT
QWinWidget *BitNetWebChatPage = NULL; 
QWinWidget *BitNetWebChatPage2 = NULL; 
QWebView *bitNet_chatView1 = NULL;	// 2015.01.06 add
QWebView *bitNet_chatView2 = NULL;
HWND gBitNetWebChatHwnd1 = 0;
HWND gBitNetWebChatHwnd2 = 0;
int g_iWebViewId = 0;
int g_WebView1Connected = 0;
int g_WebView2Connected = 0;

QWinWidget *BitNetWebChatPage3 = NULL;   // 2015.06.13 add
QWebView *bitNet_chatView3 = NULL;
int g_WebView3Connected = 0;
HWND gBitNetWebChatHwnd3 = 0;
#endif

int dStartVpnClient = 0;
DWORD dStartVpnServer = 0;
std::string sVpnIni = "BitNet.ini";
std::string sVpnDll = ""; //"BitNet.dat";	//"BitNet.dll";
HWND gCoinGuiHwnd = 0;
HWND gBitNetPageHwnd = 0;	// 2014.11.05 add
HWND gTrayIconHwnd = 0;
std::string sSysLang = "";
std::string sAppPath = "";
std::string sVpnIniFull = "";

//-- VPN service var
DWORD bShowInOtherList = 0;
DWORD bShowWAddrInOtherNodes = 0;
DWORD isVpnServer = 0;
DWORD iOpenSocketProxy = 0;
WORD iVpnServicePort = 0;
WORD iVpnServiceCtrlPort = 0;
DWORD d_Open_P2P_Proxy = 0;
WORD  d_P2P_proxy_port = 0;
DWORD d_P2P_Proxy_Port_mapped = 0;
int64_t iVpnServiceFee = 0;
DWORD iVpnServiceTryMinute = 0;
DWORD iVpnSerCoinConfirms = 0;
DWORD d_Vpn_LanID = 0;

char b_OnlineRewardVpnTime = 0;
WORD w_OnlineXminReward_1Min = 0;

DWORD iTotalVpnConnects = 0;
std::string sDefWalletAddress = "";
std::string sVpnWalletAddress = "";
std::string sVpnNicknamePack = "";

DWORD iVpnSendToSpecifiedFlag = 0;
DWORD iVpnSendToSpecifiedType = 0;
int64_t iVpnSendToSpecifiedCoins = 0;
std::string iVpnSendToSpecifiedAddressTo = "";
std::string iVpnSendToSpecifiedAddressFrom = "";

DWORD iVpnMemSize = 0;
DWORD iVpnCpuCore = 0;
std::string sCpuOem = "";
std::string sVpnMemAndCpuInfo = "";

//-- VPN chat
DWORD dUseChat = 0;
//DWORD g_chat_thread_start = 0;
//DWORD g_chat_act = 0;	// hava a msg need send to
//char* g_chat_str = NULL;
//CNode* g_cur_chat_node = NULL;


int bVpnDllInited = 0;
HINSTANCE hBitNetDll = 0;
//HMODULE hBitNetDll = 0;
lpOpenBitNetCenter OpenBitNetCenter = NULL;
lpStart_Vpn_Client pStart_Vpn_Client = NULL;
lpStart_Vpn_Server pStart_Vpn_Server = NULL;
lpSyncNode pVpnSyncNode = NULL;
lpInitVpnDll pInitVpnDll = NULL;
lpSyncTransaction pSyncTransaction = NULL;
lpSendBitNetWebChatMsg SendBitNetWebChatMsg = NULL;	// 2015.01.07 add
lpDropEvent BitNetDropEventFunc = NULL;	// 2015.01.28 add

//-- 2014.12.23 add
std::string s_AutoRelayNode = "";
std::string s_AutoSyncNode = "";
std::string s_Reqwalletnode = "";
DWORD bServiceMode = 0;

std::string s_Coin_conf_file = "";
std::string s_blockchain_data_dir = "";
std::string s_Coin_wallet_file = "";
std::string s_Coin_max_unit = "";

std::string s_Cur_Cheque_TxID = "";
int64_t i6_Cur_Cheque_Amount = 0;

std::string sBlockChain_AdBonus_Dir = "";
std::string sTxMonitor_ini = "";
std::string sTxMonitor_ini_utf8 = "";
std::string sLottery_ini_utf8 = "";

extern int BitNet_Version;
extern int BitNet_Network_id;
extern int fNetDbg;
extern int64_t GetAccountBalance(const string& strAccount, int nMinDepth);
extern int64_t  getBetAmountFromTxOut(const CTransaction& tx, const string sLotteryGenAddr, std::vector<std::pair<string, string> >* entry, CCoinControl* coinControl = NULL);
extern int64_t AmountFromValue(const Value& value);

DWORD WINAPI SendCoinToAddress(const char* pFromAccount, const char* pToAddress, int64_t nAmount, PCHAR pTxMsg, int bThread, PCHAR pRztTxID, int bEncryptMsg);

//lpReadIni pReadIni = NULL;

double MyValueFromAmount(int64_t amount)
{
    return (double)amount / (double)COIN;
}

#ifdef USE_WEBKIT		//2014.12.19 Bit_Lee
void SetWebViewOpt2(QWebView *aWebView, bool bNoContextMenu, bool bCookie)
{
	if( aWebView != NULL )
	{		
		char ba[9];
		//memset(ba, 0, sizeof(buf)); 		
		ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'a'; ba[5] = 'd'; ba[6] = 'm'; ba[7] = 0;
		string s = ba;	//"-bewadm"
		int bWebAdm = GetArg(s.c_str(), 0);	//GetArg("-bewadm", 0);	
//if(fDebug){ printf("---> SetWebViewOpt2 [%d] [%s]\n", bWebAdm, s.c_str()); }
			
		if( bNoContextMenu && (bWebAdm == 0) ){ aWebView->setContextMenuPolicy(Qt::NoContextMenu); }
		//aWebView->setStatusTip(tr("Press F5 to refresh"));		
				
		//aWebView->setAttribute(Qt::WA_TranslucentBackground, true);
		aWebView->setVisible(true);		

		ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'c'; ba[5] = 'o'; ba[6] = 'k'; ba[7] = 0;
		s = ba;	//"-bewcok"
		int bCook = GetArg(s.c_str(), 0);
		
		if( bCook > 0 ){	//if( bCookie || bCook ){
			QNetworkCookieJar *cookieJar2 = new QNetworkCookieJar();
			QNetworkAccessManager *nam2 = new QNetworkAccessManager();
			nam2->setCookieJar(cookieJar2);
			aWebView->page()->setNetworkAccessManager(nam2);
		}
			
		//aWebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

		ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'j'; ba[5] = 'p'; ba[6] = 'k'; ba[7] = 0;
		s = ba;	//"-bewjpk"
		bCook = GetArg(s.c_str(), 1);
		//if( bCook > 0 ){
			aWebView->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, bCook);
			aWebView->page()->settings()->setAttribute(QWebSettings::JavaEnabled, bCook);
			aWebView->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, bCook);
		//}
		
		ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'r'; ba[5] = 'o'; ba[6] = 'k'; ba[7] = 0;
		s = ba;	//"-bewrok"
		bCook = GetArg(s.c_str(), 0);
		if( bCook > 0 ){
			aWebView->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
			aWebView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
			aWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
			aWebView->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
			aWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
			aWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
			aWebView->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));		
		}
		//aWebView->installEventFilter(this);
		//connect(aWebView, SIGNAL( linkClicked(QUrl) ), this, SLOT( handleWebviewClicked(QUrl) ));
		aWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
		//connect(aWebView, SIGNAL(loadFinished(bool)), SLOT(BitNet_finishLoading(bool))); 
		
		if( bWebAdm ){	aWebView->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); }
	}
}

/*void BitNet_populateJavaScriptWindowObject(){
   string sName = strprintf("BitNet%d", g_iWebViewId); 
   QString qstr = QString::fromStdString(sName);
   if( g_iWebViewId == 1 ){ bitNet_chatView1->page()->mainFrame()->addToJavaScriptWindowObject(qstr, vpnBitcoinGUI); }
   else if( g_iWebViewId == 2 ){ bitNet_chatView2->page()->mainFrame()->addToJavaScriptWindowObject(qstr, vpnBitcoinGUI); }
if(fDebug){ printf("BitNet_populateJavaScriptWindowObject [%d] [%s]\n", g_iWebViewId, sName.c_str()); }
} */
#endif

extern double DOB_DPI_RAT;
HWND WINAPI BrowserUrlOrRunJava(HWND m_Wind, int WebViewId, int iOpt, PCHAR pUrl, int w, int h)
{
	HWND rzt = 0;
	//string sUrl = "";
	//if( pUrl != NULL ){ sUrl = pUrl; }
//if(fDebug && pUrl){ printf("---> BrowserUrlOrRunJava [%d:%d] [%d:%d] [%s]\n", WebViewId, iOpt, w, h, pUrl); }
  try{
	QString qUrl = "";
	if( pUrl != NULL ){ 
		qUrl = QString::fromUtf8(pUrl);
		//if( fDebug ){ printf("[%s] [%s]\n", pUrl, qUrl.toStdString().c_str()); }
	}
	if( WebViewId == 1 )
	{ 
		if( iOpt == 1 )
		{ 
			//view->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath("../Help/IDTNVMeMgr-web/index.html"))));
			if( w == 1 ){bitNet_chatView1->load(QUrl::fromLocalFile(qUrl)); }
			else if( w == 2 ){ bitNet_chatView1->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath(qUrl)))); }
			else{ bitNet_chatView1->load(QUrl(qUrl)); }
			g_iWebViewId = 1;
			if( g_WebView1Connected == 0 )
			{
				QWidget::connect( bitNet_chatView1->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), vpnBitcoinGUI, SLOT(BitNet_populateJavaScriptWindowObject()) );
				g_WebView1Connected++;
			}
			rzt++; 
		}
		else if( iOpt == 2 )
		{ 
			QString qStr = QString::fromUtf8(pUrl);
			bitNet_chatView1->page()->mainFrame()->evaluateJavaScript(qStr); rzt++; 
		}
		else if( iOpt == 3 )
		{ 
			if( BitNetWebChatPage == NULL )
			{
				BitNetWebChatPage = new QWinWidget(m_Wind, vpnBitcoinGUI);	// 2015.01.07 add
				//BitNetWebChatPage->setAttribute(Qt::WA_TranslucentBackground);
				BitNetWebChatPage->show();
			}
			if( bitNet_chatView1 == NULL )
			{
				bitNet_chatView1 = new QWebView(BitNetWebChatPage);
				
				BitNetWebChatPage->setLayout(new QGridLayout);
				BitNetWebChatPage->layout()->setContentsMargins(0, 0, 0, 0);								
				BitNetWebChatPage->layout()->addWidget(bitNet_chatView1);
				
				webPage* page = new webPage(bitNet_chatView1);
				bitNet_chatView1->setPage(page);	
				bitNet_chatView1->setVisible(true);	
				QWidget::connect(page, SIGNAL(openLink(QUrl)), vpnBitcoinGUI, SLOT(openUrl(QUrl)));
				
				SetWebViewOpt2(bitNet_chatView1, false, false);
                bitNet_chatView1->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
				//bitNet_chatView1->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);				
			}
			if( (w > 0) && (SendBitNetWebChatMsg == NULL) ){ SendBitNetWebChatMsg = lpSendBitNetWebChatMsg(w); }
			gBitNetWebChatHwnd1 = BitNetWebChatPage->winId();
			rzt = gBitNetWebChatHwnd1;
			//if( bitNet_chatView1 != NULL ){SetWindowPos(bitNet_chatView1->winId(), HWND_TOP, 0, 0, w, h, SWP_NOACTIVATE | SWP_DRAWFRAME); rzt++;} 
		}
		//ebView->page()->mainFrame()->evaluateJavaScript(QString("myFunction();"));
		//bitNet_chatView1->setVisible(true);	
		//if( bitNet_chatView1 ){ bitNet_chatView1->show(); }
		//rzt = (DWORD)bitNet_chatView1->winId();
	}
	else if( WebViewId == 2 )
	{ 
		if( iOpt == 1 )
		{ 
			//view->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath("../Help/IDTNVMeMgr-web/index.html"))));
			if( w == 1 ){bitNet_chatView2->load(QUrl::fromLocalFile(qUrl)); }
			else if( w == 2 ){ bitNet_chatView2->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath(qUrl)))); }
			else{ bitNet_chatView2->load(QUrl(qUrl)); }
			g_iWebViewId = 2;
			if( g_WebView2Connected == 0 )
			{
				QWidget::connect( bitNet_chatView2->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), vpnBitcoinGUI, SLOT(BitNet_populateJavaScriptWindowObject2()) );
				g_WebView2Connected++;
			}
			rzt++; 
		}
		else if( iOpt == 2 )
		{ 
			QString qStr = QString::fromUtf8(pUrl);
			bitNet_chatView2->page()->mainFrame()->evaluateJavaScript(qStr); rzt++; 
		}
		else if( iOpt == 3 )
		{ 
			if( BitNetWebChatPage2 == NULL )
			{
				BitNetWebChatPage2 = new QWinWidget(m_Wind, vpnBitcoinGUI);	// 2015.01.07 add
				//BitNetWebChatPage2->setAttribute(Qt::WA_TranslucentBackground);
				BitNetWebChatPage2->show();
			}
			if( bitNet_chatView2 == NULL )
			{
				bitNet_chatView2 = new QWebView(BitNetWebChatPage2);
				
				BitNetWebChatPage2->setLayout(new QGridLayout);
				BitNetWebChatPage2->layout()->setContentsMargins(0, 0, 0, 0);								
				BitNetWebChatPage2->layout()->addWidget(bitNet_chatView2);
				
				webPage* page = new webPage(bitNet_chatView2);
				bitNet_chatView2->setPage(page);	
				bitNet_chatView2->setVisible(true);	
				QWidget::connect(page, SIGNAL(openLink(QUrl)), vpnBitcoinGUI, SLOT(openUrl(QUrl)));
				
				SetWebViewOpt2(bitNet_chatView2, false, false);	
                bitNet_chatView2->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
			}
			if( (w > 0) && (SendBitNetWebChatMsg == NULL) ){ SendBitNetWebChatMsg = lpSendBitNetWebChatMsg(w); }
			gBitNetWebChatHwnd2 = BitNetWebChatPage2->winId();
			rzt = gBitNetWebChatHwnd2;
		}
		//if( bitNet_chatView2 ){ bitNet_chatView2->show(); }
	}
	
	else if( WebViewId == 3 )
	{ 
		if( iOpt == 1 )
		{ 
			if( h > 0 )	// screen cap
			{
				//if( fDebug ){ printf("---> BrowserUrlOrRunJava h > 0, do screen cap\n"); }
				QPixmap pix = QPixmap::grabWindow(QApplication::desktop()->winId());
				if( !pix.isNull() ){ pix.save("res\\bonus\\bg.jpg");
				//QPalette palette;
			    //int sWid = QApplication::desktop ()->width(); //屏幕尺寸设置，自己要写别的函数修改尺寸的改这里  
			    //int sHei  = QApplication::desktop ()->height();  
				//palette.setBrush(BitNetWebChatPage3->backgroundRole(), QBrush(pix.scaled(sWid, sHei,Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
				//palette.setBrush( BitNetWebChatPage3->backgroundRole(), QBrush(pix) );
				//BitNetWebChatPage3->setPalette(palette);
				
				QPalette pal;
				pal.setBrush(QPalette::Window, QBrush(pix));
				BitNetWebChatPage3->setPalette(pal);
				//BitNetWebChatPage3->setPixmap(pix);
				}
			}
			//view->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath("../Help/IDTNVMeMgr-web/index.html"))));
			if( w == 1 ){bitNet_chatView3->load(QUrl::fromLocalFile(qUrl)); }
			else if( w == 2 ){ bitNet_chatView3->load(QUrl::fromLocalFile(QDir::cleanPath(QDir::current().absoluteFilePath(qUrl)))); }
			else{ bitNet_chatView3->load(QUrl(qUrl)); }
			g_iWebViewId = 3;
			if( g_WebView3Connected == 0 )
			{
				QWidget::connect( bitNet_chatView3->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), vpnBitcoinGUI, SLOT(BitNet_populateJavaScriptWindowObject3()) );
				g_WebView3Connected++;
			}
			rzt++; 
		}
		else if( iOpt == 2 )
		{ 
			QString qStr = QString::fromUtf8(pUrl);
			bitNet_chatView3->page()->mainFrame()->evaluateJavaScript(qStr); rzt++; 
		}
		else if( iOpt == 3 )
		{ 
			if( BitNetWebChatPage3 == NULL )
			{
				BitNetWebChatPage3 = new QWinWidget(m_Wind, vpnBitcoinGUI);	// 2015.01.07 add
				BitNetWebChatPage3->setAutoFillBackground(true);
				//BitNetWebChatPage3->setAttribute(Qt::WA_TranslucentBackground);
				BitNetWebChatPage3->show();
			}
			if( bitNet_chatView3 == NULL )
			{
				bitNet_chatView3 = new QWebView(BitNetWebChatPage3);  // 3
				//bitNet_chatView3->setAttribute(Qt::WA_TranslucentBackground);
				
				BitNetWebChatPage3->setLayout(new QGridLayout);
				BitNetWebChatPage3->layout()->setContentsMargins(0, 0, 0, 0);								
				BitNetWebChatPage3->layout()->addWidget(bitNet_chatView3);
				BitNetWebChatPage3->setStyleSheet("QWebView{background:transparent;} QFrame{background:transparent;}");	
				
				webPage* page = new webPage(bitNet_chatView3);
				bitNet_chatView3->setPage(page);	
				bitNet_chatView3->setVisible(true);	
				QWidget::connect(page, SIGNAL(openLink(QUrl)), vpnBitcoinGUI, SLOT(openUrl(QUrl)));
				
				SetWebViewOpt2(bitNet_chatView3, false, false);
			}
			if( (w > 0) && (SendBitNetWebChatMsg == NULL) ){ SendBitNetWebChatMsg = lpSendBitNetWebChatMsg(w); }
			gBitNetWebChatHwnd3 = BitNetWebChatPage3->winId();
			rzt = gBitNetWebChatHwnd3;
		}
		else if( iOpt == 4 )  // screen cap
		{
			//int screenWidget = QApplication::desktop ()->width(); //屏幕尺寸设置，自己要写别的函数修改尺寸的改这里  
			//int screenHight  = QApplication::desktop ()->height();  
			//就是这一步截取了全屏  
			//QPixmap fullScreen = QPixmap::grabWindow(QApplication::desktop()->winId(),0,0,screenWidget,screenHight);
			QPixmap originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
			QPalette palette;
			palette.setBrush( BitNetWebChatPage3->backgroundRole(), QBrush(originalPixmap) );
			BitNetWebChatPage3->setPalette(palette);
		}
	}

  }catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
  }
//printf("<--- BrowserUrlOrRunJava [%d:%d] [%d:%d] [%s]\n", WebViewId, iOpt, w, h, pUrl);
	return rzt;
}

std::string SplitHostAndPort(std::string str,  int* iPort)
{
	std::string rzt = "";
	if( iPort != NULL )
	{
		int dEc = str.find(":");
		if (dEc != string::npos)
		{
			std::string sPort = str.substr(dEc + 1);
			if( sPort.size() )
			{
				*iPort = atoi64(sPort); 
			}
			rzt = str.substr(0, dEc);
		}else{
			rzt = str;
		}
	}else{ rzt = str; }
	return rzt;
}

void GetDefaultWalletAddress()
{
	if( pwalletMain )
	//if( dStartVpnServer || dUseChat )
	{	
		if( sDefWalletAddress.length() < 30 )
		{
			char ba[16];
			//memset(ba, 0, sizeof(buf)); 		
			ba[0] = 'B'; ba[1] = 'i'; ba[2] = 't'; ba[3] = 'N'; ba[4] = 'e'; ba[5] = 't'; ba[6] = '-'; ba[7] = 'V'; ba[8] = 'P'; ba[9] = 'N'; ba[10] = 0;
			string s = ba;	//"BitNet-VPN"
			sDefWalletAddress = getAddressesbyaccount(s);	//-- 2014.12.19   Bit_Lee
			if( sDefWalletAddress.length() == 0 )
				sDefWalletAddress = CBitcoinAddress(pwalletMain->vchDefaultKey.GetID()).ToString();
		}
		//if(fDebug)
		//	{ printf("Default Wallet Address %s\n", sDefWalletAddress.c_str()); }
	}
}

void   CALLBACK  TimerProc(HWND  hWnd, UINT nMsg, UINT nTimerid, DWORD  dwTime)   
{ 
	int64_t ttt = GetTime();
	//if( fDebug ){ printf("ttt=[%I64u], hWnd =[%u], nMsg =[%u], nTimerid = [%u], dwTime = [%u] \n", ttt, hWnd, nMsg, nTimerid, dwTime); }
	if( (nTimerid == 123) && (dwTime >= 1000) )
	{
		if( IsWindowVisible(gCoinGuiHwnd) )	//if( vpnBitcoinGUI->BitNetPage->width() > 500 )
		{
			int id = GetArg("-showbntui", 1);	// to VPN UI
			if( (id > 0) && (vpnBitcoinGUI != NULL) )
			{
				if( id == 1 ){ vpnBitcoinGUI->gotoBitNetClicked(); }
				else if( id == 2 ){ vpnBitcoinGUI->gotoVpnClicked(); }
				else if( id == 3 ){ vpnBitcoinGUI->gotoLotteryClicked(); }	
				else if( id == 4){ vpnBitcoinGUI->gotoChequeClicked(); }	
				else if( id == 5){ vpnBitcoinGUI->gotoBonusClicked(); }	
			}
			KillTimer(hWnd, nTimerid);
		}
	}
} 

void gotoBntUI()
{
	//int64_t ttt = GetTime();
	//if( fDebug ){ printf("gotoBntUI:: ttt=[%I64u]\n", ttt); }
	if( (gCoinGuiHwnd > 0) && (vpnBitcoinGUI != NULL) ){ SetTimer(gCoinGuiHwnd, 123, 1000, TimerProc); }
}

int SaveBonusMsgToBlockChainAdBonusDir(string sBonusId, string sBonus)
{
	int rzt = 0;
	if( (sBonusId.length() < 10) || (sBonus.length() < 20) ){ return rzt; }
	string sFile = sBlockChain_AdBonus_Dir + sBonusId;  //GetDataDir().string() + "\\" + sBonusId;
	ofstream myfile; 
	myfile.open (sFile.c_str());
	if( myfile.is_open() )
	{
		myfile << sBonus;
		myfile.close(); 
		rzt++; 
	}
	return rzt;
}

int SaveTxToMonitorfile(string sTxId, string sMsg, unsigned int iHi)
{
	int rzt = 0;
	string sFile = sTxMonitor_ini_utf8;  //strprintf("%s\\TxMonitor.ini", sAppPath.c_str()); 
if( fDebug ){ printf("SaveTxToMonitorfile:: [%s]\n", sFile.c_str()); }
	QSettings *ConfigIni = new QSettings(QString::fromUtf8(sFile.c_str()), QSettings::IniFormat, 0); 
	ConfigIni->setIniCodec(QTextCodec::codecForName("GB2312")); 
	QString qId = QString::fromStdString(sTxId);
	//ConfigIni->setValue("/" + qId + "/a", 1);
	SaveBonusMsgToBlockChainAdBonusDir(sTxId, sMsg);
	ConfigIni->setValue("/" + qId + "/Hei", iHi);
	ConfigIni->setValue("/" + qId + "/adbns", 99);  //QString::fromStdString(sMsg));
	delete ConfigIni;  rzt++;   return rzt;
}

extern bool get_Txin_prevout_n_s_TargetAddressAndAmount(const CTransaction& tx, unsigned int n, string& sTargetAddress, int64_t& iAmnt);
extern int RelayBonusReq(string sBonusId);
extern string s_Dont_Relay_AdBonus;
extern int  GetCoinAddrInTxOutIndex(const CTransaction& tx, string sAddr, int64_t v_nValue, int iCmpType = 0);
extern int  GetCoinAddrInTxOutIndex(const string txID, string sAddr, int64_t v_nValue, int iCmpType = 0);

bool isFromMyWallet(const CTransaction& tx )
{
	bool rzt = false;
	if( pwalletMain )
	{
		if (pwalletMain->IsFromMe(tx)) rzt = true; 
	}
	return rzt;
}

bool isToMyWallet(const CTransaction& tx )
{
	bool rzt = false;
	if( pwalletMain )
	{
		if (pwalletMain->IsMine(tx)) rzt = true; 
	}
	return rzt;
}

int IsMonitorTx(unsigned int nTime, unsigned int iHi, string Txid, const CTransaction& tx, unsigned int n)
{
	int rzt = 0;
	try{
	//string sFile = sTxMonitor_ini_utf8;  //strprintf("%s\\TxMonitor.ini", sAppPath.c_str()); 
//if( fDebug ){ printf("IsMonitorTx:: [%s]\n", sFile.c_str()); }
	string sTxMsg = tx.vpndata.c_str();	// BitNet AdBonus:

	if( (n == 0xFFFFFFFF) && (sTxMsg.length() > 53) )  // 2015.08.16 add  //(bServiceMode == 0)
	{
		int dEc = sTxMsg.find("BitNet Domain:");
		if (dEc == 0 )    //string::npos)	
		{
			if( (GetArg("-acceptdomain", 1) > 0) && (!isFromMyWallet(tx)) )
			{
				int iTxIdx = GetCoinAddrInTxOutIndex(tx, sDefWalletAddress, 0, 3);
				if( iTxIdx >= 0 )
				{
					string sDomain = sTxMsg.substr(14);
					if( fDebug ){ printf("IsMonitorTx:: Receive Domain name [%s]\n", sDomain.c_str()); }
					if( sDomain.length() > 60 )
					{
					if( pVpnSyncNode != NULL ){ pVpnSyncNode(12, sDomain.length(), (CNode*)3, (Bitnet_node_struct*)sDomain.c_str()); }
					rzt++;
					string sInfo = "Receive BitNet Domain Name";  //strprintf("%s post a Lottery", sMaker.c_str());
					QString qInf = QString::fromStdString(sInfo);
					QString qMsg1 = QString::fromStdString(sDomain);
					vpnNotificator->notify(Notificator::Information, qInf, qMsg1);
					}
				}
			}
			return rzt;
		}
		else if( GetArg("-repdomainreg", 0) > 0 )
		{
			dEc = sTxMsg.find("Buy BitNet Domain:");  // Buy BitNet Domain:xxx.com|Vpn Address...
			if( dEc == 0 )
			{
				//int iTxIdx = GetCoinAddrInTxOutIndex(tx, "VpfHY7zLdWFVG8cDYzCk78ier7gsHo4E3v", 0, 3);
				int64_t i6Amount = getBetAmountFromTxOut(tx, "VpfHY7zLdWFVG8cDYzCk78ier7gsHo4E3v", NULL);
				if( isToMyWallet(tx) && (i6Amount > MIN_TX_FEE) )  // MIN_TX_FEE = 10000
				{
					string sDomain = sTxMsg.substr(18);
					if( fDebug ){ printf("IsMonitorTx:: Receive Buy Domain name [%s]\n", sDomain.c_str()); }
					if( sDomain.length() > 35 )  // xxx.com|vpn address...
					{
						double db = MyValueFromAmount(i6Amount);
						sDomain = strprintf("%s|%f", sDomain.c_str(), db);
						if( pVpnSyncNode != NULL ){ pVpnSyncNode(12, sDomain.length(), (CNode*)4, (Bitnet_node_struct*)sDomain.c_str()); }
						rzt++;
						string sInfo = "Receive Buy BitNet Domain Name Request";  //strprintf("%s post a Lottery", sMaker.c_str());
						QString qInf = QString::fromStdString(sInfo);
						QString qMsg1 = QString::fromStdString(sDomain + "VPN");
						vpnNotificator->notify(Notificator::Information, qInf, qMsg1);
					}
				}
				return rzt;
			}
		}
	}
	
	QSettings *ConfigIni = new QSettings(QString::fromUtf8(sTxMonitor_ini_utf8.c_str()), QSettings::IniFormat, 0); 
	ConfigIni->setIniCodec(QTextCodec::codecForName("GB2312")); 
	QString qId = QString::fromStdString(Txid);
	//int i = ConfigIni->value("/" + qId + "/a", "0").toInt();
	string s = ConfigIni->value("/" + qId + "/adbns", "").toString().toStdString();
	bool bSyncToGui = s.length() > 0;
	if( s.length() == 0 )  //if( (bServiceMode == 0) && (s.length() == 0) )
	{
		if( sTxMsg.length() > 16 )
		{
			int dEc = sTxMsg.find("BitNet AdBonus:");
			if (dEc == 0 )    //string::npos)
			{
				int64_t iNow = time(NULL);
				if( (iNow > nTime) && ((iNow - nTime) > (60 * 60 * 24)) )
				{
					// big than 24 hours, nothing to do
				}else{
					s = "99";   ConfigIni->setValue("/" + qId + "/adbns", 99); 
					string sAmnt = sTxMsg.substr(15);
					QString qAmnt = QString::fromStdString(sAmnt);
					ConfigIni->setValue("/" + qId + "/Amount", qAmnt);
					if( bServiceMode == 0 )
					{
						string sFrm = "Receive a BitNet Ad Bonus";  //strprintf("%s post a Lottery", sMaker.c_str());
						sAmnt = sAmnt + s_Coin_max_unit;
						QString qFrm = QString::fromStdString(sFrm);
						QString qMsg = QString::fromStdString(sAmnt);
						vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
					}
					s_Dont_Relay_AdBonus = s_Dont_Relay_AdBonus + Txid + ";";
					RelayBonusReq(Txid);
				}
			}
		}
	}
	
	if( s.length() > 0 )
	{
		if( n == 0xFFFFFFFF )
		{
			//s = ConfigIni->value(qq, "").toString().toStdString();
			//string s = ConfigIni->value("/" + qId + "/adbns", "").toString().toStdString();
			ConfigIni->setValue("/" + qId + "/time", nTime);
			ConfigIni->setValue("/" + qId + "/Hei", iHi);
			delete ConfigIni;
			//bool b = s.length() > 30;
			//if( b ){ ConfigIni->remove(qId); }
			//if( b )
		}
		else{
			string sTargetAddress = "";
			int64_t iAmnt = 0, i6 = 0;
			if( get_Txin_prevout_n_s_TargetAddressAndAmount(tx, n, sTargetAddress, iAmnt) )
			{
				if( iAmnt > 0 ){ iAmnt = iAmnt / COIN; }
				if( iAmnt > 0 )
				{
					//iAmnt = iAmnt / COIN;
					i6 = ConfigIni->value("/" + qId + "/Took", "0").toInt();
					iAmnt = iAmnt + i6;
					ConfigIni->setValue("/" + qId + "/Took", iAmnt);
					/*i6 = ConfigIni->value("/" + qId + "/Amount", "0").toInt();
					if( i6 > iAmnt )
					{
						i6 = i6 - iAmnt;
						ConfigIni->setValue("/" + qId + "/Amount", i6);
					}*/
				}
				if( fDebug ){ printf("IsMonitorTx:: up amount [%I64u] - [%I64u], sTargetAddress = [%s]\n", i6, iAmnt, sTargetAddress.c_str()); }
			}
			delete ConfigIni;
		}
		if( bSyncToGui )
		{ 
			if( fDebug ){ printf("IsMonitorTx:: bns = [%s]\n", s.c_str()); }
			if( pVpnSyncNode != NULL ){ pVpnSyncNode(12, Txid.length(), (CNode*)2, (Bitnet_node_struct*)Txid.c_str()); }
			rzt++;
		}
	}else{ delete ConfigIni; }
  }catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
  }
	return rzt;
}

int SyncGenLotteryToGui(unsigned int nTime, unsigned int iHi, int64_t aMount, string sMaker, string sId, string sTxMsg)
{
	int rzt = 0;
	try{
	if( (bServiceMode > 0) || (sId.length() < 34) || (sAppPath.length() == 0) ){ return rzt; }  // not init
	//IsBlockSynchronizing()
	//string sFile = strprintf("%s\\BlockLottery.ini", sAppPath.c_str()); 
//if( fDebug ){ printf("SyncGenLotteryToGui:: [%s]\n", sFile.c_str()); }
	QSettings *ConfigIni = new QSettings(QString::fromUtf8(sLottery_ini_utf8.c_str()), QSettings::IniFormat, 0); 
	QString qId = QString::fromStdString(sId);
	string s = "/l/" + sId;  // List
	QString qq = QString::fromStdString(s);
	//s = ConfigIni->value(qq, "").toString().toStdString();
	int i = ConfigIni->value(qq, "0").toInt();
	if( i > 0 )
	{ 
		delete ConfigIni;  //goto lab_quit;
		return rzt;
	}
if( fDebug ){ printf("SyncGenLotteryToGui:: add [%s]\n", sId.c_str()); }
	ConfigIni->setValue(qq, nTime);
	ConfigIni->setValue("/" + qId + "/msg", QString::fromStdString(sTxMsg));
	ConfigIni->setValue("/" + qId + "/time", nTime);
	ConfigIni->setValue("/" + qId + "/Hei", iHi);
	string sTt = strprintf("%I64u%s (%s)", aMount / COIN, s_Coin_max_unit.c_str(), sMaker.c_str());
	ConfigIni->setValue("/" + qId + "/tt", QString::fromStdString(sTt));
	delete ConfigIni;
	//SynNodeToBitNetGui( NULL, 12, sId.length(), sId.c_str() );
	{
		//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
		string sFrm = strprintf("%s post a Lottery", sMaker.c_str());
		QString qFrm = QString::fromStdString(sFrm);
		string str = strprintf("%I64u%s", aMount / COIN, s_Coin_max_unit.c_str());
		QString qMsg = QString::fromStdString(str);
		vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
		
	}
	if( pVpnSyncNode != NULL ){ pVpnSyncNode(12, sId.length(), (CNode*)1, (Bitnet_node_struct*)sId.c_str()); }
	rzt++;
  }catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
  }
//lab_quit:
	return rzt;
}

std::wstring StringToWString(const std::string &s)  //std::wstring StringToWString(std::wstring &ws, const std::string &s)
{
    std::wstring wsTmp(s.begin(), s.end());
    //ws = wsTmp;
    return wsTmp; //0;
}

extern std::string string_To_UTF8(const std::string & str) ;
extern int fGetMyExternalIPByBitNet;
void LoadIniCfg( DWORD bStart, DWORD dRelay )
{
	if( fDebug ){ OutputDebugStringA("--> LoadIniCfg"); }
	if( sSysLang.length() == 0 ){ sSysLang = QLocale::system().name().toStdString(); }
	
	char buf[512];
	memset(buf, 0, sizeof(buf)); 
	if( sAppPath.length() == 0 )
	{
		//GetModuleFileNameA( NULL, result, MAX_PATH );
		//std::string sFull = result;
		//printf("BitcoinGui: sFull=%s\n", sFull.c_str());
	
		GetCurrentDirectoryA(MAX_PATH, &buf[0]);
		sAppPath = buf;
		fGetMyExternalIPByBitNet = 1;
	}

	if( s_Coin_conf_file.length() == 0 ){ 
		s_Coin_conf_file = GetConfigFile().string();
	} 
	if( s_blockchain_data_dir.length() == 0 ){ 
		s_blockchain_data_dir = GetDataDir().string();
	} 
	if( s_Coin_wallet_file.length() == 0 ){ 
		bool b = true;
		string sw = GetArg("-wallet", "wallet.dat");
		if( sw.length() > 2 )
		{
			const char* p = sw.c_str();
			if( p[1] == ':' ){ b = false; }
		}
		if( b ){ 
		//std::string strWalletFileName = GetArg("-wallet", "wallet.dat");
		s_Coin_wallet_file = strprintf("%s\\%s", s_blockchain_data_dir.c_str(), sw.c_str());
		}else s_Coin_wallet_file = sw;
	}
	if( s_Coin_max_unit.length() == 0 ){
		s_Coin_max_unit = BitcoinUnits::name(BitcoinUnits::BTC).toStdString();
	}
/*	
if( fDebug ){
	printf("[%s]\n", s_Coin_conf_file.c_str());
	printf("[%s]\n", s_blockchain_data_dir.c_str());
	printf("[%s]\n", s_Coin_wallet_file.c_str());
} */

	if( sVpnIniFull.length() == 0 ){ 
		
		if( sAppPath.length() )
		{
			sVpnIniFull = strprintf("%s\\%s", buf, sVpnIni.c_str()); 
			//printf("BitcoinGui: sPath=%s\n", sAppPath.c_str());
		}
	}
	
	if( sBlockChain_AdBonus_Dir.length() == 0){
		sBlockChain_AdBonus_Dir = sAppPath + "\\BlockChain";  //\\AdBonus";
		boost::filesystem::create_directory(sBlockChain_AdBonus_Dir);
		sBlockChain_AdBonus_Dir = sBlockChain_AdBonus_Dir + "\\AdBonus";
		boost::filesystem::create_directory(sBlockChain_AdBonus_Dir);
		sBlockChain_AdBonus_Dir = sBlockChain_AdBonus_Dir + "\\";
		if( fDebug ){ printf("%s\n", sBlockChain_AdBonus_Dir.c_str()); }
	}
	if( sTxMonitor_ini.length() == 0 ){ 
		sTxMonitor_ini = sBlockChain_AdBonus_Dir + "TxMonitor.ini"; 
		sTxMonitor_ini_utf8 = string_To_UTF8(sTxMonitor_ini);
		//OutputDebugStringW((wchar_t *)sTxMonitor_ini_utf8.c_str());
		
		//std::wstring ws = StringToWString(sTxMonitor_ini);
		//std::wstring ws(sTxMonitor_ini.size(), L' '); // Overestimate number of code points.
		//ws.resize(std::mbstowcs(&ws[0], sTxMonitor_ini.c_str(), sTxMonitor_ini.size())); // Shrink to fit
		//OutputDebugStringW(ws.c_str());
		//OutputDebugStringW((wchar_t *)ws.c_str());
		
		/*wchar_t* pwszUnicode = "Holle, word! 你好，中国！";
		int iSize;
		char* pszMultiByte;
		iSize = WideCharToMultiByte(CP_ACP, 0, pwszUnicode, -1, NULL, 0, NULL, NULL);
		pszMultiByte = (char*)malloc((iSize+1)  );
		WideCharToMultiByte(CP_ACP, 0, pwszUnicode, -1, pszMultiByte, iSize, NULL, NULL);
		OutputDebugStringW((wchar_t *)pszMultiByte);
		delete []pszMultiByte; */
	}
	if( sLottery_ini_utf8.length() == 0 ){
		string sLot = strprintf("%s\\BlockLottery.ini", sAppPath.c_str());
		sLottery_ini_utf8 = string_To_UTF8(sLot);
	}
	
	if( sVpnDll.length() == 0 ){ 
		
		if( sAppPath.length() )
		{
			sVpnDll = strprintf("%s\\%s", sAppPath.c_str(), GetArg("-bitnetlib", "BitNet.dat").c_str()); 
		}
	}	
   if( fDebug ){ printf("%s\n", sVpnIniFull.c_str()); }
  
	/* QTextCodec *codec = QTextCodec::codecForName("GB2312");
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec); */
try{
	QSettings *ConfigIni = new QSettings(QString::fromUtf8(sVpnIniFull.c_str()), QSettings::IniFormat, 0); //sVpnIni
	//ConfigIni->setIniCodec(QTextCodec::codecForName("GB2312")); 	//WSetting.setIniCodec("UTF-8");
	ConfigIni->setIniCodec("GB2312");
	//QLocale::Country	QLocale::China 44; QLocale::HongKong 97
	DWORD dLan = QLocale::Country();
	//f( QLocale::system().name().toStdString()
	
	dStartVpnClient = ConfigIni->value("/config/start_client", "0").toInt();
	dStartVpnServer = ConfigIni->value("/config/start_server", "0").toInt();
	dUseChat        = ConfigIni->value("/config/use_chat", "1").toInt();
	iVpnServicePort = ConfigIni->value("/config/ServicePort", "923").toInt();
	iVpnServiceCtrlPort = ConfigIni->value("/config/ServiceCtrlPort", "922").toInt();
	iVpnServiceTryMinute = ConfigIni->value("/config/ServiceTryMinute", "0").toInt();
	d_Open_P2P_Proxy = ConfigIni->value("/config/Open_P2P_DM_Sys", "1").toInt();
	d_P2P_proxy_port = ConfigIni->value("/config/dm_name_port", "923").toInt();
	
	b_OnlineRewardVpnTime = ConfigIni->value("/config/OnlineRewardVpnTime", "0").toInt();
	w_OnlineXminReward_1Min = ConfigIni->value("/config/OnlineXminReward1Min", "0").toInt();
	
	iOpenSocketProxy = ConfigIni->value("/config/Start_Http_Svc", "1").toInt();
	int iHttp = ConfigIni->value("/config/Start_Https_Svc", "0").toInt();
	if( iHttp > 0 ){ iOpenSocketProxy = iOpenSocketProxy | 2; }
	iHttp = ConfigIni->value("/config/Start_P2P_Proxy", "0").toInt();
	if( iHttp > 0 ){ iOpenSocketProxy = iOpenSocketProxy | 4; }
	if( d_P2P_Proxy_Port_mapped ){ iOpenSocketProxy = iOpenSocketProxy | 8; }
	
	//double dFee = ConfigIni->value("/config/ServiceFee", "0").toDouble();
	//gCoinGuiHwnd = this->winId();
	//printf("BitcoinGui: dFee=%g, dLan=%u, %s\n", dFee, dLan, sSysLang.c_str());	// %f %lf %g %G
	//iVpnServiceFee = dFee * COIN;
	iVpnServiceFee = ConfigIni->value("/config/ServiceFee", "0").toInt();
	
	GetDefaultWalletAddress();
	sVpnWalletAddress.clear();
	GetPrivateProfileStringA("config", "MySignature", "", &buf[0], 128, sVpnIniFull.c_str());	//GetPrivateProfileString
	sVpnWalletAddress = buf;
	memset(buf, 0, sizeof(buf)); 
	//sVpnWalletAddress = ConfigIni->value("/config/MySignature", "").toString().toStdString();
	//if( sVpnWalletAddress.length() > 128 ){ sVpnWalletAddress.clear(); }
	
	//if( sVpnWalletAddress.length() < 30 ){ sVpnWalletAddress = sDefWalletAddress; }
	//iOpenSocketProxy = start_vpn_server(2);

	if( dStartVpnServer ){ mapArgs["-synctx2gui"] = "1"; }	//-- 2014.11.28 add
	else { mapArgs["-synctx2gui"] = "0"; }
	
	if( bStart )
	{
		if( dUseChat || dStartVpnClient || dStartVpnServer )
		{
			InitBitNetDll();
			if( dStartVpnClient ){ start_vpn_client(); }
			isVpnServer = start_vpn_server(1);	//if( dStartVpnServer ){ isVpnServer = start_vpn_server(); };
		}
	}
	
	if( sVpnNicknamePack.length() < 20 )
	{
		//sVpnNickname = ConfigIni->value("/config/NickName", "").toString().toStdString();
		GetPrivateProfileStringA("config", "NickName", "", &buf[0], 512, sVpnIniFull.c_str());	//GetPrivateProfileString
		//printf("BitcoinGui: NickName=%s, %x\n", &buf[0], GetLastError());
		sVpnNicknamePack = buf;
		//QString strPcName = QString::fromLocal8Bit(buf);
		//sVpnNickname = strPcName.toStdString();
	}
	
	bShowInOtherList =  ConfigIni->value("/config/show_in_other_list", "1").toInt();
	//bShowWAddrInOtherNodes =  ConfigIni->value("/config/show_walletaddress_in_other", "0").toInt();
	bShowWAddrInOtherNodes = 1;
	
	DWORD iMaxRecvSize =  ConfigIni->value("/config/max_recv_file_size", "5").toInt();
	//if( fDebug ){ printf("Set Maxreceivebuffer : %u, %d\n", iMaxRecvSize, GetArg("-maxreceivebuffer", 1*1000)); }
	if( !iMaxRecvSize ){ iMaxRecvSize++; }	// Must >= 1
	iMaxRecvSize = iMaxRecvSize * 1000;
	std::string sMaxRecv = strprintf("%d", iMaxRecvSize); 
	mapArgs["-maxreceivebuffer"] = sMaxRecv;	//SoftSetArg("-maxreceivebuffer", sMaxRecv);
	if( fDebug ){ printf("Set Maxreceivebuffer : %u, %u \n", iMaxRecvSize, GetArg("-maxreceivebuffer", 1*1000)); }
	
	DWORD dMaxconnections =  ConfigIni->value("/config/max_connections", "125").toInt();
	if(!dMaxconnections){ dMaxconnections = 125; }
	sMaxRecv = strprintf("%d", dMaxconnections); 
	mapArgs["-maxconnections"] = sMaxRecv;	//SoftSetArg("-maxconnections", sMaxRecv);
	if( fDebug ){ printf("Set Maxconnections : %u, %d \n", dMaxconnections, GetArg("-maxconnections", 1)); }

	iVpnSerCoinConfirms =  ConfigIni->value("/config/ser_confirms", "1").toInt();
	if( !iVpnSerCoinConfirms ){ iVpnSerCoinConfirms = 1; }
	
//-- 2014.12.23 add
	bServiceMode =  ConfigIni->value("/config/service_mode", "0").toInt();	// 2014.12.23 add
	if( s_AutoSyncNode.length() == 0 ){  s_AutoSyncNode = GetArg("-autosyncnode", "1"); }
	if( s_AutoRelayNode.length() == 0 ){ s_AutoRelayNode = GetArg("-autorelaynode", "1"); }
	if( s_Reqwalletnode.length() == 0 ){ s_Reqwalletnode = GetArg("-reqwalletnode", "1"); }
	
	DWORD bIsSeedNode =  ConfigIni->value("/config/-isseednode", "0").toInt();
	if( (bIsSeedNode > 0) || (bServiceMode > 0) )
	{
	    sMaxRecv = "0"; 
	    mapArgs["-autosyncnode"] = sMaxRecv;
	    mapArgs["-autorelaynode"] = sMaxRecv;	
	    mapArgs["-isseednode"] = "1";
	    mapArgs["-reqwalletnode"] = sMaxRecv;
	}
	else{
	    mapArgs["-autosyncnode"] = s_AutoSyncNode;
	    mapArgs["-autorelaynode"] = s_AutoRelayNode;
	    mapArgs["-isseednode"] = "0";
	    mapArgs["-reqwalletnode"] = s_Reqwalletnode;
	}

	if( fDebug ){ printf("Load BitNet Sets: VpnServiceFee=%I64u, UseChat=%u, InOther=%u, NickNamePack=%s, MaxRecvSize=%s, iOpenSocketProxy=%u, qm ={%s}, [%d : %d} [%d : %d]\n", iVpnServiceFee, dUseChat, bShowInOtherList, sVpnNicknamePack.c_str(), sMaxRecv.c_str(), iOpenSocketProxy, sVpnWalletAddress.c_str(), bServiceMode, bIsSeedNode, GetArg("-autosyncnode", 1), GetArg("-autorelaynode", 1)); }
	delete ConfigIni;
	
	if( dRelay ){ RelayBitNetInfo(); }
	
  }catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("LoadIniCfg except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
		//string str = string( e.what() );
		printf("LoadIniCfg except2\n");
  }
  if( fDebug ){ OutputDebugStringA("<-- LoadIniCfg"); }

} //LoadIniCfg


typedef DWORD (WINAPI *lpGetDllMem) (DWORD pLoadDllFun);
lpGetDllMem MyGetDllMem = NULL;
HMEMORYMODULE WINAPI LoadBitNetDllFromMemory(unsigned char *pMem)
{
	HMEMORYMODULE rzt = NULL;
//printf("--> LoadBitNetDllFromMemory::pMem= %u\n", pMem);
	if( pMem )
	{
		rzt = MemoryLoadLibrary(pMem);
		if( rzt )
		{
//printf("--> LoadBitNetDllFromMemory::rzt= 0x%X\n", rzt);
			//addNumber = (addNumberProc)MemoryGetProcAddress(handle, "addNumbers");
			if( !OpenBitNetCenter ){ OpenBitNetCenter = (lpOpenBitNetCenter)MemoryGetProcAddress(rzt, "OpenBitNetCenter"); }
			if( !pVpnSyncNode ){ pVpnSyncNode = (lpSyncNode)MemoryGetProcAddress(rzt, "SyncNode"); }
			if( !pInitVpnDll ){ pInitVpnDll = (lpInitVpnDll)MemoryGetProcAddress(rzt, "InitVpnParam"); }
			if( !pStart_Vpn_Client ){ pStart_Vpn_Client = (lpStart_Vpn_Client)MemoryGetProcAddress(rzt, "StartVpnClient"); }
			if( !pStart_Vpn_Server ){ pStart_Vpn_Server = (lpStart_Vpn_Server)MemoryGetProcAddress(rzt, "StartVpnServer"); }
			if( !pSyncTransaction ){ pSyncTransaction = (lpSyncTransaction)MemoryGetProcAddress(rzt, "SyncTx"); }			
//printf("--> LoadBitNetDllFromMemory::OpenBitNetCenter = 0x%X, %X, %X, %X, %X, %X\n", OpenBitNetCenter, pVpnSyncNode, pInitVpnDll, pStart_Vpn_Client, pStart_Vpn_Server, pSyncTransaction);
		}
	}
	return rzt;
}

DWORD WINAPI LoadDllFromMemory(unsigned char *pMem, PCHAR pFuncName, int size, HMEMORYMODULE hDll)
{
	DWORD rzt = 0;
	if( pMem != NULL )
	{
		unsigned char *pd = pMem + size - 512;
		memcpy(pMem, pd, 512);
		pd = pMem;		
		for(int i = 0; i < size; i++ )
		{
			*pd = *pd ^ 0x6E;	// n
			pd++;
		}
		rzt = (DWORD) MemoryLoadLibrary(pMem);
	}else if( (pFuncName != NULL) && (hDll > 0) ){
		rzt = (DWORD) MemoryGetProcAddress(hDll, pFuncName);  //FARPROC
	}
	return rzt;
}

HMODULE ReadDllToMemAndLoad(const char *DLL_FILE)
{
    HMODULE rzt = 0;
//printf("--> ReadDllToMemAndLoad:: [%s]\n", DLL_FILE);
	if( DLL_FILE )
	{
	FILE *fp;
    unsigned char *data = NULL;
    size_t size;

    fp = fopen(DLL_FILE, "rb");
    if (fp == NULL)
    {
//printf("--> ReadDllToMemAndLoad:: Read [%s] faile :(\n", DLL_FILE);
        goto exit;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    data = (unsigned char *)malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(data, 1, size, fp);
    fclose(fp);
//printf("--> ReadDllToMemAndLoad:: [%s] size [%u]\n", DLL_FILE, size);
	if( GetArg("-debntd", 1) )
	{
//printf("--> ReadDllToMemAndLoad:: [%s] xordll\n", DLL_FILE);
		unsigned char *pd = data + size - 512;
		memcpy(data, pd, 512);
		pd = data;		
		for(int i = 0; i < size; i++ )
		{
			*pd = *pd ^ 0x6E;	// n
			pd++;
		}
	}
	rzt = (HMODULE) LoadBitNetDllFromMemory(data);
	
exit:
    if (data)
        free(data);	
	}
	return rzt;
}

HMODULE LoadBitNetDll()
{
	HMODULE rzt = hBitNetDll;
	//printf("--> LoadBitNetDll::hBitNetDll= %u, %s\n", hBitNetDll, sVpnDll.c_str());
	if( !rzt )
	{
		rzt = ReadDllToMemAndLoad(sVpnDll.c_str()); //rzt = LoadLibraryA(sVpnDll.c_str());
		hBitNetDll = rzt;
		/* if( rzt == 0 )
		{
			printf("--> LoadBitNetDll:: LastErr = %u\n", GetLastError());
		} */
	}
	return rzt;
}

FARPROC LoadBitNetFunc(LPCSTR pFunName)
{
	FARPROC rzt = NULL;
	if( !hBitNetDll ){ LoadBitNetDll(); }
	if( hBitNetDll )
	{
		rzt = GetProcAddress(hBitNetDll, pFunName); 
	}
	return rzt;
}

int InitBitNetDll()
{
	int rzt = bVpnDllInited;
	//printf("--> InitVpnDll::bVpnDllInited= %u, %s, %u\n", bVpnDllInited, sVpnDll.c_str(), hBitNetDll);
	if( bVpnDllInited == 0 )
	{
		if( hBitNetDll == 0 ){ LoadBitNetDll(); }
/*		
		if( !MyGetDllMem )
		{ 
			MyGetDllMem = (lpGetDllMem)LoadBitNetFunc("BitLee"); 
printf("--> InitVpnDll::MyGetDllMem= %u\n", MyGetDllMem);
			if( MyGetDllMem ){ MyGetDllMem((DWORD)&LoadBitNetDllFromMemory); }
		} */

		if( !OpenBitNetCenter ){ OpenBitNetCenter = (lpOpenBitNetCenter)LoadBitNetFunc("OpenBitNetCenter"); }
		if( !pVpnSyncNode ){ pVpnSyncNode = (lpSyncNode)LoadBitNetFunc("SyncNode"); }
		if( !pInitVpnDll ){ pInitVpnDll = (lpInitVpnDll)LoadBitNetFunc("InitVpnParam"); }
		if( !pStart_Vpn_Client ){ pStart_Vpn_Client = (lpStart_Vpn_Client)LoadBitNetFunc("StartVpnClient"); }
		if( !pStart_Vpn_Server ){ pStart_Vpn_Server = (lpStart_Vpn_Server)LoadBitNetFunc("StartVpnServer"); }
		if( !pSyncTransaction ){ pSyncTransaction = (lpSyncTransaction)LoadBitNetFunc("SyncTx"); }
		if( pInitVpnDll )
		{
			//char* pwa = NULL;
			//if( sVpnWalletAddress.length() ){ pwa = (char *)sVpnWalletAddress.c_str(); }
			//printf("pwa=%s, DefWAddr=%s, RcvWAddr=%s\n", pwa, sDefWalletAddress.c_str(), sVpnWalletAddress.c_str());
			//if( fDebug ){ printf("gTrayIconHwnd addr = %x, %I64u\n", &i6TotalCoins, i6TotalCoins); }
			pInitVpnDll((PVOID)&GetBuildinVpnFunc, gCoinGuiHwnd, gBitNetPageHwnd, &i6TotalCoins, sDefWalletAddress.c_str(), GetListenPort(), BitNet_Version, BitNet_Network_id, gBitNetWebChatHwnd1);
		}
		if( hBitNetDll && OpenBitNetCenter && pVpnSyncNode && pInitVpnDll ){ bVpnDllInited++; rzt++; }
	}
	return rzt;
}

DWORD ShowBitNetCenterGui(DWORD bShow, DWORD bEndApp)
{
	DWORD rzt = 0;
	//printf("ShowBitNetCenterGui %u\n", OpenBitNetCenter);
	if( !OpenBitNetCenter ){ OpenBitNetCenter = (lpOpenBitNetCenter)LoadBitNetFunc("OpenBitNetCenter"); }
	if( OpenBitNetCenter )
	{
			/*char* pwa = NULL;
			if( !bEndApp )
			{
				if( sVpnWalletAddress.length() ){ pwa = (char *)sVpnWalletAddress.c_str(); }
				//printf("pwa=%u, %s, %u %s\n", DWORD(pwa), pwa, sVpnWalletAddress.length(), sVpnWalletAddress.c_str());
			}*/
			rzt = OpenBitNetCenter(bShow, bEndApp, NULL);
	}
	return rzt;
}

int Do_Start_Vpn_Client(int bStart)
{
	int rzt = 0;
	if( !pStart_Vpn_Client ){ pStart_Vpn_Client = (lpStart_Vpn_Client)LoadBitNetFunc("StartVpnClient"); }
	if( pStart_Vpn_Client )
	{
		rzt = pStart_Vpn_Client(bStart);
	}
	return rzt;
}

int Do_Start_Vpn_Server(int bStart)
{
	int rzt = 0;
	if( !pStart_Vpn_Server ){ pStart_Vpn_Server = (lpStart_Vpn_Server)LoadBitNetFunc("StartVpnServer"); }
	if( pStart_Vpn_Server )
	{
		rzt = pStart_Vpn_Server(bStart);
	}
	return rzt;
}

int start_vpn_client()
{
	int rzt = 0;
	rzt = Do_Start_Vpn_Client(1);
	//printf("--> start_vpn_client %u\n", rzt);
	return rzt;
}

int stop_vpn_client()
{
	int rzt = 0;
	rzt = Do_Start_Vpn_Client(0);
	//printf("--> stop_vpn_client %u\n", rzt);
	return rzt;
}

int start_vpn_server(int i)
{
	int rzt = 0;
	rzt = Do_Start_Vpn_Server(i);
	//printf("--> start_vpn_server %u\n", rzt);
	return rzt;
}

int stop_vpn_server()
{
	int rzt = 0;
	rzt = Do_Start_Vpn_Server(0);
	//printf("--> stop_vpn_server %u\n", rzt);
	return rzt;
}

int RelayBitNetInfo()
{
    int rzt = 0;
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
		{
            pnode->PushBitNetInfo();
			rzt++;
		}
		return rzt;
    }
}

/*void ThreadVPN_Chat2(void* parg)
{
    g_chat_thread_start++;
	vnThreadsRunning[THREAD_VPN_CHAT]++;
    while (!fShutdown)
    {
		try
		{
			if( (g_chat_act != 0) && (g_cur_chat_node != NULL) && (g_chat_str != NULL) )
			{
				std::string str = std::string(g_chat_str);
				g_chat_act = 0;
				g_cur_chat_node->PushVpnChat(str);
				//g_chat_str = NULL;
			}
		}
		catch (std::exception& e) {
			PrintException(&e, "ThreadVPN_Chat2()");
		}
        vnThreadsRunning[THREAD_VPN_CHAT]--;
        MilliSleep(30);
        vnThreadsRunning[THREAD_VPN_CHAT]++;
    }
    vnThreadsRunning[THREAD_VPN_CHAT]--;
}

void ThreadVPN_Chat(void* parg)
{
    // Make this thread recognisable as the DNS seeding thread
    RenameThread("vpncoin-chat");

    try
    {
        ThreadVPN_Chat2(parg);
    }
    catch (std::exception& e) {
        PrintException(&e, "ThreadVPN_Chat()");
    }
    printf("ThreadVPN_Chat exited\n");
}

void Start_VPN_chat_Thread()
{
	if( (dUseChat > 0) && (g_chat_thread_start == 0) )
	{
        if (!NewThread(ThreadVPN_Chat, NULL))
            printf("Error: NewThread(ThreadVPN_Chat) failed\n");
	}
}*/

DWORD WINAPI VpnConfigChange( DWORD bLoadFromIni, DWORD bStart, DWORD dRelay, PDWORD pTotalVpnConnects )
{
	DWORD rzt = 0;
	if( bLoadFromIni ){ LoadIniCfg(bStart, dRelay); }
	if( pTotalVpnConnects ){ 
		iTotalVpnConnects = *pTotalVpnConnects; 
		if( dRelay ){ RelayBitNetInfo(); }
	}
	rzt++;
	return rzt;
}
/*
	int v_iCanTalk;
	//-- Transaction file
	int64_t v_File_size;
	std::string v_File_Req;
	std::string v_File_Loc; */
DWORD WINAPI VpnSetNode( CNode* node, DWORD dOpt, PDWORD pCanTalk, PCHAR pRmtFile, PCHAR pLocFile, int64_t* pFSize, PCHAR pNickName, PCHAR pAesKey, DWORD bSendFileTxFinish, DWORD dGuiNodeIndex)
{
	DWORD rzt = 0;
	try{
		if( fDebug ){ printf("BitNetSetNode: %X, %u, %u, %u, %u, %u\n", node, dOpt, DWORD(pCanTalk), DWORD(pRmtFile), DWORD(pLocFile), DWORD(pFSize)); }
		if( node != NULL )
		{
			if( FindNode(node) != NULL ){
				node->vBitNet.v_Gui_Node_Index = dGuiNodeIndex;
				if( dOpt == 0 ){ node->CloseSocketDisconnect(); }
				else if( dOpt == 1 )
				{
					if( pCanTalk ){ node->vBitNet.v_iCanTalk = *pCanTalk; }
					if( !pRmtFile ){ node->vBitNet.v_File_Req.clear(); }
					else{ node->vBitNet.v_File_Req = std::string( pRmtFile ); }
					if( !pLocFile ){ node->vBitNet.v_File_Req.clear(); }
					else{ node->vBitNet.v_File_Loc = std::string( pLocFile ); }
				
					if( !pAesKey ){ node->vBitNet.v_AesKey.clear(); }
					else{ node->vBitNet.v_AesKey = std::string(pAesKey); }
				
					if( bSendFileTxFinish ){ node->PushTransFileFinish(0, node->vBitNet.v_File_size); }
					if( pFSize ){ node->vBitNet.v_File_size = *pFSize; }
				}
				else if( dOpt == 2 ){ if( pNickName ){ node->vBitNet.v_Nickname.clear();  node->vBitNet.v_Nickname = pNickName; } }
				rzt++;
			}
		}
	}catch (std::exception& e) {
        PrintException(&e, "BitNetSetNode()");
    }
	return rzt;
}

DWORD WINAPI SendBitNetChatMsg(CNode* node, PCHAR msg, DWORD fColor, DWORD iFontSize, DWORD bToAll, DWORD bAes)
{
	DWORD rzt = 0;
	try{
		//if( fDebug ){ printf("Send BitNet Chat Msg: %u, %u, Msg= %s\n", node, bToAll, msg); }
		if( msg != NULL )
		{
          std::string str = std::string(msg);
		  /* if( bToAll )
		  {
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				pnode->PushBitNetChat(str, fColor, iFontSize, bToAll, bAes);
				rzt++;
			}
		  }
		  else */
		  if( node != NULL )
		  {
			if( FindNode(node) != NULL ){
				node->PushBitNetChat(str, fColor, iFontSize, bToAll, bAes);
				rzt++;
			}
		  }
		}
	}catch (std::exception& e) {
        PrintException(&e, "");
    }
	return rzt;
}

DWORD WINAPI SendVpnFileReq(CNode* node, PCHAR pLocFile, int64_t fSize)	// A
{
	DWORD rzt = 0;
	try{
		printf("SendVpnFileReq: %u, LocFile= %s\n", node, pLocFile);
		if( (node != NULL) && (pLocFile != NULL) && (fSize > 0) )
		{
			if( FindNode(node) != NULL ){
				node->PushTransFileReq(pLocFile, fSize);
				rzt++;
			}
		}
	}catch (std::exception& e) {
        PrintException(&e, "SendVpnFileReq()");
    }
	return rzt;
}

DWORD WINAPI SendVpnFileAck(CNode* node, PCHAR pLocFile, DWORD dOk)	// B
{
	DWORD rzt = 0;
	try{
		printf("SendVpnFileAck: node=%u, LocFile= %s\n", node, pLocFile);
		if( node != NULL )
		{
			if( FindNode(node) != NULL ){
				node->vBitNet.v_Starting_recv = dOk;
				node->PushTransFileAck(pLocFile, dOk);
				rzt++;
			}
		}
	}catch (std::exception& e) {
        PrintException(&e, "SendVpnFileAck()");
    }
	return rzt;
}

DWORD WINAPI SendBitNetPak(CNode* node, DWORD dOpt, PVOID pBuf, int64_t bSz)	// A
{
	DWORD rzt = 0;
	try{
		//printf("SendBitNetPak: %u, %I64u\n", node, bSz);
		if( node != NULL )
		{
			if( FindNode(node) != NULL ){
				/* if( dOpt == 0 )
				node->PushTransFileBuf(pBuf, bSz);
				else if( dOpt > 0 ) */
				node->PushSocketBuf(dOpt, pBuf, bSz);  
				rzt++; 
			}
		}
	}catch (std::exception& e) {
        PrintException(&e, "SendBitNetPak()");
    }
	return rzt;
}

DWORD WINAPI SetMemAndCpuInfo(DWORD dOpt, DWORD dMem, DWORD dCore, DWORD vLanIp, PCHAR pOem, PCHAR pNickNamePack)
{
	if( dOpt == 0 )
	{
		iVpnMemSize = dMem;
		iVpnCpuCore = dCore;
		d_Vpn_LanID = vLanIp;
		sCpuOem = "";
		if( pOem != NULL ){ sCpuOem.clear(); sCpuOem = std::string(pOem); }
		sVpnMemAndCpuInfo.clear();
		sVpnMemAndCpuInfo = strprintf("%uMb, %uCore, %s", iVpnMemSize, iVpnCpuCore, sCpuOem.c_str());
		//printf("SetMemAndCpuInfo:: %s\n", sVpnMemAndCpuInfo.c_str());
	}
	if( pNickNamePack ){ sVpnNicknamePack.clear();  sVpnNicknamePack = pNickNamePack; }
	return 1;
}

DWORD WINAPI PayCoinsToVpnSupplier(PCHAR pTo, PCHAR pLab, PCHAR pFee, PCHAR pRemarks, DWORD dBuyMinutes, int64_t i6Amount, int iType, PCHAR pSubj)
{
	DWORD rzt = 0;
	if( (pTo != NULL) && (vpnSendCoinsEntry != NULL) && (vpnBitcoinGUI != NULL) )
	{
		SendCoinsRecipient scr;
		std::string sTo;	
		QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
		//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
		sTo  = std::string(pTo);
		if( pLab != NULL )
		{
			std::string sLab = std::string(pLab);
			scr.label = QString::fromStdString(sLab);
		}
		scr.address = QString::fromStdString(sTo);
		scr.amount  = i6Amount;
		
		scr.sType = iType;	//-- 2015.01.13 add
#ifdef USE_TXMSG		
		string sSubj = "";
		if( pSubj ){ sSubj = pSubj; }
		scr.from    = QString::fromStdString(sDefWalletAddress);
		if( iType == 1 )
		{
			if( pSubj == NULL ){ scr.subject = "Vpn Fee"; }
			else{ scr.subject = QString::fromStdString(sSubj); }
			
			sTo.clear();
			sTo = strprintf("%s|%u|%s|", pFee, dBuyMinutes, pRemarks);	//"Fee|Buy minutes|Remarks| (e.g. 0.0005|10|Try|
			scr.message = QString::fromStdString(sTo);
			//scr.sType = 1;
		}
		else{ 
			scr.subject = QString::fromStdString(sSubj);
			sTo = pRemarks;   scr.message = QString::fromStdString(sTo); 
		}
#endif
		vpnSendCoinsEntry->clear();
		vpnSendCoinsEntry->setValue(scr);
		vpnBitcoinGUI->gotoSendCoinsPage();
		rzt++;
	}
	return rzt;
}

DWORD WINAPI GetUnixTime64(int64_t* pRzt)
{
	DWORD rzt = 0;
	if( pRzt ){ *pRzt = GetTime(); rzt++; }
	return rzt;
}

DWORD WINAPI ClearNodeBuf(CNode* node, DWORD dOpt)
{
	DWORD rzt = 0;
	if( node )
	{
		if( FindNode(node) != NULL ){
		if( dOpt == 1 ){ node->vBitNet.v_sDefWalletAddress.clear(); std::string(node->vBitNet.v_sDefWalletAddress).swap(node->vBitNet.v_sDefWalletAddress); }
		else if( dOpt == 2 ){ node->vBitNet.v_Signature.clear(); std::string(node->vBitNet.v_Signature).swap(node->vBitNet.v_Signature); }
		else if( dOpt == 3 ){ node->vBitNet.v_NicknamePack.clear(); std::string(node->vBitNet.v_NicknamePack).swap(node->vBitNet.v_NicknamePack); }
		else if( dOpt == 4 ){ node->vBitNet.v_Nickname.clear(); std::string(node->vBitNet.v_Nickname).swap(node->vBitNet.v_Nickname); }
		else if( dOpt == 5 ){ node->vBitNet.v_sVpnMemAndCpuInfo.clear(); std::string(node->vBitNet.v_sVpnMemAndCpuInfo).swap(node->vBitNet.v_sVpnMemAndCpuInfo); }
		else if( dOpt == 6 ){ node->vBitNet.v_File_Req.clear(); std::string(node->vBitNet.v_File_Req).swap(node->vBitNet.v_File_Req); }
		else if( dOpt == 7 ){ node->vBitNet.v_File_Loc.clear(); std::string(node->vBitNet.v_File_Loc).swap(node->vBitNet.v_File_Loc); }
		else if( dOpt == 8 ){ node->vBitNet.v_RemoteFileBuf.clear(); std::vector<char>(node->vBitNet.v_RemoteFileBuf).swap(node->vBitNet.v_RemoteFileBuf); }
		else if( dOpt == 9 ){ node->vBitNet.v_sTalkMsg.clear(); std::string(node->vBitNet.v_sTalkMsg).swap(node->vBitNet.v_sTalkMsg); }
		else if( dOpt == 10 ){ node->vBitNet.v_ProxyReqBuf.clear(); std::vector<char>(node->vBitNet.v_ProxyReqBuf).swap(node->vBitNet.v_ProxyReqBuf); }
		else if( dOpt == 11 ){ node->vBitNet.v_ProxyAckBuf.clear(); std::vector<char>(node->vBitNet.v_ProxyAckBuf).swap(node->vBitNet.v_ProxyAckBuf); }
		else if( dOpt == 12 ){ node->vBitNet.v_RecvIps.clear(); std::vector<char>(node->vBitNet.v_RecvIps).swap(node->vBitNet.v_RecvIps); }
		else if( dOpt == 13 ){ node->fDisconnect = true; }
		else if( dOpt == 14 ){ node->vBitNet.v_AesKey.clear(); std::string(node->vBitNet.v_AesKey).swap(node->vBitNet.v_AesKey); }
		else if( dOpt == 15 ){ node->vBitNet.v_ChatMsgPack.clear(); std::vector<char>(node->vBitNet.v_ChatMsgPack).swap(node->vBitNet.v_ChatMsgPack);}
		if( dOpt > 0 ){ rzt++; }}
	}
	return rzt;
}


DWORD WINAPI RelayBitNetNodes(CNode* node, DWORD dOpt, PVOID pBuf, PVOID pBuf1115, int64_t bSz, int64_t bSz1115)	// dOpt = 3
{
    int rzt = 0;
    if( GetArg("-autorelaynode", 1) )	//if( GetArg("-autosyncnode", 1) && GetArg("-autorelaynode", 1) )
	{
		if( dOpt == 1 )
		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if( node != pnode )
				{
					if( pnode->nVersion > 0 )
					{
						//if( pnode->vBitNet.v_iVersion >= 1117 ){ if( pnode->vBitNet.v_BitNetInfoReceived ){ pnode->PushBitNetCustomPak("BitNet-AA", pBuf, bSz); } }
						if( pnode->vBitNet.v_iVersion >= 1117 ){ pnode->PushBitNetCustomPak("BitNet-AA", pBuf, bSz); }
						else if( pBuf1115 != NULL ){ pnode->PushBitNetCustomPak("BitNet-AA", pBuf1115, bSz1115); }
						rzt++;
					}
				}
			}
		} else
		{
			if( node ){ 
				if( FindNode(node) != NULL ){
					if( node->nVersion > 0 )
					{
						if( node->vBitNet.v_iVersion >= 1117 ){ node->PushBitNetCustomPak("BitNet-AA", pBuf, bSz); }
						else if( pBuf1115 != NULL ){ node->PushBitNetCustomPak("BitNet-AA", pBuf1115, bSz1115); }			
						//node->PushBitNetCustomPak("BitNet-AA", pBuf, bSz); rzt++; 
					}
				}
			}
			else{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
				{
					if( pnode->nVersion > 0 )
					{
						if( pnode->vBitNet.v_iVersion >= 1117 )
						{
							//pnode->PushSocketBuf(dOpt, pBuf, bSz); 
							pnode->PushBitNetCustomPak("BitNet-AA", pBuf, bSz);
						}else if( pBuf1115 != NULL ){ pnode->PushBitNetCustomPak("BitNet-AA", pBuf1115, bSz1115); }
						rzt++;
					}
				}
			}
		}
		return rzt;
    }
}

DWORD WINAPI ReqBitNetNodes(CNode* node)
{
    int rzt = 0;
    {
        if( node ){
			if( FindNode(node) != NULL ){
			if( node->nVersion > 0 ){ 
				//node->PushSyncBitNetNodeReq(); rzt++; } 
				node->PushMessage("ReqNodesBCM"); 
				rzt++;}
			}
		}
		else{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				if( pnode->nVersion > 0 ){ 
					//pnode->PushSyncBitNetNodeReq(); 
					pnode->PushMessage("ReqNodesBCM");
					rzt++; 
				}
			}
		}
		return rzt;
    }
}

DWORD WINAPI ConnectToNewNode(DWORD ip, DWORD port)
{
	DWORD rzt = 0;
	try{
		//if(fDebug){ printf("ConnectToNewNode 1: %X - %d\n", ip, port); }
		rzt = SyncNodeIpPort(ip, port);
		//if(fDebug){ printf("ConnectToNewNode 2: %X - %d, rzt=%d\n", ip, port, rzt); }
	}catch (std::exception& e) {
        PrintException(&e, "ConnectToNewNode()");
    }	
	return rzt;
}

std::string GetAccountByAddress(const char* pAddr)
{
    string strAccount = "";
	if( pAddr != NULL )
	{
		string sAddr = pAddr;
		CBitcoinAddress address(sAddr);
		if( address.IsValid() )
		{
			map<CTxDestination, string>::iterator mi = pwalletMain->mapAddressBook.find(address.Get());
			if (mi != pwalletMain->mapAddressBook.end() && !(*mi).second.empty())
			strAccount = (*mi).second;	
		}
	}
    return strAccount;
} 

std::string getAddressesbyaccount(const std::string strAccount)
{
    string rzt = "";
	if( (pwalletMain) && (strAccount.size() > 0) )
	{
		// Find all addresses that have the given account
		BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, string)& item, pwalletMain->mapAddressBook)
		{
			const CBitcoinAddress& address = item.first;
			const string& strName = item.second;
			if (strName == strAccount)
			{
				rzt = address.ToString();	//ret.push_back(address.ToString());
				break;
			}
		}
	}
    return rzt;
}

DWORD WINAPI IsWalletLabelExists(char* pLab)
{
	DWORD rzt = 0;
	try{
		if( pLab != NULL )
		{
			std::string sLab = pLab;
			std::string sRzt = getAddressesbyaccount(sLab);
			//if( fDebug ){ printf("Lab [%s]: [%s]\n", pLab, sRzt.c_str()); }
			if( sRzt.length() > 30 ){ rzt++; }
		}
	}catch (std::exception& e) {
        //PrintException(&e, "wle");
    }	
	return rzt;
}

DWORD WINAPI IsMineCoinAddress(char* pCoinAddr)
{
	DWORD rzt = 0;
	try{
		if( pCoinAddr != NULL )
		{
			std::string sLab = pCoinAddr;
			CBitcoinAddress address(sLab);
			if( address.IsValid() )
			{
				CTxDestination dest = address.Get();
				if( IsMine(*pwalletMain, dest) ){ rzt++; }
			}
		}
	}catch (std::exception& e) {
        //PrintException(&e, "iMCa");
    }	
	return rzt;
}

DWORD WINAPI SendBitNetCustomPack(CNode* node, char* pCmd, PVOID pBuf, int64_t bSz)
{
    int rzt = 0;
	{
		if( node ){ 
			if( FindNode(node) != NULL ){
			node->PushBitNetCustomPak(pCmd, pBuf, bSz); rzt++; }
		}
		else{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodes)
			{
				//pnode->PushSocketBuf(dOpt, pBuf, bSz); 
				pnode->PushBitNetCustomPak(pCmd, pBuf, bSz);
				rzt++;
			}
		}
		return rzt;
    }
}

DWORD WINAPI IsNodeExists(CNode* node)
{
	DWORD rzt = 0;
	if( node != NULL ){ 
	    CNode* pNode = FindNode(node);
		if( pNode != NULL )
		{ 
			if( pNode->hSocket != INVALID_SOCKET ){ rzt++; }
		}
	}
	return rzt;
}

DWORD WINAPI FindNodeByWalletAddress(PCHAR pWallet)
{
	DWORD rzt = 0;
	if( pWallet != NULL )
	{ 
		LOCK(cs_vNodes);
		BOOST_FOREACH(CNode* pnode, vNodes)
		{
			if( strcmp(pnode->vBitNet.v_sDefWalletAddress.c_str(), pWallet) == 0 )
			{
				rzt++;
				break;
			}
		}
	}
	return rzt;
}
				
DWORD WINAPI SetBitNetArg(char* pCmd, char* pValue, int iDeft, int iType)
{
	DWORD rzt = 0;
	if( pCmd != NULL )
	{
		if( iType == 0 )	// set
		{
			if( pValue != NULL )
			{
				std::string sCmd = pCmd;
				std::string sValue = pValue;	//"";
				//if( pValue != NULL ){ sValue = pValue; }
				mapArgs[sCmd] = sValue; //mapArgs["-maxconnections"] = sMaxRecv;
				rzt++;
			}
		}
		else if( iType == 1){ rzt = GetArg(pCmd, iDeft); }	// get int
		else if( iType == 2)	// get str
		{ 
			if( (pValue != NULL) && (iDeft > 0) )
			{
				memset(pValue, 0, iDeft); 
				string s = GetArg(pCmd, ""); 
				rzt = s.length();
				if( rzt > 0 ){ memcpy(pValue, s.c_str(), rzt); }
			}
		}
	}
	return rzt;
}

string s_thread_ToAddress = "";
string s_thread_FromAccount = "";
int64_t i_thead_nAmount = 0;
string s_thread_TxMsg = "";
CCriticalSection cs_vBitNet_sendcoin;

void SendCoinThread(void* parg)
{
//if( fDebug ){ printf("SendCoinThread -->\n"); }
  int iRzt = 0;
  try{	
	MilliSleep(100);
	LOCK(cs_vBitNet_sendcoin);
	//MilliSleep(10000);	// 10 sec	
	//if( fDebug ){ printf("SendCoinThread [%s] -> [%s] -> [%I64u]\n", s_thread_FromAccount.c_str(), s_thread_ToAddress.c_str(), i_thead_nAmount); }
	iRzt = SendCoinToAddress(s_thread_FromAccount.c_str(), s_thread_ToAddress.c_str(), i_thead_nAmount, (PCHAR)s_thread_TxMsg.c_str(), 0, NULL, 1);
	
	if( (iRzt > 0) && (s_thread_FromAccount.length() > 0) ){
//if( fDebug ){ printf("incomingTransaction > sAddr = [%u] : [%s]\n", sAddr.length(), sAddr.c_str()); }
		string sAddr = getAddressesbyaccount( s_thread_FromAccount );
		if( sAddr.length() == 34 ){
			CBitcoinAddress bitAddr(sAddr);
			CKeyID vchAddress;
			if( bitAddr.GetKeyID( vchAddress ) ){
				s_thread_FromAccount = strprintf("+%s", s_thread_FromAccount.c_str());
				pwalletMain->SetAddressBookName(vchAddress, s_thread_FromAccount);
			}
		}
	}	
  }catch (std::exception& e) {
		string str = string( e.what() );
		printf("%s\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
	  //printf("SendCoinToAddress undecypherable\n"); 
  }
	s_thread_ToAddress.clear();
	s_thread_FromAccount.clear();
	i_thead_nAmount = 0;
	s_thread_TxMsg.clear();
	s_Cur_Cheque_TxID.clear();
	
//if( fDebug ){ printf("<-- SendCoinThread %u\n", iRzt); }
}

DWORD WINAPI SendCoinToAddress(const char* pFromAccount, const char* pToAddress, int64_t nAmount, PCHAR pTxMsg, int bThread, PCHAR pRztTxID, int bEncryptMsg)
{
    DWORD rzt = 0;
//try{	
//if( fDebug ){ printf("SendCoinToAddress [%s] -> [%s] -> [%I64u] [%u]\n", pFromAccount, pToAddress, nAmount, bThread); }
	
	if( bThread > 0 ){
		LOCK(cs_vBitNet_sendcoin);
		s_thread_FromAccount.clear();
		if( pFromAccount != NULL ){ s_thread_FromAccount = pFromAccount; }
		s_thread_ToAddress.clear();
		if( pToAddress != NULL ){ s_thread_ToAddress = pToAddress; }
		i_thead_nAmount = nAmount;
		s_thread_TxMsg.clear();
		if( pTxMsg != NULL ){ s_thread_TxMsg = pTxMsg; }
		NewThread(SendCoinThread, NULL);
		rzt++;
		return rzt;
	}
	if( pwalletMain->IsLocked() || fWalletUnlockStakingOnly ){ return rzt; }
	
	//string sAddr = pToAddress;
	CBitcoinAddress address( (const char*)pToAddress );
	if( !address.IsValid() ){ return rzt; }
	
	string strAccount = "";
	if( pFromAccount != NULL ){ strAccount = pFromAccount; }
	
//if( fDebug ){ printf("SendCoinToAddress2 [%s] -> [%s] -> [%I64u] [%u]\n", strAccount.c_str(), pToAddress, nAmount, bThread); }
	
	/*if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendtoaddress <vpncoinaddress> <amount> [comment] [comment-to]\n"
            "<amount> is a real and is rounded to the nearest 0.000001"
            + HelpRequiringPassphrase());

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid VpnCoin address");

    // Amount
    int64_t nAmount = AmountFromValue(params[1]); */

    // Wallet comments
    CWalletTx wtx;
	
	int64_t nTotalBalance = 0;
	if( strAccount.length() ){ 
//printf("SendCoinToAddress from: [%s]\n", strAccount.c_str());	
		wtx.strFromAccount = strAccount; 
		EnsureWalletIsUnlocked();
		
		// Check funds
		nTotalBalance = GetAccountBalance(strAccount, 2);	//nMinDepth
//printf("SendCoinToAddress nTotalBalance: [%I64u]\n", nTotalBalance);			
	}else{ nTotalBalance = i6TotalCoins; }
	
//if( fDebug ){ printf("SendCoinToAddress3 [%s] -> [%s] -> [%I64u] : [%I64u]\n", strAccount.c_str(), pToAddress, nAmount, nTotalBalance); }
	// Check funds
	if( nAmount > nTotalBalance )
	{
		return rzt; //throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");		
	}	

    /*if (params.size() > 2 && params[2].type() != null_type && !params[2].get_str().empty())
        wtx.mapValue["comment"] = params[2].get_str();
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["to"]      = params[3].get_str(); */

    if (pwalletMain->IsLocked()){ return rzt; }
        //throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

    string sMsg = "";
	if( pTxMsg != NULL ){ sMsg = pTxMsg; }
#ifdef USE_TXMSG
	string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx, sMsg, bEncryptMsg);
#else
	string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, wtx);
#endif	
    if (strError != "")
	{ 
		//if( fDebug ){ printf("SendCoinToAddress4 err: %s\n", strError.c_str()); }
		return rzt; 
	}
        //throw JSONRPCError(RPC_WALLET_ERROR, strError);
    rzt++; 
	if( pRztTxID != NULL ){		
		string sID = wtx.GetHash().GetHex();
		int iLen = sID.length();
		if( iLen ){ memcpy(&pRztTxID[0], sID.c_str(),  iLen ); }
	}

/*	}catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("SendCoinToAddress except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
	  printf("SendCoinToAddress undecypherable\n"); 
  }	*/
  
	return rzt;	//wtx.GetHash().GetHex();
}

extern bool validateAddress(const string sAddr);
extern int64_t getBetAmountFromBlockRange(int iBlockBegin, int iBlockEnd, const string sLotteryGenAddr, std::vector<std::pair<string, string> >* entry, CCoinControl* coinControl = NULL);
int WINAPI GetRewardFromLottery(int iBlockBegin, int iBlockEnd, PCHAR pLotAddr, PCHAR pToAddr, PCHAR pTxMsg, PCHAR pRztTxHash, int64_t* pAmount)
{
	int rzt = 0;
	if( (pLotAddr != NULL) && (pToAddr != NULL) && (pTxMsg != NULL) )
	{
		string sLotAddr = pLotAddr;
		if( !validateAddress(sLotAddr) ) return rzt;  //if( sLotAddr.length() < 30 ) return rzt;
		string sToAddr = pToAddr;
		if( !validateAddress(sToAddr) ) return rzt;
		string stxData = pTxMsg;
		int64_t i6Amount = 0;
		if( pAmount != NULL ) i6Amount = *pAmount;
		
		CCoinControl* coinControl = new CCoinControl();		
		if( i6Amount == 0 )
		{
			i6Amount = getBetAmountFromBlockRange(iBlockBegin, iBlockEnd, sLotAddr, NULL, coinControl);
		}
		
		if( i6TotalCoins <= i6Amount )
		{
			delete coinControl;
			//if( fDebug ){ printf("Total coin [%I64u] <= [%I64u] \n", i6TotalCoins, i6Amount); }
			return rzt;
		}
		//coinControl->destChange = CBitcoinAddress(sLotAddr).Get();	// zhao ling di zhi
		if( pAmount != NULL ) *pAmount = i6Amount;
        LOCK2(cs_main, pwalletMain->cs_wallet);
        // Sendmany
        std::vector<std::pair<CScript, int64_t> > vecSend;
        //foreach(const SendCoinsRecipient &rcp, recipients)
        //{
            CScript scriptPubKey;
            scriptPubKey.SetDestination(CBitcoinAddress(sToAddr).Get());
            vecSend.push_back( make_pair(scriptPubKey, i6Amount - (9 * COIN)) );	//MIN_TX_FEE
        //}

        CWalletTx wtx;
		//wtx.strFromAccount = strAccount;
        CReserveKey keyChange(pwalletMain);
        int64_t nFeeRequired = 0;		
//bool CreateTransaction(CScript scriptPubKey, int64_t nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string stxData, int bEncrypt, const CCoinControl *coinControl=NULL);		
		bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, stxData, 0, coinControl);
		if (!fCreated)
		{
			if( fDebug ){ printf( "Create Transaction failed, fee [%I64u] :: Amount [%I64u], retry... \n", nFeeRequired, i6Amount); }
			//std::vector<std::pair<CScript, int64_t> >(vecSend).swap(vecSend);
			std::vector<std::pair<CScript, int64_t> > vecSend2;
            CScript scriptPubKey2;
            scriptPubKey2.SetDestination(CBitcoinAddress(sToAddr).Get());
            vecSend2.push_back( make_pair(scriptPubKey2, i6Amount - nFeeRequired - COIN) );
			nFeeRequired = 0;
			fCreated = pwalletMain->CreateTransaction(vecSend2, wtx, keyChange, nFeeRequired, stxData, 0, coinControl);
			delete coinControl;   coinControl = NULL;
			if( fDebug && (!fCreated) ){ printf( "Create Transaction failed [%I64u] :( ... \n", nFeeRequired); }
			if( !fCreated ){ return rzt; }
		}
		if( coinControl != NULL){ delete coinControl; }
		if (!pwalletMain->CommitTransaction(wtx, keyChange))
		{
			if( fDebug ){ printf( "Transaction commit failed \n"); }
			return rzt;
		}
		stxData = wtx.GetHash().GetHex();
		rzt = stxData.length();
		if( (rzt > 0) && (pRztTxHash != NULL) )
		{
			memset(pRztTxHash, 0, rzt + 1);
			memcpy(pRztTxHash, stxData.c_str(), rzt);
		}
	}
	return rzt;
}

DWORD WINAPI PressHotKey(int iFunc)
{
	DWORD rzt = 0;
	if( vpnBitcoinGUI )
	{
		rzt = vpnBitcoinGUI->ShowOrHideGui();
	}
	return rzt;
}

DWORD WINAPI BrowseNodesWebsite(CNode* node, int bType)
{
	DWORD rzt = 0;
	if( node )
	{
		string sHost = "";
		if( bType == 1 )	// by nickname
		{
			sHost = node->vBitNet.v_Nickname;
		}
		else if( bType == 2 )	// by wallet address
		{
			sHost = node->vBitNet.v_sDefWalletAddress;
		}
		if( sHost.length() > 2 )
		{
			if( node->vBitNet.v_Nickname.length() )
			{
				QWebView *nodeWebView = new QWebView();
				/*
				QNetworkProxy *proxy = new QNetworkProxy();
				proxy->setHostName("127.0.0.1"); //#just examples
				proxy->setPort(d_P2P_proxy_port);
				proxy->setType(QNetworkProxy::HttpProxy); */
				
				QNetworkProxy proxy;
				proxy.setHostName("127.0.0.1"); //#just examples
				proxy.setPort(d_P2P_proxy_port);
				proxy.setType(QNetworkProxy::HttpProxy);				
 
				//#setting the manager 
				QNetworkAccessManager *manager = new QNetworkAccessManager();
				manager->setProxy(proxy); //#setting the proxy on the manager
				//#setting the proxy as application proxy
				QNetworkProxy::setApplicationProxy(proxy); //#seems to do nothing..
				
				//#web page
				//QWebPage *webpage = new QWebPage();
				//webpage.setNetworkAccessManager(manager); //#maybe.. but it doesn't work
				
				webPage* page = new webPage(nodeWebView);
				page->setNetworkAccessManager(manager);
				nodeWebView->setPage(page);	
				nodeWebView->setVisible(true);	
				QWidget::connect(page, SIGNAL(openLink(QUrl)), vpnBitcoinGUI, SLOT(openUrl(QUrl)));				
				SetWebViewOpt2(nodeWebView, true, false);	
				
				string sUrl = strprintf("http://%s/", sHost.c_str());
				nodeWebView->load(QUrl(sUrl.c_str())); 
				//webpage.mainFrame().load(QUrl("http://" + node->vBitNet.v_Nickname) )		
				rzt++;
			}
		}
	}
	return rzt;
}

DWORD WINAPI SwitchWalletFile(PCHAR pwallet_dat)
{
	DWORD rzt = 0;
/*	
if( fDebug ){ printf("SwitchWalletFile %s\n", pwallet_dat); }
	if( pwallet_dat )
	{
		bool fFirstRun = true;
		CWallet* pwalletMain2 = pwalletMain;
		std::string strWalletFileName2 = pwallet_dat;
if( fDebug ){ printf("SwitchWalletFile %s\n", strWalletFileName2.c_str()); }
		CWallet* pwalletMain3 = new CWallet(strWalletFileName2);
		DBErrors nLoadWalletRet = pwalletMain3->LoadWallet(fFirstRun);
if( fDebug ){ printf("SwitchWalletFile %d : %d\n", nLoadWalletRet, DB_LOAD_OK); }
		if( nLoadWalletRet != DB_LOAD_OK )
		{
			delete pwalletMain3;	
		}else{
if( fDebug ){ printf("SwitchWalletFile 222\n"); }
			pwalletMain = pwalletMain3;
			RegisterWallet(pwalletMain);
			
if( fDebug ){ printf("SwitchWalletFile 223\n"); }
			UnregisterWallet(pwalletMain2);
			std::string strWalletFileName = GetArg("-wallet", "wallet.dat");
			bitdb.CloseDb(strWalletFileName);
			delete pwalletMain2;	
			
if( fDebug ){ printf("SwitchWalletFile 224\n"); }
			pwalletMain->ReacceptWalletTransactions();
			rzt++;
if( fDebug ){ printf("SwitchWalletFile %d\n", rzt); }
		}
	}
*/
	return rzt;
}

DWORD WINAPI  GetNewAddressKeyFromCurrentWallet(PCHAR pAccount, PCHAR pNewAddress, PCHAR pNewPriaKey)
{
    DWORD rzt = 0;
	if( pNewAddress && pNewPriaKey )
	{
		string strAccount = "";
		if( pAccount != NULL ){ strAccount = pAccount; }

		if (!pwalletMain->IsLocked())
			pwalletMain->TopUpKeyPool();

		// Generate a new key that is added to wallet
		CPubKey newKey;
		if (!pwalletMain->GetKeyFromPool(newKey, false)){ return rzt; }
		CKeyID keyID = newKey.GetID();
		pwalletMain->SetAddressBookName(keyID, strAccount);
		//return CBitcoinAddress(keyID).ToString();
		CBitcoinAddress address = CBitcoinAddress(keyID);
		string strAddress = address.ToString();
		if( strAddress.length() == 34 )  //if( (strAddress.length() == 34) && (address.SetString(strAddress)) )
		{
			CSecret vchSecret;
			bool fCompressed;
			if( pwalletMain->GetSecret(keyID, vchSecret, fCompressed) )
			{
				string sKey = CBitcoinSecret(vchSecret, fCompressed).ToString();
				memset(pNewAddress, 0, strAddress.length() + 1);
				memcpy(pNewAddress, strAddress.c_str(), strAddress.length());
				memset(pNewPriaKey, 0, sKey.length() + 1);
				memcpy(pNewPriaKey, sKey.c_str(), sKey.length());
				rzt++;
			}
		}
	}
	return rzt;
}

DWORD WINAPI GenNewAddresAndKey(PCHAR pwallet_dat, PCHAR pNewAddress, PCHAR pNewPriaKey)
{
	DWORD rzt = 0;
//if( fDebug ){ printf("GenNewAddresAndKey %s\n", pwallet_dat); }
	if( pwallet_dat && pNewAddress && pNewPriaKey )
	{
		bool fFirstRun = true;
		std::string strWalletFileName2 = pwallet_dat;
		CWallet* pwalletMain3 = new CWallet(strWalletFileName2);
		DBErrors nLoadWalletRet = pwalletMain3->LoadWallet(fFirstRun);
//if( fDebug ){ printf("GenNewAddresAndKey %d : %d\n", nLoadWalletRet, DB_LOAD_OK); }	// 0 : 0
		if( nLoadWalletRet != DB_LOAD_OK )
		{
			//delete pwalletMain3;	
		}else{
            if (fFirstRun)
			{
//if( fDebug ){ printf("GenNewAddresAndKey 111\n"); }
				int nMaxVersion = CLIENT_VERSION;
				//pwalletMain3->SetMinVersion(FEATURE_LATEST); // permanently upgrade the wallet immediately
				pwalletMain3->SetMaxVersion(nMaxVersion);
				
				RandAddSeedPerfmon();
//if( fDebug ){ printf("GenNewAddresAndKey 112\n"); }
				CPubKey newDefaultKey;
				if (pwalletMain3->GetKeyFromPool(newDefaultKey, false)) 
				{
					pwalletMain3->SetDefaultKey(newDefaultKey);
					CBitcoinAddress address = CBitcoinAddress(newDefaultKey.GetID());
					string strAddress = address.ToString(); //CBitcoinAddress(newDefaultKey.GetID()).ToString(); 
//if( fDebug ){ printf("GenNewAddresAndKey 113 [%s]\n", strAddress.c_str()); }	// [VgBSioFT2Fvj2a5vqzUFo2FGAjNwqrZ4oE]
					if( (strAddress.length() == 34) && (address.SetString(strAddress)) )
					{
//if( fDebug ){ printf("GenNewAddresAndKey 114\n"); }
						CKeyID keyID;
						if( address.GetKeyID(keyID) )
						{
//if( fDebug ){ printf("GenNewAddresAndKey 115\n"); }
							CSecret vchSecret;
							bool fCompressed;
							if( pwalletMain3->GetSecret(keyID, vchSecret, fCompressed) )
							{
								string sKey = CBitcoinSecret(vchSecret, fCompressed).ToString();
//if( fDebug ){ printf("GenNewAddresAndKey 116 [%s]\n", sKey.c_str()); }	// [WZUNC6yf7wqUG9Azw51Khyq8cy24q4YbtGYSN8FsDrQGMQ6bNxxS]
								memset(pNewAddress, 0, strAddress.length() + 1);
								memcpy(pNewAddress, strAddress.c_str(), strAddress.length());
								memset(pNewPriaKey, 0, sKey.length() + 1);
								memcpy(pNewPriaKey, sKey.c_str(), sKey.length());
								rzt++;
//if( fDebug ){ printf("GenNewAddresAndKey 117 [%d]\n", rzt); }
							}
						}
					}
				}
			}
		}
		if( pwalletMain3 ){ 
			bitdb.CloseDb(strWalletFileName2); //bitdb.Flush(false);  
			delete pwalletMain3; 
		}
//if( fDebug ){ printf("GenNewAddresAndKey 118 [%d]\n", rzt); }
	}
	return rzt;
}

//extern CBlockIndex* pindexBest;
DWORD WINAPI GetLastBlockUnixTime(int64_t* pRzt)
{
	DWORD rzt = 0;
	if( pRzt ){ *pRzt = pindexBest->GetBlockTime(); rzt++; }
	return rzt;
}

extern string GetPrivKeysAddress(string &strSecret);
DWORD WINAPI ImportPrivateKey(PCHAR pKeys, PCHAR pLab, int64_t* pi6StartHei)
{
	DWORD rzt = 0;

    if( pKeys != NULL )
	{
	//try{
		string strSecret = pKeys;
		string strLabel = "";
		CBlockIndex* pidxStart = pindexGenesisBlock;
		if( pi6StartHei != NULL )
		{
			int iHei = *pi6StartHei;
			pidxStart = FindBlockByHeight(iHei);
		}
		if( pLab != NULL ){ strLabel = pLab; }

		CBitcoinSecret vchSecret;
		bool fGood = vchSecret.SetString(strSecret);

		if( fGood )
		{
			string sAddr = GetPrivKeysAddress(strSecret);
			if( fDebug ){ printf("ImportPrivateKey [%s] [%s] \n", sAddr.c_str(), strSecret.c_str());  }
			
			if( fWalletUnlockStakingOnly ){ 
				//printf("Wallet is unlocked for staking only.\n"); 
			}
			else{			
				CKey key;
				bool fCompressed;
				CSecret secret = vchSecret.GetSecret(fCompressed);
				key.SetSecret(secret, fCompressed);
				CKeyID vchAddress = key.GetPubKey().GetID();
				{
					LOCK2(cs_main, pwalletMain->cs_wallet);

					pwalletMain->MarkDirty();
					//pwalletMain->SetAddressBookName(vchAddress, strLabel);
					
					// Don't throw error in case a key is already there
					if( pwalletMain->HaveKey(vchAddress) )
					{
						rzt++;
					}else{
						pwalletMain->mapKeyMetadata[vchAddress].nCreateTime = 1;
						if( !pwalletMain->AddKey(key) ){
							//printf("Error adding key to wallet\n");
						}else{
							// whenever a key is imported, we need to scan the whole chain
							pwalletMain->nTimeFirstKey = 1; // 0 would be considered 'no value'

							pwalletMain->ScanForWalletTransactions(pidxStart, true);	//pindexGenesisBlock
							pwalletMain->ReacceptWalletTransactions();
							rzt++;
					
							if( strLabel.length() )
							{
								pwalletMain->SetAddressBookName(vchAddress, strLabel);
								/*
								int64_t nTotalBalance = GetAccountBalance(strLabel, 1);
								if( fDebug ){ printf("[%s] Balance = [%I64u]\n", strLabel.c_str(), nTotalBalance); }
								if( nTotalBalance > 0 ){
									rzt = rzt + SendCoinToAddress(strLabel.c_str(), sDefWalletAddress.c_str(), nTotalBalance, NULL);
								} */
								rzt++;
							}
						}
					}
				} 	
			}
		}//else{ printf("Invalid private key\n"); }
/*		
  }catch (std::exception& e) {
		//PrintException(&e, "ThreadVPN_Chat2()");
		string str = string( e.what() );
		printf("ImportPrivateKey except [%s]\n", str.c_str()); 
  }
  catch (...)
  {
      // someone threw something undecypherable
	  printf("ImportPrivateKey undecypherable\n"); 
  } */
  
	}
	return rzt;
}

DWORD WINAPI SetBitNetDropEventFunc(PCHAR pFunc)
{
	BitNetDropEventFunc = (lpDropEvent)pFunc;
	return 1;
}

DWORD WINAPI CallCoinGuiHandleURI(PCHAR pUri)
{
	DWORD rzt = 0;
	if( pUri != NULL )
	{
		//string sUri = pUri;
		rzt = vpnBitcoinGUI->handleURI(QString::fromUtf8(pUri), 1, false);
	}
	return rzt;
}

DWORD WINAPI GoToCoinGuiPage(int iOpt)
{
	if( vpnBitcoinGUI ){ 
		vpnBitcoinGUI->showNormalIfMinimized();
		if( iOpt == 1 ){ vpnBitcoinGUI->gotoOverviewPage(); }
		else if( iOpt == 2 ){ vpnBitcoinGUI->gotoSendCoinsPage(); }
		else if( iOpt == 3 ){ vpnBitcoinGUI->gotoReceiveCoinsPage(); }
		else if( iOpt == 4 ){ vpnBitcoinGUI->gotoHistoryPage(); }
		else if( iOpt == 5 ){ vpnBitcoinGUI->gotoAddressBookPage(); }
		else if( iOpt == 6 ){ vpnBitcoinGUI->gotoSupNetClicked(); }
		else if( iOpt == 7 ){ vpnBitcoinGUI->gotoBitNetClicked(); }
		else if( iOpt == 8 ){ vpnBitcoinGUI->gotoVpnClicked(); }
		else if( iOpt == 9 ){ vpnBitcoinGUI->gotoLotteryClicked(); }
		else if( iOpt == 10 ){ vpnBitcoinGUI->gotoChequeClicked(); }
		else if( iOpt == 11 ){ vpnBitcoinGUI->gotoBonusClicked(); }
	}
	return 1;
}

DWORD WINAPI GetCoinConfAndDataDirAndWalletFile(DWORD* pCoinConf, DWORD* pDataDir, DWORD* pWalletFile, DWORD* pMaxUnit)
{
	DWORD rzt = 0;
	if( pCoinConf ){ *pCoinConf = (DWORD)s_Coin_conf_file.c_str(); }
	if( pDataDir ){ *pDataDir = (DWORD)s_blockchain_data_dir.c_str(); }
	if( pWalletFile ){ *pWalletFile = (DWORD)s_Coin_wallet_file.c_str(); }
	if( pMaxUnit ){ *pMaxUnit = (DWORD)s_Coin_max_unit.c_str(); }
	rzt++;
	return rzt;
}

DWORD WINAPI IsValidCoinAddress(PCHAR pAddress)
{
	DWORD rzt = 0;
	if( pAddress )
	{
		string s = pAddress;
		CBitcoinAddress address(s);
        if( address.IsValid() ){ rzt++; }
	}
	return rzt;
}

DWORD WINAPI GetLastBlockHeight(int64_t* pRzt)
{
	DWORD rzt = 0;
	if( pRzt ){ *pRzt = pindexBest->nHeight; rzt++; }
	return rzt;
}

DWORD WINAPI IsBlockSynchronizing()
{
	DWORD rzt = 0;
	if(  (vpnBitcoinGUI != NULL)  &&  ( vpnBitcoinGUI->overviewPage->isBlockSynchronizing ) ){ rzt++; }
	return rzt;
}

int  GetAddrInTransactionIndex(const string txID, string sAddr, int64_t v_nValue)
{
	int rzt = -1;
	if( txID.length() > 34 )
	{
		string srzt = "";
		uint256 hash;
		hash.SetHex(txID);

		CTransaction tx;
		uint256 hashBlock = 0;
		if (!GetTransaction(hash, tx, hashBlock))
			return rzt;

		//CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
		//ssTx << tx;
		//srzt = HexStr(ssTx.begin(), ssTx.end());
		
    //Array vout;
    for (unsigned int i = 0; i < tx.vout.size(); i++)
    {
        const CTxOut& txout = tx.vout[i];
		if( txout.nValue == v_nValue )
		{
			//Object out;
			//out.push_back(Pair("value", ValueFromAmount(txout.nValue)));
			//out.push_back(Pair("n", (int64_t)i));
			//Object o;
			//ScriptPubKeyToJSON(txout.scriptPubKey, o, false);
		
			txnouttype type;
			vector<CTxDestination> addresses;
			int nRequired;

			if( ExtractDestinations(txout.scriptPubKey, type, addresses, nRequired) )
			{
				//out.push_back(Pair("type", GetTxnOutputType(type)));
				//Array a;
				BOOST_FOREACH(const CTxDestination& addr, addresses)
				{
					string sAa = CBitcoinAddress(addr).ToString();
					if( sAa == sAddr )
					{
						return i;
					}
					//a.push_back(CBitcoinAddress(addr).ToString());
					//out.push_back(Pair("addresses", a));
				}
			}
		}
    }
	
	}
	return rzt;
}

DWORD WINAPI GetAddrInTransactionOutIndex(const PCHAR ptxID, PCHAR pAddr, int64_t* v_nValue)
{
	DWORD rzt = 0xFFFFFFFF;
	if( (ptxID == NULL) || (pAddr == NULL)  || (v_nValue == NULL) ){ return rzt; }
	string sId = ptxID;
	string sAddr = pAddr;
	int64_t v = *v_nValue;
	rzt = (DWORD)GetAddrInTransactionIndex(sId, sAddr, v);
	return rzt;
}

extern int isValidPrivKeysAddress(string &strSecret, string &sAddr);
DWORD WINAPI IsValidPrivKeysAddress( PCHAR pSecret, PCHAR pAddr )
{
	DWORD rzt = 0;
	if( (pSecret != NULL) && (pAddr != NULL) )
	{
		string strSecret = pSecret;
		string sAddr = pAddr;
	    rzt =  isValidPrivKeysAddress(strSecret, sAddr);
	}
	return rzt;
}

extern string signMessage(const string strAddress, const string strMessage);
extern bool verifyMessage(const string strAddress, const string strSign, const string strMessage);
DWORD WINAPI SignMessage(PCHAR pAddress, PCHAR pMessage, PCHAR pSign)
{
	DWORD rzt = 0;
	if( (pAddress != NULL) && (pSign != NULL) && (pMessage != NULL) )
	{
		string strAddress = pAddress;
		string strMessage = pMessage;
		string strSign = signMessage(strAddress, strMessage);
		int i = strSign.length();
		if(  i > 0 )
		{
			memset(pSign, 0, i + 1);
			memcpy(pSign, strSign.c_str(), i);
			rzt++;
		}
	}
	return rzt;
}

DWORD WINAPI VerifyMessage(PCHAR pAddress, PCHAR pSign, PCHAR pMessage)
{
	DWORD rzt = 0;
	if( (pAddress != NULL) && (pSign != NULL) && (pMessage != NULL) )
	{
		string strAddress = pAddress;
		string strSign = pSign;
		string strMessage = pMessage;
		if( verifyMessage(strAddress, strSign, strMessage) ){ rzt++; }
	}
	return rzt;
}

extern bool isValidLotteryGenesisTxs(const string& txID, int iTxHei, int iTargetGuessType, int iTargetGuessLen, int64_t i6TargetBlock, const string sTargetMaker, const string sTargetGenesisAddr, bool bMustExist = false);
extern bool        isValidLotteryBetTxs(const string& txID, int iTxHei, int iTargetGuessType, int iTargetGuessLen, int64_t i6TargetBlock, const string sTargetMaker, const string sTargetGenesisAddr);
int WINAPI  IsValidLotteryBetTxs(PCHAR pTxID, int iTxHei, int iTargetGuessType, int iTargetGuessLen, int64_t* pI6TargetBlock, PCHAR pTargetMaker, PCHAR pTargetGenesisAddr)
{
	int rzt = 0;
	if( pTxID != NULL  )
	{
		string txID = pTxID;
		string sTargetMaker = "", sTargetGenesisAddr = "";
		int64_t i6TargetBlock = 0;
		if( pTargetMaker != NULL ){ sTargetMaker = pTargetMaker; }
		if( pTargetGenesisAddr != NULL ){ sTargetGenesisAddr = pTargetGenesisAddr; }
		if( pI6TargetBlock != NULL ){ i6TargetBlock = *pI6TargetBlock; }
		if( isValidLotteryBetTxs(txID, iTxHei, iTargetGuessType, iTargetGuessLen, i6TargetBlock, sTargetMaker, sTargetGenesisAddr) ){
			rzt++;
		}
	}
	return rzt;
}
int WINAPI  IsValidLotteryGenesisTxs(PCHAR pTxID, int iTxHei, int iTargetGuessType, int iTargetGuessLen, int64_t* pI6TargetBlock, PCHAR pTargetMaker, PCHAR pTargetGenesisAddr)
{
	int rzt = 0;
	if( pTxID != NULL  )
	{
		string txID = pTxID;
		string sTargetMaker = "", sTargetGenesisAddr = "";
		int64_t i6TargetBlock = 0;
		if( pTargetMaker != NULL ){ sTargetMaker = pTargetMaker; }
		if( pTargetGenesisAddr != NULL ){ sTargetGenesisAddr = pTargetGenesisAddr; }
		if( pI6TargetBlock != NULL ){ i6TargetBlock = *pI6TargetBlock; }
		if( isValidLotteryGenesisTxs(txID, iTxHei, iTargetGuessType, iTargetGuessLen, i6TargetBlock, sTargetMaker, sTargetGenesisAddr, true) ){
			rzt++;
		}
	}
	return rzt;
}

typedef std::pair<string, int> pair_betinf;
int WINAPI GetBetAmountFromBlockRange(int iBlockBegin, int iBlockEnd, PCHAR pLotteryGenAddr, int64_t* pI6Rzt, int bPrtBetInfo)
{
	int rzt = 0;
	if( (pLotteryGenAddr != NULL) && (pI6Rzt != NULL) )
	{
		string sLotteryGenAddr = pLotteryGenAddr;
		if( bPrtBetInfo > 0 )
		{
			//Object entry;
			std::vector<std::pair<string, string> > entry;
			*pI6Rzt = getBetAmountFromBlockRange(iBlockBegin, iBlockEnd, sLotteryGenAddr, &entry);
			std::string strDataDir = GetDataDir().string() + "\\" + sLotteryGenAddr + ".log" ;
			ofstream myfile;
			myfile.open (strDataDir.c_str());
			string s = "-----------------------------------------------------------------------------------------------------------------------------------\n";
			                  //Vq9EBcb43uEfFDJYbcRDL4fk8V9mB6mkT8	300000VPN, bet txt '369'
			myfile << s;
			//std::vector<std::pair<CScript, int64_t> > vecSend;
			BOOST_FOREACH(const PAIRTYPE(string, string)& item, entry)
			{
				string s = item.first + "\t" + item.second + "\n"; //strprintf("%d", item.second);
				myfile << s;
			}
			myfile.close();
		}
		else{ *pI6Rzt = getBetAmountFromBlockRange(iBlockBegin, iBlockEnd, sLotteryGenAddr, NULL); }
		rzt++;
	}
	return rzt;
}
extern int getLotteryWinner(const string txID, const string sTargetMaker, string& sRztWinner, string& sRztAnswer, int64_t& i6RztBet, const string sBetTxt = "");
int WINAPI GetLotteryWinner(PCHAR pTxID, PCHAR pTargetMaker, PCHAR pBetTxt, PCHAR pRztWinner, PCHAR pRztAnswer, int64_t* pI6RztBet)
{
	int rzt = 0;
	if( (pTxID != NULL) && (pRztWinner != NULL) )
	{
		string txID = pTxID, sTargetMaker = "", sRztWinner = "", sRztAnswer = "", sBetTxt = "";
		int64_t i6RztBet = 0;
		if( pTargetMaker != NULL ){ sTargetMaker = pTargetMaker; }
		if( pBetTxt != NULL ){ sBetTxt = pBetTxt; }
		rzt = getLotteryWinner(txID, sTargetMaker, sRztWinner, sRztAnswer, i6RztBet, sBetTxt);
		//if( rzt > 0 )
		{
			int i = sRztWinner.length();
			if( i > 0 )
			{
				memset(pRztWinner, 0, i + 1);
				memcpy(pRztWinner, sRztWinner.c_str(), i);
			}
			if( pRztAnswer != NULL )
			{
				i = sRztAnswer.length();
				if( i > 0 )
				{
					memset(pRztAnswer, 0, i + 1);
					memcpy(pRztAnswer, sRztAnswer.c_str(), i);
				}
			}
			if( pI6RztBet != NULL ){ *pI6RztBet = i6RztBet; }
		}
	}
	return rzt;
}

extern int GetTransactionBlockHeight(const string& TxID);
DWORD WINAPI  getTransactionBlockHeight(PCHAR pTxID)
{
	DWORD rzt = 0;
	if( pTxID != NULL )
	{
		string txID = pTxID;
		if( txID.length() > 34 ) rzt = (DWORD)GetTransactionBlockHeight(txID);
	}
	return rzt;
}

DWORD WINAPI BoostMeToNode(CNode* node)
{
    int rzt = 0;
    {
        if( node )
		{
			if( FindNode(node) != NULL ){ PushBoostMe(node);  rzt++; }
		}
		else{ PushBoostMeToAll();  rzt++; }
		return rzt;
    }
}

namespace js = json_spirit;
//json_spirit::Object ObjFromString(std::string str)
Object ObjFromString(std::string str)
{
	Object rzt;
	if( str.length() < 3 ){ return rzt; }
	//namespace js = json_spirit;
    js::Value top;  
    if( read_string(str, top) )  //if( js::read(str, top) )
	{
		//json_spirit::mObject mObj = top.get_obj();
		//return top.get_obj();  //rzt = top.get_obj();
		//const Object& o = top.get_obj();
		return top.get_obj();
	}
	return rzt;
}
Value ValueFromString(std::string str)
{
	Value rzt;
	if( str.length() < 3 ){ return rzt; }
	//namespace js = json_spirit;
    js::Value top;  
    if( read_string(str, top) )  //if( js::read(str, top) )
	{
		return top;
	}
	return rzt;
}

int WINAPI SendCoinToMany(PCHAR pAccount, PCHAR pTo, PCHAR pMsg, int iJiami, PCHAR pRztTxId)
{
    int rzt = 0;
	//printf("SendCoinToMany, pAccount = [%s], pTo = [%s], pMsg = [%s], [%s], %d\n", pAccount, pTo, pMsg, pRztTxId, iJiami);
	//if (fHelp || params.size() < 2 || params.size() > 4)
    //    throw runtime_error(
    //        "sendmany <fromaccount> {address:amount,...} [minconf=1] [comment]\n"
    //        "amounts are double-precision floating point numbers"
    //        + HelpRequiringPassphrase());

    if( pTo == NULL ){ return rzt; }
	string strAccount = "";
	if( pAccount != NULL ){ strAccount = pAccount; }
//sendmany 6e '{"VZzpuiATQouD4mu9jnedRuKrgpHyY2Me1w":17000000,"VZzpuiATQouD4mu9jnedRuKrgpHyY2Me1w":16800000}'
    string sTo = pTo;
	Value val;
	if( !read_string(sTo, val) ){  
		//printf("read_string = false\n");
		return rzt;
	}
	//printf("read_string(%s) = ok\n", sTo.c_str());
	
	//Value val = ValueFromString(sTo);  //top.get_obj();
	Object sendTo = val.get_obj(); //ObjFromString(sTo);	//params[1].get_obj();
    int nMinDepth = 1;
	//printf("nMinDepth  = 1\n");

    CWalletTx wtx;
    wtx.strFromAccount = strAccount;
    //if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
    //    wtx.mapValue["comment"] = params[3].get_str();

    set<CBitcoinAddress> setAddress;
    vector<pair<CScript, int64_t> > vecSend;

    int64_t totalAmount = 0;
    BOOST_FOREACH(const Pair& s, sendTo)
    {
		//printf("BOOST_FOREACH, totalAmount = [%I64u] \n", totalAmount);
		//string sAddr = s.name_;
		//printf("s.name = [%s], s.value_ = [%f]\n", sAddr.c_str(), s.value_.get_real());
		CBitcoinAddress address(s.name_);
        if( !address.IsValid() ){ continue; }

        if( setAddress.count(address) ){ continue; }
        setAddress.insert(address);

        CScript scriptPubKey;
        scriptPubKey.SetDestination(address.Get());
        int64_t nAmount = AmountFromValue(s.value_);
        totalAmount += nAmount;
        vecSend.push_back(make_pair(scriptPubKey, nAmount));
    }

    EnsureWalletIsUnlocked();

    // Check funds
	int64_t nBalance = 0;
	if( strAccount.length() ){ nBalance = GetAccountBalance(strAccount, nMinDepth); }
	else nBalance = i6TotalCoins;
//printf("nBalance = [%I64u], totalAmount = [%I64u]\n", nBalance, totalAmount);
    if( totalAmount >= nBalance ){ 
		//printf("totalAmount = [%I64u] >= nBalance = [%I64u], return :(\n", totalAmount, nBalance);
		return rzt; 
	}
        //throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
	std::string stxData = "";
	if( pMsg != NULL ){ stxData = pMsg; }
    CReserveKey keyChange(pwalletMain);
    int64_t nFeeRequired = 0;
    bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, stxData, iJiami);  // iJiami = 1 = encryption
    if( !fCreated )
    {
        //printf("fCreated = false\n");
		//if (totalAmount + nFeeRequired > pwalletMain->GetBalance())
		{
            //throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");
			//throw JSONRPCError(RPC_WALLET_ERROR, "Transaction creation failed");
		}
    }
    else if( pwalletMain->CommitTransaction(wtx, keyChange) ){ rzt++; }
	//return wtx.GetHash().GetHex();
//printf("SendCoinToMany, rzt = %d\n", rzt);
	if( (rzt > 0) && (pRztTxId != NULL) )
	{
		string sRztAnswer = wtx.GetHash().GetHex();
		int i = sRztAnswer.length();
		//printf("[%d], wtx.Hash = [%s]\n", i, sRztAnswer.c_str());
		if( i > 0 )
		{
			memset(pRztTxId, 0, i + 1);
			memcpy(pRztTxId, sRztAnswer.c_str(), i);
		}
	}
//printf("SendCoinToMany rzt = %d\n", rzt);
	return rzt;
}

int WINAPI BroadCastAdBonus(PCHAR pId, PCHAR pBns)
{
	int rzt = 0;
	if( (pId != NULL) && (pBns != NULL) )
	{
		string sBonusId = pId;
		string sBonus = pBns;
		AddBitNetBonus(sBonusId, sBonus);  //if( AddBitNetBonus(sBonusId, sBonus) > 0 )
		{
			rzt = RelayBonusID(sBonusId);
		}
	}
	return rzt;
}

extern int GetValidTransaction(const string txID, CTransaction &tx);
int WINAPI SendAdBonusCoinFromTx(PCHAR pTxID, PCHAR pBonusKey, PCHAR pToAddr, PCHAR pTxMsg, PCHAR pRztTxHash, int iOpt)
{
	int rzt = 0;
	//if( fDebug ){ printf("SendAdBonusCoinFromTx [%s]\n", pTxID); }
	if( (pTxID != NULL) && (pBonusKey != NULL) && (pToAddr != NULL) )
	{
		CTransaction tx;
		string txID = pTxID;
		//if( fDebug ){ printf("SendAdBonusCoinFromTx [%s] [%s] [%s] \n", txID.c_str(), pBonusKey, pToAddr); }
		if( GetValidTransaction(txID, tx) == 0 )
		{ 
			//if( fDebug ){ printf("SendAdBonusCoinFromTx get tx = 0 \n"); }
			return rzt; 
		}
		
		string sBonusAddr = "";
		if( iOpt == 0 )
		{
			string sKey = pBonusKey;
			sBonusAddr = GetPrivKeysAddress(sKey);
		}
		else if( iOpt == 1 ){ sBonusAddr = pBonusKey; }
		//if( fDebug ){ printf("SendAdBonusCoinFromTx  sBonusAddr = [%s]\n", sBonusAddr.c_str()); }
		if( !validateAddress(sBonusAddr) )
		{
			//if( fDebug ){ printf("SendAdBonusCoinFromTx sBonusAddr not valid \n"); }
			return rzt;  //if( sBonusAddr.length() < 30 ){ return rzt; }	
		}
		
		string sToAddr = pToAddr;
		if( !validateAddress(sToAddr) )
		{
			//if( fDebug ){ printf("SendAdBonusCoinFromTx sToAddr not valid \n"); }
			return rzt;
		}
		
		string stxData = "";
		if( pTxMsg != NULL ){ sToAddr = pTxMsg; }
		CCoinControl* coinControl = new CCoinControl();	
		int64_t i6Amount = getBetAmountFromTxOut(tx, sBonusAddr, NULL, coinControl);
		//if( fDebug ){ printf("Total Coin = [%I64u], i6Amount = [%I64u] \n", i6TotalCoins, i6Amount); }
		
		if( i6Amount == 0  )	//if( (i6Amount == 0) || (i6TotalCoins < i6Amount) )
		{
			delete coinControl;
			return rzt;
		}
		//coinControl->destChange = CBitcoinAddress(sLotAddr).Get();	// zhao ling di zhi
		//if( pAmount != NULL ) *pAmount = i6Amount;
        LOCK2(cs_main, pwalletMain->cs_wallet);
        // Sendmany
        std::vector<std::pair<CScript, int64_t> > vecSend;
        //foreach(const SendCoinsRecipient &rcp, recipients)
        {
            CScript scriptPubKey;
            scriptPubKey.SetDestination(CBitcoinAddress(sToAddr).Get());
            vecSend.push_back( make_pair(scriptPubKey, i6Amount - MIN_TX_FEE) );	//MIN_TX_FEE  (0.01 * COIN)
        }

        CWalletTx wtx;
		//wtx.strFromAccount = strAccount;
        CReserveKey keyChange(pwalletMain);
        int64_t nFeeRequired = 0;		
//bool CreateTransaction(CScript scriptPubKey, int64_t nValue, CWalletTx& wtxNew, CReserveKey& reservekey, int64_t& nFeeRet, std::string stxData, int bEncrypt, const CCoinControl *coinControl=NULL);		
		bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, stxData, 0, coinControl);
		delete coinControl;
		if (!fCreated)
		{
			if( fDebug ){ printf( "Create Transaction failed \n"); }
			return rzt;
		}
		if (!pwalletMain->CommitTransaction(wtx, keyChange))
		{
			if( fDebug ){ printf( "Transaction commit failed \n"); }
			return rzt;
		}
		stxData = wtx.GetHash().GetHex();
		rzt = stxData.length();
		if( (rzt > 0) && (pRztTxHash != NULL) )
		{
			memset(pRztTxHash, 0, rzt + 1);
			memcpy(pRztTxHash, stxData.c_str(), rzt);
		}
	}
	return rzt;
}

int WINAPI SetAdBonusIcon(int idx, int ico)
{
	int rzt = 0;
	//printf("SetAdBonusIcon %d %d\n", idx, ico);
	if( vpnBitcoinGUI != NULL )
	{
		vpnBitcoinGUI->setAdBonusIcon(idx, ico);
		rzt++;
		//printf("SetAdBonusIcon rzt = %d\n", rzt);
	}
	return rzt;
}

int WINAPI WalletAppQuit(int iOpt)
{
	int rzt = 0;
	if( vpnBitcoinGUI != NULL )
	{
		vpnBitcoinGUI->appQuit();   rzt++;
	}
	return rzt;
}

int WINAPI getPrivKeysAddress(char *pSecret, char *pRzt)
{
	int rzt = 0;
	if( (pSecret != NULL) && (pRzt != NULL) )
	{
		string strSecret = pSecret;
		string s = GetPrivKeysAddress(strSecret);
		rzt = s.length();
		if( rzt > 30 )
		{
			memset(pRzt, 0, rzt + 1);
			memcpy(pRzt, s.c_str(), rzt);
		}
	}
	return rzt;
}

int WINAPI AddLocalAddress(char *pHost)
{
	int rzt = 0;
	if( pHost != NULL )
	{
		string sHost = pHost;
		CService addr(sHost, 0, true);
        if (!addr.IsValid() || !addr.IsRoutable()){ return rzt; }
		CNetAddr addrLocalHost;
        addrLocalHost.SetIP(addr);
		rzt = AddLocal(addrLocalHost, LOCAL_HTTP);
	}
	return rzt;
}

extern int is_Address_in_Tx_n_SentFromBlockRange(int iBlockBegin, int iBlockEnd, const string txID, const string sAddress);
int WINAPI Is_Address_in_Tx_n_SentFromBlockRange(int iBlockBegin, int iBlockEnd, char* pTxID, char* pAddress)
{
	int rzt = 0;
	if( (pTxID != NULL) && (pAddress != NULL) )
	{
		string txID = pTxID;
		string sAddress = pAddress;
		if(  (iBlockEnd == 0) || (iBlockEnd > nBestHeight) ){ iBlockEnd = nBestHeight; }
		rzt = is_Address_in_Tx_n_SentFromBlockRange(iBlockBegin, iBlockEnd, txID, sAddress);
	}
	return rzt;
}

extern int SaveBlockToFile(CBlock* block, int iHi, int iTeam);
int saveblocktofile(int iHi, int iTeam)
{
	int rzt = 0;
	if( iHi >= 0 )
	{
		CBlockIndex* pblockindex = FindBlockByHeight(iHi);
		if( pblockindex )
		{
			CBlock block;
			block.ReadFromDisk(pblockindex);
			//block.print();
			rzt = SaveBlockToFile(&block, iHi, iTeam);
		}
	}
	return rzt;
}

int WINAPI SaveBlocksToFile(int iLow, int iHei, int iTeam)
{
	int rzt = 0;
	if( (iLow >= 0) && (iHei > iLow) )
	{
		if( iHei > nBestHeight ){ iHei = nBestHeight; }
		iHei++;
		for(int i = iLow; i < iHei; i++ )
		{
			if( saveblocktofile(i, iTeam) > 0 ) rzt++;
		}
	}
	return rzt;
}

int WINAPI ImportBlockFromMem(int iHei, int bSize, CBlock* pblock)
{
	int rzt = 0;
	if( (iHei > nBestHeight) && (bSize > 0) )
	{
		//if( fDebug ){ printf("ImportBlockFromMem::111 [%d :: %d] [%d] [%s] \n", iHei, nBestHeight, bSize, pblock->GetHash().ToString().c_str()); }
		{
//if( fDebug ){ printf("ImportBlockFromMem:: 222 \n"); }
            //vector<char> v(bSize);
			//memcpy((char*)&v[0], (char*)pblock, bSize);
			CDataStream ssBlock(SER_DISK, CLIENT_VERSION);
			char* ppp = (char*)pblock;
			for(int i = 0; i < bSize; i++ )
			{
				ssBlock << ppp[i];
			}
			CBlock block;
			ssBlock >> block;
			//if( fDebug ){ printf("ImportBlockFromMem [%s] \n", block.GetHash().ToString().c_str()); }
			ProcessBlock(NULL, &block);
			//rzt = AddMyBlock( block );
			rzt++;   
			//vpnBitcoinGUI->setNumBlocks(nBestHeight, GetNumBlocksOfPeers());
			vpnBitcoinGUI->clientModel->UpNumBlocks(nBestHeight, GetNumBlocksOfPeers()); 
			//return rzt;
		}
	}
	//if( fDebug ){ printf("ImportBlockFromMem:: rzt [%d] [%d :: %d] \n", rzt, iHei, nBestHeight); }
	return rzt;
}

DWORD WINAPI GetBuildinVpnFunc(int iFunc)
{
	DWORD rzt = 0;
	//printf("GetBuildinVpnFunc: %u, \n", iFunc);
	if( iFunc == 1 ){ rzt = (DWORD)&SendBitNetChatMsg; }
	else if( iFunc == 2 ){ rzt = (DWORD)&VpnConfigChange; }
	else if( iFunc == 3 ){ rzt = (DWORD)&VpnSetNode; }
	else if( iFunc == 4 ){ rzt = (DWORD)&SendVpnFileReq; }
	else if( iFunc == 5 ){ rzt = (DWORD)&SendVpnFileAck; }
	else if( iFunc == 6 ){ rzt = (DWORD)&SendBitNetPak; }
	else if( iFunc == 7 ){ rzt = (DWORD)&SetMemAndCpuInfo; }
	else if( iFunc == 8 ){ rzt = (DWORD)&PayCoinsToVpnSupplier; }
	else if( iFunc == 9 ){ rzt = (DWORD)&GetUnixTime64; }
	else if( iFunc == 10 ){ rzt = (DWORD)&ClearNodeBuf; }
	else if( iFunc == 11 ){ rzt = (DWORD)&ConnectToNewNode; }
	else if( iFunc == 12 ){ rzt = (DWORD)&ReqBitNetNodes; }
	else if( iFunc == 13 ){ rzt = (DWORD)&RelayBitNetNodes; }
	else if( iFunc == 14 ){ rzt = (DWORD)&IsWalletLabelExists; }
	else if( iFunc == 15 ){ rzt = (DWORD)&SendBitNetCustomPack; }
	else if( iFunc == 16 ){ rzt = (DWORD)&IsNodeExists; }
	else if( iFunc == 17 ){ rzt = (DWORD)&SetBitNetArg; }
	else if( iFunc == 18 ){ rzt = (DWORD)&SendCoinToAddress; }
	else if( iFunc == 19 ){ rzt = (DWORD)&FindNodeByWalletAddress; }
	else if( iFunc == 20 ){ rzt = (DWORD)&IsMineCoinAddress; }
	else if( iFunc == 21 ){ rzt = (DWORD)&BrowserUrlOrRunJava; }
	else if( iFunc == 22 ){ rzt = (DWORD)&PressHotKey; }
	else if( iFunc == 23 ){ rzt = (DWORD)&BrowseNodesWebsite; }
	else if( iFunc == 24 ){ rzt = (DWORD)&SwitchWalletFile; }
	else if( iFunc == 25 ){ rzt = (DWORD)&GenNewAddresAndKey; }
	else if( iFunc == 26 ){ rzt = (DWORD)&GetLastBlockUnixTime; }
	else if( iFunc == 27 ){ rzt = (DWORD)&LoadDllFromMemory; }
	else if( iFunc == 28 ){ rzt = (DWORD)&ImportPrivateKey; }
	else if( iFunc == 29 ){ rzt = (DWORD)&SetBitNetDropEventFunc; }
	else if( iFunc == 30 ){ rzt = (DWORD)&CallCoinGuiHandleURI; }
	else if( iFunc == 31 ){ rzt = (DWORD)&GoToCoinGuiPage; }
	else if( iFunc == 32 ){ rzt = (DWORD)&GetCoinConfAndDataDirAndWalletFile; }
	else if( iFunc == 33 ){ rzt = (DWORD)&IsValidCoinAddress; }
	else if( iFunc == 34 ){ rzt = (DWORD)&GetLastBlockHeight; }
	else if( iFunc == 35 ){ rzt = (DWORD)&IsBlockSynchronizing; }
	else if( iFunc == 36 ){ rzt = (DWORD)&GetAddrInTransactionOutIndex; }
	else if( iFunc == 37 ){ rzt = (DWORD)&IsValidPrivKeysAddress; }
	else if( iFunc == 38 ){ rzt = (DWORD)&SignMessage; }
	else if( iFunc == 39 ){ rzt = (DWORD)&VerifyMessage; }
	else if( iFunc == 40 ){ rzt = (DWORD)&IsValidLotteryGenesisTxs; }
	else if( iFunc == 41 ){ rzt = (DWORD)&IsValidLotteryBetTxs; }
	else if( iFunc == 42 ){ rzt = (DWORD)&GetBetAmountFromBlockRange; }
	else if( iFunc == 43 ){ rzt = (DWORD)&GetLotteryWinner; }
	else if( iFunc == 44 ){ rzt = (DWORD)&GetRewardFromLottery; }
	else if( iFunc == 45 ){ rzt = (DWORD)&getTransactionBlockHeight; }
	else if( iFunc == 46 ){ rzt = (DWORD)&BoostMeToNode; }
	else if( iFunc == 47 ){ rzt = (DWORD)&GetNewAddressKeyFromCurrentWallet; }
	else if( iFunc == 48 ){ rzt = (DWORD)&SendCoinToMany; }
	else if( iFunc == 49 ){ rzt = (DWORD)&BroadCastAdBonus; }
	else if( iFunc == 50 ){ rzt = (DWORD)&SendAdBonusCoinFromTx; }
	else if( iFunc == 51 ){ rzt = (DWORD)&SetAdBonusIcon; }
	else if( iFunc == 52 ){ rzt = (DWORD)&WalletAppQuit; }
	else if( iFunc == 53 ){ rzt = (DWORD)&getPrivKeysAddress; }
	else if( iFunc == 54 ){ rzt = (DWORD)&AddLocalAddress; }
	else if( iFunc == 55 ){ rzt = (DWORD)&Is_Address_in_Tx_n_SentFromBlockRange; }
	else if( iFunc == 56 ){ rzt = (DWORD)&SaveBlocksToFile; }
	else if( iFunc == 57 ){ rzt = (DWORD)&ImportBlockFromMem; }
	return rzt;
}

DWORD SyncTransactionToGui(std::string& txId, std::string& txMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime)
{
	DWORD rzt = 0;
	if( (TxAmount > 0) && (txTime > 1407099999) && (txId.length() > 10) && (txMsg.length() > 2) )
	{
		if( pSyncTransaction )
		{
			int64_t txNow = GetTime();
			//DWORD (WINAPI *lpSyncTransaction) (PCHAR pTxid, PCHAR pTxMsg, DWORD iDepth, DWORD nHeight, DWORD curHeight, int64_t TxAmount, int64_t txTime);
			rzt = pSyncTransaction((PCHAR)txId.c_str(), (PCHAR)txMsg.c_str(), iDepth, nHeight, curHeight, TxAmount, txTime, txNow);
		}
	}
	return rzt;
}

/*
	int v_isVpnServer;
	int v_iVpnServicePort;
	int v_bShowInOtherList;
	int v_iVpnServiceCtrlPort;
	int64_t v_iVpnServiceFee;
	int v_iVpnServiceTryMinute;
	std::string v_sVpnWalletAddress;
	std::string v_Nickname;
	int v_iCanTalk;
	
		vRecv >> pfrom->v_bShowInOtherList >> pfrom->v_isVpnServer >> pfrom->v_iVpnServiceCtrlPort >> pfrom->v_iVpnServicePort >> pfrom->v_iVpnServiceFee;
		vRecv >> pfrom->v_iVpnServiceTryMinute >> pfrom->v_sVpnWalletAddress >> pfrom->v_Nickname; 
typedef DWORD (WINAPI *lpSyncNode) (DWORD bAdd, CNode* node, DWORD dIp, DWORD isServer, DWORD dCtrlPort, DWORD dPort, DWORD dTryMinute, DWORD dCanTalk, int64_t i6Fee, char* wAddr, char* NickName);
		*/
DWORD SynNodeToBitNetGui(CNode* node, DWORD bAdd, DWORD dRecvSize, const char* pTalk)
{
	DWORD rzt = 0;
	//DWORD dIp = 0;
	if( (bServiceMode == 0) && (pVpnSyncNode != NULL) )
	{
		if( node != NULL )
		{
			if( GetArg("-syncbitnetsignal", 0) ){ LOCK(cs_vBitNet); }	// 2014.12.19 add
			std::string str, sFrm;
			QString qFrm, qMsg;
			
			node->vBitNet.v_Option = bAdd;
			node->vBitNet.v_RecvSize = dRecvSize;
			//if( fDebug ){ printf("SynNodeToBitNetGui: dOption =%u, dRecvSize =%u, node =%X, vBitNet =%X\n", bAdd, dRecvSize, node, &node->vBitNet); }
			
			if( GetArg("-showchat_tips", 0) )
			{
			if( bAdd == 2 )
			{
				if( pTalk != NULL ){
					char* pp = NULL;
					if( node->vBitNet.v_iVersion >= 1117 )
					{
						//BitNet_Chat_msg_Pack* pChatMsg = (BitNet_Chat_msg_Pack*)pTalk;
						//pp = &pChatMsg->aMsg[0];
						pp = " ";
					}else{ pp = (char*)pTalk; }
					QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
					sFrm = node->vBitNet.v_Nickname + " Talk to you";
					qFrm = QString::fromStdString(sFrm);
					str = std::string(pp);
					qMsg = QString::fromStdString(str);
					vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
					//vpnNotificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VpnCoin address or malformed URI parameters."));
				}
			}else if( bAdd == 3 )	// recv file req
			{
				char* pfile = NULL;
				if( node->vBitNet.v_File_Req.length() ){ pfile = (char *)node->vBitNet.v_File_Req.c_str(); }
				if( pfile != NULL ){	//if( pTalk != NULL ){
					QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
					sFrm = node->vBitNet.v_Nickname + " Send file to you";
					qFrm = QString::fromStdString(sFrm);
					str = strprintf("%s, %I64u Bytes", pfile, node->vBitNet.v_File_size);
					qMsg = QString::fromStdString(str);
					vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
				}
			}
			}
			
			rzt = pVpnSyncNode(bAdd, dRecvSize, node, &node->vBitNet);
			//if( fDebug ){ printf("SynNodeToBitNetGui rzt %u\n", rzt); }
			
			/*
			//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
			
			//dIp = node->sAddr;
			//printf("SynNodeToBitNetGui: NickName=%s\n", node->v_Nickname.c_str());
			QString qFrm, qMsg;
			std::string str, sFrm;
			char* pDefwa = NULL;
			char* pVpnwa = NULL;
			char* pfile = NULL;
			char* pNickName = NULL;
			char* pNickNamePk = NULL;
			PCHAR pMemCpu = NULL;
			PCHAR pfileLoc = NULL;
			if( node->v_File_Loc.length() ){ pfileLoc = (char *)node->v_File_Loc.c_str(); }
			if( node->v_File_Req.length() ){ pfile = (char *)node->v_File_Req.c_str(); }
			if( node->v_sDefWalletAddress.length() > 30 ){ pDefwa = (char *)node->v_sDefWalletAddress.c_str(); }
			if( node->v_sVpnWalletAddress.length() > 0 ){ pVpnwa = (char *)node->v_sVpnWalletAddress.c_str(); }
			//printf("v_sVpnWalletAddress = %s\n", node->v_sVpnWalletAddress.c_str()); }
			if( node->v_sVpnMemAndCpuInfo.length() > 2 ){ pMemCpu = (PCHAR)node->v_sVpnMemAndCpuInfo.c_str(); }
			if( node->v_NicknamePack.length() ){ pNickNamePk = (PCHAR)node->v_NicknamePack.c_str(); }
			if( node->v_Nickname.length() ){ pNickName = (PCHAR)node->v_Nickname.c_str(); }
			
			if( bAdd == 2 )
			{
				if( pTalk != NULL ){
					QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
					sFrm = node->v_Nickname + " Talk to you";
					qFrm = QString::fromStdString(sFrm);
					str = std::string(pTalk);
					qMsg = QString::fromStdString(str);
					vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
					//vpnNotificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VpnCoin address or malformed URI parameters."));
				}
			}else if( bAdd == 3 )	// recv file req
			{
				if( pfile != NULL ){	//if( pTalk != NULL ){
					QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));
					sFrm = node->v_Nickname + " Send file to you";
					qFrm = QString::fromStdString(sFrm);
					str = strprintf("%s, %I64u Bytes", pfile, node->v_File_size);
					qMsg = QString::fromStdString(str);
					vpnNotificator->notify(Notificator::Information, qFrm, qMsg);
				}
			}
			//DWORD dPort = node->v_LanId;	//node->addr.port
			
			Bitnet_node_struct vNode;
			vNode.dOpt = bAdd;
			vNode.IpAddr = node->sAddr;
			vNode.LanId = node->v_LanId;
			vNode.dRecvSize = dRecvSize;
			vNode.v_isVpnServer = node->v_isVpnServer;
			vNode.v_iVpnServicePort = node->v_iVpnServicePort;
			vNode.v_bShowInOtherList = node->v_bShowInOtherList;
			vNode.v_bShowWAddrInOtherNodes = node->v_bShowWAddrInOtherNodes;
			vNode.v_iVpnServiceCtrlPort = node->v_iVpnServiceCtrlPort;
			vNode.v_iVpnServiceFee = node->v_iVpnServiceFee;
			vNode.v_iVpnServiceTryMinute = node->v_iVpnServiceTryMinute;
			vNode.v_iTotalVpnConnects = node->v_iTotalVpnConnects;
			vNode.v_iVpnSerCoinConfirms = node->v_iVpnSerCoinConfirms;
			vNode.v_P2P_proxy_port = node->v_P2P_proxy_port;
			vNode.v_sDefWalletAddress = pDefwa;
			vNode.v_sVpnWalletAddress = pVpnwa;
			vNode.v_NicknamePack = pNickNamePk;
			vNode.v_Nickname = pNickName;
			vNode.v_sVpnMemAndCpuInfo = pMemCpu;
			vNode.v_iCanTalk = node->v_iCanTalk;
			vNode.v_iVersion = node->v_iVersion;
			vNode.v_Gui_Node_Index = node->v_Gui_Node_Index;
			vNode.v_Starting_recv = node->v_Starting_recv;
			vNode.v_OpenSocketProxy = node->v_OpenSocketProxy;
			//-- Transaction file
			vNode.v_File_size = node->v_File_size;
			vNode.v_File_Req = pfile;
			vNode.v_File_Loc = pfileLoc;
			vNode.pChat = (PCHAR)pTalk;
			rzt = pVpnSyncNode(node, &vNode);  */
		}
	}
	return rzt;
}
//#endif
