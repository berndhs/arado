
function PluginTranslate (theUrl)
{
  var homeLang = AradoInterface.getLanguage ();
  var newUrl = "http://translate.google.com/translate?hl=";
  newUrl += homeLang;
  newUrl += "&sl=auto&u=";
  newUrl += theUrl;
  AradoInterface.openUrl (newUrl);
}