#include "mainframe.h"
#include "ui_mainframe.h"
//#include "menubar.h"
#include <QMainWindow>
#include "bitcoingui.h"

MainFrame::MainFrame(QWidget *parent)
: QFrame(parent)
, ui(new Ui::MainFrame)
{
    ui->setupUi(this);
	mainGui = NULL;  //(BitcoinGUI*)parent;
	isMyMouseEvent = false;
    //this->setMouseTracking(true);  
    //ui->main_tab_nav_bar->setCornerWidget(m_menuBar,Qt::BottomRightCorner);
}

MainFrame::~MainFrame()
{
    delete ui;
}

QWidget* MainFrame::getTitleWidget()
{
    return ui->titleWidget;
}

QWidget* MainFrame::getToolBarWin()
{
   return ui->toolBarWin;
}

QFrame* MainFrame::getMainFrame()
{
   return ui->Main;
}

void MainFrame::mousePressEvent(QMouseEvent *e)
{
	/*isMyMouseEvent = true;
	if( mainGui != NULL ){ mainGui->mousePressEvent(e); }
	isMyMouseEvent = false; */
	QFrame::mousePressEvent(e);
}
void MainFrame::mouseMoveEvent(QMouseEvent *event)
{
	isMyMouseEvent = true;
	if( mainGui != NULL ){ mainGui->mouseMoveEvent(event); }
	isMyMouseEvent = false;
	QFrame::mouseMoveEvent(event);
}

void MainFrame::mouseReleaseEvent(QMouseEvent *event)
{
	isMyMouseEvent = true;
	if( mainGui != NULL ){ mainGui->mouseReleaseEvent(event); }
	isMyMouseEvent = false;
	QFrame::mouseReleaseEvent(event);
}

void MainFrame::leaveEvent(QEvent *event)
{
	//QMainWindow::leaveEvent(event);
	if( mainGui != NULL )
	{   
		if ( mainGui->isMax_ || mainGui->isLeftPressDown ) { return; }
		{
			//if(dir != NONE) {
				//this->releaseMouse();
				mainGui->setCursor(QCursor(Qt::ArrowCursor));
			//}
		}
	}
}

/*void MainFrame::addTab(QWidget* widget,QString name)
{
    ui->main_tab_nav_bar->addTab(widget,name);
}

void MainFrame::setActiveTab(QWidget* widget)
{
    ui->main_tab_nav_bar->setCurrentWidget(widget);
}

TabBar* MainFrame::getTabBar()
{
    return ui->main_tab_nav_bar;
}

MenuBar* MainFrame::getMenuBar()
{
    return m_menuBar;
}

LockBar* MainFrame::getLockBar()
{
    return ui->lockBar;
}*/
