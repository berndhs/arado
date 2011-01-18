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
  copyUrlAction = new QAction (QObject::tr("Copy URL"),parentWidget);
  copyHashAction = new QAction (QObject::tr("Copy Flashmark"),parentWidget);
  mailUrlAction = new QAction (QObject::tr("Mail Url"),parentWidget);
  mailHashAction = new QAction (QObject::tr("Mail Flashmark"),parentWidget);
  standardActions.append (copyUrlAction);
  standardActions.append (copyHashAction);
  standardActions.append (mailUrlAction);
  standardActions.append (mailHashAction);
}

QAction *
ItemMenu::MenuBasic (const AradoUrl & url,
                               const QList<QAction *>  extraActions)
{
  QMenu menu (parentWidget);
  if (extraActions.size() > 0) {
    menu.addActions (extraActions);
    menu.addSeparator ();
  }
  menu.addActions (standardActions);

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
ItemMenu::MenuMail (const AradoUrl & aurl)
{
  QAction * mailAction = new QAction (QObject::tr("Mail All Details"),
                                parentWidget);
  QList <QAction*> alist;
  alist << mailAction;
  QAction * choice = MenuBasic (aurl, alist);
  if (choice == 0) {
    return;
  }
  QStringList details;
  details << QObject::tr ("Url: ") + aurl.Url().toString();
  details << QObject::tr ("Arado-Flashmark: " ) + aurl.Hash().toUpper();
  details << QObject::tr ("Title: ") + aurl.Description();
  details << QObject::tr ("Keywords: ");
  details << aurl.Keywords ();
  if (choice == mailAction) {
    MailString (details.join ("\r\n"));
  }
}

void
ItemMenu::MenuCopy (const AradoUrl & aurl)
{
  QAction * allAction = new QAction (QObject::tr("Copy All Details"),
                                parentWidget);
  QAction * keyAction = new QAction (QObject::tr("Copy Keywords"),
                                parentWidget);
  QList <QAction*> alist;
  alist << allAction << keyAction;
  QAction * choice = MenuBasic (aurl, alist);
  if (choice == 0) {
    return;
  }
  QClipboard * clip = QApplication::clipboard ();
  if (clip == 0) {
    return;
  }
  if (choice == keyAction) {
    clip->setText (aurl.Keywords().join ("\n"));
  } else if (choice == allAction) {
    QStringList details;
    details << QObject::tr ("Url: ") + aurl.Url().toString();
    details << QObject::tr ("Arado-Flashmark: " ) + aurl.Hash().toUpper();
    details << QObject::tr ("Title: ") + aurl.Description();
    details << QObject::tr ("Keywords: ");
    details << aurl.Keywords ();
    clip->setText (details.join ("\n"));
  }
}
void
ItemMenu::MenuKeywords (const AradoUrl & aurl)
{
  QAction * mailAction = new QAction (QObject::tr("Mail All Keywords"),
                                parentWidget);
  QAction * copyAction = new QAction (QObject::tr("Copy All Keywords"),
                                parentWidget);
  QList <QAction*> alist;
  alist << copyAction << mailAction;
  QAction * choice = MenuBasic (aurl, alist);
  if (choice == 0) {
    return;
  }
  QStringList details;
  details << aurl.Keywords ();
  if (choice == mailAction) {
    MailString (details.join ("\r\n"));
  } else if (choice == copyAction) {
    if (choice == copyAction) {
      QClipboard *clip = QApplication::clipboard ();
      if (clip) {
        clip->setText (details.join ("\n"));
      }
    }
  }
}

void
ItemMenu::MenuCrawl (const AradoUrl & aurl)
{
  QAction * copyAction = new QAction (QObject::tr("Crawl Coming Soon!"),
                                parentWidget);
  QList <QAction*> alist;
  alist << copyAction;
  MenuBasic (aurl, alist);
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

