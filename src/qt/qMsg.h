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

    void showAsQQ();//�����½���ʾ
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

    QPoint normalPoint;//��ȫ��ʾʱ���Ͻ�λ��
    QRect deskRect;

    QTimer *timerShow;//ƽ����ʾ�Ķ�ʱ��
    QTimer *timerStay;//ͣ��ʱ��Ķ�ʱ�� 10s
    QTimer *timerClose;//�رյ����Ķ�ʱ��

protected:
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *obj, QEvent *ev);
};

#endif // QMSG_H
