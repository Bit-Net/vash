#ifndef MYCOOKIEJAR_H

#define MYCOOKIEJAR_H

 

#include <QNetworkCookieJar>

 

class MyCookieJar : public QNetworkCookieJar

{

    Q_OBJECT

 

public:

    explicit MyCookieJar(QObject *parent = 0);

    ~MyCookieJar();

    QList<QNetworkCookie> mycookies();

    void setCookies(const QList<QNetworkCookie>& cookieList);

private:

     

};

 

#endif // MYCOOKIEJAR_H