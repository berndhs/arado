#include "settings-help.h"

namespace arado
{

SettingsHelp::SettingsHelp (QWidget *parent)
  :QWidget (parent)
{
  ui.setupUi (this);
  hide ();
  connect (ui.closeButton, SIGNAL (clicked()), this, SLOT (DoClose()));
  connect (ui.editButton, SIGNAL (clicked()), this, SLOT (ClickedEdit ()));
}

void
SettingsHelp::Run ()
{
  ui.htmlView->setSource (QUrl("qrc:help/settingshelp.html"));
  show ();
}

void
SettingsHelp::DoClose ()
{
  hide ();
  emit Done();
}

void
SettingsHelp::ClickedEdit ()
{
  emit EditSettings ();
}

} // namespace
