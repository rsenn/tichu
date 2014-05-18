;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Tichu"
  OutFile "tichu-0.10-installer.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Tichu"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Tichu" ""

  ;Most effective compression for tichu
  SetCompressor LZMA

;--------------------------------
; Design
  ;icon
  !insertmacro MUI_DEFAULT MUI_ICON "tichu-client.ico"
  !insertmacro MUI_DEFAULT MUI_UNICON "tichu-client.ico"
  
  ;finish
  !insertmacro MUI_DEFAULT MUI_FINISHPAGE_RUN "$INSTDIR\tichu-client.exe"
  !insertmacro MUI_DEFAULT MUI_FINISHPAGE_LINK "Online Tichu-Community und Gameserver - www.tichu.ch"
  !insertmacro MUI_DEFAULT MUI_FINISHPAGE_LINK_LOCATION "http://www.tichu.ch"
 
  ; install
  ShowInstDetails show
  InstallColors 00FF00 000000
  

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING_DE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "German"

;--------------------------------
;Installer Sections

;------------------------------------------
Section "Tichu" SecTichu
  SectionIn RO

  SetOutPath $INSTDIR

  File "src\tichu-client.exe"
  File "libsgui\src\sgui.dll"
  File "\dev-cpp\bin\SDL.dll"
  File "\dev-cpp\bin\SDL_net.dll"
  File "\dev-cpp\bin\z.dll"
  File "\dev-cpp\bin\png.dll"
  File "\dev-cpp\bin\mingwm10.dll"
  
  ; cursors
  CreateDirectory "$INSTDIR\cursors"
  SetOutPath "$INSTDIR\cursors"
  File "libsgui\cursors\crystal-small.cur"
  File "libsgui\cursors\crystal-small.png"
  File "libsgui\cursors\crystal-big.cur"
  File "libsgui\cursors\crystal-big.png"
  File "libsgui\cursors\deep-sky.cur"
  File "libsgui\cursors\deep-sky.png"
  File "libsgui\cursors\ghost.cur"
  File "libsgui\cursors\ghost.png"
  File "libsgui\cursors\grounation.cur"
  File "libsgui\cursors\grounation.png"
  File "libsgui\cursors\nouveau-onyx.cur"
  File "libsgui\cursors\nouveau-onyx.png"  
  
  ; fonts
  CreateDirectory "$INSTDIR\fonts"
  SetOutPath "$INSTDIR\fonts"
  File "libsgui\fonts\font-normal.png"
  File "libsgui\fonts\font-bold.png"
  File "libsgui\fonts\font-fixed.png"
  
  ; images
  CreateDirectory "$INSTDIR\images"
  SetOutPath "$INSTDIR\images"
  File "images\accept.png"
  File "images\cards-alpha.png"
  File "images\chat.png"
  File "images\circle.png"
  File "images\cursor.png"
  File "images\dragon-16.png"
  File "images\dragon-24.png"
  File "images\dragon-32.png"
  File "images\menu.png"
  File "images\num.png"
  File "images\options.png"
  File "images\preview-bg.png"
  File "images\settings.png"
  File "images\stack.png"
  File "images\team.png"

  ; data
  CreateDirectory "$INSTDIR\data"
  SetOutPath "$INSTDIR\data"
  File "data\cards.ini"
  File "data\client.ini"

  ; sounds
  CreateDirectory "$INSTDIR\sounds"
  SetOutPath "$INSTDIR\sounds"
  File "sounds\zap.wav"
  File "sounds\rs.wav"
  File "sounds\click.wav"
  File "sounds\select.wav"
  File "sounds\orchestral.xm"

  SetOutPath $INSTDIR

  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Tichu" "Install_Dir" "$INSTDIR"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
    
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" SecStart
  CreateDirectory "$SMPROGRAMS\Tichu"
  CreateShortCut "$SMPROGRAMS\Tichu\Tichu.lnk" "$INSTDIR\tichu-client.exe" "" "$INSTDIR\tichu-client.exe" 0
  CreateShortCut "$SMPROGRAMS\Tichu\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0  
SectionEnd

; Optional section (can be disabled by the user)
Section "Quicklaunch Icon" SecQuick
  CreateShortCut "$QUICKLAUNCH\Tichu.lnk" "$INSTDIR\tichu-client.exe" "" "$INSTDIR\tichu-client.exe" 0
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Icon" SecDeskT
  CreateShortCut "$DESKTOP\Tichu.lnk" "$INSTDIR\tichu-client.exe" "" "$INSTDIR\tichu-client.exe" 0
SectionEnd




;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecTichu ${LANG_GERMAN} "Installiert alle benötigten Komponenten um Tichu spielen zu können."
  LangString DESC_SecStart ${LANG_GERMAN} "Erstellt einen Startmenü Eintrag."
  LangString DESC_SecQuick ${LANG_GERMAN} "Erstellt einen Quicklaunch Eintrag."
  LangString DESC_SecDeskT ${LANG_GERMAN} "Erstellt ein Desktop Icon."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecTichu} $(DESC_SecTichu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStart} $(DESC_SecStart)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQuick} $(DESC_SecQuick)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDeskT} $(DESC_SecDeskT)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\tichu-client.exe"
  Delete "$INSTDIR\SDL.dll"
  Delete "$INSTDIR\SDL_net.dll"
  Delete "$INSTDIR\png.dll"
  Delete "$INSTDIR\z.dll"
  Delete "$INSTDIR\mingwm10.dll"
  Delete "$INSTDIR\sgui.dll"
  
  Delete "$INSTDIR\stderr.txt"
  Delete "$INSTDIR\stdout.txt"

  Delete "$INSTDIR\fonts\font-bold.png"
  Delete "$INSTDIR\fonts\font-fixed.png"
  Delete "$INSTDIR\fonts\font-normal.png"

  Delete "$INSTDIR\cursors\crystal-big.cur"
  Delete "$INSTDIR\cursors\crystal-big.png"
  Delete "$INSTDIR\cursors\crystal-small.cur"
  Delete "$INSTDIR\cursors\crystal-small.png"
  Delete "$INSTDIR\cursors\deep-sky.cur"
  Delete "$INSTDIR\cursors\deep-sky.png"
  Delete "$INSTDIR\cursors\ghost.cur"
  Delete "$INSTDIR\cursors\ghost.png"
  Delete "$INSTDIR\cursors\grounation.cur"
  Delete "$INSTDIR\cursors\grounation.png"
  Delete "$INSTDIR\cursors\nouveau-onyx.cur"
  Delete "$INSTDIR\cursors\nouveau-onyx.png"

  Delete "$INSTDIR\images\accept.png"
  Delete "$INSTDIR\images\cards-alpha.png"
  Delete "$INSTDIR\images\chat.png"
  Delete "$INSTDIR\images\circle.png"
  Delete "$INSTDIR\images\cursor.png"
  Delete "$INSTDIR\images\dragon-16.png"
  Delete "$INSTDIR\images\dragon-24.png"
  Delete "$INSTDIR\images\dragon-32.png"
  Delete "$INSTDIR\images\menu.png"
  Delete "$INSTDIR\images\num.png"
  Delete "$INSTDIR\images\options.png"
  Delete "$INSTDIR\images\preview-bg.png"
  Delete "$INSTDIR\images\settings.png"
  Delete "$INSTDIR\images\stack.png"
  Delete "$INSTDIR\images\team.png"
  
  Delete "$INSTDIR\data\cards.ini"
  Delete "$INSTDIR\data\client.ini"

  Delete "$INSTDIR\sounds\zap.wav"
  Delete "$INSTDIR\sounds\rs.wav"
  Delete "$INSTDIR\sounds\click.wav"
  Delete "$INSTDIR\sounds\select.wav"
  Delete "$INSTDIR\sounds\orchestral.xm"

  RMDir "$INSTDIR\cursors"
  RMDir "$INSTDIR\fonts"
  RMDir "$INSTDIR\images"
  RMDir "$INSTDIR\data"
  RMDir "$INSTDIR\sounds"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\Tichu\Tichu.lnk"
  Delete "$SMPROGRAMS\Tichu\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\Tichu"

  Delete "$QUICKLAUNCH\Tichu.lnk"
 
  Delete "$DESKTOP\Tichu.lnk"

  DeleteRegKey HKCU "Software\Tichu"

SectionEnd
