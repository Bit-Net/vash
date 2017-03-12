#include "mycookiejar.h"

 

MyCookieJar::MyCookieJar(QObject *parent)

    : QNetworkCookieJar(parent)

{

 

}

 

MyCookieJar::~MyCookieJar()

{

}

 

QList<QNetworkCookie> MyCookieJar::mycookies()

{

    return  this->allCookies();

}

 

void MyCookieJar::setCookies(const QList<QNetworkCookie>& cookieList)

{

    if(this == NULL)

        return;

    this->setAllCookies(cookieList);

}