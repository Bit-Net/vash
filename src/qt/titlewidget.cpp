#include "titlewidget.h"
#include <QLabel>
#include "mypushbutton.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QLinearGradient>
#include <QIcon>
extern double DOB_DPI_RAT;

titleWidget::titleWidget(QString iCon, QString qss, QString hint, int btnWidth, bool bDrawDefault, QWidget *parent) :
    QWidget(parent)
{
    titleText=new QLabel;
    btnClose=new myPushButton(iCon);  //"close");
    connect(btnClose,SIGNAL(clicked()),this,SIGNAL(myClose()));
	//btnClose->setCursor(Qt::OpenHandCursor);    
    drawDefault = bDrawDefault;     btnClose->setToolTip(hint);
	if( btnWidth > 0 ){ btnClose->setFixedWidth(btnWidth); }
	titleText->setStyleSheet(qss);   //"font: bold 14px;");

    QHBoxLayout *layout=new QHBoxLayout;
    layout->addWidget(titleText,0,Qt::AlignLeft);
    layout->addStretch();
    layout->addWidget(btnClose,0,Qt::AlignRight);
    layout->setMargin(0);
    setLayout(layout);
    setFixedHeight(22 * DOB_DPI_RAT);
}

void titleWidget::paintEvent(QPaintEvent *pe)
{
    if( drawDefault )
	{
		// 画一条直线
		QPainter painter(this);             // 创建QPainter一个对象
		QPen pen;
		//pen.setStyle(Qt::DashLine);        
		pen.setColor(Qt::black);           // 设置画笔为黄色
		painter.setPen(pen);                // 设置画笔
		QPoint p2(rect().bottomRight().x(), rect().topLeft().y());
		painter.drawLine(rect().topLeft(), p2);
		QWidget::paintEvent(pe);
	}
	else{
    QLinearGradient linear(rect().topLeft(),rect().bottomRight());
    linear.setColorAt(0,QColor(227,207,87));
    linear.setColorAt(0.5,QColor(245,222,179));
    linear.setColorAt(1,QColor(189,252,201));

    QPainter painter(this);
    painter.setBrush(QBrush(linear));
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
	}
}

void titleWidget::setTitleText(QString title)
{
    titleText->setText(title);
}
