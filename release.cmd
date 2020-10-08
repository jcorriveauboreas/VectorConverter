@echo off
Rem Modifier les 3 lignes suivantes selon l'application
set qt_dir=C:\Qt\5.12.4\mingw73_64\bin
set gpp_dir=C:\Qt\Tools\mingw730_64\bin
set installer_framework_dir=C:\Qt\Tools\QtInstallerFramework\3.1\bin
set project_name="Converter"

Rem On se deplace de le dossier de release du projet
set project_dir=%cd%
cd ..
cd ..

Rem Trouve le nom du dossier en fonction du nom du projet
dir /B | findstr /R /I /C:%project_name% | findstr /R /I /C:"release" > temp.tmp
<temp.tmp (
  set /p release_dir=
)
DEL temp.tmp

Rem Supprime tous les fichiers dans le dossier release (garde seulement le .exe)
cd %release_dir%\release
FOR /f "tokens=*" %%a in ('dir /b ^| findstr /L /I /C:".o"') DO DEL %%a 
FOR /f "tokens=*" %%a in ('dir /b ^| findstr /L /I /C:".cpp"') DO DEL %%a
FOR /f "tokens=*" %%a in ('dir /b ^| findstr /L /I /C:".h"') DO DEL %%a
FOR /f "tokens=*" %%a in ('dir /b ^| findstr /L /I /C:".dll"') DO DEL %%a
FOR /f "tokens=*" %%a in ('dir /b ^| findstr /L /I /C:".qm"') DO DEL %%a

Rem Set les variables d'environement et lance la generation des DLL
SET PATH=%PATH%;%qt_dir%;%gpp_dir%
windeployqt.exe .

Rem Copie les fichiers vers le dossier du projet
ROBOCOPY %cd% %project_dir%\packages\ca.boreas.root\data /E /NDL /NP /NJS /NJH /NFL /NC /NS

Rem Ajoute la DLL pour QtCSV (peut etre retire pour d'autres projets)
cd %project_dir%
CD ../qtcsv
COPY qtcsv.dll %project_dir%\packages\ca.boreas.root\data

Rem Lance la generation de l'installateur
cd %project_dir%
ECHO Wait for command to end before closing
%installer_framework_dir%\binarycreator.exe --offline-only -t %installer_framework_dir%\installerbase.exe -p %project_dir%\packages -c %project_dir%\config\config.xml TestGUI_Installer.exe
ECHO Installer created succesfully