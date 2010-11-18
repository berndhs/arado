/****************************************************************
 * This file is distributed under the following license:
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

 #include <QtCore>
 #include <QtGui>
 #include <QtNetwork>

 #include "rssfeed.h"

 RSSListing::RSSListing(QWidget *parent)
     : QWidget(parent)
 {
     lineEdit = new QLineEdit(this);
     lineEdit->setText("http://rss.feedsportal.com/c/617/f/8093/index.rss");

     fetchButton = new QPushButton(tr("Read"), this);
     abortButton = new QPushButton(tr("Cancel"), this);
     abortButton->setEnabled(false);

     treeWidget = new QTreeWidget(this);
     connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
             this, SLOT(itemActivated(QTreeWidgetItem*)));
     QStringList headerLabels;
     headerLabels << tr("Title/Description") << tr("Keywords") << tr("URL");
     treeWidget->setHeaderLabels(headerLabels);
     treeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

     connect(&http, SIGNAL(readyRead(QHttpResponseHeader)),
              this, SLOT(readData(QHttpResponseHeader)));

     connect(&http, SIGNAL(requestFinished(int,bool)),
              this, SLOT(finished(int,bool)));

     connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(fetch()));
     connect(fetchButton, SIGNAL(clicked()), this, SLOT(fetch()));
     connect(abortButton, SIGNAL(clicked()), &http, SLOT(abort()));

     QVBoxLayout *layout = new QVBoxLayout(this);

     QHBoxLayout *hboxLayout = new QHBoxLayout;

     hboxLayout->addWidget(lineEdit);
     hboxLayout->addWidget(fetchButton);
     hboxLayout->addWidget(abortButton);

     layout->addLayout(hboxLayout);
     layout->addWidget(treeWidget);

     setWindowTitle(tr("Arado-Feeds"));
     resize(640,480);
 }



 void RSSListing::fetch()
 {
     lineEdit->setReadOnly(true);
     fetchButton->setEnabled(false);
     abortButton->setEnabled(true);
     treeWidget->clear();

     xml.clear();

     QUrl url(lineEdit->text());

     http.setHost(url.host());
     connectionId = http.get(url.path());
 }



 void RSSListing::readData(const QHttpResponseHeader &resp)
 {
     if (resp.statusCode() != 200)
         http.abort();
     else {
         xml.addData(http.readAll());
         parseXml();
     }
 }


 void RSSListing::finished(int id, bool error)
 {
     if (error) {
         qWarning("Error during HTTP fetch.");
         lineEdit->setReadOnly(false);
         abortButton->setEnabled(false);
         fetchButton->setEnabled(true);
     }
     else if (id == connectionId) {
         lineEdit->setReadOnly(false);
         abortButton->setEnabled(false);
         fetchButton->setEnabled(true);
     }
 }


 void RSSListing::parseXml()
 {
     while (!xml.atEnd()) {
         xml.readNext();
         if (xml.isStartElement()) {
             if (xml.name() == "item")
                 urlString = xml.attributes().value("rss:about").toString();
             currentTag = xml.name().toString();
         } else if (xml.isEndElement()) {
             if (xml.name() == "item") {

                 QTreeWidgetItem *item = new QTreeWidgetItem;
                 item->setText(0, titleString);
                 item->setText(1, keywordsString);
                 item->setText(2, urlString);

                 treeWidget->addTopLevelItem(item);

                 titleString.clear();
                 urlString.clear();
                 keywordsString.clear();
             }

         } else if (xml.isCharacters() && !xml.isWhitespace()) {
             if (currentTag == "title")
                 titleString += xml.text().toString();
             else if (currentTag == "link")
                 urlString += xml.text().toString();
             else if (currentTag == "category")
                 keywordsString += xml.text().toString();
             // add a blank after each _C_apital letter for category keywords
         }
     }
     if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
         qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
         http.abort();
     }
 }
