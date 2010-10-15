
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

namespace arado
{

HttpClientReply::HttpClientReply (QNetworkReply   *nr,
                                  HttpRequestType  hrt,
                                  HttpDataType     hdt)
  :netReply (nr),
   reqType (hrt),
   dataType (hdt)
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
HttpClientReply::Get (QNetworkAccessManager * nam, 
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt)
{
  if (nam) {
    QNetworkReply * reply = nam->get (req);
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt);
    }
  }
  return 0;
}

HttpClientReply *
HttpClientReply::Put (QNetworkAccessManager * nam, 
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              QIODevice             * data)
{
  if (nam) {
    QNetworkReply * reply = nam->put (req, data);
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt);
    }
  }
  return 0;
}

HttpClientReply *
HttpClientReply::Put (QNetworkAccessManager * nam, 
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              QByteArray            & data)
{
  if (nam) {
    QNetworkReply * reply = nam->put (req, data);
    if (reply) {
      return new HttpClientReply (reply, hrt, hdt);
    }
  }
  return 0;
}


} // namespace

