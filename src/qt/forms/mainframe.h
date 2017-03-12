#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QFrame>
#include <QMainWindow>
#include "bitcoingui.h"

namespace Ui
{
    class MainFrame;
}

class MenuBar;
class TabBar;
class LockBar;
class BitcoinGUI;

/*enum Direction{
    UP = 0,
    DOWN=1,
    LEFT,
    RIGHT,
    LEFTTOP,
    LEFTBOTTOM,
    RIGHTBOTTOM,
    RIGHTTOP,
    NONE
};*/

class MainFrame : public QFrame
{
    Q_OBJECT

public:
    Ui::MainFrame *ui;
	BitcoinGUI *mainGui;
	bool isMyMouseEvent;
    /*bool isLeftPressDown;
    QPoint dragPosition;
    Direction dir;
    bool isMax_;  
    bool isPress_;  */

    explicit MainFrame(QWidget *parent = 0);
    ~MainFrame();

    QWidget* getTitleWidget();
	QWidget* getToolBarWin();
	QFrame* getMainFrame();
    //MenuBar* getMenuBar();

    void mousePressEvent(QMouseEvent *e);  
    void mouseMoveEvent(QMouseEvent *event);  
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    //bool nativeEvent(const QByteArray & eventType, void * message, long * result);  
    //void region(const QPoint &cursorPoint);
private:
    //Ui::MainFrame *ui;
    //MenuBar* m_menuBar;
};

#endif // MAINFRAME_H
