#include "qMsg.h"
#include "util.h"
#include "titlewidget.h"
#include "bitchainpage.h";
#include "bitcoingui.h";
#include <QVBoxLayout>
#include "mypushbutton.h"
#include <QLabel>
#include <QPainter>
#include <QBitmap>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

#include "wallet.h"
#include "walletdb.h"
#include "bitcoinrpc.h"
#include "init.h"

//#include "coincontrol.h"
#include <iostream>
#include <iterator>
#include <vector>

//using namespace json_spirit;
using namespace std;
using namespace boost;

extern double DOB_DPI_RAT;
extern boost::signals2::signal<void (Widget* pMsg)> NotifyFreeWidget;
int iDlgShow = 0;
#ifdef USE_BITNET
extern BitcoinGUI *vpnBitcoinGUI;
#endif

Widget::Widget(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
	bitChainP = NULL;   nType = 1;     timeCount = 0;     tran=1.0;
	beginY = QApplication::desktop()->height();     iDlgShow = GetArg("-bitchaintipboxdisplaytime", 30);

	QString sTitleHei = "font: bold " + QString::number(DOB_DPI_RAT * 14, 'f', 0) + "px;";
    titleW=new titleWidget("close", sTitleHei, tr("Close"), 0, false);
    connect(titleW, SIGNAL(myClose()), this, SLOT(close()));

    content=new QLabel;     content->setCursor(Qt::OpenHandCursor);
    content->setWordWrap(true);
    content->setAlignment(Qt::AlignTop);     content->installEventFilter(this);
    content->setFixedSize(480 * DOB_DPI_RAT, 300 * DOB_DPI_RAT);
    //btnLook=new myPushButton("look");
    //connect(btnLook,SIGNAL(clicked()),this,SLOT(openWeb()));
	
    sTitleHei.replace("bold ", "");      titleWidget* titleW2 = new titleWidget("filesave", sTitleHei, tr("Export BitChain"), (DOB_DPI_RAT * 56), true);  // look
    connect(titleW2, SIGNAL(myClose()), this, SLOT(saveBitChain()));
	titleW2->setTitleText( " " + tr("Message from BitChain") );

    QVBoxLayout*mainLayout=new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(titleW);
    mainLayout->addWidget(content);
    content->setMargin(5);
    mainLayout->addWidget(titleW2);  //mainLayout->addWidget(btnLook,0,Qt::AlignRight);
    setLayout(mainLayout);

    setFixedSize(sizeHint().width(),sizeHint().height());

    timerShow=new QTimer(this);
    connect(timerShow,SIGNAL(timeout()),this,SLOT(myMove()));
    timerStay=new QTimer(this);
    connect(timerStay,SIGNAL(timeout()),this,SLOT(myStay()));
    timerClose=new QTimer(this);
    connect(timerClose,SIGNAL(timeout()),this,SLOT(qMsgClose()));
	//this->setAttribute(Qt::WA_DeleteOnClose, true);
}

Widget::~Widget()
{
}

void Widget::setMsg(QString title, QString content, QString url, BitchainPage* bp, int iType)
{
    titleW->setTitleText(title);
    this->content->setText(content);
    this->url=url;     bitChainP = bp;     nType = iType;
}

void Widget::paintEvent(QPaintEvent *)
{
    QBitmap bitmap(this->size());
    bitmap.fill(Qt::white);
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(250,240,230)));
    painter.setPen(QPen(QBrush(QColor(255,222,173)),4));
    painter.drawRoundedRect(bitmap.rect(),5,5);
    setMask(bitmap);
}
#ifdef WIN32
extern void BringWindowToTop2(WId id);
#endif

void Widget::showAsQQ()
{
    QDesktopWidget *deskTop=QApplication::desktop();
    deskRect=deskTop->availableGeometry();

    normalPoint.setX( deskRect.width() - rect().width() - (10 * DOB_DPI_RAT) );
    normalPoint.setY( deskRect.height()-rect().height() );
    move(normalPoint.x(), beginY);  //768-1);
    show();    timerShow->start(6);     this->setFocus(); 
#ifdef WIN32
    BringWindowToTop2( this->winId() );
#else
    activateWindow();
#endif
}
//平滑显示出来
void Widget::myMove()
{
    //static int beginY=QApplication::desktop()->height();
    beginY--;
    move(normalPoint.x(), beginY);
    if(beginY<=normalPoint.y())
    {
        timerShow->stop();
        timerStay->start(1000);
    }
}
//停留显示
bool bClose=false;
void Widget::myStay()
{
    //static int timeCount=0;
    timeCount++;
    if(timeCount >= iDlgShow)
    {
        timerStay->stop();
        bClose = true;     timerClose->start(123);
    }
}
//自动关闭时实现淡出效果
void Widget::qMsgClose()
{
    //static double tran=1.0;
	/*if( bClose )
	{
	    bClose = false;     timerClose->stop();     this->close();     return true;
	}*/
    tran-=0.1;
    if( tran <= 0.0 )
    {
        timerClose->stop();
        //NotifyFreeWidget( this );  //close();  
		emit close();
    }
    else
        setWindowOpacity(tran);
	/*for( int i = 0; i++; i < 10 )
	{
		tran-=0.1;     setWindowOpacity(tran);     MilliSleep(200);
	}*/
	//return true;
}
void Widget::openWeb()
{
	if( nType == 0 ){ QDesktopServices::openUrl(QUrl(url)); }
	else if( bitChainP != NULL )
	{
#ifdef USE_BITNET
        if( vpnBitcoinGUI != NULL ){ vpnBitcoinGUI->gotoBitchainPageAndSetAppActive(); }
#endif
		bitChainP->setCurrentPage(nType);
		bitChainP->gotoPage(nType, false, url);     timerClose->start(123);
		//if( nType == 1 ){   }
	}
}

bool Widget::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        if( obj == content ){ openWeb(); }
    }
	else if (ev->type() == QEvent::MouseMove)
	{
		//
	}
    return QWidget::eventFilter(obj, ev);
}

extern std::string sBitChainDecryptKey;
extern std::string sBitChainDatDir;
extern int exportBitChainTx(std::string txID, string sAesKey, int iCheckHashBlock, int txMust2MeOrFromTeam, int saveBcpFile, int saveTxtFile, int saveImgFile, int saveMp3File, int showTipBox, int saveToMysql, int mkDir, BitchainPage* bp);
void Widget::saveBitChain()
{
    std::string txID = this->url.toStdString();
    int iCheckHashBlock = 1, txMust2MeOrFromTeam = 0, saveBcpFile = 0, saveTxtFile = 1, saveImgFile = 1, saveMp3File = 1, showTipBox = 0, saveToMysql = 0, mkDir = 1;
#ifdef USE_MYSQL
        //showTipBox = 0;   iCheckHashBlock = 0;   saveBcpFile++;   saveImgFile++;   saveMp3File++;   saveToMysql++;
#endif
    int i = exportBitChainTx(txID, sBitChainDecryptKey, iCheckHashBlock, txMust2MeOrFromTeam, saveBcpFile, saveTxtFile, saveImgFile, saveMp3File, showTipBox, saveToMysql, mkDir, bitChainP);
	if( i > 0 )
	{
		    txID = sBitChainDatDir + txID;     QString qKey = QString::fromStdString(txID);
			QDesktopServices::openUrl( QUrl::fromLocalFile(qKey) );
	}
}
