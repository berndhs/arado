

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

#include "entry-form.h"
#include "db-manager.h"
#include <QMessageBox>
#include <QSound>

namespace arado
{

EntryForm::EntryForm (QWidget *parent)
  :QWidget(parent),
   db (0)
{
  ui.setupUi (this);
  connect (ui.saveButton, SIGNAL (clicked()), this, SLOT (Save()));
  connect (ui.cancelButton, SIGNAL (clicked()), this, SLOT (Clear()));
  connect (ui.doneButton, SIGNAL (clicked()), this, SLOT (Done ()));
  hide ();
}

void
EntryForm::Start (QString urlText)
{
  Clear ();
  ui.urlEdit->setText (urlText);
  show ();
}

void
EntryForm::Clear ()
{
  ui.urlEdit->clear ();
  ui.keyEdit->clear ();
  ui.descEdit->clear ();
}

void
EntryForm::Done ()
{
  Clear ();
  hide ();
  emit Finished ();
}

void
EntryForm::Save ()
{
  AradoUrl  newurl;
  QString urlText = ui.urlEdit->text ();
  newurl.SetUrl (QUrl (urlText, QUrl::TolerantMode));
  if (!newurl.IsValid ()) {
    QMessageBox box (this);
    box.setWindowTitle ("Arado");
    box.setIconPixmap(QPixmap(":/images/no.png"));
    QString message (tr("Invalid URL string \"%2\"\nwill not be saved.").arg(urlText));
    box.setText (message);
    box.exec ();
    return ;
  }
  //QSound::play("arado.wav"); // http://bugreports.qt.nokia.com/browse/QTBUG-9253
  QSound arado("arado.wav");
  arado.play();
  //
  newurl.SetDescription (ui.descEdit->toPlainText ());
  QString words = ui.keyEdit->toPlainText ();
  QStringList wordList = words.split (QRegExp ("[\n,;]"),
                                      QString::SkipEmptyParts);
  QStringList trimmedList;
  QString word;
  for (int w=0; w<wordList.size(); w++) {
    word = wordList.at(w).trimmed();
    if (word.length() > 0) {
      trimmedList.append (word);
    }
  }
  newurl.SetKeywords (trimmedList);
  if (db) {
    db->AddUrl (newurl);
  }
  Clear ();
  emit NewUrl (newurl);
}

} // namespace

