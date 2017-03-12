#include "webPage.h"
 
webPage::webPage(QObject *parent) :    QWebPage(parent)
{
}

bool webPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    if( type == 0 )	// NavigationTypeLinkClicked ������û����
	{	
        if(frame != mainFrame()){ //��������ڱ����ڵ�����
            emit openLink(request.url());//���ʹ��������ź�
        }else{
            emit loadLink(request.url());//���͵�������ź�
        }
		// http://www.haodaima.net/art/1967323
		return false;	// �������Է�ֹ����ִ�и����QWebPage::acceptNavigationRequest(frame,request, type), ��д�Ļ� �����γɳ���Ī������������������
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);
}