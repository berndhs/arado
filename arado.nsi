;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

  !include x64.nsh


;--------------------------------
; Define your application name
!define APPNAME "Arado-Websearch"
!define VERSION "0.01"
!define APPNAMEANDVERSION "${APPNAME} ${VERSION}"



;--------------------------------
;General


Name "${APPNAMEANDVERSION}"
;Default installation folder:
InstallDir "$INSTDIR\Arado"

;InstallDirRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\Dooble-Web-Browser" "UninstallString"

OutFile "Arado-Websearch-${VERSION}-Win-Setup.exe"
BrandingText "${APPNAMEANDVERSION}"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin


; Adds an XP manifest to the installer
XPStyle on

; Use compression
SetCompressor LZMA

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
; http://forums.winamp.com/showthread.php?s=f9514f23b6cf9fd97bf7b177f2f4a04e&threadid=261299
; http://nsis.sourceforge.net/Adding_3_Extra_Text_Lines_on_MUI_Welcome-Finish_Pages
	!define MUI_WELCOMEPAGE_TITLE_3LINES
  !define MUI_FINISHPAGE_TITLE_3LINES
	!define MUI_UNWELCOMEPAGE_TITLE_3LINES
	!define MUI_UNFINISHPAGE_TITLE_3LINES
	!define MUI_UNPAGE_WELCOME_TITLE_3LINES
	


;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\Arado" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
	!define MUI_FINISHPAGE_RUN "$INSTDIR\Arado.exe"

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_WELCOME
 ; !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH


;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "SpanishInternational"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Japanese"
  !insertmacro MUI_LANGUAGE "Korean"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Dutch"
  !insertmacro MUI_LANGUAGE "Danish"
  !insertmacro MUI_LANGUAGE "Swedish"
  !insertmacro MUI_LANGUAGE "Norwegian"
  !insertmacro MUI_LANGUAGE "NorwegianNynorsk"
  !insertmacro MUI_LANGUAGE "Finnish"
  !insertmacro MUI_LANGUAGE "Greek"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Portuguese"
  !insertmacro MUI_LANGUAGE "PortugueseBR"
  !insertmacro MUI_LANGUAGE "Polish"
  !insertmacro MUI_LANGUAGE "Ukrainian"
  !insertmacro MUI_LANGUAGE "Czech"
  !insertmacro MUI_LANGUAGE "Slovak"
  !insertmacro MUI_LANGUAGE "Croatian"
  !insertmacro MUI_LANGUAGE "Bulgarian"
  !insertmacro MUI_LANGUAGE "Hungarian"
  !insertmacro MUI_LANGUAGE "Thai"
  !insertmacro MUI_LANGUAGE "Romanian"
  !insertmacro MUI_LANGUAGE "Latvian"
  !insertmacro MUI_LANGUAGE "Macedonian"
  !insertmacro MUI_LANGUAGE "Estonian"
  !insertmacro MUI_LANGUAGE "Turkish"
  !insertmacro MUI_LANGUAGE "Lithuanian"
  !insertmacro MUI_LANGUAGE "Slovenian"
  !insertmacro MUI_LANGUAGE "Serbian"
  !insertmacro MUI_LANGUAGE "SerbianLatin"
  !insertmacro MUI_LANGUAGE "Arabic"
  !insertmacro MUI_LANGUAGE "Farsi"
  !insertmacro MUI_LANGUAGE "Hebrew"
  !insertmacro MUI_LANGUAGE "Indonesian"
  !insertmacro MUI_LANGUAGE "Mongolian"
  !insertmacro MUI_LANGUAGE "Luxembourgish"
  !insertmacro MUI_LANGUAGE "Albanian"
  !insertmacro MUI_LANGUAGE "Breton"
  !insertmacro MUI_LANGUAGE "Belarusian"
  !insertmacro MUI_LANGUAGE "Icelandic"
  !insertmacro MUI_LANGUAGE "Malay"
  !insertmacro MUI_LANGUAGE "Bosnian"
  !insertmacro MUI_LANGUAGE "Kurdish"
  !insertmacro MUI_LANGUAGE "Irish"
  !insertmacro MUI_LANGUAGE "Uzbek"
  !insertmacro MUI_LANGUAGE "Galician"
  !insertmacro MUI_LANGUAGE "Afrikaans"
  !insertmacro MUI_LANGUAGE "Catalan"
	
LangString stillRunning ${LANG_ENGLISH} "Arado is still active. Please stop it first."

;--------------------------------
;Installer Section DOOBLE

# Default section start.

section

# Define output path.

setOutPath $INSTDIR

# Specify files to go in output path.

file .\release\*.dll
file .\release\Arado.exe



;Create uninstaller
WriteUninstaller "$INSTDIR\Uninstall.exe"


SetOutPath "$INSTDIR"
CreateDirectory "$SMPROGRAMS\${APPNAME}"
CreateShortCut "$SMPROGRAMS\${APPNAME}\Start ${APPNAME}.lnk" "$INSTDIR\Arado.exe" "" "$INSTDIR\Arado.exe" 0
CreateShortCut "$SMPROGRAMS\Arado.lnk" "$INSTDIR\Arado.exe" "" "$INSTDIR\Arado.exe" 0


CreateShortCut "$FAVORITES\Websearch.lnk"  "http://arado.sourceforge.net/" "" "" 0

; Autorun in Start menu: not needed because it is in registry /=> switched now to startup = Autorun: 
; WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "Arado"   "$INSTDIR\Arado.exe -a  silent /nosplash /minimized" 
CreateShortCut "$SMSTARTUP\${APPNAME}.lnk" "$INSTDIR\Arado.exe" "" "$INSTDIR\Arado.exe" 0 SW_SHOWMINIMIZED




; Homepage Link icon disabled:
; CreateShortCut "$SMPROGRAMS\${APPNAME}\Homepage.lnk" "$INSTDIR\Homepage" "" "$INSTDIR\Homepage" 0


CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\Arado.exe" "" "$INSTDIR\Arado.exe" 0
CreateShortCut "$QUICKLAUNCH\${APPNAME}.lnk" "$INSTDIR\Arado.exe" "" "$INSTDIR\Arado.exe" 0
 
 
; currently disabled from startmenu, please start uninstaller.exe from hard disk or windows removal,
; because the uninstall.exe is on top of dooble.exe, which is the wrong order. -> enabled with v. 0.07
 CreateShortCut "$SMPROGRAMS\${APPNAME}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
 

# Default section end.

sectionEnd

;--------------------------------



# Create a section to define what the uninstaller does.
# The section will always be named "Uninstall".

section "Uninstall"

# Always delete uninstaller first.

RMDir /r $INSTDIR

sectionEnd
