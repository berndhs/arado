
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

#ifndef UPNPCLIENT_H
#define UPNPCLIENT_H

#include <QObject>

#include "miniupnp/miniupnpc.h"

namespace arado {

class UPnPClient : public QObject
{
    Q_OBJECT
public:
    explicit UPnPClient(QObject *parent = 0);
    ~UPnPClient();
    bool Init();
    bool RemoveRedirection();
    bool ChangeRedirection(const char * eport);
    bool AddRedirection(const char * iport, const char * eport);

signals:

public slots:

private:
    char lanaddr[64];	/* my ip address on the LAN */
    bool hasUPnP;
    char eport[64];
    struct UPNPUrls urls;
    struct IGDdatas data;

};

};

#endif // UPNPCLIENT_H

