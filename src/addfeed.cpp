/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Arado Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#include "addfeed.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDomDocument>
#include "networkaccessmanager.h"

namespace arado
{

AddRssFeed::AddRssFeed(QObject *parent) :
  QObject(parent)
{
  qnam=new NetworkAccessManager(parent);
}

void
AddRssFeed::httpFinished (QNetworkReply *reply)
{
  QDomDocument dom;
  dom.setContent(reply);
  QDomNodeList nodeList=dom.elementsByTagName("item");
  for (unsigned int n=0; n<nodeList.length(); n++) {
    QDomNode node=nodeList.item(n);
    QString title,link,category;
    for (unsigned int c=0; c<node.childNodes().length(); c++) {
      QDomNode child=node.childNodes().item(c);
      if(child.nodeName().toLower()=="title") {
        title=child.firstChild().nodeValue();
      } else if(child.nodeName().toLower()=="link") {
        link=child.firstChild().nodeValue();
      } else if(child.nodeName().toLower()=="category") {
        category=child.firstChild().nodeValue();
      }
    }
    if (title.length()>0 && link.length()>0) {
      AradoUrl  newurl;
      newurl.SetUrl (link);
      newurl.SetDescription(title);
      //newurl.SetKeywords (category);
      if(newurl.IsValid ()) {
        db->AddUrl (newurl);
      }
    }
  }

  this->reply->deleteLater();
  this->reply=NULL;
}

void
AddRssFeed::AddFeedUrl (QString urlText)
{
  QUrl feedUrl=QUrl (urlText, QUrl::TolerantMode);

  request=new QNetworkRequest(feedUrl);

  reply=qnam->get(*request);

  connect (qnam, SIGNAL(finished(QNetworkReply *)),
          this, SLOT(httpFinished(QNetworkReply *)));
}

}
