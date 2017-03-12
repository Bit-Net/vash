#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"

#include "guiutil.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "addresstablemodel.h"
#include "main.h"

#ifdef USE_BITNET
#include "bitnet.h"
#endif

#include <QApplication>
#include <QClipboard>
#include <QTextEdit>
#include <QPainter>
//#include <QColor>

const QString BitNetTeam_DomainAddress = "VpfHY7zLdWFVG8cDYzCk78ier7gsHo4E3v";
extern std::string sDefWalletAddress;

class TextEdit : public QTextEdit 
{
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
public:
    TextEdit(QWidget *parent=0) : QTextEdit(parent) {}
 
    void setPlaceholderText(QString text){
        QString t = toPlainText();
		if( t == placeholderText ){ clear(); }
		if (placeholderText != text){ placeholderText = text; }
        if(toPlainText().isEmpty())
		{
            setText(placeholderText);
			//setHtml(QString("<font color=\"#808080\"><i>%1</i></font>").arg(placeholderText));
		}
    }
 
protected:
    /*void paintEvent( QPaintEvent* pe )
	{
		//QTextEdit::paintEvent(pe);
		if( (toPlainText().isEmpty()) && (!placeholderText.isNull()) )
		{
			QPainter p(this);
			p.drawText(geometry(), Qt::AlignHCenter | Qt::AlignVCenter, placeholderText);
		}
		else
		{
			//QTextEdit::paintEvent(pe);
		}
	}*/
	
	void focusInEvent(QFocusEvent *e){
        if (!placeholderText.isEmpty()){
            QString t = toPlainText();
            if (t.isEmpty() || (t == placeholderText) ) clear();
        }
        QTextEdit::focusInEvent(e);
    }
 
    void focusOutEvent(QFocusEvent *e){
        if (!placeholderText.isEmpty()){
            if (toPlainText().isEmpty())
			{
                setText(placeholderText);
				//setHtml(QString("<font color=\"#808080\"><i>%1</i></font>").arg(placeholderText));
			}
        }
        QTextEdit::focusOutEvent(e);
    }
 
public:
    QString placeholderText;
};

//TextEdit *txtMessage;
SendCoinsEntry::SendCoinsEntry(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SendCoinsEntry),
    model(0)
{
    ui->setupUi(this);

    txtMessage = new TextEdit(this);
	ui->vLayout_Msg->addWidget(txtMessage);
	ui->txtFrom->setVisible(true); ui->txtFrom->setMaxLength(MAX_TX_DATA_FROM);
    ui->txtSubject->setVisible(true); ui->txtSubject->setMaxLength(MAX_TX_DATA_SUBJ);
    //txtMessage->setVisible(true);	//ui->txtMessage->setVisible(true);
    ui->lblCharRemain->setVisible(true);
    ui->rbt_mod_coinfreeze->setVisible(false);
    ui->rbt_mod_coinjoin->setVisible(false);

    if (!GetBoolArg("-message", true))
    {
        ui->lblHMessage->setVisible(false);

        ui->lblFrom->setVisible(false);
        ui->btnFrom->setVisible(false);

        ui->lblSubject->setVisible(false);
        ui->btnSubject->setVisible(false);

        ui->lblMessage->setVisible(false);
        ui->btnMessage->setVisible(false);
    }
	
#ifdef Q_OS_MAC
    ui->payToLayout->setSpacing(4);
#endif
#if QT_VERSION >= 0x040700
    /* Do not move this to the XML file, Qt before 4.7 will choke on it */
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
    ui->payTo->setPlaceholderText(tr("The address to send the payment to  (e.g. Vvpywwpq4uKGvouhNv7jzKZ4uoYm3ECsbM)"));
	ui->txtFrom->setPlaceholderText(tr("Enter your wallet address/account name/E-mail address(e.g. Vpn-China)"));
#endif
    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(ui->payTo);

    GUIUtil::setupAddressWidget(ui->payTo, this);
	//connect(ui->rbt_mod_buydomain, SIGNAL(stateChanged(int)), this, SLOT(rbt_mod_buydomain_stateChanged(int)));
	
#ifdef USE_BITNET
vpnSendCoinsEntry = this;
#endif
    QFont font(ui->lblCharRemain->font());
    font.setBold(true);
    ui->lblCharRemain->setFont(font);
    ui->lblCharRemain->setStyleSheet("QLabel { color : red; }");  //ui->lblCharRemain->setStyleSheet("QLabel { background-color : black; color : red; }");
}

SendCoinsEntry::~SendCoinsEntry()
{
    ui->vLayout_Msg->removeWidget(txtMessage);
	delete txtMessage;
	delete ui;
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    // Paste text from clipboard into recipient field
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_addressBookButton_clicked()
{
    if(!model)
        return;
    AddressBookPage dlg(AddressBookPage::ForSending, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if(dlg.exec())
    {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}

void SendCoinsEntry::on_payTo_textChanged(const QString &address)
{
    if(!model)
        return;
    // Fill in label from address book, if address has an associated label
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if(!associatedLabel.isEmpty())
        ui->addAsLabel->setText(associatedLabel);
}

void SendCoinsEntry::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    connect(ui->payAmount, SIGNAL(textChanged()), this, SIGNAL(payAmountChanged()));

    clear();
}

void SendCoinsEntry::setRemoveEnabled(bool enabled)
{
    ui->deleteButton->setEnabled(enabled);
}

void SendCoinsEntry::clearCore()
{
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->txtFrom->clear();
    ui->txtSubject->clear();
	ui->txtSubject->setEnabled(true);
    txtMessage->placeholderText.clear();
	txtMessage->clear();	//ui->txtMessage->clear();
}
void SendCoinsEntry::clear()
{
    /*ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->txtFrom->clear();
    ui->txtSubject->clear();
	ui->txtSubject->setEnabled(true);
    txtMessage->placeholderText.clear();
	txtMessage->clear();	//ui->txtMessage->clear(); */
	clearCore();
    ui->payTo->setFocus();
    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void SendCoinsEntry::on_deleteButton_clicked()
{
    emit removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    // Check input validity
    bool retval = true;

    if(!ui->payAmount->validate())
    {
        retval = false;
    }
    else
    {
        int64_t aMount = ui->payAmount->value();
		if( ui->rbt_mod_coinfreeze->isChecked() )    // 2015.09.20 add
		{
			if( aMount < (10 * COIN) )
			{
                ui->payAmount->setValid(false);
                retval = false;
			}
		}
		else if(aMount <= MIN_TX_FEE)  //if(ui->payAmount->value() <= 0)  // 2015.08.17 edit
        {
            // Cannot send 0 coins or less
            ui->payAmount->setValid(false);
            retval = false;
        }
    }

    if(!ui->payTo->hasAcceptableInput() ||
       (model && !model->validateAddress(ui->payTo->text())))
    {
        ui->payTo->setValid(false);
        retval = false;
    }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    SendCoinsRecipient rv;

    rv.address = ui->payTo->text();
    rv.label = ui->addAsLabel->text();
    rv.amount = ui->payAmount->value();

    rv.from = ui->txtFrom->text(); rv.from.trimmed().truncate(MAX_TX_DATA_FROM);
    rv.subject = ui->txtSubject->text(); rv.subject.trimmed().truncate(MAX_TX_DATA_SUBJ);
	
	if( ui->rbt_mod_normal->isChecked() ){ rv.sType = 0; }
	else if( ui->rbt_mod_vpn->isChecked() ){ rv.sType = 1; }
	else if( ui->rbt_mod_buydomain->isChecked() ){ rv.sType = 2; }
	else if( ui->rbt_mod_lottery->isChecked() ){ rv.sType = 3; }
	else if( ui->rbt_mod_coinfreeze->isChecked() ){ rv.sType = 4; }
	else if( ui->rbt_mod_coinjoin->isChecked() ){ rv.sType = 5; }

	QString t = txtMessage->toPlainText();
	if( t == txtMessage->placeholderText ){ txtMessage->clear(); }
    rv.message = txtMessage->toPlainText(); rv.message.trimmed().truncate(MAX_TX_DATA_MSG);
	//rv.message = ui->txtMessage->text(); rv.message.trimmed().truncate(MAX_TX_DATA_MSG);

    return rv;
}

QWidget *SendCoinsEntry::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    QWidget::setTabOrder(ui->deleteButton, ui->addAsLabel);
    return ui->payAmount->setupTabChain(ui->addAsLabel);
}

void SendCoinsEntry::setValue(const SendCoinsRecipient &value)
{
	clearCore();
	if( (value.sType == 0) || (value.sType > 4) ){
		ui->rbt_mod_normal->setChecked(true);
		on_rbt_mod_normal_clicked();   ui->rbt_mod_normal->setFocus();
	}
	else if( value.sType == 1 ){
		ui->rbt_mod_vpn->setChecked(true);
		on_rbt_mod_vpn_clicked();   ui->rbt_mod_vpn->setFocus();
	}
	else if( value.sType == 2 ){
		ui->rbt_mod_buydomain->setChecked(true);
		on_rbt_mod_buydomain_clicked();   ui->rbt_mod_buydomain->setFocus();
	}
	else if( value.sType == 3 ){
		ui->rbt_mod_lottery->setChecked(true);
		on_rbt_mod_lottery_clicked();   ui->rbt_mod_lottery->setFocus();
	}
	else if( value.sType == 4 ){
		ui->rbt_mod_coinfreeze->setChecked(true);
		on_rbt_mod_coinfreeze_clicked();   ui->rbt_mod_coinfreeze->setFocus();
	}

	//if( value.sType != 2 )
	{  // not buy domain
        ui->payTo->setText(value.address);
        ui->addAsLabel->setText(value.label);
        ui->payAmount->setValue(value.amount);
	}
	
    if( value.sType == 0 )
	{
	    ui->txtFrom->setText(value.from);
	    ui->txtSubject->setText(value.subject);
	}
	txtMessage->setText(value.message);	
}

/* void SendCoinsEntry::setAddress(const QString &address)
{
    ui->payTo->setText(address);
    ui->payAmount->setFocus();
} */

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        // Update payAmount with the current unit
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

void SendCoinsEntry::clearIfIsDomainAddress()
{
    ui->txtFrom->setPlaceholderText("");
	/*QPalette palette( ui->txtFrom->palette() );
	palette.setColor(QPalette::Text, Qt::black);
	ui->txtFrom->setPalette(palette); */
	ui->txtFrom->setStyleSheet("QLineEdit { color : black; }");
	
	ui->txtFrom->setEnabled(true);
	if( BitNetTeam_DomainAddress == ui->payTo->text() )
	{
		on_rbt_mod_buydomain_stateChanged(0);
	}
}

void SendCoinsEntry::on_rbt_mod_normal_clicked()
{
	clearIfIsDomainAddress();
	ui->txtFrom->clear();
	//ui->txtFrom->setPlaceholderText(tr("Enter your wallet address/account name/E-mail address(e.g. Vpn-China)"));
    ui->txtSubject->clear();   txtMessage->clear();
	ui->txtSubject->setEnabled(true);
	txtMessage->setPlaceholderText(QString());
	ui->lblCharRemain->clear();
}

void SendCoinsEntry::on_rbt_mod_vpn_clicked()
{
	clearIfIsDomainAddress();
//#ifdef USE_BITNET
	ui->txtFrom->setText(QString::fromStdString(sDefWalletAddress));
//#endif
	ui->txtSubject->setText("Vpn Fee");
	ui->txtSubject->setEnabled(false);   txtMessage->clear();
	QString q = tr("Fee|Buy minutes|Remarks| (e.g. 0.0005|10|Try|)");
	txtMessage->setPlaceholderText(q);
	ui->lblCharRemain->setText(q);
}
void SendCoinsEntry::on_rbt_mod_coinjoin_clicked()
{
	clearIfIsDomainAddress();
//#ifdef USE_BITNET
	ui->txtFrom->setText(QString::fromStdString(sDefWalletAddress));
//#endif
	ui->txtSubject->setText("Coin Join");   txtMessage->clear();
	ui->txtSubject->setEnabled(false);
	QString q = tr("Delay x Minutes send to|Really receive Address|Remarks| (e.g. 30|Vvpy...CsbM|Good luck|)");
	txtMessage->setPlaceholderText(q);
	ui->lblCharRemain->setText(q);
}

void SendCoinsEntry::on_rbt_mod_buydomain_stateChanged(int state)
{
    //printf("on_rbt_mod_buydomain_stateChanged %d \n", state);
	ui->payTo->clear();
	ui->addAsLabel->clear();
	/*if( state == Qt::Checked )   // Qt::Unchecked == 0
	{
		ui->addAsLabel->setText("BitNet Domain");
		ui->payTo->setText("VpfHY7zLdWFVG8cDYzCk78ier7gsHo4E3v");
	}*/
}
void SendCoinsEntry::on_rbt_mod_buydomain_clicked()
{
	/*QPalette palette( ui->txtFrom->palette() );
	palette.setColor(QPalette::Text, Qt::red);
	ui->txtFrom->setPalette(palette);*/
	//ui->txtFrom->setStyleSheet("QLineEdit { color : red; }");  // 2016.04.06
	//ui->payAmount->clear();
	//ui->payAmount->setPlaceholderText( tr("Please contact BitNet Team to determine the price") );
	ui->addAsLabel->setText("BitNet Domain");
	ui->payTo->setText(BitNetTeam_DomainAddress);
	ui->txtFrom->clear();   //ui->txtFrom->setPlaceholderText( tr("Please contact BitNet Team to determine the price") );   // 2016.04.06
	ui->txtSubject->setText("Buy BitNet Domain");   txtMessage->clear();
	ui->txtSubject->setEnabled(false);   ui->txtFrom->setEnabled(false);
	QString q = tr("Enter you want to buy domain name");
	txtMessage->setPlaceholderText( q );
	ui->lblCharRemain->setText(q);
}

void SendCoinsEntry::on_rbt_mod_lottery_clicked()
{
    clearIfIsDomainAddress();
	ui->txtFrom->setText(tr("Please enable coin control features and select lottery address as input"));
	ui->txtSubject->setText("Encash lottery");  //ui->txtSubject->setText("Guarantee");	//Encash lottery
	ui->txtSubject->setEnabled(false);
	//txtMessage->setPlaceholderText(tr("Delay x Minutes send to|Seller's wallet address|Transaction information in X mall| (e.g. 60|Vvpyw...CsbM|shopping id:xxxx|)"));
	QString q = tr("BitNet Lottery:|Lottery id|3|0|0|0|0|0|0|0|Lottery coin address|0|Your default coin address|Your bet tx hash|Signature text(signature your bet tx hash with your default coin address)");
	txtMessage->setText(q);	
	ui->lblCharRemain->setText(q);
}
	
void SendCoinsEntry::on_rbt_mod_coinfreeze_clicked()
{
	clearIfIsDomainAddress();
//#ifdef USE_BITNET
	ui->txtFrom->setText("");
//#endif
	ui->txtSubject->setText(tr("Coin Freeze"));   txtMessage->clear();
	ui->txtSubject->setEnabled(false);
	QString q = tr("Freeze X minutes before using these coins|Pay To Address|After thaw's target address(not required)  [e.g. 60|Vf4DFB...VYQQlm]");
	txtMessage->setPlaceholderText(q);
	ui->lblCharRemain->setText(q);
}
