#include "item-menu.h"


/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2011, Arado Team
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
#include <QApplication>
#include <QDesktopServices>
#include <QStringList>
#include <QUrl>
#include <QClipboard>
#include <QCursor>
#include <QObject>
#include <QDebug>

namespace arado 
{

ItemMenu::ItemMenu (QWidget *parent)
  :parentWidget (parent),
   copyUrlAction (0),
   copyHashAction (0),
   mailUrlAction (0),
   mailHashAction (0)
{
  actions.append ( new QAction (QObject::tr("Copy URL"),parentWidget));
  copyHashAction = new QAction (QObject::tr("Copy Flashmark"),parentWidget);
  mailUrlAction = new QAction (QObject::tr("Mail Url"),parentWidget);
  mailHashAction = new QAction (QObject::tr("Mail Flashmark"),parentWidget);
  actions.append (copyUrlAction);
  actions.append (copyHashAction);
  actions.append (mailUrlAction);
  actions.append (mailHashAction);
}

QAction *
ItemMenu::BasicExec (const AradoUrl & url,
                               const QList<QAction *>  extraActions)
{
  QMenu menu (parentWidget);
  menu.addActions (actions);
  if (extraActions.size() > 0) {
    menu.addSeparator ();
  }
  for (int a=0; a < extraActions.size(); a++) {
    menu.addAction (extraActions.at (a));
  }

  QAction * select = menu.exec (QCursor::pos());
  if (select == copyUrlAction) {
    QClipboard *clip = QApplication::clipboard ();
    if (clip) {
      clip->setText (url.Url().toString());
    }
    return 0;
  } else if (select == copyHashAction) {
    QClipboard *clip = QApplication::clipboard ();
    if (clip) {
      clip->setText (url.Hash().toUpper());
    }
    return 0;
  } else if (select == mailUrlAction) {
    MailString (url.Url().toString());
    return 0;
  } else if (select == mailHashAction) {
    MailString (url.Hash());
    return 0;
  } else {
    return select;
  }
}

void
ItemMenu::MailString (const QString & message)
{
  QStringList mailBodytotal;
  QString blankline (QObject::tr("    "));
  QString webpageline (QObject::tr("Discover http://arado.sf.net Websearch - Syncs, shortens "
                          "and searches within (y)our URLs and Bookmarks."));
  mailBodytotal << message
                << blankline
                << webpageline ;
  QString urltext = QObject::tr("mailto:?subject=Aradofied\%20Web\%20Alert\%20for\%20you&body=%1")
                    .arg (mailBodytotal.join("\r\n"));
qDebug () << "ItemMeny try to mail " << urltext;
  QDesktopServices::openUrl (QUrl(urltext));
}

} // namespace

