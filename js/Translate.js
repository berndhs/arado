
function PluginTranslate (theId)
{
  var homeLang = AradoInterface.getLanguage ();
  var theUrl = AradoInterface.getUrl (theId);
  if (theUrl.length < 1) {
    return;
  }
  var newUrl = "http://translate.google.com/translate?hl=";
  newUrl += homeLang;
  newUrl += "&sl=auto&u=";
  newUrl += theUrl;
  AradoInterface.openUrl (newUrl);
}