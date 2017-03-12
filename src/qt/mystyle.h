#ifndef MYSTYLE_H
#define MYSTYLE_H
#include <QWindowsStyle>
#include<QtGui>
#include<QPixmap>
#include<QIcon>
#include<QPalette>
#include <QStyle>

QT_USE_NAMESPACE

class MyStyle : public QWindowsStyle
{
   Q_OBJECT
public:
    MyStyle();
    void init();
/*    {
        mySize = 10;
		m_toolBarVExtendButtonIcon = QPixmap(":icons/tb_arrow");
		if( DOB_DPI_RAT > 1 )
		{
			mySize = mySize * DOB_DPI_RAT;
			m_toolBarVExtendButtonIcon = setDevicePixelRatio(m_toolBarVExtendButtonIcon, DOB_DPI_RAT);
		}
    } */
// int pixelMetric(PixelMetric pm, const QStyleOption* option, const QWidget* widget) const;
    int pixelMetric(PixelMetric pm, const QStyleOption *option, const QWidget *widget) const;
/*    {
        if (pm == QStyle::PM_ToolBarExtensionExtent) 
        return mySize; // width of a toolbar extension button in a horizontal toolbar and the height of the button in a vertical toolbar
        return QStyle::pixelMetric(pm, option, widget);
    } */
protected Q_SLOTS:
    QIcon standardIconImplementation(StandardPixmap standardIcon, const QStyleOption * option = 0, const QWidget * widget = 0) const;
/*    { 
        switch (standardIcon) 
        {
            //case QStyle::SP_ToolBarHorizontalExtensionButton : return m_toolBarHExtendButtonIcon;
            case QStyle::SP_ToolBarVerticalExtensionButton :     return QIcon(m_toolBarVExtendButtonIcon);
        }
        return QStyle::standardIconImplementation(standardIcon, option, widget);
    } */
private:
    int mySize;
	QPixmap m_toolBarVExtendButtonIcon;

};

#endif