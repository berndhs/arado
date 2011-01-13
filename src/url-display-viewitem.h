#ifndef URLDISPLAYVIEWITEM_H
#define URLDISPLAYVIEWITEM_H

#include "arado-url.h"

class AradoUrl;

namespace arado
{

class UrlDisplayViewItem
{
public:
  UrlDisplayViewItem(const AradoUrl);

  QString Html();

private:
  AradoUrl url;

  QString Timestamp();
  QString Keywords();
  QString Flashmark();
};

}

#endif // URLDISPLAYVIEWITEM_H
