# Define installer name.

Name "Arado Installer"
outFile "Arado-Installer.exe"

# Install directory.

installDir C:\Arado

DirText "Please choose an installation directory for Arado."

# Default section start.

section

# Define output path.

setOutPath $INSTDIR

# Specify files to go in output path.

file .\release\*.dll
file .\release\Arado.exe

# Define uninstaller name.

writeUninstaller $INSTDIR\Arado-Uninstaller.exe

# Default section end.

sectionEnd

# Create a section to define what the uninstaller does.
# The section will always be named "Uninstall".

section "Uninstall"

# Always delete uninstaller first.

RMDir /r $INSTDIR

sectionEnd
