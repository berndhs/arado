#ifndef ARADO_ITEM_MENU_H
#define ARADO_ITEM_MENU_H


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
#include "arado-url.h"
#include <QList>
#include <QAction>
#include <QMenu>
#include <QString>
#include <QWidget>

namespace arado
{
class ItemMenu
{

public:

  enum Parts {
    Menu_None = 0,
    Menu_Copy = 1,
    Menu_Mail = 2
  };

  ItemMenu (QWidget *parent=0);

  QAction * MenuBasic (const AradoUrl & url,
                       Parts parts = Parts (Menu_Copy | Menu_Mail),
                      const QList<QAction *>  extraActions
                           = QList<QAction*> ());
  void MenuMail (const AradoUrl & aurl);
  void MenuKeywords (const AradoUrl & aurl);
  void MenuCrawl (const AradoUrl & aurl);
  void MenuCopy (const AradoUrl & aurl);
  void MenuHash (const AradoUrl & aurl);
  void MenuUrlText (const AradoUrl & aurl);

private:

  void MailString (const QString & message);
  void Mail (const QAction * action, const AradoUrl & aurl);
  void Copy (const QAction * action, const AradoUrl & aurl);

  QWidget           *parentWidget;
  QList <QAction*>   copyActions;
  QList <QAction*>   mailActions;
  QAction           *copyUrlAction;
  QAction           *copyHashAction;
  QAction           *copyKeyAction;
  QAction           *copyTitleAction;
  QAction           *mailUrlAction;
  QAction           *mailHashAction;
  QAction           *mailKeyAction;
  QAction           *mailTitleAction;
};

} // namespace


#endif