#include "webPage.h"
 
webPage::webPage(QObject *parent) :    QWebPage(parent)
{
}

bool webPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if( type == 0 )	// NavigationTypeLinkClicked 如果是用户点击
	{	
        if(frame != mainFrame()){ //如果不是在本窗口的连接
            emit openLink(request.url());//发送打开新连接信号
        }else{
            emit loadLink(request.url());//发送点击连接信号
        }
		// http://www.haodaima.net/art/1967323
		return false;	// 这样可以防止两次执行父类的QWebPage::acceptNavigationRequest(frame,request, type), 不写的话 “会形成程序莫名其妙的随机崩溃。”
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}