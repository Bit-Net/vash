#pragma once  

#include <QMainWindow>
#include <QFrame>  
  
class QToolButton;  
class CustomFrame : public QFrame   //QFrame   QMainWindow
{  
    Q_OBJECT  
public:  
    explicit CustomFrame(QWidget *contentWidget, const QString &title);  
  
public slots:  
    void slotShowSmall();  
    void slotShowMaxRestore();  
  
protected:  
    void mousePressEvent(QMouseEvent *);  
    void mouseMoveEvent(QMouseEvent *);  
    void mouseReleaseEvent(QMouseEvent *);  
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);  
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *event);
  
private:  
    bool isMax_;  
    bool isPress_;  
    QPoint startPos_;  
    QPoint clickPos_;  
    QWidget *contentWidget_;  
    QPixmap maxPixmap_;  
    QPixmap restorePixmap_;  
    QToolButton *maxButton_;  
};  