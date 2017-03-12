#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "util.h"
#include "guiutil.h"
#include "guiconstants.h"

#include <QKeyEvent>
#include <QClipboard>

#ifdef USE_QRCODE
#include <qrencode.h>
#include "qrcodedialog.h"
#include <QImage>
#endif

#ifdef USE_BITNET
#include "bitnet.h"
#endif

#include <QAbstractItemDelegate>
#include <QPainter>

using namespace std;

#define DECORATION_SIZE 64
#define NUM_ITEMS 3

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    currentBalance(-1),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    ui->labelAddress->installEventFilter(this);
    ui->label_qrcode->installEventFilter(this);
    ui->labelYourAddr->installEventFilter(this);
	ui->wallet_overvie_Value->installEventFilter(this);
	ui->wallet_overvie_Value->setText("");
    //this->installEventFilter(this);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // init "out of sync" warning labels
    ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

#ifdef USE_QRCODE
QImage myImage;
QString qDefWalletAddress = "";
void OverviewPage::ShowQrCodeFromStr(std::string str)
{
    QString uri = QString::fromStdString(str);
    qDefWalletAddress = uri;
    if (uri != "")
    {
        QString qAds = uri;
        std::string shh = "\n";
        qAds.insert(8, QString::fromStdString(shh));  qAds.insert(17, QString::fromStdString(shh));
        qAds.insert(26, QString::fromStdString(shh));  qAds.insert(35, QString::fromStdString(shh));
        ui->labelAddress->setText(qAds);   
        uri = "vpncoin:" + uri;
		ui->label_qrcode->setText("");

        QRcode *code = QRcode_encodeString(uri.toUtf8().constData(), 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        //QRcode *code = QRcode_encodeString(uri.toStdString().c_str(), 1, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!code)
        {
            ui->label_qrcode->setText(tr("Error encoding URI into QR Code."));
            return;
        }
        myImage = QImage(code->width + 8, code->width + 8, QImage::Format_RGB32);
        myImage.fill(0xffffff);
        unsigned char *p = code->data;
        for (int y = 0; y < code->width; y++)
        {
            for (int x = 0; x < code->width; x++)
            {
                myImage.setPixel(x + 4, y + 4, ((*p & 1) ? 0x0 : 0xffffff));
                p++;
            }
        }
        QRcode_free(code);

        ui->label_qrcode->setPixmap(QPixmap::fromImage(myImage).scaled(300, 300));

        //ui->labelYourAddr->setPlainText(uri);
    }
}
#endif

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    i6TotalCoins = balance + stake + unconfirmedBalance + immatureBalance;
	
	currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(BitcoinUnits::formatWithUnit(unit, balance));
    ui->labelStake->setText(BitcoinUnits::formatWithUnit(unit, stake));
    ui->labelUnconfirmed->setText(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance));
    ui->labelImmature->setText(BitcoinUnits::formatWithUnit(unit, immatureBalance));
	ui->labelTotal->setText(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance + immatureBalance));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    ui->labelImmature->setVisible(showImmature);
    ui->labelImmatureText->setVisible(showImmature);
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
    {	
		// Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(10);   //NUM_ITEMS
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter);
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
	if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    isBlockSynchronizing = fShow;
	ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}

extern string getAccount(string sAddr);
void OverviewPage::on_btn_showQrc_clicked()
{
#ifdef USE_QRCODE
    string sLab = getAccount(sDefWalletAddress);
        QString address = QString::fromStdString(sDefWalletAddress);  //QString address = index.data().toString(), 
		QString label = QString::fromStdString(sLab);  //label = index.sibling(index.row(), 0).data(Qt::EditRole).toString();

        QRCodeDialog *dialog = new QRCodeDialog(address, label, true, this);
        //if(optionsModel)
        //    dialog->setModel(optionsModel);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
#endif
}

extern double fVpn_Cny_price;
extern int bUpdate_price_now;
double fVpn_Cny_price_cache = 0;
int64_t i6TotalCoinsCache = 0;
bool OverviewPage::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::MouseButtonPress)
    {
        if( obj == ui->label_qrcode )
        {
            on_btn_showQrc_clicked();
			/*const QPixmap *pm = ui->label_qrcode->pixmap();
            if( pm != NULL )
            {
                QApplication::clipboard()->setPixmap( *pm );
                ovShowMsg();
            }*/
        }
        else if( (obj == ui->labelAddress) || (obj == ui->labelYourAddr) )
        {
			QApplication::clipboard()->setText( qDefWalletAddress );
            //ovShowMsg(qDefWalletAddress);
        }
		else if( obj == ui->wallet_overvie_Value )
		{
			bUpdate_price_now = 1;
			/*if( fVpn_Cny_price_cache > 0 )
			{
				string s = strprintf("%.4f", fVpn_Cny_price_cache);
				QString qq = QString::fromStdString(s) + " CNY";
				//ShowToastMsg(qq);
			}*/
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

void OverviewPage::syncVpnValue(int opt)
{
    if( (opt > 0) || (fVpn_Cny_price_cache != fVpn_Cny_price) || (i6TotalCoins != i6TotalCoinsCache) )
	{		
		//if( i6TotalCoins > 0 )
		{
		    i6TotalCoinsCache = i6TotalCoins;
		    fVpn_Cny_price_cache = fVpn_Cny_price;
		    if( fVpn_Cny_price_cache > 0 )
		    {
			    double d6 = (double)(i6TotalCoinsCache / COIN) * fVpn_Cny_price_cache;
			    string s = strprintf("%.3f", d6);
			    string sUtil = strprintf("%.3f", fVpn_Cny_price_cache);
			    QString qq = QString::fromStdString(s) + " (1 VASH = " + QString::fromStdString(sUtil) + ") CNY";
			    ui->wallet_overvie_Value->setText(qq);
		    }
		}
	}
	//ui->wallet_overvie_Value->setText(sValue);
}

