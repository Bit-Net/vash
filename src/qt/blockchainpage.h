#ifndef BLOCKCHAINPAGE_H
#define BLOCKCHAINPAGE_H

#include <QFrame>
#include <QDialog>
#include <QImage>

namespace Ui {
    class BlockchainPage;
}
class OptionsModel;
class WalletModel;
class SendCoinsEntry;
class SendCoinsRecipient;

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class BlockchainPage : public QFrame
{
    Q_OBJECT

public:
    explicit BlockchainPage(int activeTab, QWidget *parent = 0);
    ~BlockchainPage();

    void setModel(WalletModel *model);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

private slots:
    void on_chain_tabWidget_currentChanged(int index);
    void on_ptEdit_loveWords_textChanged();
    void on_ptEdit_kidInfo_textChanged();
    void on_ptEdit_blessWords_textChanged();
    void on_ptEdit_eventTxt_textChanged();
    void on_pBtn_subLoveToChain_clicked();
    void on_pBtn_subKidsInfoToChain_clicked();
    void on_pBtn_subBlessToChain_clicked();
    void on_pBtn_subEventToChain_clicked();
    void on_pBtn_regNick_clicked();

    void clearBtnClicked(int tabId);
    void on_pBtn_clearLove_clicked();
    void on_pBtn_clearKids_clicked();
    void on_pBtn_clearBless_clicked();
    void on_pBtn_clearEvent_clicked();

    //void on_lnLabel_textChanged();
    //void on_lnMessage_textChanged();
    //void on_chkReqPayment_toggled(bool fChecked);
    //void on_chkKeyPass_toggled(bool fChecked);
    void submit_data_toChain(int tabId);
    void select_photo_toChain(int tabId);
    void updatePayForChain(int tabId);

private:
    Ui::BlockchainPage *ui;
    WalletModel *model;
    QString toChainImgFile;
    int iNeedPay;
	int aTabId;
};

#endif // BLOCKCHAINPAGE_H
