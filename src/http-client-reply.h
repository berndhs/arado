#ifndef HTTP_CLIENT_REPLY_H
#define HTTP_CLIENT_REPLY_H

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

#include "http-types.h"
#include <QUrl>
#include "networkaccessmanager.h"

class NetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QByteArray;
class QIODevice;

namespace arado
{

class HttpClientReply
{
public:

  ~HttpClientReply ();

  static HttpClientReply * Get (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick);
  static HttpClientReply * Put (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick,
              QIODevice             * data);
  static HttpClientReply * Put (NetworkAccessManager * nam,
              QNetworkRequest       & req, 
              HttpRequestType         hrt,
              HttpDataType            hdt,
              const QString         & nick,
              QByteArray            & data);

  QNetworkReply    *Reply ()    const   { return netReply; }
  HttpRequestType   Type ()     const   { return reqType; }
  HttpDataType      DataType () const   { return dataType; }
  QUrl              OrigUrl ()  const   { return origUrl; }
  QString           PeerNick () const   { return peerNick; }

  void              SetOrigUrl (const QUrl & url) { origUrl = url; }
              

private:

  HttpClientReply ();
  HttpClientReply (QNetworkReply     *nr,
                   HttpRequestType    hrt,
                   HttpDataType       hdt,
                   const QString     &nick);

  QNetworkReply     * netReply;
  HttpRequestType     reqType;
  HttpDataType        dataType;
  QUrl                origUrl;
  QString             peerNick;

};

} // namespace

#endif
