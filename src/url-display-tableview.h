#ifndef URLDISPLAYTABLEVIEW_H
#define URLDISPLAYTABLEVIEW_H

#include "url-display-view.h"

class QTableWidgetItem;

namespace Ui {
    class UrlDisplayTableView;
}

namespace arado {

class UrlDisplayTableView : public UrlDisplayView
{
    Q_OBJECT

public:
    enum ModelData {
         Url_Keywords = Qt::UserRole + 1,
         Url_Celltype = Qt::UserRole + 2
         };
    enum UrlCellType {
         Cell_None = 0,
         Cell_Hash = 1,
         Cell_Desc = 2,
         Cell_Url = 3,
         Cell_Time = 4,
         Cell_Browse = 5
         };

    explicit UrlDisplayTableView(QWidget *parent = 0);
    ~UrlDisplayTableView();

    void ShowUrls(QList<arado::AradoUrl> & urls);
    void Updating();
    void UpdatingFinished();
    void ClearContents();
    void ClearSelection();
    void SortByTime(Qt::SortOrder);

private slots:
    void ActiveCell (int row, int col, int oldRow, int oldCol);
    void Picked (QTableWidgetItem *item);

protected:
  void resizeEvent ( QResizeEvent * event );

private:
    void Lock();
    void Unlock();
    void DoOpenUrl (QTableWidgetItem * urlItem);

    void     CellMenuUrl (const QTableWidgetItem * item);
    void     CellMenuDesc (const QTableWidgetItem * item);
    void     CellMenuTime (const QTableWidgetItem * item);
    QAction* CellMenu (const QTableWidgetItem *item,
                      const QList <QAction*> extraActions
                            = QList <QAction*>() );

    Ui::UrlDisplayTableView *ui;

    static const int Col_Mark;
    static const int Col_Title;
    static const int Col_Url;
    static const int Col_Time;
    static const int Col_Browse;

    QIcon   * browseIcon;
    bool    locked;
    bool    allowSort;
    int     normalRowHeight;
    int     bigRowHeight;
};

}

#endif // URLDISPLAYTABLEVIEW_H
