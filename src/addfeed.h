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

#ifndef ADDFEED_H
#define ADDFEED_H

#include <QObject>
#include <QNetworkReply>
#include "db-manager.h"

namespace arado
{

class AddRssFeed : public QObject
{
    Q_OBJECT
public:
    explicit AddRssFeed(QObject *parent = 0);

    void SetDB (DBManager *dbm) { db = dbm; }
    void AddFeedUrl (QString url);

signals:

public slots:
    virtual void httpFinished (QNetworkReply *reply);

private:

    QNetworkAccessManager *qnam;
    DBManager             *db;
    QNetworkReply         *reply;
    QNetworkRequest       *request;
};

}

#endif // ADDFEED_H
