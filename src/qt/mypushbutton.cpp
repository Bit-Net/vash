#include "mypushbutton.h"
#include <QPalette>
#include <QPixmap>

myPushButton::myPushButton(QWidget *parent) :
    QPushButton(parent)
{
}
myPushButton::myPushButton(QString iconStr, QWidget *parent):QPushButton(parent)
{
    QPixmap pixmap(":/icons/"+iconStr);
    setIcon(QIcon(pixmap));
    setIconSize(pixmap.size());
    resize(pixmap.size());
    setBkPalette(0);//���ñ�����ȫ͸��
    setFlat(true);
    setAutoFillBackground(true);
}

void myPushButton::setBkPalette(int transparency)//���ñ���͸����
{
   QPalette palette;
   palette.setBrush(QPalette::Button,QBrush(QColor(255,255,255,transparency)));
   setPalette(palette);
}
void myPushButton::enterEvent(QEvent *)
{
    setBkPalette(120);
}
void myPushButton::leaveEvent(QEvent *)
{
    setBkPalette(0);
}
void myPushButton::mousePressEvent(QMouseEvent *e)
{
    setBkPalette(150);
}
void myPushButton::mouseReleaseEvent(QMouseEvent *e)
{
    setBkPalette(120);
    emit clicked();//����д�ϣ����򲻻ᷢ��clicked�ź�
}
