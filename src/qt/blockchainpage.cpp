#include "blockchainpage.h"
#include "ui_blockchainpage.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "main.h"

#include <QPixmap>
#include <QUrl>
#include <QFileDialog>

#include <qrencode.h>
#include "util.h"
#include "txdb.h"
#include "walletdb.h"
#include "init.h"

extern std::string dumpPrivaKey(string strAddress);

BlockchainPage::BlockchainPage(int activeTab, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::BlockchainPage),
    model(0),
    aTabId(activeTab)
{
    ui->setupUi(this);
    ui->chain_tabWidget->setCurrentIndex(aTabId);
    ui->lab_setLoveImg->installEventFilter(this);
    ui->lab_setKidImg->installEventFilter(this);
    ui->lab_setBlessImg->installEventFilter(this);
    ui->lab_setEventImg->installEventFilter(this);
    iNeedPay = 1;   toChainImgFile = "";
    QString q = tr("Parent topic, it is optional");
    ui->lEdit_loveParentTopic->setPlaceholderText( q );
    ui->lEdit_kidsParentTopic->setPlaceholderText( q );
    ui->lEdit_blessParentTopic->setPlaceholderText( q );
    ui->lEdit_eventParentTopic->setPlaceholderText( q );
}

BlockchainPage::~BlockchainPage()
{
    delete ui;
}

void BlockchainPage::setModel(WalletModel *model)
{
    this->model = model;

    // update the display unit, to not use the default ("BTC")
    //updateDisplayUnit();
}

void BlockchainPage::on_chain_tabWidget_currentChanged(int index)
{
    aTabId = index;
    updatePayForChain( aTabId );
}

void BlockchainPage::on_ptEdit_loveWords_textChanged()
{
    updatePayForChain( 0 );
}

void BlockchainPage::on_ptEdit_kidInfo_textChanged()
{
    updatePayForChain( 1 );
}

void BlockchainPage::on_ptEdit_blessWords_textChanged()
{
    updatePayForChain( 2 );
}

void BlockchainPage::on_ptEdit_eventTxt_textChanged()
{
    updatePayForChain( 3 );
}

void BlockchainPage::updatePayForChain(int tabId)
{
    int i = 0;
	QString t = "";  //txtMessage->toPlainText();
    if( tabId == 0 ){ t = ui->ptEdit_loveWords->toPlainText(); }
    else if( tabId == 1 ){ t = ui->ptEdit_kidInfo->toPlainText(); }
    else if( tabId == 2 ){ t = ui->ptEdit_blessWords->toPlainText(); }
    else if( tabId == 3 ){ t = ui->ptEdit_eventTxt->toPlainText(); }
    i = t.length();
	int c = 1;
	if( i > 0 )
	{
		 c = i / 512;
		 if( c == 0 ){ c++; }
	}
	iNeedPay = c;
	t = QString("%1").arg(c) + " VPN";  //QString s = QString::number(a, 10);      // s == "63"
    if( tabId == 0 ){ ui->lab_payForLove->setText( t ); }
    else if( tabId == 1 ){ ui->lab_payForKid->setText( t ); }
    else if( tabId == 2 ){ ui->lab_payForBless->setText( t ); }
    else if( tabId == 3 ){ ui->lab_payForEvent->setText( t ); }
}

void BlockchainPage::on_pBtn_subLoveToChain_clicked()
{
    submit_data_toChain( 0 );
}

void BlockchainPage::on_pBtn_subKidsInfoToChain_clicked()
{
    submit_data_toChain( 1 );
}

void BlockchainPage::on_pBtn_subBlessToChain_clicked()
{
    submit_data_toChain( 2 );
}

void BlockchainPage::on_pBtn_subEventToChain_clicked()
{
    submit_data_toChain( 3 );
}

void BlockchainPage::on_pBtn_regNick_clicked()
{
    submit_data_toChain( 4 );
}

void BlockchainPage::submit_data_toChain(int tabId)
{
    //updatePayForChain( aTabId );
}

bool BlockchainPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        if( obj == ui->lab_setLoveImg )
        {
            select_photo_toChain( 0 );
			/*const QPixmap *pm = ui->label_qrcode->pixmap();
            if( pm != NULL )
            {
                QApplication::clipboard()->setPixmap( *pm );
                ovShowMsg();
            }*/
        }
        else if( obj == ui->lab_setKidImg ){ select_photo_toChain( 1 ); }
        else if( obj == ui->lab_setBlessImg ){ select_photo_toChain( 2 ); }
        else if( obj == ui->lab_setEventImg ){ select_photo_toChain( 3 ); }
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

void BlockchainPage::select_photo_toChain(int tabId)
{
    toChainImgFile = "";
	QString filename = QFileDialog::getOpenFileName(this, tr("Choose Picture"), ".", tr("Picture file (*.png *.bmp *.jpg)"));
    if(!filename.isEmpty())
	{
        toChainImgFile = filename;
		QPixmap pic = QPixmap(toChainImgFile);
		QPixmap pic2 = pic.scaled(66, 66,  Qt::KeepAspectRatio);
		if( tabId == 0 ){ ui->lab_setLoveImg->setPixmap( pic2 ); }
		else if( tabId == 1 ){ ui->lab_setKidImg->setPixmap( pic2 ); }
		else if( tabId == 2 ){ ui->lab_setBlessImg->setPixmap( pic2 ); }
		else if( tabId == 3 ){ ui->lab_setEventImg->setPixmap( pic2 ); }
    }
}

void BlockchainPage::on_pBtn_clearLove_clicked()
{
    clearBtnClicked(0);
}
void BlockchainPage::on_pBtn_clearKids_clicked()
{
    clearBtnClicked(1);
}
void BlockchainPage::on_pBtn_clearBless_clicked()
{
    clearBtnClicked(2);
}
void BlockchainPage::on_pBtn_clearEvent_clicked()
{
    clearBtnClicked(3);
}
void BlockchainPage::clearBtnClicked(int tabId)
{
    QString q = tr("Click to select photo");
	if( tabId == 0 )
    {
        ui->lEdit_loveParentTopic->clear();
        ui->ptEdit_loveWords->clear();
        ui->lab_setLoveImg->setPixmap( QPixmap() );
        ui->lab_setLoveImg->setText(q);
		ui->ptEdit_loveWords->setFocus(); 
	}
    else if( tabId == 1 )
    {
        ui->lEdit_kidsParentTopic->clear();
        ui->ptEdit_kidInfo->clear();
        ui->lEdit_kidsUrl->clear();
        ui->lab_setKidImg->setPixmap( QPixmap() );
        ui->lab_setKidImg->setText(q);
		ui->ptEdit_kidInfo->setFocus(); 
	}
    else if( tabId == 2 )
    {
        ui->lEdit_blessParentTopic->clear();
        ui->ptEdit_blessWords->clear();
        ui->lab_setBlessImg->setPixmap( QPixmap() );
        ui->lab_setBlessImg->setText(q);
		ui->ptEdit_blessWords->setFocus(); 
	}
    else if( tabId == 3 )
    {
        ui->lEdit_eventParentTopic->clear();
        ui->ptEdit_eventTxt->clear();
        ui->lEdit_eventLink->clear();
        ui->lab_setEventImg->setPixmap( QPixmap() );
        ui->lab_setEventImg->setText(q);
		ui->ptEdit_eventTxt->setFocus(); 
	}
}