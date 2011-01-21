#include "url-display-view.h"

#include "url-display-viewitem.h"
#include "arado-url.h"

namespace arado
{

UrlDisplayView::UrlDisplayView(QWidget *parent)
  :QWidget (parent)
{
}

bool UrlDisplayView::GetConvertedYoutubeUrl(QString & convertedurl)
{
    if (convertedurl.trimmed().toLower().startsWith("http://www.youtube.com/watch?v=")) {
      convertedurl = convertedurl.replace("http://www.youtube.com/watch?v=", "http://www.youtube-mp3.org/get?video_id=");
      return true;
    }

    return false;
}

}
