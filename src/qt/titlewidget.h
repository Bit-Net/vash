#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QWidget>

class QLabel;
class myPushButton;

class titleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit titleWidget(QString iCon, QString qss, QString hint, int btnWidth, bool bDrawDefault = false, QWidget *parent = 0);
    
signals:
    void myClose();

public:
    void setTitleText(QString title);
private:
    QLabel *titleText;//标题文字
	bool drawDefault;
    myPushButton *btnClose;//标题关闭按钮

protected:
    void paintEvent(QPaintEvent *pe);
};

#endif // TITLEWIDGET_H
