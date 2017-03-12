#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QPushButton>

class myPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit myPushButton(QWidget *parent = 0);
    explicit myPushButton(QString iconStr,QWidget *parent=0);
    
signals:
    
public slots:

private:
    void setBkPalette(int transparency);//����͸����
protected:
    void enterEvent(QEvent *);//�������¼�
    void leaveEvent(QEvent *);//�뿪
    void mousePressEvent(QMouseEvent *e);//���
    void mouseReleaseEvent(QMouseEvent *e);//�ͷ�
};

#endif // MYPUSHBUTTON_H
