// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee
#ifndef ZKPDIALOG_H
#define ZKPDIALOG_H
#include <deque>
#include <boost/array.hpp>
#include <boost/foreach.hpp> 
#include <stdint.h>
#include <string>
#include <QFrame>
#include <QDialog>
#include <QImage>
#include <QUrl>
#include <QMainWindow>
#include <QVariant>
#include <QMetaType>

using namespace std;
using namespace boost;

namespace Ui {
    class ZkpForm;
}
class OptionsModel;
class WalletModel;
class SendCoinsEntry;
class SendCoinsRecipient;
class CTransaction;
class Widget;

QT_BEGIN_NAMESPACE
class QUrl;
class QToolBar;
QT_END_NAMESPACE


class ZkpForm : public QWidget
{
    Q_OBJECT

public:
    explicit ZkpForm(QWidget *parent = 0);
    ~ZkpForm();

    //void setModel(WalletModel *model);
	bool expZkpBurnTx(const CTransaction& tx, bool checkExists, QString& qRztZkpHash);
	bool expZkpMintTx(const CTransaction& tx, QString& qRztZkpMintInfo);
	void firstEnter();
	void setMaxValues();
protected:
    //bool eventFilter(QObject *obj, QEvent *ev);

public slots:
    //void exportBitChain();

private slots:
    //void addJavaScriptObject();
	void on_pb_newPubKey_clicked();
	void on_pb_g_new_clicked();
    void on_pb_burn_clicked();
    void on_pb_genKey_clicked();
    void on_pb_Mint_clicked();
    //void on_chain_tabWidget_currentChanged(int index);
    void on_le_zkpForMintKey_textChanged();
    //void on_chkKeyPass_toggled(bool fChecked);

private:
    Ui::ZkpForm *ui;
    WalletModel *model;
    //QString toChainImgFile;
};

#endif // BITCHAINPAGE_H
