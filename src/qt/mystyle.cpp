#include "mystyle.h"
//#include "mywidgets.h"
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QRadioButton>
#include <QString>
#include <QStyleOption>
#include <QtGui>
#include <QPixmap>
#include <QIcon>
#include <QWidget>
#include <QPalette>
#include <QStyle>

extern QPixmap setDevicePixelRatio(QPixmap orig, double x);
extern QPixmap getPixelRatioPixmap(QString icon, Qt::AspectRatioMode arOpt = Qt::KeepAspectRatio);
extern double DOB_DPI_RAT;

MyStyle::MyStyle()
    : QWindowsStyle()
{
    //Q_INIT_RESOURCE(shared);
	this->init();
}

void MyStyle::init()
{
        mySize = 32;
		m_toolBarVExtendButtonIcon = getPixelRatioPixmap(":/icons/tb_arrow");  //IgnoreAspectRatio  QPixmap(":/icons/tb_arrow");
		if( DOB_DPI_RAT > 1 )
		{
			mySize = mySize * DOB_DPI_RAT;
			//m_toolBarVExtendButtonIcon = setDevicePixelRatio(m_toolBarVExtendButtonIcon, DOB_DPI_RAT);
		}
}

int MyStyle::pixelMetric(PixelMetric pm, const QStyleOption *option, const QWidget *widget) const
{
        if (pm == QStyle::PM_ToolBarExtensionExtent)  return mySize; // width of a toolbar extension button in a horizontal toolbar and the height of the button in a vertical toolbar
        return QWindowsStyle::pixelMetric(pm, option, widget);
}

QIcon MyStyle::standardIconImplementation(StandardPixmap standardIcon, const QStyleOption * option, const QWidget * widget) const 
{ 
    if( standardIcon == QStyle::SP_ToolBarVerticalExtensionButton ){ return QIcon(m_toolBarVExtendButtonIcon); }
	/*switch (standardIcon) 
    {
            //case QStyle::SP_ToolBarHorizontalExtensionButton : return m_toolBarHExtendButtonIcon;
            case QStyle::SP_ToolBarVerticalExtensionButton :     return QIcon(m_toolBarVExtendButtonIcon);
    }*/
    else return QWindowsStyle::standardIconImplementation(standardIcon, option, widget);
}

