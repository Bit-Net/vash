#include "splashscreen.h"
#include "clientversion.h"
#include "util.h"
#include <QLabel>
#include <QMovie>
#include <QPainter>
#undef loop /* ugh, remove this when the #define loop is gone from util.h */
#include <QApplication>
extern double DOB_DPI_RAT;

SplashScreen::SplashScreen(const QPixmap &pixmap, Qt::WindowFlags f) :
    QSplashScreen(pixmap, f)
{
    //this->setWindowOpacity(0.9);
	setStyleSheet("background:transparent");
	
    this->setWindowFlags(Qt::FramelessWindowHint);//ÉèÖÃ´°ÌåÎÞ±ß¿ò
    this->setAttribute(Qt::WA_TranslucentBackground);//ÉèÖÃ±³¾°Í¸Ã÷
	
	// set reference point, paddings
    int paddingLeftCol2         = 119;
    int paddingTopCol2          = 105;
    int line1 = 0;
    int line2 = 13 * DOB_DPI_RAT;
    int line3 = 26 * DOB_DPI_RAT;
    if( DOB_DPI_RAT > 1 )
    {
        line2 = line2 + (DOB_DPI_RAT * 10);
        line3 = line3 + (DOB_DPI_RAT * 20);
    }

    float fontFactor            = DOB_DPI_RAT;  //1.0;

    // define text to place
    QString titleText       = QString(QApplication::applicationName()).replace(QString("-testnet"), QString(""), Qt::CaseSensitive); // cut of testnet, place it as single object further down
    QString versionText     = QString("Version %1 ").arg(QString::fromStdString(FormatFullVersion()));
    QString copyrightText1   = QChar(0xA9)+QString(" 2009-%1 ").arg(COPYRIGHT_YEAR) + QString(tr("The Bitcoin developers"));  //QString::number(DOB_DPI_RAT) + "   " + 
    QString copyrightText2   = QChar(0xA9)+QString(" 2013-%1 ").arg(COPYRIGHT_YEAR) + QString(tr("The BitNet developers"));

    QString font            = "Arial";

    // load the bitmap for writing some text over it
    QPixmap newPixmap;
	if( DOB_DPI_RAT > 1 )
	{
    int w = pixmap.width(), h = pixmap.height();
    w = w * DOB_DPI_RAT;   h = h * DOB_DPI_RAT;
    newPixmap = pixmap.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);    
	}
    /*if(GetBoolArg("-testnet")) {
        newPixmap     = QPixmap(":/images/splash");	//QPixmap(":/images/splash_testnet");
    }
    else {
        newPixmap     = QPixmap(":/images/splash");
    }*/
    else newPixmap = pixmap;
    QPainter pixPaint(&newPixmap);
    pixPaint.setPen(QColor(255, 255, 255));	//pixPaint.setPen(QColor(70,70,70));

    pixPaint.setFont(QFont(font, 10*fontFactor));
    pixPaint.drawText(paddingLeftCol2,paddingTopCol2 + line3, versionText);

    // draw copyright stuff
    pixPaint.setFont(QFont(font, 10*fontFactor));
    pixPaint.drawText(paddingLeftCol2, paddingTopCol2 + line1,copyrightText2);
    pixPaint.drawText(paddingLeftCol2, paddingTopCol2 + line2,copyrightText1);

    pixPaint.end();

    this->setContentsMargins(0,0,0,0);    
    /*QLabel *movieLabel = new QLabel(this);
	movieLabel->setContentsMargins(0,0,0,0);	
	movieLabel->setFixedSize(400, 450);
	movieLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QMovie *movie = new QMovie("vpncoin.gif");
	movieLabel->setMovie( movie );
	movie->start();
	movieLabel->show(); */

	//setAttribute( Qt::WA_TranslucentBackground );
    this->setPixmap(newPixmap);
	
	//setWindowFlags(Qt::FramelessWindowHint);
}
QWidget *gwMain=NULL;
void SplashScreen::DelayFinish(QWidget *wMain, int deLay)
{
    gwMain = wMain;
	movieTimer = new QTimer();
	movieTimer->setInterval(deLay * 1000);	// 3s
	movieTimer->start();
	connect(movieTimer, SIGNAL(timeout()), this, SLOT(finishSplashTimer()));
}

void SplashScreen::finishSplashTimer()
{
	if( gwMain != NULL ){ gwMain->show(); }
	movieTimer->stop();
	this->finish(gwMain);
}
