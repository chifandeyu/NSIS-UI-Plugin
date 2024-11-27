﻿# ========================= User Defined Macro ==============================

# Most time you just need edit user defined macro
!define PRODUCT_NAME           "云PC远程控制终端"
!define APP_NAME               "yunVM"
!define EXE_NAME               "yunVM.exe"
!define EXE_RELATIVE_PATH      "yunVM.exe"
!ifdef PRODUCT_VERSION

!else
  !define PRODUCT_VERSION "1.0.0.0"
!endif
!define PRODUCT_PUBLISHER      "Bangyan Technology Co., Ltd"
!define PRODUCT_LEGAL          "Copyright (C) 2023-2030 bangyan technology, All Rights Reserved"
!define INSTALL_ICON_PATH      "Install.ico"
!define UNINSTALL_ICON_PATH    "Uninstall.ico"
!define DEFAULT_INSTALL_DIR    "$PROGRAMFILES\${APP_NAME}"

Var /GLOBAL installDir

Var /GLOBAL UpdateInstall

!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
!define PRODUCT_SOFTWARE_KEY "Software\${APP_NAME}"

!define PRODUCT_AUTORUN_KEY "Software\Microsoft\Windows\CurrentVersion\Run"

!ifdef DEBUG
!define UI_PLUGIN_NAME         nsQtPluginD
!define VC_RUNTIME_DLL_SUFFIX  d
!define QT_DLL_SUFFIX          d
!else
!define UI_PLUGIN_NAME         nsQtPlugin
!define VC_RUNTIME_DLL_SUFFIX
!define QT_DLL_SUFFIX
!endif

# ========================= User Defined Macro End ============================

!include "LogicLib.nsh"
!include "nsDialogs.nsh"


# ===================== Setup Info =============================
VIProductVersion                    "${PRODUCT_VERSION}"
VIAddVersionKey "ProductVersion"    "${PRODUCT_VERSION}"
VIAddVersionKey "ProductName"       "${APP_NAME}"
VIAddVersionKey "CompanyName"       "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileVersion"       "${PRODUCT_VERSION}"
VIAddVersionKey "InternalName"      "${EXE_NAME}"
VIAddVersionKey "FileDescription"   "${APP_NAME} for windows"
VIAddVersionKey "LegalCopyright"    "${PRODUCT_LEGAL}"

# ==================== NSIS Attribute ================================

Unicode True
SetCompressor LZMA
!ifdef DEBUG
Name "${APP_NAME} [Debug]"
OutFile "${APP_NAME}-${PRODUCT_VERSION}-x86-setup-debug.exe"
!else
Name "${APP_NAME}"
OutFile "${APP_NAME}-${PRODUCT_VERSION}-x86-setup.exe"
!endif

# ICON
Icon              "${INSTALL_ICON_PATH}"
UninstallIcon     "${UNINSTALL_ICON_PATH}"

# UAC
# RequestExecutionLevel none|user|highest|admin
RequestExecutionLevel admin


# Custom Install Page
Page custom QtUiPage


# Show Uninstall details
UninstPage instfiles

# ======================= Qt Page =========================
Function QtUiPage
    ${UI_PLUGIN_NAME}::OutputDebugInfo "NSIS Plugin Dir: $PLUGINSDIR, UpdateInstall: $UpdateInstall"

    GetFunctionAddress $0 OnUIPrepared
    ${UI_PLUGIN_NAME}::BindInstallEventToNsisFunc "UI_PREPARED" $0

    GetFunctionAddress $0 OnStartExtractFiles
    ${UI_PLUGIN_NAME}::BindInstallEventToNsisFunc "START_EXTRACT_FILES" $0

    GetFunctionAddress $0 OnBeforeFinished
    ${UI_PLUGIN_NAME}::BindInstallEventToNsisFunc "BEFORE_FINISHED" $0

    GetFunctionAddress $0 OnUserCancelInstall
    ${UI_PLUGIN_NAME}::BindInstallEventToNsisFunc "USER_CANCEL" $0

    ${UI_PLUGIN_NAME}::ShowSetupUI "${APP_NAME}" "$installDir" "$PLUGINSDIR" "$UpdateInstall"
FunctionEnd

Function OnUIPrepared
    ${UI_PLUGIN_NAME}::OutputDebugInfo "OnUIPrepared"
FunctionEnd

Function OnStartExtractFiles
    ${UI_PLUGIN_NAME}::OutputDebugInfo "OnStartExtractFiles"

    ${UI_PLUGIN_NAME}::GetInstallDirectory
    Pop $0
    StrCmp $0 "" InstallAbort 0
    StrCpy $INSTDIR "$0"
    ${UI_PLUGIN_NAME}::OutputDebugInfo "Install Dir: $0"

    SetOutPath $INSTDIR
    SetOverwrite on
    GetFunctionAddress $0 ___ExtractFiles
    ${UI_PLUGIN_NAME}::BackgroundRun $0


InstallAbort:
FunctionEnd


Function OnUserCancelInstall
    ${UI_PLUGIN_NAME}::OutputDebugInfo "OnUserCancelInstall"

    Abort
FunctionEnd


Function OnBeforeFinished
    ${UI_PLUGIN_NAME}::OutputDebugInfo "OnBeforeFinished"
    Call CreateSoftware
    SetShellVarContext current


    # Create Desktop Shortcut
    ${UI_PLUGIN_NAME}::IsCreateDesktopShortcutEnabled
    Pop $0
    ${If} $0 == 1
        SetShellVarContext all
        CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_RELATIVE_PATH}"
        SetShellVarContext current
    ${EndIf}


    # Auto Startup On Boot
    ${UI_PLUGIN_NAME}::IsAutoStartupOnBootEnabled
    Pop $0
    ${If} $0 == 1
        WriteRegStr HKCU ${PRODUCT_AUTORUN_KEY} "${APP_NAME}" "$INSTDIR\${EXE_RELATIVE_PATH}"
    ${EndIf}

    # Run app now
    ${UI_PLUGIN_NAME}::IsRunNowEnabled
    Pop $0
    ${If} $0 == 1
        Exec '"$INSTDIR\${EXE_RELATIVE_PATH}"'
    ${EndIf}

FunctionEnd


# !!! Don't Edit This Function !!!
# This Function Generated by Python Script(NsisScriptGenerate.py)
Function ___ExtractFiles
    Call OnAfterExtractFiles
FunctionEnd


Function OnAfterExtractFiles
    ${UI_PLUGIN_NAME}::OutputDebugInfo "OnAfterExtractFiles"
    ${UI_PLUGIN_NAME}::NsisExtractFilesFinished
    Call CreateUninstall
FunctionEnd

Function CreateSoftware
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_RELATIVE_PATH}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\卸载${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"

    WriteRegStr HKLM ${PRODUCT_SOFTWARE_KEY} "InstallDir" "$INSTDIR"
    WriteRegStr HKLM ${PRODUCT_SOFTWARE_KEY} "Version" ${PRODUCT_VERSION}

FunctionEnd

Function CreateUninstall
    WriteUninstaller "$INSTDIR\uninst.exe"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "UninstallString" "$INSTDIR\uninst.exe"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "QuietUninstallString" "$\"$INSTDIR\uninst.exe$\" /S"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "DisplayIcon" "$INSTDIR\${EXE_NAME}"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "HelpLink" "https://www.bangyan.com.cn/"
FunctionEnd

# Add an empty section, avoid compile error.
Section "None"
SectionEnd


# Uninstall Section
Section "Uninstall"

  SetShellVarContext all
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\卸载${PRODUCT_NAME}.lnk"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}\"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  DeleteRegKey HKLM ${PRODUCT_SOFTWARE_KEY}

  DeleteRegKey HKLM ${PRODUCT_UNINST_KEY}
  ;删除开机自启动
  DeleteRegValue HKCU ${PRODUCT_AUTORUN_KEY} "${APP_NAME}"
  ; MessageBox MB_ICONQUESTION|MB_YESNO "Uninstall" /SD IDYES IDYES +2 IDNO +1

  SetShellVarContext current

  SetOutPath "$INSTDIR"

  ; Delete installed files
  Delete "$INSTDIR\*.*"

  SetOutPath "$DESKTOP"

  RMDir /r $APPDATA\${APP_NAME}
  RMDir /r "$INSTDIR"
  RMDir "$INSTDIR"

  SetAutoClose true
SectionEnd



Function .onInit
    # makesure plugin directory exist
    InitPluginsDir

    # place Qt dlls to plugin directory
    File /oname=$PLUGINSDIR\Qt5Core${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\Qt5Core${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\Qt5Gui${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\Qt5Gui${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\Qt5Widgets${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\Qt5Widgets${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\Qt5Svg${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\Qt5Svg${QT_DLL_SUFFIX}.dll"

    CreateDirectory $PLUGINSDIR\platforms
    File /oname=$PLUGINSDIR\platforms\qwindows${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\platforms\qwindows${QT_DLL_SUFFIX}.dll"

    CreateDirectory $PLUGINSDIR\styles
    File /oname=$PLUGINSDIR\styles\qwindowsvistastyle${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\styles\qwindowsvistastyle${QT_DLL_SUFFIX}.dll"

    CreateDirectory $PLUGINSDIR\imageformats
    File /oname=$PLUGINSDIR\imageformats\qgif${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\imageformats\qgif${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\imageformats\qicns${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\imageformats\qicns${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\imageformats\qico${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\imageformats\qico${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\imageformats\qjpeg${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\imageformats\qjpeg${QT_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\imageformats\qsvg${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\imageformats\qsvg${QT_DLL_SUFFIX}.dll"
    CreateDirectory $PLUGINSDIR\iconengines

    File /oname=$PLUGINSDIR\iconengines\qsvgicon${QT_DLL_SUFFIX}.dll "QtRuntimeDLL\iconengines\qsvgicon${QT_DLL_SUFFIX}.dll"

    # place vc runtime dlls to plugin directory
    File /oname=$PLUGINSDIR\concrt140${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\concrt140${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\msvcp140${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\msvcp140${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\msvcp140_1${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\msvcp140_1${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\msvcp140_2${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\msvcp140_2${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\ucrtbase${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\ucrtbase${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\vccorlib140${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\vccorlib140${VC_RUNTIME_DLL_SUFFIX}.dll"
    File /oname=$PLUGINSDIR\vcruntime140${VC_RUNTIME_DLL_SUFFIX}.dll "VCRuntimeDLL\vcruntime140${VC_RUNTIME_DLL_SUFFIX}.dll"

    ${UI_PLUGIN_NAME}::OutputDebugInfo "====== Function .onInit ======"
    # read install dir from registry
    ReadRegStr $installDir HKLM ${PRODUCT_UNINST_KEY} "InstallLocation"

    # modify install dir, if it is empty to set default value
    StrCmp $installDir "" 0 +2
    StrCpy $installDir "${DEFAULT_INSTALL_DIR}"

    ; Initialize UpdateInstall variable to default value "0"
    StrCpy $UpdateInstall "0"

    ; 获取命令行参数
    StrCpy $0 $CMDLINE
    ${UI_PLUGIN_NAME}::OutputDebugInfo "CMDLINE: $CMDLINE"

    ${UI_PLUGIN_NAME}::ParseUpdateInstall "$CMDLINE"
    Pop $0
    ; MessageBox MB_ICONQUESTION|MB_YESNO "$0"
    StrCmp $0 "1" 0 +2
    StrCpy $UpdateInstall "1"
    ; MessageBox MB_ICONQUESTION|MB_YESNO "$UpdateInstall"

    # 读取注册表的应用版本号
    ReadRegStr $0 HKLM ${PRODUCT_SOFTWARE_KEY} "Version"
    ${UI_PLUGIN_NAME}::OutputDebugInfo "Old Version: $0"
    # 如果注册表中没有找到版本号，继续安装，不做逻辑处理
    StrCmp $0 "" done

    ${UI_PLUGIN_NAME}::VersionCompare ${PRODUCT_VERSION} $0
    Pop $1  # Pop the return value into $1
    ${UI_PLUGIN_NAME}::OutputDebugInfo "VersionCompare ret: $1"
    # 检查 $1 是否等于 -1
    IntCmp $1 -1 is_less done done  # 如果 $1 == -1，跳转到 is_less 标签

    Goto done

    is_less:
    MessageBox MB_OK "当前安装的版本较低（版本号: ${PRODUCT_VERSION}），安装程序将退出。"
    Quit

    done:

FunctionEnd



Function .onInstSuccess
    ${UI_PLUGIN_NAME}::OutputDebugInfo "---- start .onInstSuccess"

    ${UI_PLUGIN_NAME}::OutputDebugInfo "---- end .onInstSuccess"
FunctionEnd


Function .onInstFailed
    MessageBox MB_ICONQUESTION|MB_YESNO "安装失败!" /SD IDYES IDYES +2 IDNO +1
FunctionEnd



# Before Uninstall
Function un.onInit
    MessageBox MB_ICONQUESTION|MB_YESNO "你确定卸载 ${PRODUCT_NAME} 吗?" /SD IDYES IDYES +2 IDNO +1
    Abort
    ${UI_PLUGIN_NAME}::KillProcess "${EXE_NAME}"
    ; 尝试结束 yunVM.exe 进程
    nsExec::ExecToLog 'taskkill /F /IM ${EXE_NAME}'
FunctionEnd

Function un.onUninstSuccess

FunctionEnd

