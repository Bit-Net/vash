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
    setBkPalette(0);//设置背景完全透明
    setFlat(true);
    setAutoFillBackground(true);
}

void myPushButton::setBkPalette(int transparency)//设置背景透明度
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
    emit clicked();//必须写上，否则不会发出clicked信号
}
