#ifndef BITCOINGUI_H
#define BITCOINGUI_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include <stdint.h>

#ifdef USE_WEBKIT
#include <QUrl>
#endif

#include <QMouseEvent>
#include <QPoint>
#include <QCursor>
#include <QRect>
#define PADDING 2

class TransactionTableModel;
class ClientModel;
class WalletModel;
class TransactionView;
class OverviewPage;
//class BlockchainPage;
class BitchainPage;
class AddressBookPage;
class SendCoinsDialog;
class SignVerifyMessageDialog;
class Notificator;
class RPCConsole;
//class CustomFrame;
class QToolButton;
class MultisigDialog;
class ZkpForm;

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QTableView;
class QAbstractItemModel;
class QModelIndex;
class QProgressBar;
class QStackedWidget;
class QUrl;
QT_END_NAMESPACE

//extern CustomFrame* mainFrame;

enum Direction{
    UP = 0,
    DOWN=1,
    LEFT,
    RIGHT,
    LEFTTOP,
    LEFTBOTTOM,
    RIGHTBOTTOM,
    RIGHTTOP,
    NONE
};

/**
  Bitcoin GUI main class. This class represents the main window of the Bitcoin UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class BitcoinGUI : public QMainWindow
{
    Q_OBJECT
public:
    OverviewPage *overviewPage;
    BitchainPage *bitchainPage;
	
	explicit BitcoinGUI(QWidget *parent = 0);
    ~BitcoinGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel *walletModel);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
	void resizeEvent(QResizeEvent *event);
	void paintEvent( QPaintEvent* pe );
	
public:
    ClientModel *clientModel;
	QWidget *BitNetPage;
	QLabel *titleLabel;
	ZkpForm *zkpForm;
    bool isMax_;    
    bool isLeftPressDown;
    QPoint dragPosition;
    Direction dir;
    bool isPress_;  
    //void closeEvent(QCloseEvent *event);

    void mousePressEvent(QMouseEvent *e);  
    void mouseMoveEvent(QMouseEvent *event);  
    void mouseReleaseEvent(QMouseEvent *event);
    //void leaveEvent(QEvent *event);	
    //void enterEvent(QEvent *e);
    //bool nativeEvent(const QByteArray & eventType, void * message, long * result);  
    void region(const QPoint &cursorPoint);  
    void ShowQrCodeFromStr(std::string str);
    void gotoBitchainPageAndSetAppActive();
private:
    //ClientModel *clientModel;
    WalletModel *walletModel;

    QStackedWidget *centralWidget;
	
    QPoint startPos_;  
    QPoint clickPos_;  
    QWidget *contentWidget_;  
    QPixmap maxPixmap_;  
    QPixmap restorePixmap_;  
    QToolButton *maxButton_;  

    //OverviewPage *overviewPage;
    QWidget *transactionsPage;
	//QWidget *BitNetPage;
    AddressBookPage *addressBookPage;
    AddressBookPage *receiveCoinsPage;
    SendCoinsDialog *sendCoinsPage;
    SignVerifyMessageDialog *signVerifyMessageDialog;
    MultisigDialog *multisigPage;

    QLabel *blockchaincompressIcon;
    QLabel *labelEncryptionIcon;
    QLabel *labelStakingIcon;
    QLabel *labelConnectionsIcon;
    QLabel *labelBlocksIcon;
	QLabel *labelBitNetIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;

    QMenuBar *appMenuBar;
    QAction *overviewAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *addressBookAction;
    QAction *signMessageAction;
    QAction *verifyMessageAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *exportAction;
    QAction *encryptWalletAction;
    QAction *backupWalletAction;
    QAction *restoreWalletAction;
    QAction *setChainFolderAction;
    QAction *changePassphraseAction;
    QAction *unlockWalletAction;
    QAction *lockWalletAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;
	QAction *bitnetAction;
	QAction *bitnetWebAction;
	QAction *bitnetBroAction;
	QAction *vpnAction;
	QAction *lotteryAction;
    QAction *chequeAction;
    QAction *bonusAction;
    QAction *bitnetSetAction;
    QAction *loadQssAction;
    QAction *openBMarketAction;
    QAction *bitchainAction;
    QAction *multisigAction;
    QAction *saveRedeemScriptAction;
    QAction *zkpformAction;
    QAction *exportBitChainTxAction;

#ifdef USE_WEBKIT
    QWidget *BitNetWebPage;
	QWidget *SupNetPage;
    QAction *supnetAction;
    QAction *runSupnetAction;
#endif	

    QSystemTrayIcon *trayIcon;
    Notificator *notificator;
    TransactionView *transactionView;
    RPCConsole *rpcConsole;

    QMovie *syncIconMovie;
	QMovie *BitNetMovie;

    uint64_t nWeight;

    /** Create the main UI actions. */
    void createActions();
    /** Create the menu bar and sub-menus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray (notification) icon */
    void createTrayIcon();

    /** Save window size and position */
    void saveWindowGeometry();
	void SyncBitNetFormGeometry();
    /** Restore window size and position */
    void restoreWindowGeometry();
public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void processClipboardChange();
    void setNumBlocks(int count, int nTotalBlocks);
    void setNumBlocks2(int count, int nTotalBlocks);
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);

    /** Notify the user of an error in the network or transaction handling code. */
    void error(const QString &title, const QString &message, bool modal);
    /** Asks the user whether to pay the transaction fee or to cancel the transaction.
       It is currently not possible to pass a return value to another thread through
       BlockingQueuedConnection, so an indirected pointer is used.
       https://bugreports.qt-project.org/browse/QTBUG-10440

      @param[in] nFeeRequired       the required fee
      @param[out] payFee            true to pay the fee, false to not pay the fee
    */
    void askFee(qint64 nFeeRequired, bool *payFee);
    int handleURI(QString strURI, int bTip, bool bNotify = true);

    /** Switch to overview (home) page */
    void gotoOverviewPage();
	void gotoSendCoinsPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to address book page */
    void gotoAddressBookPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to send coins page */
    //void gotoSendCoinsPage();
	void gotoBitNetClicked(int idx = 0);
	void openBitNetBroClicked();
    void gotoVpnClicked();
    void gotoLotteryClicked();
    void gotoChequeClicked();
    void gotoBonusClicked();
    void gotoBitNetSetClicked();
	void setAdBonusIcon(int idx, int ico);
	void animIconTimer();
	void clipTimer();
	void appQuit();
	void loadQss(QString qssFile);
	void loadQssClicked();
    void slotShowSmall();  
    void slotShowMaxRestore();  
    void bmarketClicked();
    void gotoBitchainPage();
    void gotoMultisigPage();
    void gotoZkpForm();
    void saveRedeemScript();
    void exportBitChainTx();
#ifdef USE_WEBKIT
    void BitNet_populateJavaScriptWindowObject();
	void BitNet_populateJavaScriptWindowObject2();
	void BitNet_populateJavaScriptWindowObject3();
	//void handleWebviewClicked(QUrl url);
	void gotoBitNetWebClicked();
	void bitNetF5Clicked();
	void BitNet_finishLoading(bool noErr);	
    void gotoSupNetClicked();
    void runSupNetClicked();
    void supNetF5Clicked();
    void SupNet_finishLoading(bool noErr);
#endif

    void showNormalIfMinimized(bool fToggleHidden = false);	
#ifdef USE_BITNET
	int ShowOrHideGui();
#endif
#ifdef USE_WEBKIT
	int SendChatMessage(QString Msgs, int WebId, int EventId);
	void linkUrl(QUrl url);
    void openUrl(QUrl url);
#endif

private slots:
    /** Show Sign/Verify Message dialog and switch to sign message tab */
    void gotoSignMessageTab(QString addr = "");
    /** Show Sign/Verify Message dialog and switch to verify message tab */
    void gotoVerifyMessageTab(QString addr = "");

    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();
	
#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif
    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void incomingTransaction(const QModelIndex & parent, int start, int end);
    /** Encrypt the wallet */
    void encryptWallet(bool status);
    /** Backup the wallet */
    void backupWallet();
    void restoreWallet();
    void setChainFolder();
    /** Change encrypted wallet passphrase */
    void changePassphrase();
    /** Ask for passphrase to unlock wallet temporarily */
    void unlockWallet();

    void lockWallet();

    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    //void showNormalIfMinimized(bool fToggleHidden = false);
    /** simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    void updateWeight();
    void updateStakingIcon();
};

#endif
