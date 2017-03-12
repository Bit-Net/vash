// Copyright (c) 2016 The Shenzhen BitChain Technology Company
// Copyright (c) 2016 Vpncoin development team, Bit Lee
#ifndef BITCHAINPAGE_H
#define BITCHAINPAGE_H
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
    class BitchainPage;
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

Q_DECLARE_METATYPE(std::string)

class BitchainPage : public QMainWindow
{
    Q_OBJECT

public:
    explicit BitchainPage(int activeTab, QWidget *parent = 0);
    ~BitchainPage();

    void setModel(WalletModel *model);
    void bitGridClick();
    int doBitChainCmd(QString sCmd);
    void setCurrentPage(int iPage);
    void gotoPage(int pageId, bool setChecked = false, QString qTxid = "");
protected:
    bool eventFilter(QObject *obj, QEvent *ev);

public slots:
    void freshPage(int pageId);
    void buyDomain(QString sDomain, QString sAddress, QString sCoin);
    void updateBitChainMsgOne(std::string txID);
    void freeBitChainMsgDlg(Widget* pDlg);
    void updateBitChainMsg(const CTransaction& tx, const string txMsg);
    void exportBitChain();

private slots:
    void addJavaScriptObject();
    void on_chain_tabWidget_currentChanged(int index);
    void on_ptEdit_loveWords_textChanged();
    void on_ptEdit_kidInfo_textChanged();
    void on_ptEdit_blessWords_textChanged();
    void on_ptEdit_eventTxt_textChanged();
    void on_ptEdit_prophesyTxt_textChanged();
    void on_ptEdit_contractInfo_textChanged();
    void on_ptEdit_medicalInfo_textChanged();
    void on_ptEdit_forumTxt_textChanged();
    void on_pBtn_subLoveToChain_clicked();
    void on_pBtn_subKidsInfoToChain_clicked();
    void on_pBtn_subBlessToChain_clicked();
    void on_pBtn_subEventToChain_clicked();
    void on_pBtn_subProphesyToChain_clicked();
    void on_pBtn_subContractToChain_clicked();
    void on_pBtn_subMedicalToChain_clicked();
    void on_pBtn_subForumToChain_clicked();
    void on_pBtn_regNick_clicked();

    void clearBtnClicked(int tabId);
    void on_pBtn_clearLove_clicked();
    void on_pBtn_clearKids_clicked();
    void on_pBtn_clearBless_clicked();
    void on_pBtn_clearEvent_clicked();
    void on_pBtn_clearProphesy_clicked();
    void on_pBtn_clearContract_clicked();
    void on_pBtn_clearMedical_clicked();
    void on_pBtn_clearForum_clicked();

    //void on_lnLabel_textChanged();
    //void on_lnMessage_textChanged();
    //void on_chkReqPayment_toggled(bool fChecked);
    //void on_chkKeyPass_toggled(bool fChecked);
    void submit_data_toChain(int tabId);
    void select_photo_toChain(int imgId, int mp3Id = -1);

    void gotoLoveChain();
    void gotoKidsChain();
    void gotoBlessChain();
    void gotoEventChain();
    void gotoProphesyChain();
    void gotoForumChain();
    void gotoContractChain();
    void gotoMedicalChain();
    void gotoRegNick();
    void updatePayForChain(int tabId);
    void createActions();
    void createToolBars();
    void handleWebviewClicked(QUrl url);
    void slotViewZoomIn();
    void slotViewZoomOut();

private:
    Ui::BitchainPage *ui;
    WalletModel *model;
    //QString toChainImgFile;
    int iNeedPay;
	int aTabId;
	QToolBar *toolbar;
    QAction *loveChainAction;
    QAction *kidsChainAction;
    QAction *blessChainAction;
    QAction *eventChainAction;
    QAction *prophesyChainAction;
    QAction *contractChainAction;
    QAction *medicalChainAction;
    QAction *forumChainAction;
    QAction *regNickAction;
    QAction *blockChainAASAction;
    QAction *exportBitChainAction;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();
    int getSendToChainDatSize(int tabId, int& imgSz, int& mp3Sz, int addNameSz = 1);
};

#endif // BITCHAINPAGE_H
