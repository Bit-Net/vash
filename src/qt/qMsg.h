#ifndef QMSG_H
#define QMSG_H

#include <QWidget>

class titleWidget;
class myPushButton;
class QLabel;
class QTimer;
class BitchainPage;
class BitcoinGUI;

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void showAsQQ();//在右下角显示
    QString url;
public slots:
    void setMsg(QString title,QString content,QString url, BitchainPage* bp = NULL, int iType = 0);
private slots:
    void openWeb();
    void saveBitChain();

    void myMove();
    void myStay();
    void qMsgClose();
    //void danChu();
    
private:
    titleWidget *titleW;
    QLabel *content;
    myPushButton *btnLook;
    //QString url;
	int nType, timeCount, beginY;
	double tran;
	BitchainPage* bitChainP;

    QPoint normalPoint;//完全显示时左上角位置
    QRect deskRect;

    QTimer *timerShow;//平滑显示的定时器
    QTimer *timerStay;//停留时间的定时器 10s
    QTimer *timerClose;//关闭淡出的定时器

protected:
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // QMSG_H
