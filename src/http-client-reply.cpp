
#include "http-client-reply.h"

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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>
#include <QDebug>
#include "networkaccessmanager.h"

namespace arado
{

HttpClientReply::HttpClientReply (QNetworkReply   *nr,
                                  HttpRequestType  hrt,
                                  HttpDataType     hdt,
                                  const QString   &nick)
  :netReply (nr),
   reqType (hrt),
   dataType (hdt),
   peerNick (nick)
{
}

HttpClientReply::HttpClientReply ()
  :netReply (0),
   reqType (HRT_None),
   dataType (HDT_None)
{
}

HttpClientReply::~HttpClientReply ()
{
  if (netReply) {
    netReply->deleteLater ();
  }
}

HttpClientReply *
HttpClientReply::Get (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick)
{
  if (nam) {
    QNetworkReply * reply = nam->get (req);
    qDebug () << " sent GET to url " << req.url();
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt, nick);
    }
  }
  return 0;
}

HttpClientReply *
HttpClientReply::Put (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick,
              QIODevice             * data)
{
  if (nam) {
    QNetworkReply * reply = nam->put (req, data);
    qDebug () << " sent PUT1 to url " << req.url();
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt, nick);
    }
  }
  return 0;
}

HttpClientReply *
HttpClientReply::Put (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick,
              QByteArray            & data)
{
  if (nam) {
    QNetworkReply * reply = nam->put (req, data);
    qDebug () << " sent PUT2 to url " << req.url();
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt, nick);
    }
  }
  return 0;
}


} // namespace

