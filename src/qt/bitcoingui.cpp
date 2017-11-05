/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2012
 * The Bitcoin Developers 2011-2012
 */
#include "bitcoingui.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"
#include "sendcoinsdialog.h"
#include "multisigdialog.h"
#include "zkpdialog.h"
#include "signverifymessagedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "rpcconsole.h"
#include "wallet.h"
#include "qpainter.h"
#include "transactionrecord.h"
//#include "CustomFrame.h"
#include "forms/mainframe.h"
#include "bitchainpage.h"

#ifdef USE_BITNET
#include <windows.h>
#include "bitnet.h"
#include <qsystemtrayicon.h>
#include <qsystemtrayicon_p.h>
#endif

#ifdef USE_WEBKIT
#include <QWebPage>
#include <QWebView>
#include <QWebFrame>
#include <QUrl>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include "webPage.h"
#endif

#ifdef WIN32
#include "qwinwidget.h"
#endif

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <fstream>
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
#include <QInputDialog>
#include <QMovie>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>
#include <QStyle>
#include <QSettings>
#include <QDesktopWidget>
#include <QListWidget>
#include <iostream>
//#include <qsystemtrayicon>
#include <QToolButton>
#include <QClipboard>
//#include <QGraphicsDropShadowEffect>

class QSystemTrayIconPrivate;

extern CWallet* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
extern int dw_zip_block;

#ifdef USE_BITNET
extern BitcoinGUI *vpnBitcoinGUI;

extern QWebView *bitNet_chatView1;	// 2015.01.06 add
extern QWebView *bitNet_chatView2;
extern QWebView *bitNet_chatView3;
extern QWinWidget *BitNetWebChatPage;
extern QWinWidget *BitNetWebChatPage2;
extern HWND gBitNetWebChatHwnd1;
extern HWND gBitNetWebChatHwnd2;
extern int g_iWebViewId;

extern lpStart_Vpn_Client pStart_Vpn_Client;
extern lpSendBitNetWebChatMsg SendBitNetWebChatMsg;	// 2015.01.07 add
extern int BitNet_Version;
extern int BitNet_Network_id;
extern lpDropEvent BitNetDropEventFunc;
extern std::string getAddressesbyaccount(const std::string strAccount);

extern std::string sDefWalletAddress;	// 2015.02.06 add

extern std::string s_Cur_Cheque_TxID;	// 2015.02.18 add
extern int64_t i6_Cur_Cheque_Amount;			// 2015.02.18 add

//extern std::string GetAccountByAddress(const char* pAddr);	
DWORD WINAPI SendCoinToAddress(const char* pFromAccount, const char* pToAddress, int64_t nAmount, PCHAR pTxMsg, int bThread, PCHAR pRztTxID, int bEncryptMsg);
#endif

double GetPoSKernelPS();

static const int TITLE_HEIGHT = 30;  
static const int FRAME_BORDER = 2;
//QClipboard* clipBoard = NULL;
QPixmap* pixmapBitNet = 0;
QPixmap* pixmap = 0; 
double dOpacity=0.2;
int bShowBg = 0, bShowNews = 0;
QLabel *currentBlockLabel; 
QTimer *aniIconTmer = NULL;
QTimer *clipTmer = NULL;
AddressBookPage *addressBookPage2 = NULL;
//CustomFrame* aMainFrame = NULL;
QMainWindow * f2MainWindow = NULL;
bool bQssFileExists = false;

#ifdef USE_WEBKIT		//2014.12.19 Bit_Lee
QWebView *supNet_view = NULL;
QWebView *bitNet_view = NULL;
QNetworkCookieJar *cookieJar = NULL;
QNetworkCookieJar *cookieJar2 = NULL;
std::string sBitNetUrl = "";
std::string sSuperNetUrl = ""; 
std::string sNxtAppPath = "";
std::string sBitNetCurDir = "";
QString qBitNetCurDir, qSkinFilePath;
int bSupNetClicked = 0;
int bntGuiID = 1;
bool bBitNetViewOk = false;
bool bSupNetViewOk = false;

QWidget *BitNetNewsPage;
QWebView *BitNetNews_view = NULL;
#endif

// Font sizes - NB! We specifically use 'px' and not 'pt' for all font sizes, as qt scales 'pt' dynamically in a way that makes our fonts unacceptably small on OSX etc. it doesn't do this with px so we use px instead.
QString CURRENCY_DECIMAL_FONT_SIZE = "11px"; // For .00 in currency and text.
QString BODY_FONT_SIZE = "12px"; // Standard body font size used in 'most places'.
QString TitleWidget_FontSize = "15px";
QString TitleButton_Width = "30px";
QString TitleButton_Height = "20px";
QString ContextMenuFontSize = "12px";
QString QToolButtonHeight = "66px";
QString CURRENCY_FONT_SIZE = "13px"; // For currency
QString TOTAL_FONT_SIZE = "15px"; // For totals and account names
QString HeadingLabFontSize = "16px"; // For large headings
QString HeadingLabHeight = "35px";
QString EditWidgetHeight = "20px";
QString QToolBarFontSize = "13px";
QString QPushButtonHeight = "35px";
QString QScrollBarWidth = "8px";
QString QWebView_FontSize = "12px";
QString OverviewLabFontSize = "13px";
QString QValueComboBoxWidth = "24px";
extern double DOB_DPI_RAT;
int bWebAdm = 0;

// Fonts
// We 'abuse' the translation system here to allow different 'font stacks' for different languages.
QString MAIN_FONTSTACK = QObject::tr("Arial, 'Helvetica Neue', Helvetica, sans-serif"); 

string GetDefaultWalletAddress1()
{
	string rzt = "";
	if( pwalletMain )
	{	
#ifdef USE_BITNET	
		rzt = getAddressesbyaccount("BitNet-nXt");	//-- 2014.12.19   Bit_Lee
#endif
		if( rzt.length() == 0 )
			rzt = CBitcoinAddress(pwalletMain->vchDefaultKey.GetID()).ToString();
	}
	return rzt;
}

#ifdef WIN32
std::string string_To_UTF8(const std::string & str)  
{  
int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);  
  
wchar_t * pwBuf = new wchar_t[nwLen + 1];	//一定要加1，不然会出现尾巴  
ZeroMemory(pwBuf, nwLen * 2 + 2);  
  
::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);  
  
int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);  
//OutputDebugStringW( pwBuf );
  
char * pBuf = new char[nLen + 1];  
ZeroMemory(pBuf, nLen + 1);  
  
::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);  
//OutputDebugStringA( pBuf );
std::string retStr(pBuf);  
//printf("string_To_UTF8 [%s] [%s] [%s] \n", str.c_str(), retStr.c_str(), pBuf);
  
delete []pwBuf;
delete []pBuf;  
  
pwBuf = NULL;  
pBuf = NULL;  
  
return retStr;  
}
#endif

#ifdef USE_WEBKIT		//2014.12.19 Bit_Lee
void SetWebViewOpt(QWebView *aWebView, bool bNoContextMenu, bool bCookie)
{
	if( aWebView != NULL )
	{		
		if( (bWebAdm == 0) && bNoContextMenu ){ aWebView->setContextMenuPolicy(Qt::NoContextMenu); }
		//aWebView->setStatusTip(tr("Press F5 to refresh"));		
				
		//aWebView->setAttribute(Qt::WA_TranslucentBackground, true);
		aWebView->setVisible(true);		
		
		if( bCookie ){
			cookieJar2 = new QNetworkCookieJar();
			QNetworkAccessManager *nam2 = new QNetworkAccessManager();
			nam2->setCookieJar(cookieJar2);
			aWebView->page()->setNetworkAccessManager(nam2);
		}
			
		aWebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        aWebView->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, false);
		aWebView->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
		aWebView->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
		aWebView->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
#ifdef WIN32
		//aWebView->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
#endif
		aWebView->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
		aWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true); 	
		aWebView->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));		
		//aWebView->installEventFilter(this);
		//connect(aWebView, SIGNAL( linkClicked(QUrl) ), this, SLOT( handleWebviewClicked(QUrl) ));
		//aWebView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
		//connect(aWebView, SIGNAL(loadFinished(bool)), SLOT(BitNet_finishLoading(bool))); 
		if( bWebAdm > 0 ){ aWebView->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); }
	}
}
#endif

QPixmap setDevicePixelRatio(QPixmap orig, double x, Qt::AspectRatioMode arOpt = Qt::KeepAspectRatio)
{
    if( x > 1 )
    {
        int w = orig.width(), h = orig.height();
        w = w * x;   h = h * x;
        return orig.scaled(w, h, arOpt, Qt::SmoothTransformation);  //Qt::IgnoreAspectRatio, KeepAspectRatio
	}
    return orig;
}
QPixmap getPixelRatioPixmap(QString icon, Qt::AspectRatioMode arOpt = Qt::KeepAspectRatio)
{
    QPixmap orig(icon);  
	return setDevicePixelRatio(orig, DOB_DPI_RAT, arOpt);
	/* int w = orig.width(), h = orig.height();
    if( DOB_DPI_RAT > 1 )
    {
        w = w * DOB_DPI_RAT;   h = h * DOB_DPI_RAT;
        return orig.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);	//orig.setDevicePixelRatio( DOB_DPI_RAT );  
	}
    return orig; */
}
MainFrame* fMainFrame = NULL;      ZkpForm* gZkpForm=NULL;
QString qScrollBarStyleStr;
BitcoinGUI::BitcoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0),
    nWeight(0)
{
    /* === QScrollBar:vertical === */
    qScrollBarStyleStr = "QScrollBar:vertical{ width:QScrollBarWidth; background:rgba(0,0,0,0%); margin:0px,0px,0px,0px; padding-top:9px; padding-bottom:9px; }"
                                     "QScrollBar::handle:vertical{ width:QScrollBarWidth; background:rgba(0,0,0,25%); border-radius:4px; min-height:20; }"
                                     "QScrollBar::handle:vertical:hover{ width:QScrollBarWidth; background:rgba(0,0,0,50%); border-radius:4px; min-height:20; }"
                                     "QScrollBar::add-line:vertical{ height:9px; width:QScrollBarWidth; border-image:url(:/icons/scroll_bar_3); subcontrol-position:bottom; }"
                                     "QScrollBar::sub-line:vertical{ height:9px; width:QScrollBarWidth; border-image:url(:/icons/scroll_bar_1); subcontrol-position:top; }"
                                     "QScrollBar::add-line:vertical:hover{ height:9px; width:QScrollBarWidth; border-image:url(:/icons/scroll_bar_4); subcontrol-position:bottom; }"
                                     "QScrollBar::sub-line:vertical:hover{ height:9px; width:QScrollBarWidth; border-image:url(:/icons/scroll_bar_2); subcontrol-position:top; }"
                                     "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{ background:rgba(0,0,0,10%); border-radius:4px; }"
    /* === QScrollBar:horizontal === */
                                     "QScrollBar:horizontal { background: #111111; height: 16px; margin: 0 16px 0 16px; }"
                                     "QScrollBar::handle:horizontal { background: #555555; min-width: 16px; }"
                                     "QScrollBar::add-line:horizontal { background: #444444; width: 16px; subcontrol-position: right; subcontrol-origin: margin; }"
                                     "QScrollBar::sub-line:horizontal { background: #444444; width: 16px; subcontrol-position: left; subcontrol-origin: margin; }"
                                     "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }"
                                     "QScrollBar:left-arrow:horizontal, QScrollBar:right-arrow:horizontal { border: 2px solid #333333; width: 6px; height: 6px; background: #5f5f5f; }";
    appMenuBar = NULL;
#ifdef WIN32
    //if( lang_territory.indexOf("zh_CN") != -1 )
	{ MAIN_FONTSTACK = '"Microsoft YaHei", Arial, Helvetica, sans-serif, "宋体" !important'; }
	//OutputDebugStringA("--> BitcoinGUI");
	int dpiX = GetDeviceCaps(GetDC(0), LOGPIXELSX);  //LOGPIXELSX = 88,   96, 120, 144
	double dx = (double)dpiX / 96;
	DOB_DPI_RAT = dx;
	//printf( "DPI = %d, dx = %f \n", dpiX, dx);
    if( dx > 1 )
	{
		CURRENCY_DECIMAL_FONT_SIZE = QString::number(dx * 11) + "px";
		BODY_FONT_SIZE = QString::number(dx * 13, 'f', 0) + "px";
		TitleWidget_FontSize = QString::number(dx * 15, 'f', 0) + "px";
		TitleButton_Width = QString::number(dx * 30, 'f', 0) + "px";
		TitleButton_Height = QString::number(dx * 20, 'f', 0) + "px";
		ContextMenuFontSize = QString::number(dx * 12, 'f', 0) + "px";
        QToolButtonHeight = QString::number(dx * 66, 'f', 0) + "px";
		HeadingLabFontSize = QString::number(dx * 16, 'f', 0) + "px";
		HeadingLabHeight = QString::number(dx * 35, 'f', 0) + "px";
		EditWidgetHeight =  QString::number(dx * 20, 'f', 0) + "px";
        QToolBarFontSize =  QString::number(dx * 15, 'f', 0) + "px";
		QPushButtonHeight =  QString::number(dx * 35, 'f', 0) + "px";
		QScrollBarWidth =  QString::number(dx * 8, 'f', 0) + "px";
		QWebView_FontSize =  QString::number(dx * 13, 'f', 0) + "px";
		OverviewLabFontSize =  QString::number(dx * 13, 'f', 0) + "px";
		QValueComboBoxWidth =  QString::number(dx * 24, 'f', 0) + "px";
	}
	/*HMODULE m = LoadLibraryA("Shcore.dll");
	if( m > 1000 )
	{
		lpSetProcessDpiAwareness SetProcessDpiAwareness = (lpSetProcessDpiAwareness)GetProcAddress(m, "SetProcessDpiAwareness");
		if( SetProcessDpiAwareness != NULL ){ SetProcessDpiAwareness(2); }  // PROCESS_SYSTEM_DPI_AWARE = 1,  PROCESS_PER_MONITOR_DPI_AWARE = 2
		FreeLibrary(m);
	}*/
#endif
	//bShowBg = GetArg("-showbg", 1);
    bShowNews = GetArg("-shownews", 1);
	char ba[9];
	ba[0] = '-'; ba[1] = 'b'; ba[2] = 'e'; ba[3] = 'w'; ba[4] = 'a'; ba[5] = 'd'; ba[6] = 'm'; ba[7] = 0;
	string s = ba;	//"-bewadm"
	bWebAdm = GetArg(s.c_str(), 0);	
	//resize(850, 550);
	restoreWindowGeometry();
    //setWindowTitle(tr("VpnCoin Test") + " - " + tr("Wallet"));
    QString qTitle = tr("VpnCoin") + " - " + tr("Wallet") + " - 1.7.11.3";
	setWindowTitle(qTitle);
	fMainFrame = new MainFrame();
    //f2MainWindow =  fMainFrame->getToolBarWin();   
	f2MainWindow = new QMainWindow();
	
    /* QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);    
	shadow->setBlurRadius(10.0);    
	shadow->setColor(QColor(0, 0, 0, 100));   //setColor(Qt::red);    
	shadow->setOffset(0);    
	this->setGraphicsEffect(shadow); */

//string s = strprintf("--> BitcoinGUI 2 %x", f2MainWindow);
//OutputDebugStringA(s.c_str());

#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    setWindowIcon(QIcon(":icons/bitcoin"));
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

    if( !GetArg("-baseui", 0) )
    {
    //this->setWindowFlags(Qt::FramelessWindowHint);  
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint );
    this->setMouseTracking(true);  
  
    isMax_ = false;  
    isPress_ = false;  
    isLeftPressDown = false;
    this->dir = NONE;
	fMainFrame->mainGui = this;

    QLabel *logoLabel = new QLabel();  
    //QPixmap logoPixmap = this->style()->standardPixmap(QStyle::SP_TitleBarMenuButton);  
    QPixmap* logoPixmap = new QPixmap(":icons/bitcoin");
    logoLabel->setPixmap(*logoPixmap);  
    logoLabel->setFixedSize(16 * DOB_DPI_RAT, 16 * DOB_DPI_RAT);  
    logoLabel->setScaledContents(true);
  
    titleLabel = new QLabel();      //QString qTitle = tr("VpnCoin") + " - " + tr("Wallet");
	//if( fTestNet ){ qTitle = qTitle + QString(" ") + tr("[testnet]"); }
    titleLabel->setText( qTitle );
    QFont titleFont = titleLabel->font();  
    titleFont.setBold(true);  
    titleLabel->setFont(titleFont);  
    titleLabel->setObjectName("whiteLabel");  
  
    QToolButton *minButton = new QToolButton();  
    QPixmap minPixmap = this->style()->standardPixmap(QStyle::SP_TitleBarMinButton);  
    if( DOB_DPI_RAT > 0 ) minPixmap = setDevicePixelRatio(minPixmap, DOB_DPI_RAT);  //minPixmap.setDevicePixelRatio( DOB_DPI_RAT ); 
    minButton->setIcon(minPixmap);  
    connect(minButton, SIGNAL(clicked()), this, SLOT(slotShowSmall()));  
  
    maxButton_ = new QToolButton();  
    maxPixmap_ = this->style()->standardPixmap(QStyle::SP_TitleBarMaxButton);  
    if( DOB_DPI_RAT > 0 ) maxPixmap_ = setDevicePixelRatio(maxPixmap_, DOB_DPI_RAT);   //maxPixmap_.setDevicePixelRatio( DOB_DPI_RAT ); 
    restorePixmap_ = this->style()->standardPixmap(QStyle::SP_TitleBarNormalButton);  
    if( DOB_DPI_RAT > 0 ) restorePixmap_ = setDevicePixelRatio(restorePixmap_, DOB_DPI_RAT);  //restorePixmap_.setDevicePixelRatio( DOB_DPI_RAT ); 
    maxButton_->setIcon(maxPixmap_);  
    connect(maxButton_, SIGNAL(clicked()), this, SLOT(slotShowMaxRestore()));  
  
    QToolButton *closeButton = new QToolButton();  
    QPixmap closePixmap = this->style()->standardPixmap(QStyle::SP_TitleBarCloseButton);  
	if( DOB_DPI_RAT > 0 ) closePixmap = setDevicePixelRatio(closePixmap, DOB_DPI_RAT);
    closeButton->setIcon(closePixmap);  
	//closeButton->setText("x");
#ifndef Q_OS_MAC
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close())); 
#else
    connect(closeButton, SIGNAL(clicked()), qApp, SLOT(quit())); 
#endif
  
    QHBoxLayout *titleLayout = new QHBoxLayout();  
    titleLayout->addWidget(logoLabel);  
    titleLayout->addWidget(titleLabel);  
    titleLabel->setContentsMargins(5, 0, 0, 0);  
    titleLayout->addStretch();  
    titleLayout->addWidget(minButton, 0, Qt::AlignTop);  
    titleLayout->addWidget(maxButton_, 0, Qt::AlignTop);  
    titleLayout->addWidget(closeButton, 0, Qt::AlignTop);  
    titleLayout->setSpacing(0);  
    titleLayout->setContentsMargins(5, 0, 0, 0);  
      
    QWidget *titleWidget = fMainFrame->getTitleWidget();  //new QWidget();  
    titleWidget->setLayout(titleLayout);  
    titleWidget->installEventFilter(0);  
    
	//this->layout()->addWidget(titleWidget);
	//this->layout()->addWidget(f2MainWindow);
	//f2MainWindow->setVisible(true);
    }

/*
    //QVBoxLayout *mainLayout = new QVBoxLayout();  
    QHBoxLayout *mainLayout = new QHBoxLayout();  
    mainLayout->addWidget(titleWidget);  
    mainLayout->addWidget(f2MainWindow);  
    mainLayout->setSpacing(0);  
    mainLayout->setMargin(0);   // 5
    this->setLayout(mainLayout);  
*/

    sBitNetCurDir = "";   qBitNetCurDir = "";
    boost::filesystem::path pcu = boost::filesystem::current_path();
    sBitNetCurDir = strprintf("%s//", pcu.string().c_str()); // + "\\BlockChain\\AdBonus\\";

    /* char buf[512];
    memset(buf, 0, sizeof(buf)); 			
    if( GetCurrentDirectoryA(MAX_PATH, &buf[0]) ) */
    {
	//sBitNetCurDir = strprintf("%s\\", buf);
	std::replace( sBitNetCurDir.begin(), sBitNetCurDir.end(), '\\', '\x2f'); // replace all 'x' to 'y'
#ifdef WIN32
	string s2 = string_To_UTF8(sBitNetCurDir);
	qBitNetCurDir = QString::fromUtf8(s2.c_str());
#else
	qBitNetCurDir = QString::fromStdString(sBitNetCurDir.c_str());
#endif
	qSkinFilePath = qBitNetCurDir + "Res/ui/bitnet.qss";
    }

    ifstream t;
	t.open( qSkinFilePath.toStdString().c_str() ); 
	if( t.is_open() )
	{
		t.close();   bQssFileExists = true;
	}

    // We always want menu icons to be visible on all platforms as this is *not* a native application but a skinned UI.
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);

    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create the toolbars
    createToolBars();

    // Create application menu bar
    createMenuBar();

	//if( bShowBg )
	//{ appMenuBar->setAttribute(Qt::WA_TranslucentBackground, true); }
	
    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

	zkpForm = new ZkpForm(this);      gZkpForm = zkpForm;      // 2017.03.06 add
    // Create tabs
    overviewPage = new OverviewPage();

    transactionsPage = new QWidget(this);  // this
    QVBoxLayout *vbox = new QVBoxLayout();
    transactionView = new TransactionView(this);  // this
    vbox->addWidget(transactionView);
    transactionsPage->setLayout(vbox);
	

    BitNetPage = new QWidget(this);	// 2014.11.05 add  this
    bitchainPage = new BitchainPage(0);   // 2016.03.01 add  this
    multisigPage = new MultisigDialog(0);
	
	if( bShowNews )
	{
	BitNetNewsPage = new QWidget(this);	// 2015.04.13 add  // this
	BitNetNewsPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	BitNetNewsPage->setMaximumHeight(20);
	BitNetNewsPage->setAttribute(Qt::WA_TranslucentBackground);
	BitNetNews_view = new QWebView(BitNetNewsPage); 
	BitNetNews_view->setVisible(true);		
	webPage* page = new webPage(BitNetNewsPage);
	BitNetNews_view->setPage(page);
	//connect(page, SIGNAL(loadLink(QUrl)), this, SLOT(linkUrl(QUrl)));
	connect(page, SIGNAL(openLink(QUrl)), this, SLOT(openUrl(QUrl)));
		
	//BitNetNews_view->setContextMenuPolicy(Qt::NoContextMenu);	
    BitNetNewsPage->setContentsMargins(0,0,0,0);
				
	//SupNetPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	BitNetNews_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	BitNetNews_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
	BitNetNews_view->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
	BitNetNews_view->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	//BitNetNews_view->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
	//BitNetNews_view->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
	//BitNetNews_view->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true); 	
	//BitNetNews_view->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));	
	BitNetNews_view->load(QUrl("http://coin.bitnet.cc/news.php?ver=1703"));	//-- 2014.12.19   Bit_Lee
	BitNetNewsPage->setLayout(new QGridLayout);
	BitNetNewsPage->layout()->addWidget(BitNetNews_view);
    BitNetNewsPage->layout()->setContentsMargins(0,0,0,0);
    BitNetNews_view->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
	}
    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);
	addressBookPage2 = addressBookPage;  // 2015.06.29 add

    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);

    sendCoinsPage = new SendCoinsDialog(this);  // this

    signVerifyMessageDialog = new SignVerifyMessageDialog(this);  // this

    centralWidget = new QStackedWidget(this);  // this
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(BitNetPage);	// 2014.11.05 add
    centralWidget->addWidget(bitchainPage);	// 2016.03.01 add
    centralWidget->addWidget(multisigPage);	// 2016.05.16 add
    centralWidget->addWidget(zkpForm);	// 2017.03.3 add
	//centralWidget->addWidget(BitNetNewsPage);	// 2014.04.13 add

    /*QHBoxLayout *main2Layout = new QHBoxLayout();  
    main2Layout->addWidget(centralWidget);  
    main2Layout->setSpacing(0);  
    main2Layout->setMargin(0);  
    main2Layout->setContentsMargins(0, 0, 0, 0); 
    f2MainWindow->setLayout(main2Layout);  */
	//f2MainWindow->layout()->addWidget(centralWidget);
	

#ifdef USE_WEBKIT
	BitNetWebPage = new QWidget(this);	// 2014.12.17 add  // this
	BitNetWebPage->setAttribute(Qt::WA_TranslucentBackground);
	centralWidget->addWidget(BitNetWebPage);	
	
	SupNetPage = new QWidget(this);	// 2014.12.17 add  // this
	SupNetPage->setAttribute(Qt::WA_TranslucentBackground);
	centralWidget->addWidget(SupNetPage);
	sSuperNetUrl = "";
	sBitNetUrl = "";	//strprintf("http://www.bitnet.cc//wallet/?coin=", BitcoinUnits::name(BitcoinUnits::BTC).toStdString().c_str());
	
    if( !GetArg("-baseui", 0) )
    {
        if( bQssFileExists ){ loadQss(qSkinFilePath); }
        else loadQss(":qss/main");  //loadQss(qSkinFilePath);
    }
	//printf("BitcoinGUI --> %s\n", sBitNetCurDir.c_str());	
	sNxtAppPath = GetArg("-sprnetapp", "supernet\\run.bat"); 
	int i = sNxtAppPath.length();
	//printf("%u sNxtAppPath 0 = %c, 1 = %c, 2 = %c\n", i, sNxtAppPath[0], sNxtAppPath[1], sNxtAppPath[2]); 
	if( (i == 0) || ( (i > 1) && (sNxtAppPath[1] != ':') ) )
	{
		//printf("sNxtAppPath 0 = %c, 1 = %c, 2 = %c\n", sNxtAppPath[0], sNxtAppPath[1], sNxtAppPath[2]); 
		//string sApp = GetArg("-sprnetapp", "superNet\\run.bat"); 
		string s = strprintf("%s%s", sBitNetCurDir.c_str(), sNxtAppPath.c_str());	//strprintf("%s\\superNet\\run.bat ", buf);
		sNxtAppPath	= s.c_str();
	}	

	//printf("sBitNetCurDir = %s\n", sBitNetCurDir.c_str()); 
	//printf("sNxtAppPath = %s, %d\n", sNxtAppPath.c_str(), i); 
	
	if( supNet_view == NULL ){ supNet_view = new QWebView(SupNetPage); }
	{	
		//supNet_view = new QWebView(SupNetPage);
		SupNetPage->setLayout(new QGridLayout);
		SupNetPage->layout()->addWidget(supNet_view);
		//supNet_view->page()->action(QWebPage::NavigationTypeBackOrForward)->setVisible(false);	//NavigationTypeReload
		
		//supNet_view->setContextMenuPolicy(Qt::NoContextMenu);	
		supNet_view->setStatusTip(tr("Press F5 to refresh"));		
		
		/*QPalette palette = supNet_view->palette();
		//palette.setBrush(QPalette::Base, Qt::transparent);
		palette.setBrush(QPalette::Window, Qt::transparent);
		supNet_view->page()->setPalette(palette);
		supNet_view->setAttribute(Qt::WA_OpaquePaintEvent, false); */
		
		//supNet_view->setAttribute(Qt::WA_TranslucentBackground, true);
		supNet_view->setVisible(true);		
		
		cookieJar = new QNetworkCookieJar();
		QNetworkAccessManager *nam = new QNetworkAccessManager();
		nam->setCookieJar(cookieJar);
		supNet_view->page()->setNetworkAccessManager(nam);		
			
		//SupNetPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		supNet_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		supNet_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
		supNet_view->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
		supNet_view->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
		//supNet_view->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
		supNet_view->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
		supNet_view->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true); 	
		supNet_view->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
		//QWebSettings::globalSettings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));

		if( bWebAdm ){	supNet_view->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true); }		
		
		supNet_view->installEventFilter(this);
		
		//supNet_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
		connect(supNet_view, SIGNAL(loadFinished(bool)), SLOT(SupNet_finishLoading(bool)));
        supNet_view->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
		
		/* QKeySequence keys_refresh(Qt::Key_F5);
		//QAction *action_refresh = new QAction(QIcon("Res/refresh.png"), tr("Refresh"), supNet_view);
		QAction *action_refresh = new QAction(supNet_view);
		//action_refresh->setStatusTip(tr("Refresh"));
		//action_refresh->setToolTip(tr("Refresh"));
		action_refresh->setShortcut(keys_refresh);
		supNet_view->addAction(action_refresh);
		//connect(action_refresh, SIGNAL(triggered()), supNet_view, SLOT(reload()));
		connect(action_refresh, SIGNAL(triggered()), this, SLOT(supNetF5Clicked())); */
		
		supNet_view->load(QUrl("http://nxt.bitnet.cc"));	//-- 2014.12.19   Bit_Lee
	}
	
	if( bitNet_view == NULL ){ bitNet_view = new QWebView(BitNetWebPage); }
	{
		//bitNet_view = new QWebView(BitNetWebPage);
		BitNetWebPage->setLayout(new QGridLayout);
		BitNetWebPage->layout()->addWidget(bitNet_view);
		
		webPage* page = new webPage(BitNetWebPage);
		bitNet_view->setPage(page);
		connect(page, SIGNAL(loadLink(QUrl)), this, SLOT(linkUrl(QUrl)));
		connect(page, SIGNAL(openLink(QUrl)), this, SLOT(openUrl(QUrl)));
		
		bitNet_view->setContextMenuPolicy(Qt::NoContextMenu);	
		bitNet_view->setStatusTip(tr("Press F5 to refresh"));		
				
		//bitNet_view->setAttribute(Qt::WA_TranslucentBackground, true);
		bitNet_view->setVisible(true);		
		
		cookieJar2 = new QNetworkCookieJar();
		QNetworkAccessManager *nam2 = new QNetworkAccessManager();
		nam2->setCookieJar(cookieJar2);
		bitNet_view->page()->setNetworkAccessManager(nam2);		
			
		//SupNetPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		bitNet_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		bitNet_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
		bitNet_view->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
		bitNet_view->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
		//bitNet_view->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
		bitNet_view->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
		bitNet_view->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true); 	
		bitNet_view->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));		
		bitNet_view->installEventFilter(this);
		//connect(bitNet_view, SIGNAL( linkClicked(QUrl) ), this, SLOT( handleWebviewClicked(QUrl) ));
		//bitNet_view->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
		connect(bitNet_view, SIGNAL(loadFinished(bool)), SLOT(BitNet_finishLoading(bool)));
        bitNet_view->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );
		
		/* QKeySequence keys_refresh2(Qt::Key_F5);
		QAction *action_refresh2 = new QAction(bitNet_view);
		action_refresh2->setShortcut(keys_refresh2);
		bitNet_view->addAction(action_refresh2);
		connect(action_refresh2, SIGNAL(triggered()), this, SLOT(bitNetF5Clicked())); */
		
		//bitNet_view->load(QUrl("http://nxt.bitnet.cc"));		
	}
	
/*	BitNetWebChatPage = new QWinWidget(this);	// 2015.01.06 add
	BitNetWebChatPage->setAttribute(Qt::WA_TranslucentBackground);
	centralWidget->addWidget(BitNetWebChatPage);
	if( bitNet_chatView1 == NULL ){ bitNet_chatView1 = new QWebView(BitNetWebChatPage); }
	{
		//bitNet_chatView1 = new QWebView(BitNetWebPage);
		
		//BitNetWebChatPage->setLayout(new QGridLayout);
		//BitNetWebChatPage->layout()->addWidget(bitNet_chatView1);
		
		webPage* page = new webPage(BitNetWebChatPage);
		bitNet_chatView1->setPage(page);
		connect(page, SIGNAL(loadLink(QUrl)), this, SLOT(linkUrl(QUrl)));
		connect(page, SIGNAL(openLink(QUrl)), this, SLOT(openUrl(QUrl)));
		
		SetWebViewOpt(bitNet_chatView1, false, true);
		bitNet_chatView1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		gBitNetWebChatHwnd1 = BitNetWebChatPage->winId();	//bitNet_chatView1->winId();
		// make the widget window style be WS_CHILD so SetParent will work
        SetWindowLongA(gBitNetWebChatHwnd1, GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        QEvent e(QEvent::EmbeddingControl);
        QApplication::sendEvent(BitNetWebChatPage, &e);		*/
/*		
		bitNet_chatView1->setContextMenuPolicy(Qt::NoContextMenu);	
		bitNet_chatView1->setStatusTip(tr("Press F5 to refresh"));		
				
		//bitNet_chatView1->setAttribute(Qt::WA_TranslucentBackground, true);
		bitNet_chatView1->setVisible(true);		
		
		cookieJar2 = new QNetworkCookieJar();
		QNetworkAccessManager *nam2 = new QNetworkAccessManager();
		nam2->setCookieJar(cookieJar2);
		bitNet_chatView1->page()->setNetworkAccessManager(nam2);		
			
		//SupNetPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		bitNet_chatView1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		bitNet_chatView1->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
		bitNet_chatView1->page()->settings()->setAttribute(QWebSettings::JavaEnabled, true);
		bitNet_chatView1->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
		bitNet_chatView1->settings()->setAttribute(QWebSettings::WebSecurityEnabled, false); 
		bitNet_chatView1->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true); 
		bitNet_chatView1->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true); 	
		bitNet_chatView1->settings()->setOfflineStoragePath(QDesktopServices::storageLocation(QDesktopServices::DataLocation));		
		bitNet_chatView1->installEventFilter(this);
		//connect(bitNet_chatView1, SIGNAL( linkClicked(QUrl) ), this, SLOT( handleWebviewClicked(QUrl) ));
		//bitNet_chatView1->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);	//Handle link clicks by yourself 
		connect(bitNet_chatView1, SIGNAL(loadFinished(bool)), SLOT(BitNet_finishLoading(bool))); 
		//bitNet_view->load(QUrl("http://nxt.bitnet.cc"));		
*/
	//}	
#endif
	
    //setCentralWidget(centralWidget);  //centralWidget   f2MainWindow
	//f2MainWindow->setCentralWidget(centralWidget);
	
    centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    /*QMyFrame *frameMM = fMainFrame->getMainFrame();   //new QMyFrame();  //QFrame
    frameMM->setContentsMargins(0,0,0,0);
    frameMM->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QVBoxLayout *frameMMLayout = new QVBoxLayout(frameMM);
    frameMMLayout->setContentsMargins(0,0,0,0);
    frameMMLayout->setSpacing(1);
    frameMMLayout->addWidget(titleWidget, 0);
    frameMMLayout->addWidget(f2MainWindow, 0);
    frameMMLayout->addWidget(centralWidget, 1);
	setCentralWidget(frameMM); */
	
    QHBoxLayout *aLayout = new QHBoxLayout(fMainFrame->getMainFrame());
    aLayout->setContentsMargins(0, 0, 0, 0);
	aLayout->addWidget(centralWidget, 1);
	
    QHBoxLayout *toolBarLyt = new QHBoxLayout(fMainFrame->getToolBarWin());  
    toolBarLyt->addWidget(f2MainWindow);  
    toolBarLyt->setSpacing(0);  
    toolBarLyt->setMargin(0);  
    toolBarLyt->setContentsMargins(0, 0, 0, 0); 
//fMainFrame->getMainFrame()->addWidget(centralWidget, 1)
	setCentralWidget(fMainFrame);

	//frameMM->setStyleSheet("QMyFrame{background:transparent; border:3px solid black; border-radius:3px;}");	

    // Create status bar
    statusBar();


    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setMinimumWidth(218);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    labelEncryptionIcon = new QLabel();
    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
	currentBlockLabel = new QLabel();
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(currentBlockLabel);
    frameBlocksLayout->addStretch();
    //if( dw_zip_block > 0 )
    {
        blockchaincompressIcon = new QLabel();
        QPixmap* bcaPix = new QPixmap(":icons/blockchaincompress");
        blockchaincompressIcon->setPixmap(*bcaPix);  
        blockchaincompressIcon->setFixedSize(16 * DOB_DPI_RAT, 16 * DOB_DPI_RAT);  
        blockchaincompressIcon->setScaledContents(true);
        blockchaincompressIcon->setToolTip(tr("Blockchain compression has been activated"));
        frameBlocksLayout->addWidget(blockchaincompressIcon);
        frameBlocksLayout->addStretch();
        blockchaincompressIcon->hide();
    }
    frameBlocksLayout->addWidget(labelEncryptionIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);
	

    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = qApp->style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #FF8000, stop: 1 orange); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);
	if( bShowNews ){ statusBar()->addWidget(BitNetNewsPage); }	// 2015.04.13 add
    //statusBar()->addWidget(currentBlockLabel);
	//BitNetNewsPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//statusBar()->setStyleSheet("QWebView{background:transparent;} QFrame{background:transparent;}");	
	
	//if( bShowBg )
	{ statusBar()->setAttribute(Qt::WA_TranslucentBackground, true); }

    syncIconMovie = new QMovie(":/movies/update_spinner", "mng", this);
    syncIconMovie->setScaledSize(QSize(16 * DOB_DPI_RAT, 16 * DOB_DPI_RAT));

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));
    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

	gotoOverviewPage();	//gotoSendCoinsPage();

	if( bShowBg )
	{
		dOpacity = GetArg("-bgopacity", 12) * 0.01;
		this->setStyleSheet("menuBar{background:transparent;} QMenuBar{background:transparent;}");	//-- 2014.12.19   Bit_Lee
		//centralWidget->setStyleSheet("OverviewPage{background:transparent;} QStackedWidget{background:transparent;} QFrame{background:transparent;} menuBar{background:transparent;} QMenuBar{background:transparent;} SendCoinsDialog{background:transparent;}");	
		centralWidget->setStyleSheet("OverviewPage{background:transparent;} QStackedWidget{background:transparent;} QFrame{background:transparent;} menuBar{background:transparent;} QMenuBar{background:transparent;}");	
		
		std::string sLab = GetArg("-vpnbg", "Res/vpncoin.png");
		//printf("vpnbg=%s\n", sLab.c_str());
		QString qImg = QString::fromStdString(sLab);
		pixmapBitNet = new QPixmap(qImg);
	}

#ifdef USE_BITNET
	vpnBitcoinGUI = this;
	gCoinGuiHwnd = this->winId();
	gBitNetPageHwnd = BitNetPage->winId();
#endif
#ifdef WIN32
	//OutputDebugStringA("<-- BitcoinGUI");
#endif

    //aMainFrame = new CustomFrame(this, tr("VpnCoin") + " - " + tr("Wallet"));   //CustomFrame* frame(this, tr("VpnCoin") + " - " + tr("Wallet"));  
    //aMainFrame->setWindowTitle(tr("VpnCoin") + " - " + tr("Wallet"));
    //aMainFrame->show();  
	
	//clipBoard = QApplication::clipboard();
	//QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(processClipboardChange()));

    clipTmer = new QTimer(this);
    clipTmer->setInterval(500);
    clipTmer->start();
    connect(clipTmer, SIGNAL(timeout()), this, SLOT(clipTimer()));
}

BitcoinGUI::~BitcoinGUI()
{
    if( bShowBg )
	{
		delete pixmapBitNet;
		if( pixmap != NULL ) delete pixmap;
	}
	if( (aniIconTmer != NULL) && aniIconTmer->isActive() ){ aniIconTmer->stop(); }
	if( aniIconTmer != NULL ){ delete aniIconTmer; }
	if( (clipTmer != NULL) && clipTmer->isActive() ){ clipTmer->stop(); }
	if( clipTmer != NULL ){ delete clipTmer; }
    //delete multisigPage;
	saveWindowGeometry();
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
    if( appMenuBar != NULL ) delete appMenuBar;
}

void BitcoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);
    overviewAction->setToolTip(tr("Show general overview of wallet"));
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    bitchainAction = new QAction(QIcon(":/icons/bitChainTab"), tr("&BitChain"), this);
    bitchainAction->setToolTip(tr("Diversified blockchain applications"));
    bitchainAction->setCheckable(true);
    //bitchainAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(bitchainAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("&Send coins"), this);
    sendCoinsAction->setToolTip(tr("Send coins to a VpnCoin address"));
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Receive coins"), this);
    receiveCoinsAction->setToolTip(tr("Show the list of addresses for receiving payments"));
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(QIcon(":/icons/history"), tr("&Transactions"), this);
    historyAction->setToolTip(tr("Browse transaction history"));
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    addressBookAction = new QAction(QIcon(":/icons/address-book"), tr("&Address Book"), this);
    addressBookAction->setToolTip(tr("Edit the list of stored addresses and labels"));
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(bitchainAction, SIGNAL(triggered()), this, SLOT(gotoBitchainPage()));

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/bitcoin"), tr("&About VpnCoin"), this);
    aboutAction->setToolTip(tr("Show information about VpnCoin"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for VpnCoin"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Show / Hide"), this);
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    restoreWalletAction = new QAction(QIcon(":/icons/tx_input"), tr("Restore Wallet"), this);
    //restoreWalletAction->setToolTip(tr("Backup wallet to another location"));
    setChainFolderAction = new QAction(QIcon(":/icons/blockchain_folder"), tr("Set Blockchain Folder"), this);
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);

    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));
#ifdef WIN32
    //bitnetAction = new QAction(QIcon(":/icons/vpncenter"), tr("&BitNet"), this);
    bitnetAction = new QAction(QIcon("Res/Chat.png"), tr("&P2P Chat"), this);
    //bitnetAction->setStatusTip(tr("Enter BitNet"));
    bitnetAction->setToolTip(tr("Enter BitNet"));
    bitnetAction->setCheckable(true);	
    connect(bitnetAction, SIGNAL(triggered()), this, SLOT(gotoBitNetClicked()));	
    tabGroup->addAction(bitnetAction);
#endif
	
    vpnAction = new QAction(QIcon("Res/GotoVpn.png"), tr("VPN Dial"), this);
    vpnAction->setToolTip(tr("VPN Dial"));
    vpnAction->setCheckable(true);	
    connect(vpnAction, SIGNAL(triggered()), this, SLOT(gotoVpnClicked()));	
    tabGroup->addAction(vpnAction);
	
    chequeAction = new QAction(QIcon("Res/GotoCheque.png"), tr("Cheque"), this);
    //chequeAction->setToolTip(tr("BitNet Cheque"));
    chequeAction->setCheckable(true);	
	connect(chequeAction, SIGNAL(triggered()), this, SLOT(gotoChequeClicked()));	
    tabGroup->addAction(chequeAction);
#ifdef WIN32
    bonusAction = new QAction(QIcon("Res/GotoBonus.png"), tr("Ad Bonus"), this);
    //bonusAction->setToolTip(tr("BitNet Bonus"));
    bonusAction->setCheckable(true);	
    connect(bonusAction, SIGNAL(triggered()), this, SLOT(gotoBonusClicked()));	
    tabGroup->addAction(bonusAction);
#endif
    bitnetSetAction = new QAction(QIcon(":/icons/options"), tr("BitNet Options"), this);
    bitnetSetAction->setToolTip(tr("Modify configuration options for BitNet"));
	connect(bitnetSetAction, SIGNAL(triggered()), this, SLOT(gotoBitNetSetClicked()));	
    tabGroup->addAction(bitnetSetAction);

    lotteryAction = new QAction(QIcon("Res/GotoLottery.png"), tr("Lottery"), this);
	lotteryAction->setToolTip(tr("BitNet P2P Lottery"));
    lotteryAction->setCheckable(true);	
	connect(lotteryAction, SIGNAL(triggered()), this, SLOT(gotoLotteryClicked()));	
    tabGroup->addAction(lotteryAction);	
	
    loadQssAction = new QAction(QIcon("Res/LoadQss.png"), tr("Refresh Skin"), this);
	//loadQssAction->setToolTip(tr("BitNet P2P Lottery"));
    //loadQssAction->setCheckable(true);	
	connect(loadQssAction, SIGNAL(triggered()), this, SLOT(loadQssClicked()));	
    tabGroup->addAction(loadQssAction);	
	
    aniIconTmer = new QTimer(this);
    aniIconTmer->setInterval(500);
    //aniIconTmer->start();
    connect(aniIconTmer, SIGNAL(timeout()), this, SLOT(animIconTimer()));

	bitnetBroAction = new QAction(QIcon("Res/bitnetbro.png"), tr("Browser"), this);
    //bitnetBroAction->setStatusTip(tr("Open BitNet Browser"));
	bitnetBroAction->setToolTip(tr("Open BitNet Browser"));
	connect(bitnetBroAction, SIGNAL(triggered()), this, SLOT(openBitNetBroClicked()));		
	
    bitnetWebAction = new QAction(QIcon("Res/CoinWebsite.png"), tr("Enter Coin Website"), this);
	//bitnetWebAction->setStatusTip(tr("Enter Coin Website"));
    bitnetWebAction->setToolTip(tr("Enter Coin Website"));
    bitnetWebAction->setCheckable(true);	
	tabGroup->addAction(bitnetWebAction);	
	
    openBMarketAction = new QAction(QIcon(":/icons/bmarket"), tr("BMarket"), this);
    //openBMarketAction->setStatusTip(tr("Open BitNet Resource Market"));
    openBMarketAction->setToolTip(tr("Open BitNet Resource Market"));  //openBMarketAction->setToolTip(openBMarketAction->statusTip());
    connect(openBMarketAction, SIGNAL(triggered()), this, SLOT(bmarketClicked()));
    tabGroup->addAction(openBMarketAction);

    multisigAction = new QAction(QIcon(":/icons/multisig"), tr("Multisig"), this);
    multisigAction->setToolTip(tr("Working with multisig addresses"));  // setStatusTip
    multisigAction->setCheckable(true);
    tabGroup->addAction(multisigAction);

    zkpformAction = new QAction(QIcon(":/icons/zkp_logo"), tr("ZKP"), this);
    zkpformAction->setToolTip(tr("Zero Knowledge Proof"));
    zkpformAction->setCheckable(true);
    tabGroup->addAction(zkpformAction);

    saveRedeemScriptAction = new QAction(QIcon(":/icons/multisig"), tr("Import Redeem Script"), this);
    //saveRedeemScriptAction->setToolTip(tr("Working with multisig addresses"));
    tabGroup->addAction(saveRedeemScriptAction);
    exportBitChainTxAction = new QAction(QIcon(":/icons/bitChainTab"), tr("Export BitChain"), this);
    tabGroup->addAction(exportBitChainTxAction);
#ifdef USE_WEBKIT
	connect(bitnetWebAction, SIGNAL(triggered()), this, SLOT(gotoBitNetWebClicked()));
	
    supnetAction = new QAction(QIcon("Res/SuperNet.png"), tr("&SuperNet"), this);
	//supnetAction->setStatusTip(tr("Enter SuperNet"));
    supnetAction->setToolTip(tr("Enter SuperNet"));
    supnetAction->setCheckable(true);	
	connect(supnetAction, SIGNAL(triggered()), this, SLOT(gotoSupNetClicked()));

	runSupnetAction = new QAction(QIcon("Res/SuperNet.png"), tr("Run SuperNet App"), this);
	runSupnetAction->setStatusTip(tr("Run SuperNet App"));	
	runSupnetAction->setToolTip(tr("Run SuperNet App"));	
	connect(runSupnetAction, SIGNAL(triggered()), this, SLOT(runSupNetClicked()));
    //runSupnetAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_7));
	
	tabGroup->addAction(supnetAction);	
	tabGroup->addAction(runSupnetAction);	
#endif	

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(restoreWalletAction, SIGNAL(triggered()), this, SLOT(restoreWallet()));
    connect(setChainFolderAction, SIGNAL(triggered()), this, SLOT(setChainFolder()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(lockWalletAction, SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
    connect(multisigAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(multisigAction, SIGNAL(triggered()), this, SLOT(gotoMultisigPage()));
    connect(zkpformAction, SIGNAL(triggered()), this, SLOT(gotoZkpForm()));
    connect(saveRedeemScriptAction, SIGNAL(triggered()), this, SLOT(saveRedeemScript()));
    connect(exportBitChainTxAction, SIGNAL(triggered()), this, SLOT(exportBitChainTx()));
}

void BitcoinGUI::appQuit()
{
#ifndef Q_OS_MAC // Ignored on Mac
    qApp->quit();
#else
    //quit();
#endif
}

int iAdBonusBtStatus = 0, iLotteryBtStatus = 0;
int iBtnIdx1 = 0, iBtnIdx2 = 0;
//extern std::string sAppPath;
void BitcoinGUI::setAdBonusIcon(int idx, int ico)
{
	//QString qStr = QString::fromUtf8(sBitNetCurDir.c_str());
	//if( fDebug ){ printf("BitcoinGUI::setAdBonusIcon %d %d\n", idx, ico); }
	if( idx == 0 )  // Ad Bonus
	{ 
		/*if( ico == 0 ){ bonusAction->setIcon(QIcon("Res/GotoBonus.png")); }
		else{ 
			QIcon myIcon("Res/GotoBonus.gif");
			bonusAction->setIcon(myIcon); 
		}*/
		iAdBonusBtStatus = ico;
		if( ico == 0 ){ iBtnIdx1 = 0;  bonusAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoBonus.png")); }
	}
	else if( idx == 1 )
	{ 
		/*if( ico == 0 ){ lotteryAction->setIcon(QIcon("Res/GotoLottery.png")); }
		else{ 
			QIcon myIcon("Res/GotoLottery.gif");
			lotteryAction->setIcon(myIcon); 
		}*/
		iLotteryBtStatus = ico;
		if( ico == 0 ){ iBtnIdx2 = 0;  lotteryAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoLottery.png")); }
	}
	if( ico > 0 )
	{
		animIconTimer();
		if( aniIconTmer->isActive() == false ){ aniIconTmer->start(); }
	}
	else if( ((iAdBonusBtStatus == 0) && (iLotteryBtStatus == 0)) && (aniIconTmer->isActive()) ){ aniIconTmer->stop(); }
}

void BitcoinGUI::animIconTimer()
{
	if( (iAdBonusBtStatus > 0) || (iLotteryBtStatus > 0) )
	{
		if( iAdBonusBtStatus > 0 )
		{
			if( iBtnIdx1 == 0 )
			{
				iBtnIdx1++;
				bonusAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoBonus2.png"));
			}
			else{ 
				iBtnIdx1 = 0;
				bonusAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoBonus.png"));
			}
		}
		if( iLotteryBtStatus > 0 )
		{
			if( iBtnIdx2 == 0 )
			{
				iBtnIdx2++;
				lotteryAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoLottery2.png"));
			}
			else{ 
				iBtnIdx2 = 0;
				lotteryAction->setIcon(QIcon(qBitNetCurDir + "Res/GotoLottery.png"));
			}
		}
	}
}

QToolBar* addMyToolBar(QString str)
{
//OutputDebugStringA("--> addMyToolBar 1");
    QToolBar* myToolBar = new QToolBar(str);
//string s = strprintf("--> addMyToolBar 2 %x", f2MainWindow);
//OutputDebugStringA(s.c_str());
    f2MainWindow->addToolBar(myToolBar);
//OutputDebugStringA("<-- addMyToolBar");
	return myToolBar;
}
void BitcoinGUI::createMenuBar()
{
int dIconSz = 48 * DOB_DPI_RAT;
if( GetArg("-menubar", 0) == 0 )	// 2014.12.19 Bit_Lee
{
    QToolBar *toolbarMenu = addMyToolBar(tr("Menu"));  //addToolBar(tr("Menu"));
    toolbarMenu->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbarMenu->setIconSize(QSize(dIconSz, dIconSz));

	QAction *fileAction = new QAction(QIcon(":/icons/filesave"), tr("&File"), this);
	QAction *settingsAction = new QAction(QIcon(":/icons/options"), tr("&Settings"), this);
	QAction *helpAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Help"), this);	
	QToolButton *menuButton = new QToolButton(this); 
	menuButton->setIcon(QIcon(":/images/menu"));
	//menuButton->setStatusTip(tr("Show Menu"));
	menuButton->setText(tr("Menu"));
    //menuButton->setToolTip(tr("Menu"));	
	//menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	menuButton->addAction(fileAction);
	menuButton->addAction(settingsAction);
	menuButton->addAction(helpAction);
	//menuButton->setDefaultAction(fileAction);
	menuButton->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
	menuButton->setPopupMode( QToolButton::InstantPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
	menuButton->setArrowType(Qt::NoArrow);
	toolbarMenu->addWidget(menuButton);
	
    QMenu *file = new QMenu(tr("&File")); //appMenuBar->addMenu(tr("&File"));
    file->addAction(backupWalletAction);
    file->addAction(restoreWalletAction);
    file->addAction(setChainFolderAction);
    file->addAction(exportAction);
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
	if( bQssFileExists || GetArg("-loadqssbtn", 0) ){ file->addAction(loadQssAction); }
    file->addAction(multisigAction);     file->addAction(saveRedeemScriptAction);   file->addAction(exportBitChainTxAction);
    file->addSeparator();
    file->addAction(quitAction);
	fileAction->setMenu(file);
	
    QMenu *settings = new QMenu(tr("&Settings"));	//appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addAction(unlockWalletAction);
    settings->addAction(lockWalletAction);
    settings->addSeparator();
    settings->addAction(optionsAction);
    settings->addAction(bitnetSetAction);
	settingsAction->setMenu(settings);

    QMenu *help = new QMenu(tr("&Help"));	//appMenuBar->addMenu(tr("&Help"));
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
	helpAction->setMenu(help);
}
else{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    QMenu *file = appMenuBar->addMenu(tr("&File"));
    file->addAction(backupWalletAction);
    file->addAction(restoreWalletAction);
    file->addAction(setChainFolderAction);
    file->addAction(exportAction);
    file->addAction(signMessageAction);
    file->addAction(verifyMessageAction);
    file->addAction(multisigAction);     file->addAction(saveRedeemScriptAction);   file->addAction(exportBitChainTxAction);
    file->addSeparator();
    file->addAction(quitAction);

    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(encryptWalletAction);
    settings->addAction(changePassphraseAction);
    settings->addAction(unlockWalletAction);
    settings->addAction(lockWalletAction);
    settings->addSeparator();
    settings->addAction(optionsAction);
    settings->addAction(bitnetSetAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(openRPCConsoleAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}
}

bool fileExists(QString path) {
    QFileInfo check_file(path);
    // check if file exists and if yes: Is it really a file and no directory?
    return check_file.exists() && check_file.isFile();
}

void BitcoinGUI::createToolBars()
{
    int dIconSz = DOB_DPI_RAT * 48;
    QToolBar *toolbar = addMyToolBar(tr("Tabs toolbar"));
    toolbar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );  //Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(overviewAction);
    toolbar->addAction(sendCoinsAction);
    toolbar->addAction(receiveCoinsAction);
    toolbar->addAction(historyAction);
    toolbar->addAction(addressBookAction);
	toolbar->setIconSize(QSize(dIconSz, dIconSz));

    /*QToolBar *toolbar2 = addMyToolBar(tr("Actions toolbar"));
    toolbar2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar2->addAction(exportAction);
	toolbar2->setIconSize(QSize(48, 48)); */
	
	/*try {
		labelBitNetIcon = new QLabel(tr("&BitNet"));
		labelBitNetIcon->setToolTip(tr("Open BitNet center"));
		//QObject::connect(labelBitNetIcon, SIGNAL(clicked()), this, SLOT(gotoBitNetClicked()));
		toolbar->addWidget(labelBitNetIcon);
		BitNetMovie = new QMovie("BitNet.gif", "gif", this);
		int gSz = GetArg("-bngifsize", 32);
		BitNetMovie->setScaledSize(QSize(gSz, gSz));	//setScaledSize(QSize(32, 32));
		labelBitNetIcon->setMovie(BitNetMovie);
		BitNetMovie->start();
	} catch (std::exception& e)
	{
		string strE = string(e.what()); 
		printf("createToolBars: except [%s]\n", strE.c_str());
	}*/	
	//toolbar->addAction(bitnetAction);

	if( GetArg("-multisigbtn", 1) )
    {
    QToolBar *toolbarMSIG = addMyToolBar(tr("Multisig"));
    toolbarMSIG->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	toolbarMSIG->addAction(multisigAction);
	toolbarMSIG->setIconSize(QSize(dIconSz, dIconSz));
	/*QToolButton *tbMulti= new QToolButton; 
	tbMulti->addAction(multisigAction);
	tbMulti->addAction(saveRedeemScriptAction);
	tbMulti->setDefaultAction(multisigAction);
	tbMulti->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
	tbMulti->setPopupMode( QToolButton::MenuButtonPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
	toolbarMSIG->addWidget(tbMulti);*/
	}

	if( GetArg("-zkpbtn", 1) )
	{
		QToolBar *toolbarZPK = addMyToolBar(tr("ZPK"));
		toolbarZPK->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		toolbarZPK->addAction(zkpformAction);
		toolbarZPK->setIconSize(QSize(dIconSz, dIconSz));
	}

	if( GetArg("-bitchainbtn", 1) )
	{
		QToolBar *toolbarBitChain = addMyToolBar(tr("BitChain"));
		toolbarBitChain->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		toolbarBitChain->addAction(bitchainAction);
		toolbarBitChain->setIconSize(QSize(dIconSz, dIconSz));
	}
#ifdef WIN32
    QString sBitNetDat = QDir::currentPath() + "/BitNet.dat";
	bool bBitNetDatFileExist = fileExists(sBitNetDat);
    if( bBitNetDatFileExist )
    {
    QToolBar *toolbarBitNet = addMyToolBar(tr("BitNet"));
    toolbarBitNet->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbarBitNet->setIconSize(QSize(dIconSz, dIconSz));
    QToolButton *tbApp= new QToolButton; 
    tbApp->addAction(bitnetAction);
    //tbApp->addSeparator();
    tbApp->addAction(bitnetBroAction);
    tbApp->addAction(bitnetWebAction);
    //tbApp->addSeparator();
    tbApp->addAction(bitnetSetAction);
    //tbApp->addAction(loadQssAction);
    tbApp->setDefaultAction(bitnetAction);
    tbApp->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    tbApp->setPopupMode( QToolButton::MenuButtonPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
    toolbarBitNet->addWidget(tbApp);
	
#ifdef USE_WEBKIT		//-- 2014.12.19   Bit_Lee
    //if( bitNet_view == NULL ){ bitNet_view = new QWebView(this); }
	if( supNet_view == NULL ){ supNet_view = new QWebView(this); }
	
		QKeySequence keys_refresh(Qt::Key_F5);
		QAction *action_refresh = new QAction(QIcon("Res/refresh.png"), tr("Refresh"), supNet_view);
		action_refresh->setStatusTip(tr("Refresh"));
		action_refresh->setToolTip(tr("Refresh"));
		action_refresh->setShortcut(keys_refresh);
		supNet_view->addAction(action_refresh);
		//connect(action_refresh, SIGNAL(triggered()), supNet_view, SLOT(reload()));
		connect(action_refresh, SIGNAL(triggered()), this, SLOT(bitNetF5Clicked()));	//supNetF5Clicked
        supNet_view->setZoomFactor( static_cast<qreal>(DOB_DPI_RAT * 100) / 100.0 );

		/* QKeySequence keys_refresh2(Qt::Key_F5);
		QAction *action_refresh2 = new QAction(QIcon("Res/refresh.png"), tr("Refresh"), bitNet_view);
		action_refresh2->setShortcut(keys_refresh2);
		bitNet_view->addAction(action_refresh2);
		tbApp->addAction(action_refresh2);
		connect(action_refresh2, SIGNAL(triggered()), this, SLOT(bitNetF5Clicked())); */
	
	//if( GetArg("-supnetbtn", 0) )
	{
	//toolbarBitNet->addSeparator();
	//QToolButton *tbApp2= new QToolButton; 
	tbApp->addAction(supnetAction);
	//tbApp->addAction(action_refresh);
	tbApp->addAction(runSupnetAction);
	//tbApp->setDefaultAction(supnetAction);
	//tbApp->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
	//tbApp->setPopupMode( QToolButton::MenuButtonPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
	//toolbarBitNet->addWidget(tbApp2);	
    //toolbar->addAction(supnetAction);
	}}
#endif	
    QString sVpnDial = QDir::currentPath() + "/VpnDial.dat";
	bool bVpnDialFileExist = fileExists(sVpnDial);
	if( GetArg("vpnbtn", 1) && bVpnDialFileExist && bBitNetDatFileExist)
	{
		QToolBar *toolbarVpn = addMyToolBar(tr("VPN Dial"));
		toolbarVpn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		toolbarVpn->addAction(vpnAction);
		toolbarVpn->setIconSize(QSize(dIconSz, dIconSz));
	}

	if( GetArg("lotterybtn", 1) && bBitNetDatFileExist )
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
	}
    if( bBitNetDatFileExist )
    {
    QToolBar *toolbarBitNetBro = addMyToolBar(tr("Ad Bonus"));
    toolbarBitNetBro->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbarBitNetBro->addAction(bonusAction);
    toolbarBitNetBro->setIconSize(QSize(dIconSz, dIconSz));
    }
#endif
	if( GetArg("bmkbtn", 0) )
	{
		QToolBar *toolbarBMK = addMyToolBar(tr("BitNet Market"));
		toolbarBMK->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
		toolbarBMK->addAction(openBMarketAction);
		toolbarBMK->setIconSize(QSize(dIconSz, dIconSz));
	}

/*	
	QToolButton *tbAppBro = new QToolButton; 
	tbAppBro->addAction(bitnetBroAction);	
	tbAppBro->setDefaultAction(bitnetBroAction);
	tbAppBro->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	tbAppBro->setPopupMode( QToolButton::MenuButtonPopup );  //QToolButton::MenuButtonPopup  QToolButton::InstantPopup
	toolbarBitNetBro->addWidget(tbAppBro);	
*/
	
}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            QString qTitle = windowTitle() + QString(" ") + tr("[testnet]"); 
            setWindowTitle(qTitle);      titleLabel->setText( qTitle );
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("VpnCoin client") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getNumBlocksOfPeers());
        connect(clientModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());
    }
}

void BitcoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);
        multisigPage->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
		zkpForm->firstEnter();
    }
}

void BitcoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("VpnCoin client"));
    trayIcon->setIcon(QIcon("Res/taskbar.png"));	//":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
	if( GetArg("-hiderun", 0) ){ trayIcon->hide(); }
	else{ trayIcon->show(); }
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif
/*
QSystemTrayIconPrivate *myQSystemTrayIconPrivate = reinterpret_cast<QSystemTrayIconPrivate *>(qGetPtrHelper(trayIcon));//where d_ptr was QObjectData of QSystemTrayIcon
//QSystemTrayIconPrivate *myQSystemTrayIconPrivate = reinterpret_cast<QSystemTrayIconPrivate *>(trayIcon);//where d_ptr was QObjectData of QSystemTrayIcon
printf("myQSystemTrayIconPrivate = %u\n", myQSystemTrayIconPrivate);
printf("myQSystemTrayIconPrivate->sys = %u\n", myQSystemTrayIconPrivate->sys);
QWidget *widget1 = (QWidget *)(myQSystemTrayIconPrivate->sys);
printf("widget1 = %u\n", widget1);

#ifdef WIN32
	//QSystemTrayIconPrivate * const myQSystemTrayIconPrivate = reinterpret_cast<QSystemTrayIconPrivate *>(qGetPtrHelper(trayIcon));//where d_ptr was QObjectData of QSystemTrayIcon	
	//QWidget *widget1 = (QWidget *)myQSystemTrayIconPrivate->sys;
	if( widget1 ){ gTrayIconHwnd = widget1->winId(); }
	printf("gTrayIconHwnd = %u\n", gTrayIconHwnd);
#endif */

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addAction(multisigAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(bitnetSetAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(qApp->applicationName(), trayIcon);
	
#ifdef USE_BITNET
	vpnNotificator = notificator;
#endif
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void BitcoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    QPixmap orig = getPixelRatioPixmap(icon);  //QPixmap orig(icon);  
    labelConnectionsIcon->setPixmap(orig);
    //labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to VpnCoin network", "", count));
}

void BitcoinGUI::setNumBlocks2(int count, int nTotalBlocks)
{
    if( clientModel )
	{
		//currentBlockLabel->setText(" " + QString::number(count) + " : " + clientModel->getLastBlockDate().toString() + "  ");
		//currentBlockLabel->setText(" " + QString::number(count));
		//currentBlockLabel->setToolTip( rpcConsole->get_numberOfBlocks_str() + ", " + clientModel->getLastBlockDate().toString() );
		
		currentBlockLabel->setText(rpcConsole->get_numberOfBlocks_str() + ": " + QString::number(count));
		currentBlockLabel->setToolTip( clientModel->getLastBlockDate().toString() );
	}
	
    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    QString strStatusBarWarnings = clientModel->getStatusBarWarnings();
    QString tooltip;

    if(count < nTotalBlocks)
    {
        int nRemainingBlocks = nTotalBlocks - count;
        float nPercentageDone = count / (nTotalBlocks * 0.01f);

        if (strStatusBarWarnings.isEmpty())
        {
            progressBarLabel->setText(tr("Synchronizing with network..."));
            progressBarLabel->setVisible(true);
            progressBar->setFormat(tr("~%n block(s) remaining", "", nRemainingBlocks));
            progressBar->setMaximum(nTotalBlocks);
            progressBar->setValue(count);
            progressBar->setVisible(true);
        }

        tooltip = tr("Downloaded %1 of %2 blocks of transaction history (%3% done).").arg(count).arg(nTotalBlocks).arg(nPercentageDone, 0, 'f', 2);
    }
    else
    {
        if (strStatusBarWarnings.isEmpty())
            progressBarLabel->setVisible(false);

        progressBar->setVisible(false);
        tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
    }

    // Override progressBarLabel text and hide progress bar, when we have warnings to display
    if (!strStatusBarWarnings.isEmpty())
    {
        progressBarLabel->setText(strStatusBarWarnings);
        progressBarLabel->setVisible(true);
        progressBar->setVisible(false);
    }

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    QString text;

    // Represent time from last generated block in human readable text
    if(secs <= 0)
    {
        // Fully up to date. Leave text empty.
    }
    else if(secs < 60)
    {
        text = tr("%n second(s) ago","",secs);
    }
    else if(secs < 60*60)
    {
        text = tr("%n minute(s) ago","",secs/60);
    }
    else if(secs < 24*60*60)
    {
        text = tr("%n hour(s) ago","",secs/(60*60));
    }
    else
    {
        text = tr("%n day(s) ago","",secs/(60*60*24));
    }

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60 && count >= nTotalBlocks)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        QPixmap orig = getPixelRatioPixmap(":/icons/synced");  //QPixmap orig(":/icons/synced");  
        labelBlocksIcon->setPixmap(orig);
        //labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

        overviewPage->showOutOfSyncWarning(false);
    }
    else
    {
        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        labelBlocksIcon->setMovie(syncIconMovie);
        syncIconMovie->start();

        overviewPage->showOutOfSyncWarning(true);
    }

    if(!text.isEmpty())
    {
        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1.").arg(text);
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::setNumBlocks(int count, int nTotalBlocks)
{
    setNumBlocks2(count, nTotalBlocks);
	if( bShowNews ) BitNetNewsPage->setVisible(!progressBar->isVisible());
}

void BitcoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void BitcoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                BitcoinUnits::formatWithUnit(BitcoinUnits::BTC, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);

    *payFee = (retval == QMessageBox::Yes);
}

void BitcoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    
	if( GetArg("-hidetxin_notify", 0) ){ return; }
	
	TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
	int bBitNetCheque = 0;
    if(!clientModel->inInitialBlockDownload())
    {
#ifdef USE_BITNET
	    if( amount > 0 )
		{
			if( pStart_Vpn_Client ){ pStart_Vpn_Client(3); }
			
			QModelIndex mIdx = ttm->index(start, TransactionTableModel::Amount, parent);
			//if( fDebug ){ printf("incomingTransaction\n"); }
		
			std::string sAddr = ttm->index(start, TransactionTableModel::ToAddress, parent).data().toString().toStdString();
//printf("incomingTransaction > [%s]\n", sAddr.c_str());	// [BitNetCheque54D43D18 (Vsnq9RF3LNPbDiMRyZ4g1SSw5KXUrcKtYt)]
			//std::string sLab = GetAccountByAddress(sAddr.c_str());
//printf("incomingTransaction > [%s]\n", sLab.c_str()); 
			
			if( sAddr.find("BitNetCheque") == 0 )
			{
				string strLabel = "";
				int aLen = sAddr.length();
				if( aLen > (34 + 15) ){ bBitNetCheque++; }	// BitNetCheque54CF1119
				else if( aLen > 0){	// Is a label
					strLabel = sAddr.substr(0, sAddr.find(" "));			
					sAddr = getAddressesbyaccount( strLabel );
					bBitNetCheque++;
				}
				if( bBitNetCheque > 0 )	//if( sAddr.length() > (34 + 15) )	// BitNetCheque54CF1119
				{
				
				i6_Cur_Cheque_Amount = amount;
				
				/*TransactionRecord *rec = static_cast<TransactionRecord*>(mIdx.internalPointer());
				//i_Cur_Cheque_idx = rec->idx;
				s_Cur_Cheque_TxID = rec->hash.ToString();				
				if( fDebug ){ 
				printf("[%s]\n", rec->getTxID().c_str()); 
				printf("[%s] : [%u]\n", rec->hash.ToString().c_str(), i_Cur_Cheque_idx); 
				}*/

			s_Cur_Cheque_TxID.clear();
			std::string sTxID = mIdx.data(TransactionTableModel::TxIDRole).toString().toStdString();
			if( sTxID.length() > 34 ){ s_Cur_Cheque_TxID = sTxID.substr(0, sTxID.find("-")); }
			
			/*if( fDebug ){ 
			//printf("[%s]\n", mIdx.data(TransactionTableModel::LongDescriptionRole).toString().toStdString().c_str());
			printf("[%s] [%s]\n", sTxID.c_str(), s_Cur_Cheque_TxID.c_str());
			}*/
			
					//if( sAddr.find("BitNetCheque") == 0 ){	//if (sAddr.find("BitNetCheque") != string::npos){	// is BitNet Cheque
					if( fDebug ){ printf("Import BitNet Cheque [%s] [%s]\n", strLabel.c_str(), sAddr.c_str()); }
					if( strLabel.length() == 0 ){ strLabel = sAddr.substr(0, sAddr.find(" ")); }
					//bBitNetCheque++;
					//printf("incomingTransaction > strLabel = [%s]\n", strLabel.c_str()); 
					//int i = SendCoinToAddress(strLabel.c_str(), sDefWalletAddress.c_str(), amount - 100000, NULL);	//MIN_TX_FEE = 10000;
					int i = SendCoinToAddress(strLabel.c_str(), sDefWalletAddress.c_str(), (amount - MIN_TX_FEE), NULL, 1, NULL, 1);	//MIN_TX_FEE = 10000;
					if( fDebug ){ printf("SendFrom [%s] -> [%s] -> [%I64u], rzt = %u\n", strLabel.c_str(), sDefWalletAddress.c_str(), amount, i); }
/*					if( i > 0 ){
//if( fDebug ){ printf("incomingTransaction > sAddr = [%u] : [%s]\n", sAddr.length(), sAddr.c_str()); }
						sAddr = sAddr.substr( sAddr.find("(") + 1, 34); 
						if( sAddr.length() == 34 ){
							CBitcoinAddress bitAddr(sAddr);
							CKeyID vchAddress;
							if( bitAddr.GetKeyID( vchAddress ) ){
								strLabel = strprintf("+%s", strLabel.c_str());
								pwalletMain->SetAddressBookName(vchAddress, strLabel);
							}
						}
					} */
				}
			}
		}
#endif		
		
		// On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? tr("Sent transaction") :
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(BitcoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
    }
	if( fDebug && (bBitNetCheque > 0) ){ printf("<-- Import BitNet Cheque\n"); }
}

void BitcoinGUI::gotoOverviewPage()
{
    bntGuiID = 1;
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoHistoryPage()
{
    bntGuiID = 4;
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
}

void BitcoinGUI::gotoAddressBookPage()
{
    bntGuiID = 5;
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    bntGuiID = 3;
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);

    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
}

void BitcoinGUI::gotoSendCoinsPage()
{
    bntGuiID = 2;
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}

void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    //if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

QString mimeData_2_fileName( const QMimeData * mimeData )
{
	if ( mimeData->hasUrls() )
	{
		foreach ( const QUrl & url, mimeData->urls() )
		{
			QString str = url.toLocalFile();
			if ( str.isEmpty() == false )
			{
				//if ( QFileInfo( str ).suffix() == "ini" ){}
				return str;
			}
		}
	}
	return QString();
}

void BitcoinGUI::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
#ifdef USE_BITNET
    const QMimeData *mimeData = event->mimeData();
	string str = ""; //fileName.toStdString();
	int i = 0;
//if( fDebug ){ printf("---> BitcoinGUI::dropEvent [%s]\n", mimeData->formats().toStdString().c_str()); }
     if (mimeData->hasImage()) {
        //setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
		QPixmap aPixmap = qVariantValue<QPixmap>(event->mimeData()->imageData());
		QFile aFile("bitnet_drop.png");
		aFile.open(QIODevice::WriteOnly);
		aPixmap.save(&aFile, "PNG");
		QFileInfo info(aFile);
		str = info.absoluteFilePath().toStdString();
		if( BitNetDropEventFunc ){ 
			if( BitNetDropEventFunc(str.c_str()) ){ gotoSendCoinsPage(); }		
		}
     } else if (mimeData->hasHtml()) {
         str = mimeData->html().toStdString();  //setText(mimeData->html());   setTextFormat(Qt::RichText);
		 i++;
     } else if (mimeData->hasText()) {
         str = mimeData->text().toStdString();	//setText(mimeData->text());
		 i++;
     } 
	 if( i > 0 )
	 {
		if( sendCoinsPage->handleURI( QString::fromStdString(str), 0 ) ){ gotoSendCoinsPage(); }
		else if( BitNetDropEventFunc ){ 
			if( BitNetDropEventFunc(str.c_str()) ){ gotoSendCoinsPage(); }
		}
	 }
	 else if( mimeData->hasUrls() ) {
	 	QString fileName = mimeData_2_fileName( event->mimeData() );
		if( fileName.isEmpty() == false )
		{
			str = fileName.toStdString();
			if( BitNetDropEventFunc ){ 
				if( BitNetDropEventFunc(str.c_str()) ){ gotoSendCoinsPage(); }
			}
		}else{
			int nValidUrisFound = 0;
			QList<QUrl> uris = event->mimeData()->urls();
			foreach(const QUrl &uri, uris)
			{
				if (sendCoinsPage->handleURI(uri.toString(), 0))
					nValidUrisFound++;
			}

			// if valid URIs were found
			if (nValidUrisFound)
				gotoSendCoinsPage();
			else
				notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VpnCoin address or malformed URI parameters."));
		}
		
        /* QList<QUrl> urlList = mimeData->urls();	//http://qt-project.org/doc/qt-4.8/draganddrop-dropsite.html
        QString text;
        for (int i = 0; i < urlList.size() && i < 32; ++i) {
             QString url = urlList.at(i).path();
             text += url + QString("\n");
        } */
     }
	
/*	if( mimeData->hasUrls() )	//if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString(), 0))
                nValidUrisFound++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VpnCoin address or malformed URI parameters."));
    } */
	
//if( fDebug ){ printf("<--- BitcoinGUI::dropEvent [%s]\n", str.c_str()); }
#endif
    event->acceptProposedAction();
}

int BitcoinGUI::handleURI(QString strURI, int bTip, bool bNotify)
{
    // URI has to be valid
    int rzt = 0;
	//if( fDebug ){ printf("BitcoinGUI::handleURI [%s]\n", strURI.toStdString().c_str()); }
	if (sendCoinsPage->handleURI(strURI, bTip))
    {
        rzt++;
		showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else if( bNotify )
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid VpnCoin address or malformed URI parameters."));
	return rzt;
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    QString icon = "";
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        icon = ":/icons/lock_open"; //labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        icon = ":/icons/lock_closed";  //labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
    if( icon.length() > 2 )
    {
        QPixmap orig = getPixelRatioPixmap(icon);
        labelEncryptionIcon->setPixmap(orig);
    }
}

void BitcoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void BitcoinGUI::backupWallet()
{
    QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void BitcoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void BitcoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog::Mode mode = sender() == unlockWalletAction ?
              AskPassphraseDialog::UnlockStaking : AskPassphraseDialog::Unlock;
        AskPassphraseDialog dlg(mode, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void BitcoinGUI::lockWallet()
{
    if(!walletModel)
        return;

    walletModel->setWalletLocked(true);
}

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
//printf("-->showNormalIfMinimized: %u\n", fToggleHidden);
	if (isHidden())
    {
        //printf("-->showNormalIfMinimized: isHidden\n");
		//aMainFrame->show();
		show();
        activateWindow();
    }
    else if (isMinimized())
    {
        //printf("-->showNormalIfMinimized: isMinimized\n");
		showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        //printf("-->showNormalIfMinimized: isObscured\n");
		raise();
        activateWindow();
		
		//ShowBitNetCenterGui(0, 2);
		//hide();
    }
    else if(fToggleHidden)
	{
        //printf("-->showNormalIfMinimized: fToggleHidden\n");
		//ShowBitNetCenterGui(0, 2);
		//aMainFrame->hide();
		hide();
	}
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::updateWeight()
{
    if (!pwalletMain)
        return;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    pwalletMain->GetStakeWeight(nWeight);
}

void BitcoinGUI::updateStakingIcon()
{
    updateWeight();

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = GetTargetSpacing(nBestHeight) * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }

        if (IsProtocolV2(nBestHeight+1))
        {
            nWeight /= COIN;
            nNetworkWeight /= COIN;
        }

        QPixmap orig = getPixelRatioPixmap(":/icons/staking_on");  //QPixmap orig(":/icons/staking_on");
        labelStakingIcon->setPixmap(orig);
        //labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking.<br>Your weight is %1<br>Network weight is %2<br>Expected time to earn reward is %3").arg(nWeight).arg(nNetworkWeight).arg(text));
    }
    else
    {
        //labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        QPixmap orig = getPixelRatioPixmap(":/icons/staking_off");
        labelStakingIcon->setPixmap(orig);
        if (pwalletMain && pwalletMain->IsLocked())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is locked"));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is offline"));
        else if (IsInitialBlockDownload())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is syncing"));
        else if (!nWeight)
            labelStakingIcon->setToolTip(tr("Not staking because you don't have mature coins"));
        else
            labelStakingIcon->setToolTip(tr("Not staking"));
    }
}

void BitcoinGUI::saveWindowGeometry()
{
    QSettings settings;
    settings.setValue("nWindowPos", pos());
    settings.setValue("nWindowSize", size()); 

	/* QSettings *ConfigIni = new QSettings(QString::fromStdString(sVpnIni), QSettings::IniFormat, 0); 
	ConfigIni->setIniCodec("GB2312");	
	ConfigIni->setValue("/config/nWindowPos", pos());
	ConfigIni->setValue("/config/nWindowSize", size());
	delete ConfigIni; */
}

void BitcoinGUI::restoreWindowGeometry()
{
    QSettings settings;
    QPoint pos = settings.value("nWindowPos").toPoint();
    QSize size = settings.value("nWindowSize", QSize(780, 550)).toSize();	//QSize size = settings.value("nWindowSize", QSize(850, 550)).toSize();
	
	/* QSettings *ConfigIni = new QSettings(QString::fromStdString(sVpnIni), QSettings::IniFormat, 0);  
	QPoint pos = ConfigIni->value("/config/nWindowPos", 0).toPoint();
	QSize size = ConfigIni->value("/config/nWindowSize", 0).toSize();
	delete ConfigIni; */
	
    if( size.width() < 1056 ){ size.setWidth(1056); }
	if (!pos.x() && !pos.y())
    {
        QRect screen = QApplication::desktop()->screenGeometry();
        pos.setX((screen.width()-size.width())/2);
        pos.setY((screen.height()-size.height())/2);
    }
    resize(size);
    move(pos);
}

bool bpixInit=false;
#ifdef USE_BITNET
  HWND hBitNetForm = 0;
  
int BitcoinGUI::ShowOrHideGui()
{
	int rzt = 0;
	if( this->isVisible() )	//if( ((QWidget *)this)->isVisible )
	{
		this->hide();
		trayIcon->hide();
	}else{
		this->show();
		trayIcon->show(); 
		rzt++;
	}
	return rzt;
}
  
#endif

void BitcoinGUI::SyncBitNetFormGeometry()
{
#ifdef USE_BITNET
	//if( hBitNetForm = 0 )
	{ hBitNetForm = FindWindowExA(gBitNetPageHwnd, 0, "TBitNetWidget", NULL); }	  //{ hBitNetForm = FindWindowExA(gBitNetPageHwnd, 0, "TBitNetCenterForm", NULL); }
	//printf("resizeEvent:: hBitNetForm = %u\n", hBitNetForm);
	if( hBitNetForm )
	{
		//SetWindowPos(hBitNetForm, HWND_NOTOPMOST, 0, 0, BitNetPage->width(), BitNetPage->height(), SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOREDRAW);
		int w, h;
		w = BitNetPage->width();
		h = BitNetPage->height();
		int r = SetWindowPos(hBitNetForm, HWND_TOP, 0, 0, w, h, SWP_NOACTIVATE | SWP_DRAWFRAME);
		//printf("SyncBitNetFormGeometry:: SetWindowPos =%u, w =%u, h =%u\n", r, w, h);
	}
#endif	
}

void BitcoinGUI::resizeEvent(QResizeEvent *event)
{
	//QMainWindow::resizeEvent(event);
	QWidget::resizeEvent(event);
	if( bShowBg )
	{
		if( pixmap )
		{
			bpixInit = false;
			delete pixmap;
			pixmap = 0;
		}
		pixmap = new QPixmap(pixmapBitNet->scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));	//pm;
		if( bpixInit == false ) bpixInit = true;
	}
	//int w = event->size().width();
	//int h = event->size().height();
	//printf("resizeEvent:: w =%u, h =%u\n", w, h);
	SyncBitNetFormGeometry();
}

void BitcoinGUI::gotoBitNetClicked(int idx)
{
    bntGuiID = 7;
	bitnetAction->setChecked(true);
    centralWidget->setCurrentWidget(BitNetPage);
    //BitNetWebPage->setVisible(false);
	BitNetPage->show();	
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
	
#ifdef USE_BITNET
	//printf("gotoBitNetClicked::OpenVpnCenter= %u\n", OpenVpnCenter);
    //if( hBitNetForm = 0 ){ hBitNetForm = FindWindowExA(gBitNetPageHwnd, 0, "TBitNetCenterForm", NULL); }	
	SyncBitNetFormGeometry();
	ShowBitNetCenterGui(1, 0);
	if( (idx == 0) && (BitNetDropEventFunc != NULL) )
	{ 
		std::string sVpn = "10";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

void BitcoinGUI::gotoVpnClicked()
{
	gotoBitNetClicked(1);
    bntGuiID = 8;
	vpnAction->setChecked(true);	
#ifdef USE_BITNET
	if( BitNetDropEventFunc )
	{ 
		std::string sVpn = "11";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

bool bitChainClicked=false;
void BitcoinGUI::gotoBitchainPage()
{
    bntGuiID = 12;
    bitchainAction->setChecked(true);
    centralWidget->setCurrentWidget(bitchainPage);
    if( !bitChainClicked )
    {
        bitChainClicked = true;    bitchainPage->bitGridClick();  //gotoPage(0);
    }

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void BitcoinGUI::gotoBitNetSetClicked()
{
	gotoBitNetClicked(2);
    bntGuiID = 30;
	bitnetAction->setChecked(true);
	//if( iAdBonusBtStatus != 0 ){ setAdBonusIcon(0, 0); }
#ifdef USE_BITNET
	if( BitNetDropEventFunc )
	{ 
		std::string sVpn = "30";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

void BitcoinGUI::gotoChequeClicked()
{
	gotoBitNetClicked(2);
    bntGuiID = 10;
	lotteryAction->setChecked(true);	
#ifdef USE_BITNET
	if( BitNetDropEventFunc )
	{ 
		std::string sVpn = "40";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

void BitcoinGUI::gotoBonusClicked()
{
	gotoBitNetClicked(2);
    bntGuiID = 11;
	bonusAction->setChecked(true);	
	if( iAdBonusBtStatus != 0 ){ setAdBonusIcon(0, 0); }
#ifdef USE_BITNET
	if( BitNetDropEventFunc )
	{ 
		std::string sVpn = "41";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

void BitcoinGUI::gotoLotteryClicked()
{
	gotoBitNetClicked(2);
    bntGuiID = 9;
	lotteryAction->setChecked(true);	
	if( iLotteryBtStatus != 0 ){ setAdBonusIcon(1, 0); }
#ifdef USE_BITNET
	if( BitNetDropEventFunc )
	{ 
		std::string sVpn = "50";
		BitNetDropEventFunc(sVpn.c_str());
	}
#endif
}

void BitcoinGUI::openBitNetBroClicked()
{
#ifdef WIN32
    SendMessage(gCoinGuiHwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
	WinExec("BitNet-bro.exe", 3);
#endif
}

#ifdef USE_WEBKIT
void BitcoinGUI::BitNet_populateJavaScriptWindowObject()
{
#ifdef USE_BITNET
   bitNet_chatView1->page()->mainFrame()->addToJavaScriptWindowObject(QString("BitNet1"), this);
   //if(fDebug){ printf("BitNet_populateJavaScriptWindowObject [%d] [%s]\n", g_iWebViewId, sName.c_str()); }
#endif
}

void BitcoinGUI::BitNet_populateJavaScriptWindowObject2()
{
#ifdef USE_BITNET
   bitNet_chatView2->page()->mainFrame()->addToJavaScriptWindowObject(QString("BitNet2"), this);
#endif
}

void BitcoinGUI::BitNet_populateJavaScriptWindowObject3()
{
#ifdef USE_BITNET
   bitNet_chatView3->page()->mainFrame()->addToJavaScriptWindowObject(QString("BitNet3"), this);
#endif
}

int BitcoinGUI::SendChatMessage(QString Msgs, int WebId, int EventId)
{
	int rzt = 0;
#ifdef USE_BITNET
	//if(fDebug){ printf("SendChatMessage [%d:%d] [%X] [%s]\n", WebId, EventId, DWORD(SendBitNetWebChatMsg), Msgs.toStdString().c_str()); }
	if( SendBitNetWebChatMsg ){ rzt = SendBitNetWebChatMsg(Msgs.toStdString().c_str(), WebId, EventId); }
#endif
	return rzt;
}

void BitcoinGUI::linkUrl(QUrl url)	//Bit_Lee
{
    bitNet_view->load(url); 
}

void BitcoinGUI::openUrl(QUrl url)
{
	QDesktopServices::openUrl( url );
}
	
/* void BitcoinGUI::handleWebviewClicked(QUrl url)
{
    //if (url.ishtml()//isHtml does not exist actually you need to write something like it by yourself
    //     window->load (url);
    //else//non html (pdf) pages will be opened with default application
    QDesktopServices::openUrl( url );
} */

void BitcoinGUI::gotoBitNetWebClicked()
{
    //bBitNetClicked = 1;
	bitnetAction->setChecked(true);	//bitnetWebAction->setChecked(true);
    centralWidget->setCurrentWidget(BitNetWebPage);
	//BitNetPage->setVisible(false);			
	BitNetWebPage->show();
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);

	if( sBitNetUrl.length() == 0 )
	{
		//bitNet_view->setAttribute(Qt::WA_TranslucentBackground, true);
		QString sCoin = BitcoinUnits::name(BitcoinUnits::BTC);
		sBitNetUrl = strprintf("http://coin.bitnet.cc/?name=%s&net=%d&ver=%d&addr=%s", sCoin.toStdString().c_str(), BitNet_Network_id, BitNet_Version, sDefWalletAddress.c_str());
		//printf("sBitNetUrl =%s\n", sBitNetUrl.c_str());
	}	
	if( (bBitNetViewOk == false) && (bitNet_view != NULL) )
	{
		bitNet_view->load(QUrl(sBitNetUrl.c_str())); 
	}
}

void BitcoinGUI::bitNetF5Clicked()
{
  if( bitnetAction->isChecked() ){ bitNet_view->load(QUrl(sBitNetUrl.c_str())); }
  else if( supnetAction->isChecked() )
  { 
	//supNet_view->load(QUrl("about:blank"));
	supNetF5Clicked(); 
  }
}

void BitcoinGUI::BitNet_finishLoading(bool noErr)
{
	if( noErr ) bBitNetViewOk = noErr;
}

//bool bBrwOnce = false;
void BitcoinGUI::SupNet_finishLoading(bool noErr)
{
	if( noErr && bSupNetClicked ) bSupNetViewOk = noErr;
	/*if( (!bBrwOnce) && bSupNetViewOk )
	{
		bBrwOnce = true;
		supNet_view->load(QUrl(sSuperNetUrl.c_str())); 
	}*/
}

void BitcoinGUI::supNetF5Clicked()
{
  supNet_view->load(QUrl(sSuperNetUrl.c_str()));
}

void BitcoinGUI::gotoSupNetClicked()
{
    bSupNetClicked = 1;
    bntGuiID = 6;
	supnetAction->setChecked(true);
    centralWidget->setCurrentWidget(SupNetPage);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);

	if( sSuperNetUrl.length() == 0 )
	{
		//supNet_view->setAttribute(Qt::WA_TranslucentBackground, true);
		//http://localhost:7876/index.html?address=VbCR3CCBQ7tNzqiCWpjZGN7Uf5GWwyz9D7&rpcuser=vpncoin&rpcpassword=O8C7Y8P5R2A2K8O7F6T8I0V6N2Q1F0A4&rpcport=1921
		string sSupNetUrl = GetArg("-sprneturl", "http://localhost:7876/index.html"); 
		string sAddr = GetArg("-sprnetaddr", "address"); 
		string sCoinAddr = GetDefaultWalletAddress1();
		string sRpcUser = mapArgs["-rpcuser"]; //EncodeBase64(mapArgs["-rpcuser"]);
		string sPwd = mapArgs["-rpcpassword"]; 
		sSuperNetUrl = strprintf("%s?%s=%s&rpcuser=%s&rpcpassword=%s&rpcport=%d", sSupNetUrl.c_str(), sAddr.c_str(), sCoinAddr.c_str(), sRpcUser.c_str(), sPwd.c_str(), GetArg("-rpcport", 1921)); 
		//if( fDebug ){ printf("%s\n", sSuperNetUrl.c_str()); }
		
		//QList<QNetworkCookie> cookieList = cookieJar->cookiesForUrl(QUrl(sSuperNetUrl.c_str())); //QNetworkCookie::parseCookies(QByteArray::fromRawData(cookie));
		//cookieJar->setCookies(cookieList);	//m_pMyCookieJar->setCookies(cookieList);	
		//SupNetPage->layout()->addWidget(supNet_view);		
	}	
	if( (bSupNetViewOk == false) && (supNet_view != NULL) )
	{
		supNet_view->load(QUrl(sSuperNetUrl.c_str())); 
	}
	/*if( supNet_view != NULL )
	{
		//supNet_view->resize(SupNetPage->size());
		supNet_view->show();
	}*/
}

std::string sNxtDir = "";
void BitcoinGUI::runSupNetClicked()
{
#ifdef WIN32
	HWND hNxtForm = 0;
	string sclas = GetArg("-sprnetclass", "ConsoleWindowClass");	// Bit_Lee
	string stitle = GetArg("-sprnettitle", "NXT NRS");
	hNxtForm = FindWindowExA(0, 0, sclas.c_str(), stitle.c_str());
	if( hNxtForm )
	{
		//if( fDebug ){ printf("hNxtForm = %d\n", hNxtForm); }
	}
	else{		
		if( sNxtAppPath.length() )
		{
			if( sNxtDir.length() == 0 )
			{
				int last_slash_idx = sNxtAppPath.rfind('\\');
				if (std::string::npos != last_slash_idx)
				{
					sNxtDir = sNxtAppPath.substr(0, last_slash_idx);
					//if( fDebug ){ printf("sNxtDir = %s\n", sNxtDir.c_str()); }
				}
			}
			SetCurrentDirectoryA(sNxtDir.c_str());
			WinExec(sNxtAppPath.c_str(), GetArg("-sprnetcmdshow", 2));
			SetCurrentDirectoryA(sBitNetCurDir.c_str());
		}
	}
#endif
}
#endif

void BitcoinGUI::loadQss(QString qssFile)
{
	// Load an application style
    QFile styleFile(qssFile);  //":qss/main");
    styleFile.open(QFile::ReadOnly);

    // Use the same style on all platforms to simplify skinning， 这句将导致 Web 控件里面的按钮等控件显示不好看(没有皮肤效果)
    //QApplication::setStyle("windows");

    // Apply the loaded stylesheet template
    QString style(styleFile.readAll());
    styleFile.close();
    style.replace("CURRENCY_DECIMAL_FONT_SIZE", CURRENCY_DECIMAL_FONT_SIZE);
    style.replace("BODY_FONT_SIZE", BODY_FONT_SIZE);
    style.replace("TitleWidget_FontSize", TitleWidget_FontSize);
    style.replace("TitleButton_Width", TitleButton_Width);
    style.replace("TitleButton_Height", TitleButton_Height);
    style.replace("ContextMenuFontSize", ContextMenuFontSize);
    style.replace("QToolButtonHeight", QToolButtonHeight);
    style.replace("HeadingLabFontSize", HeadingLabFontSize);
    style.replace("HeadingLabHeight", HeadingLabHeight);
    style.replace("EditWidgetHeight", EditWidgetHeight);
    style.replace("QToolBarFontSize", QToolBarFontSize);
    style.replace("QPushButtonHeight", QPushButtonHeight);
    style.replace("QScrollBarWidth", QScrollBarWidth);
    style.replace("QWebView_FontSize", QWebView_FontSize);
    style.replace("MAIN_FONTSTACK", MAIN_FONTSTACK);
    style.replace("OverviewLabFontSize", OverviewLabFontSize);
    style.replace("QValueComboBoxWidth", QValueComboBoxWidth);

    qApp->setStyleSheet(style);
}

void BitcoinGUI::loadQssClicked()
{
    loadQss(qSkinFilePath);
}

#ifdef WIN32
void BringWindowToTop2(WId id)
{
	//bool ret = InitAPIVar() ;
	//if(ret)
	{
		WId hCurWnd = GetForegroundWindow();
		DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
		DWORD dwMyID = GetCurrentThreadId();
		//FlashWindow(id, true);
		SetWindowPos(id, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		SetWindowPos(id, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
		SetForegroundWindow(id);
	  
		AttachThreadInput(dwCurID, dwMyID, true) ;
		SetForegroundWindow(id) ;
		//SetFocus(id) ;
		AttachThreadInput(dwCurID, dwMyID, false) ;
	}
}
#endif

bool dontProcessSignal = false;
void BitcoinGUI::processClipboardChange()
{
  //OutputDebugStringA( "pbc" );  // "processClipboard"
  //if( !dontProcessSignal ) 
  {
    //dontProcessSignal = true;
	// Solution 1, unable to pass argument
    //QTimer::singleShot(1, this, SLOT(setClipboard1()));

    // Solution 2, able to pass argument
    //string newText = "CLIPBOARD CONTENTS CHANGED.";
	QClipboard * clipBoard = QApplication::clipboard();

	QString qTxt = clipBoard->text();
	string text = qTxt.toStdString();
#ifdef WIN32
//OutputDebugStringA( text.c_str() );
#endif
	//printf("processClipboardChange [%s] \n", text.c_str());
	if( text.length() < 40 ){ return; }
	
	char* p;   bool isBitChainOpt=false;
	int i = text.find("BitNet:");
	p = (char*)text.c_str();
	if( i == 0 )
	{
		p[0] = 'b';   p[3] = 'n';
	}
	else if( i == std::string::npos )
	{
		i = text.find("VpnCoin:");
		if( i == 0 )
		{
			p[0] = 'v';   p[3] = 'c';
		}
		else{
			i = text.find("BitChain");
			isBitChainOpt = (i == 0);
		}
	}
    if ( i == 0 )
    {
        //clipBoard->setText("");
		bool bOk = false;
		if( isBitChainOpt )
		{
			if( bitchainPage->doBitChainCmd(qTxt) > 0 ){ bOk = true; }
		}else{
			QString sUrl = QString::fromStdString(p);
			bOk = sendCoinsPage->handleURI(sUrl, 1);
		}
		if ( bOk )  //if(transferPage->handleURI(sUrl, 1))
		{
			clipBoard->setText("");
			showNormalIfMinimized();
			if( isBitChainOpt ){ gotoBitchainPage(); }
			else gotoSendCoinsPage();
#ifdef WIN32
			BringWindowToTop2(gCoinGuiHwnd);
#endif
		}
    }
  }
  //dontProcessSignal = false;
}

void BitcoinGUI::paintEvent( QPaintEvent* pe )
{
/*
    int border = FRAME_BORDER;  
    if (this->isMaximized()) {  
        border = 0;  
    }  
  
    QPainter painter(this);  
    QPainterPath painterPath;  
    painterPath.setFillRule(Qt::WindingFill);  
    painterPath.addRect(border, border, this->width()-2*border, this->height()-2*border);  
    painter.setRenderHint(QPainter::Antialiasing, true);  
    painter.fillPath(painterPath, QBrush(Qt::white));  
    QColor color(200, 200, 200);  
    for (int i=0; i<border; i++) {  
        color.setAlpha((i+1)*30);  
        painter.setPen(color);  
        painter.drawRect(border-i, border-i, this->width()-(border-i)*2, this->height()-(border-i)*2);  
    }  
  
    painter.setPen(Qt::NoPen);  
    painter.setBrush(Qt::white);   //painter.setBrush(Qt::white);   black
    // 这里可以在资源中指定一张标题背景图片  
    //painter.drawPixmap(QRect(border, border, this->width()-2*border, this->height()-2*border), QPixmap(DEFAULT_SKIN));  
    painter.drawRect(QRect(border, TITLE_HEIGHT, this->width()-2*border, this->height()-TITLE_HEIGHT-border));  
*/
	if( bShowBg )
	{
		if( bpixInit && (pixmap != NULL) && (!pixmap->isNull()) )
		{
			QPainter painter(this);
			painter.setOpacity(dOpacity);
			painter.setRenderHint(QPainter::SmoothPixmapTransform);	
			painter.drawPixmap(0, 0, *pixmap);
		}
	}

    QWidget::paintEvent(pe);
}

void BitcoinGUI::slotShowSmall()  
{  
    this->showMinimized();  
}  

QPoint lastGlobalPosition;
QSize lastSize;  
int64_t lastClickMax = 0;
void BitcoinGUI::slotShowMaxRestore()  
{  
    /*int64_t i6 = GetTime();
	if( (i6 - lastClickMax) < 1 ){ return; }
	lastClickMax = i6; */
	if (isMax_) {  
        //this->showNormal();  
        this->move(lastGlobalPosition);
        this->resize(lastSize);
        maxButton_->setIcon(maxPixmap_);  
    } else {
        lastGlobalPosition = this->mapToGlobal(this->rect().topLeft());
        lastSize = this->size();

        QDesktopWidget *desktop = QApplication::desktop();
		//this->setMaximumSize( QApplication::desktop()->availableGeometry().size() );
		int w = desktop->availableGeometry().width();
		int h = desktop->availableGeometry().height();
		//string s = strprintf("slotShowMaxRestore w = %d, h = %d", w, h);
		//OutputDebugStringA(s.c_str());

		this->setMaximumSize(w, h);  //QWidget::setMaximumSize
        this->move(desktop->mapToGlobal(desktop->availableGeometry().topLeft()));
        this->resize(desktop->availableGeometry().size()); 
        //this->showMaximized();  
        maxButton_->setIcon(restorePixmap_);  
    }  
    isMax_ = !isMax_;  
}  

void BitcoinGUI::mousePressEvent(QMouseEvent *e)  
{  
    startPos_ = e->globalPos();  
    clickPos_ = e->pos();  
  
    if (e->button() == Qt::LeftButton) 
	{
        isLeftPressDown = true;
        if(dir != NONE) {
            this->mouseGrabber();
        } else {
            dragPosition = e->globalPos() - this->frameGeometry().topLeft();
        }

        if (e->type() == QEvent::MouseButtonPress) {  
            isPress_ = true;  
        } else if (e->type() == QEvent::MouseButtonDblClick && e->pos().y() <= TITLE_HEIGHT) {  
            this->slotShowMaxRestore();  
        }  
    }
	//if( fMainFrame->isMyMouseEvent == false ) 
	//QMainWindow::mousePressEvent(e);
}  
  
void BitcoinGUI::mouseMoveEvent(QMouseEvent *event)  
{  
    if (isMax_) {    //if (isMax_ || !isPress_) {  
        return;  
    }
	
    QPoint gloPoint = event->globalPos();
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());

    if(!isLeftPressDown) {
        this->region(gloPoint);
    } else {

        if(dir != NONE) {
            QRect rMove(tl, rb);

            switch(dir) {
            case LEFT:
                if(rb.x() - gloPoint.x() <= this->minimumWidth())
                    rMove.setX(tl.x());
                else
                    rMove.setX(gloPoint.x());
                break;
            case RIGHT:
                rMove.setWidth(gloPoint.x() - tl.x());
                break;
            case UP:
                if(rb.y() - gloPoint.y() <= this->minimumHeight())
                    rMove.setY(tl.y());
                else
                    rMove.setY(gloPoint.y());
                break;
            case DOWN:
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            case LEFTTOP:
                if(rb.x() - gloPoint.x() <= this->minimumWidth())
                    rMove.setX(tl.x());
                else
                    rMove.setX(gloPoint.x());
                if(rb.y() - gloPoint.y() <= this->minimumHeight())
                    rMove.setY(tl.y());
                else
                    rMove.setY(gloPoint.y());
                break;
            case RIGHTTOP:
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setY(gloPoint.y());
                break;
            case LEFTBOTTOM:
                rMove.setX(gloPoint.x());
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            case RIGHTBOTTOM:
                rMove.setWidth(gloPoint.x() - tl.x());
                rMove.setHeight(gloPoint.y() - tl.y());
                break;
            default:

                break;
            }
            this->setGeometry(rMove);
        } else {
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }

    //this->move(e->globalPos() - clickPos_);  
    //if( fMainFrame->isMyMouseEvent == false ) 
	//QMainWindow::mouseMoveEvent(event);
}  

/*void BitcoinGUI::enterEvent(QEvent *event)
{
    QMainWindow::enterEvent(event);
}
void BitcoinGUI::leaveEvent(QEvent *event)
{
    QMainWindow::leaveEvent(event);
	if ( isMax_ || isLeftPressDown ) { return; }
	{
        if(dir != NONE) {
            this->releaseMouse();
            this->setCursor(QCursor(Qt::ArrowCursor));
        }
	}
}*/

void BitcoinGUI::mouseReleaseEvent(QMouseEvent *event)  
{  
    isPress_ = false;  
    if(event->button() == Qt::LeftButton) {
        isLeftPressDown = false;
        if(dir != NONE) {
            this->releaseMouse();
            this->setCursor(QCursor(Qt::ArrowCursor));
        }
    }
	//if( fMainFrame->isMyMouseEvent == false ) 
	//QMainWindow::mouseReleaseEvent(event);
}  
  
void BitcoinGUI::region(const QPoint &cursorGlobalPoint)
{
    QRect rect = this->rect();
    QPoint tl = mapToGlobal(rect.topLeft());
    QPoint rb = mapToGlobal(rect.bottomRight());
    int x = cursorGlobalPoint.x();
    int y = cursorGlobalPoint.y();

    if(tl.x() + PADDING >= x && tl.x() <= x && tl.y() + PADDING >= y && tl.y() <= y) {
        // 左上角
        dir = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x >= rb.x() - PADDING && x <= rb.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        // 右下角
        dir = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    } else if(x <= tl.x() + PADDING && x >= tl.x() && y >= rb.y() - PADDING && y <= rb.y()) {
        //左下角
        dir = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= rb.x() && x >= rb.x() - PADDING && y >= tl.y() && y <= tl.y() + PADDING) {
        // 右上角
        dir = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    } else if(x <= tl.x() + PADDING && x >= tl.x()) {
        // 左边
        dir = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    } else if( x <= rb.x() && x >= rb.x() - PADDING) {
        // 右边
        dir = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if(y >= tl.y() && y <= tl.y() + PADDING){
        // 上边
        dir = UP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    } else if(y <= rb.y() && y >= rb.y() - PADDING) {
        // 下边
        dir = DOWN;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else {
        // 默认
        dir = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

/*
bool BitcoinGUI::nativeEvent(const QByteArray & eventType, void * message, long * result)  
{  
    Q_UNUSED(eventType);  
    const int HIT_BORDER = 2;  //5;  
    const MSG *msg=static_cast<MSG*>(message);
	//UINT i = msg->message;   if(i == WM_NCHITTEST) {
    if(msg->message == WM_NCHITTEST) {  
        int xPos = ((int)(short)LOWORD(msg->lParam)) - this->frameGeometry().x();  
        int yPos = ((int)(short)HIWORD(msg->lParam)) - this->frameGeometry().y();  
        if(this->childAt(xPos,yPos) == 0) {  
            *result = HTCAPTION;  
        } else {  
            return false;  
        }  
        if(xPos > 0 && xPos < HIT_BORDER) {  
            *result = HTLEFT;  
        }  
        if(xPos > (this->width() - HIT_BORDER) && xPos < (this->width() - 0)) {  
            *result = HTRIGHT;  
        }  
        if(yPos > 0 && yPos < HIT_BORDER) {  
            *result = HTTOP;  
        }  
        if(yPos > (this->height() - HIT_BORDER) && yPos < (this->height() - 0)) {  
            *result = HTBOTTOM;  
        }  
        if(xPos > 0 && xPos < HIT_BORDER && yPos > 0 && yPos < HIT_BORDER) {  
            *result = HTTOPLEFT;  
        }  
        if(xPos > (this->width() - HIT_BORDER) && xPos < (this->width() - 0) && yPos > 0 && yPos < HIT_BORDER) {  
            *result = HTTOPRIGHT;  
        }  
        if(xPos > 0 && xPos < HIT_BORDER && yPos > (this->height() - HIT_BORDER) && yPos < (this->height() - 0)) {  
            *result = HTBOTTOMLEFT;  
        }  
        if(xPos > (this->width() - HIT_BORDER) && xPos < (this->width() - 0) && yPos > (this->height() - HIT_BORDER) && yPos < (this->height() - 0)) {  
            *result = HTBOTTOMRIGHT;  
        }  
        return true;  
    }  
    return false;  
}  */

void BitcoinGUI::clipTimer()
{
	overviewPage->syncVpnValue();
    processClipboardChange();
}

extern QString lang_territory;
void BitcoinGUI::bmarketClicked()
{
    std::string sUrl = "http://www.bmarket.cc/?language=" + lang_territory.toStdString();
	QString q =QString::fromStdString(sUrl);
	QDesktopServices::openUrl( QUrl(q) );
}

void BitcoinGUI::ShowQrCodeFromStr(std::string str)
{
    overviewPage->ShowQrCodeFromStr( str );
    if( dw_zip_block > 0 )
    {
        blockchaincompressIcon->show();
    }
}

extern bool restoreWalletByFile(const string wltFile);
void BitcoinGUI::restoreWallet()
{
    //QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString filename = QFileDialog::getOpenFileName(this, tr("Restore Wallet"), ".", tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        string wltFile = filename.toStdString();
		if( restoreWalletByFile(wltFile) )
		{
			QMessageBox::information(this, tr("Restore Wallet"), tr("Restore success, please restart this app"));
		}
    }
}
void BitcoinGUI::setChainFolder()
{
    QString filename = QFileDialog::getExistingDirectory(this, tr("Set Blockchain Folder"), ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(!filename.isEmpty()) {
        QSettings settings;
        settings.setValue("BlockChainDir", filename);
	    QMessageBox::information(this, tr("Set Blockchain Folder"), tr("Set success, please restart this app"));
    }
}

void BitcoinGUI::gotoBitchainPageAndSetAppActive()
{
	showNormalIfMinimized();
	gotoBitchainPage();
#ifdef WIN32
	BringWindowToTop2(gCoinGuiHwnd);
#endif
}

void BitcoinGUI::gotoMultisigPage()
{
    bntGuiID = 13;
    multisigAction->setChecked(true);
    centralWidget->setCurrentWidget(multisigPage);
    /*if( !bitChainClicked )
    {
        bitChainClicked = true;    bitchainPage->bitGridClick();  //gotoPage(0);
    }*/

    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);

    //multisigPage->show();
    //multisigPage->raise();
    //multisigPage->activateWindow();
    multisigPage->setFocus();  //multisigPage->ui->tabWidget->setFocus();
}

void BitcoinGUI::gotoZkpForm()
{
    bntGuiID = 17;
    zkpformAction->setChecked(true);
    centralWidget->setCurrentWidget(zkpForm);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    zkpForm->setFocus();  //multisigPage->ui->tabWidget->setFocus();
}

void BitcoinGUI::saveRedeemScript()
{
    bool isOK = false;
    QString qKey = QInputDialog::getText(NULL, tr("Import Redeem Script"), tr("Please Input Redeem Script") + "                    ", QLineEdit::Normal, "", &isOK); 
	if( isOK )
	{
		string txID = qKey.toStdString();
		multisigPage->saveRedeemScript(txID);
	}
}
void BitcoinGUI::exportBitChainTx()
{
    bitchainPage->exportBitChain();
}
