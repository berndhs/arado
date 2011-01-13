#include "url-display-tableview.h"
#include "ui_url-display-tableview.h"

#include "arado-url.h"
#include "deliberate.h"

#include <QDateTime>
#include <QResizeEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QClipboard>

using namespace deliberate;

namespace arado
{

const int UrlDisplayTableView::Col_Mark (0);
const int UrlDisplayTableView::Col_Title (2);
const int UrlDisplayTableView::Col_Url (3);
const int UrlDisplayTableView::Col_Time (4);
const int UrlDisplayTableView::Col_Browse (1);

UrlDisplayTableView::UrlDisplayTableView(QWidget *parent) :
  UrlDisplayView(parent),
  ui(new Ui::UrlDisplayTableView),
  allowSort(false)
{
  ui->setupUi(this);

  browseIcon = new QIcon (":/images/kugar.png");

  allowSort = ui->urlTable->isSortingEnabled ();
  connect (ui->urlTable, SIGNAL (currentCellChanged (int, int, int, int)),
           this, SLOT (ActiveCell (int, int, int, int)));
  connect (ui->urlTable, SIGNAL (itemDoubleClicked (QTableWidgetItem *)),
           this, SLOT (Picked (QTableWidgetItem*)));
}

UrlDisplayTableView::~UrlDisplayTableView()
{
  ui->urlTable->clearContents();
  delete ui;
  delete browseIcon;
}

void UrlDisplayTableView::ShowUrls (QList<arado::AradoUrl> & urls)
{
  QList <QTableWidgetItem*> selection = ui->urlTable->selectedItems();
  if (!selection.isEmpty()) {
    return;
  }
  ui->urlTable->clearContents ();
  ui->urlTable->setRowCount (urls.size());
  ui->urlTable->setEditTriggers (0);
  ui->urlTable->setColumnWidth (Col_Browse,
                                browseIcon->availableSizes().at(0).width());
  for (int u=0; u<urls.size(); u++) {
    quint64 stamp;
    arado::AradoUrl url = urls[u];
    stamp = url.Timestamp ();
    Lock ();
    ui->urlTable->setSortingEnabled (false);
    QTableWidgetItem * item = new QTableWidgetItem (QString(url.Hash().toUpper()));
    item->setData (Url_Celltype, Cell_Hash);
    item->setToolTip (tr("Arado-Flashmark"));
    ui->urlTable->setItem (u,Col_Mark,item);

    item = new QTableWidgetItem (url.Description ());
    item->setData (Url_Celltype, Cell_Desc);
    item->setData (Url_Keywords, url.Keywords ());
    QString words = url.Keywords().join("\n");
    if (words.length() < 1) {
      words = tr ("no keywords");
    }
    item->setToolTip (words);
    ui->urlTable->setItem (u,Col_Title,item);

    item = new QTableWidgetItem (url.Url().toString());
    item->setData (Url_Celltype, Cell_Url);
    ui->urlTable->setItem (u,Col_Url,item);

    QString time = QDateTime::fromTime_t (stamp).toString(Qt::ISODate);
    time.replace ('T'," ");
    item = new QTableWidgetItem (time);
    item->setData (Url_Celltype, Cell_Time);
    ui->urlTable->setItem (u,Col_Time,item);
    Unlock ();
    ui->urlTable->setSortingEnabled (allowSort);
    item = new QTableWidgetItem (tr(""));
    item->setToolTip (tr("Browse"));
    item->setData (Url_Celltype, Cell_Browse);
    item->setIcon (*browseIcon);
    ui->urlTable->setItem (u, Col_Browse, item);
  }
  if (ui->urlTable->rowCount() > 0) {
    normalRowHeight = ui->urlTable->rowHeight (0);
    bigRowHeight = qMin (3*normalRowHeight, ui->urlTable->size().height());
  }
}

void
UrlDisplayTableView::Lock ()
{
  locked = true;
}

void
UrlDisplayTableView::Unlock ()
{
  locked = false;
}

void
UrlDisplayTableView::ActiveCell (int row, int col, int oldRow, int oldCol)
{
  QTableWidgetItem * item = ui->urlTable->item (row, col);
  QTableWidgetItem * oldItem = ui->urlTable->item (oldRow, oldCol);
  if (oldItem) {
    ui->urlTable->setRowHeight (oldRow, normalRowHeight);
  }
  if (item) {
    UrlCellType ctype = UrlCellType (item->data (Url_Celltype).toInt());
    if (ctype == Cell_Desc) {
      ui->urlTable->setRowHeight (row, bigRowHeight);
    } else if (ctype == Cell_Browse) {
      QUrl url;
      QTableWidgetItem * urlItem (0);
      for (int c=0; c < ui->urlTable->columnCount(); c++) {
        urlItem = ui->urlTable->item (row, c);
        if (urlItem &&
            UrlCellType (urlItem->data (Url_Celltype).toInt())
            == Cell_Url) {
          DoOpenUrl (urlItem);
          break;
        }
      }
    }
  }
}

void
UrlDisplayTableView::DoOpenUrl (QTableWidgetItem * urlItem)
{
  QTableWidgetItem * current (0);
  if (urlItem == 0) {
    current = ui->urlTable->currentItem ();
  } else {
    current = urlItem;
  }
  if (current) {
    if (UrlCellType (current->data(Url_Celltype).toInt())
        == Cell_Url) {
      QUrl target (current->text ());
      QDesktopServices::openUrl (target);
    }
  }
}

void
UrlDisplayTableView::resizeEvent (QResizeEvent * event)
{
  if (event) {
    bigRowHeight = qMin (3*normalRowHeight, ui->urlTable->size().height());
  }
}

void UrlDisplayTableView::Updating()
{
  ui->urlTable->clearSelection ();
  ui->urlTable->clearContents();
  ui->urlTable->setAlternatingRowColors(false);
  ui->urlTable->setStyleSheet("background-color: black");
}

void UrlDisplayTableView::UpdatingFinished()
{
  ui->urlTable->setStyleSheet("background-color: white");
  ui->urlTable->setAlternatingRowColors(true);
}

void UrlDisplayTableView::ClearContents()
{
  ui->urlTable->clearContents();
}

void UrlDisplayTableView::ClearSelection()
{
  ui->urlTable->clearSelection();
}

void
UrlDisplayTableView::Picked (QTableWidgetItem *item)
{
  if (item) {
    Lock ();
    UrlCellType  tipo;
    tipo = UrlCellType (item->data(Url_Celltype).toInt());
    if (tipo == Cell_Url) {
      CellMenuUrl (item);
    } else if (tipo == Cell_Desc) {
      CellMenuDesc (item);
    } else if (tipo == Cell_Hash) {
      CellMenu (item);
    } else if (tipo == Cell_Time) {
      CellMenuTime (item);
    } else {
      CellMenu (item);
    }
    Unlock ();
  }
}

void
UrlDisplayTableView::CellMenuUrl (const QTableWidgetItem * item)
{
  if (item == 0) {
    return;
  }
  //
  QString convertedurl (item->text());

  if (convertedurl.trimmed().toLower().startsWith("http://www.youtube.com/watch?v=")) {
    convertedurl = convertedurl.replace("http://www.youtube.com/watch?v=", "http://www.youtube-mp3.org/get?video_id=");

    QAction * playAction = new QAction (tr("Convert Youtube-URL to MP3"),this);
    playAction->setIcon(QPixmap(":/images/youtubemp3play.png"));

    QList<QAction*> list;
    list.append (playAction);

    QAction * select = CellMenu (item, list);
    if (select == playAction) {
      QDesktopServices::openUrl (convertedurl);
    }
  } else {
    //
    QAction * openAction = new QAction (tr("Browse URL"),this);
    openAction->setIcon(QPixmap(":/images/kugar.png"));
    QList<QAction*> list;
    list.append (openAction);

    QAction * select = CellMenu (item, list);
    if (select == openAction) {
      QUrl url (item->text());
      QDesktopServices::openUrl (url);
    }
  }
}

QAction *
UrlDisplayTableView::CellMenu (const QTableWidgetItem *item,
                               const QList<QAction *>  extraActions)
{
  if (item == 0) {
    return 0;
  }
  QMenu menu (this);
  QAction * copyAction = new QAction (tr("Copy Text"),this);
  copyAction->setIcon(QPixmap(":/images/copy.png"));
  QAction * mailAction = new QAction (tr("Mail Text"),this);
  mailAction->setIcon(QPixmap(":/images/mail.png"));
  menu.addAction (copyAction);
  menu.addAction (mailAction);
  if (extraActions.size() > 0) {
    menu.addSeparator ();
  }
  for (int a=0; a < extraActions.size(); a++) {
    menu.addAction (extraActions.at (a));
  }

  QAction * select = menu.exec (QCursor::pos());
  if (select == copyAction) {
    QClipboard *clip = QApplication::clipboard ();
    if (clip) {
      clip->setText (item->text());
    }
    return 0;
  } else if (select == mailAction) {
    QStringList mailBodytotal;
    QString mailBody = item->text();
    QString blankline (tr("    "));
    QString webpageline (tr("Discover http://arado.sf.net Websearch - Syncs, shortens "
                            "and searches within (y)our URLs and Bookmarks."));
    mailBodytotal << mailBody
                  << blankline
                  << webpageline ;
    QString urltext = tr("mailto:?subject=Aradofied\%20Web\%20Alert\%20for\%20you&body=%1")
                      .arg (mailBodytotal.join("\r\n"));
    QDesktopServices::openUrl (urltext);
    return 0;
  } else {
    return select;
  }
}


void
UrlDisplayTableView::CellMenuTime (const QTableWidgetItem * item)
{
  if (item == 0) {
    return;
  }
  QList<QAction*> list;

  CellMenu (item, list);
}


void
UrlDisplayTableView::CellMenuDesc (const QTableWidgetItem * item)
{
  if (item == 0) {
    return;
  }
  QAction *copyKeysAction = new QAction (tr("Copy Keywords"), this);
  copyKeysAction->setIcon(QPixmap(":/images/copy.png"));
  QAction *mailKeysAction = new QAction (tr("Mail Keywords"), this);
  mailKeysAction->setIcon(QPixmap(":/images/mail.png"));
  QList <QAction*> list;
  list.append (copyKeysAction);
  list.append (mailKeysAction);

  QAction * select = CellMenu (item, list);
  QString mailBody;
  bool    mailit (false);
  if (select == copyKeysAction) {
    QClipboard * clip = QApplication::clipboard ();
    if (clip) {
      QString keytext = item->data (Url_Keywords).toStringList().join("\n");
      clip->setText (keytext);
    }
  } else if (select == mailKeysAction) {
    mailBody = item->data (Url_Keywords).toStringList().join("\n");
    mailit = (mailBody.length() > 0);
  }
  if (mailit) {
    QString urltext = tr("mailto:?subject=Arado\%20Data&body=%1")
                      .arg (mailBody);
    QDesktopServices::openUrl (urltext);
  }
}

void
UrlDisplayTableView::SortByTime(Qt::SortOrder order)
{
  ui->urlTable->sortByColumn(Col_Time, order);
}

}
