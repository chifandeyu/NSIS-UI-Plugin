﻿/*******************************************************************************************************************************************************
  #@@        *@    *@@%#@@#    &@    #@%@@,         @(        &@   .@.     @@@@@@@%     @         @(        &@     .@@@%&@@&     &@    @@#        %@
  #@/@       *@   *@      @%   &@   %@      @/      @(        &@   .@.     @,     ,@    @         @(        &@    @@        @*   &@    @,&@       %@
  #@  @(     *@   ,@           &@   #@              @(        &@   .@.     @,      @*   @         @(        &@   @&              &@    @, *@      %@
  #@   &@    *@     @@@,       &@    *@@%           @(        &@   .@.     @,     &@    @         @(        &@   @,              &@    @,   @(    %@
  #@    ,@   *@         .@@.   &@         (@@       @(        &@   .@.     @@@@@@%      @         @(        &@   @,     @@@@@&   &@    @,    @@   %@
  #@      @/ *@           *@   &@           &@      @(        @@   .@.     @,           @         @(        @@   @&         &&   &@    @,     (@. %@
  #@       @@,@   @@      (@   &@   @#      &@      (@       (@.   .@.     @,           @         (@       (@.    @@        &&   &@    @,       @%%@
  #@        *@@    (@@%#&@&    &@    %@@#%@@(         @@@%&@@(     .@.     @,           @@@@@@@@    @@@%&@@(        @@@%&@@@     &@    @,        %@@

* Copyright (C) 2018 - 2020, winsoft666, <winsoft666@outlook.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
* Expect bugs
*
* Please use and enjoy.
* Please let me know of any bugs/improvements that you have found/implemented and I will fix/incorporate them into this file.
********************************************************************************************************************************************************/

#include "stdafx.h"
#include "Qt-UI/SetupPage-Qt.h"
#include <tlhelp32.h>

void printLog(const QString& info);
void printLogW(const TCHAR* szMsg);

bool IsProcessRunning(const wchar_t* processName) {
    // 获取系统中的所有进程的快照
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printLog("Failed to create process snapshot.");
        return false;
    }

    // 初始化PROCESSENTRY32结构体
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // 获取第一个进程的信息
    if (!Process32First(hProcessSnap, &pe32)) {
        printLog("Failed to retrieve first process.");
        CloseHandle(hProcessSnap); // 关闭句柄
        return false;
    }

    // 遍历系统中的所有进程
    do {
        // 比较进程名称
        if (_wcsicmp(pe32.szExeFile, processName) == 0) {
            CloseHandle(hProcessSnap); // 找到进程后关闭句柄
            return true;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    // 如果没有找到进程，关闭句柄并返回false
    CloseHandle(hProcessSnap);
    return false;
}
void printLog(const QString& info) {
    // 获取临时文件夹路径
    QString tempDirPath = QDir::tempPath();
    QString logFilePath = tempDirPath + "/yunVM_install.log";

    // 打开日志文件
    QFile logFile(logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);

        // 获取当前时间，写入时间戳和日志内容
        QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        out << "[NSIS-UI-Plugin] " << timeStamp << " - " << info << "\n";

        logFile.close();
    }
    else {
        // 错误处理：无法打开日志文件
        OutputDebugString(TEXT("Unable to open log file.\n"));
    }
}

void printLogW(const TCHAR* szMsg) {
    // 将 TCHAR 转换为 QString
    QString info = QString::fromWCharArray(szMsg);
    printLog(info);
}

#define NSMETHOD_INIT() do {\
        PluginContext::Instance()->SetExtraParameters(extra); \
        PluginContext::Instance()->SetParentHwnd(hwndParent); \
        if(extra) { \
        extra->RegisterPluginCallback(PluginContext::Instance()->pluginHandle(), PluginCallback); } \
        EXDLL_INIT(); } while(false)

static UINT_PTR PluginCallback(enum NSPIM msg) {
    return 0;
}

NSISAPI ShowSetupUI(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    //Sleep(10 * 1000);
    TCHAR szAppName[MAX_PATH] = { 0 };
    popstring(szAppName);

    TCHAR szDefaultInstallDir[MAX_PATH] = { 0 };
    popstring(szDefaultInstallDir);

    TCHAR szNsisPluginDir[MAX_PATH] = { 0 };
    popstring(szNsisPluginDir);

    TCHAR szNsAutoInstall[MAX_PATH] = { 0 };
    popstring(szNsAutoInstall);

    // Start show Qt UI
    //
    if (_tcslen(szNsisPluginDir) > 0) {
        QApplication::addLibraryPath(tstringToQString(szNsisPluginDir));
    }

    std::wstring szTitle = std::wstring(szAppName) + _T(" Setup");

#ifdef Q_OS_WIN
    qputenv("QT_QPA_PLATFORM", "windows:fontengine=freetype");
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    int argc = 1;
    char currentPath[MAX_PATH] = { 0 };
    GetModuleFileNameA(NULL, currentPath, MAX_PATH);
    char *argv[2] = { {currentPath}, {} };
    QApplication app(argc, argv);
    app.setFont(QFont("Microsoft YaHei UI", 9));
    SetupPage_Qt *mainPage = new SetupPage_Qt();
    mainPage->setWindowTitle(tstringToQString(szTitle));
    mainPage->SetInstallDirectory(szDefaultInstallDir);
    PluginContext::Instance()->SetSetupPage(mainPage);
    QString strAutoInstall = tstringToQString(szNsAutoInstall);
    if (strAutoInstall == "1") {
        mainPage->StartInstall(true);
    }
    while (true)
    {
        std::wstring szExeName = std::wstring(szAppName) + std::wstring(L".exe");
        bool isRuning = IsProcessRunning(szExeName.c_str());
        if (isRuning) {
            std::wstring msgTitle = _T("警告");
            std::wstring msgContent = _T("应用程序正在运行，请先关闭应用程序再重试安装！");
            int ret = QMessageBox::warning(0, tstringToQString(msgTitle), tstringToQString(msgContent));
            if (ret == QMessageBox::Ok) {
                continue;
            }
            return;
        }
        else {
            break;
        }
    }
    mainPage->setAppName(szAppName);
    mainPage->show();
    app.exec();
}

NSISAPI ParseAutoInstall(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    TCHAR szCMDLINE[MAX_PATH] = { 0 };
    popstring(szCMDLINE);

    tstring strAutoInstall = _T("0");
    std::wstring strCmdLine = szCMDLINE;
    printLog("ParseAutoInstall...");
    std::wstring key = _T("/AutoInstall=");
    size_t startPos = strCmdLine.find(key);
    //printLogW(strCmdLine.c_str());
    if (startPos != std::wstring::npos) {
        printLog("startPos != std::wstring::npos");
        startPos += key.length();
        //printLog("startPos: " + QString::number(startPos));
        size_t endPos = strCmdLine.find(_T(" "), startPos); // 找到值后面的空格位置
        if (endPos == std::basic_string<TCHAR>::npos) {
            endPos = strCmdLine.length(); // 如果没有空格，取到字符串的末尾
        }
        //printLog("endPos: " + QString::number(endPos));
        std::wstring paramValue = strCmdLine.substr(startPos, endPos - startPos);
        printLog("AutoInstall paramValue: ");
        printLogW(paramValue.c_str());
        if (paramValue == _T("1")) {
            printLog(("---- found AutoInstall"));
            strAutoInstall = _T("1");
        }
    }
    pushstring(strAutoInstall.c_str());
}

NSISAPI KillProcess(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    TCHAR szProcessName[MAX_PATH] = { 0 };
    popstring(szProcessName);

    QProcess::startDetached("taskkill", QStringList() << "/f" << "/im" << tstringToQString(szProcessName));
}

NSISAPI OutputDebugInfo(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();

    TCHAR szInfo[1024] = { 0 };
    popstring(szInfo);

    //TCHAR szAllInfo[1124] = { 0 };
    //StringCchPrintf(szAllInfo, 1124, TEXT("[NSIS-UI-Plugin] %s\r\n"), szInfo);
    //OutputDebugString(szAllInfo);

    printLogW(szInfo);
}

NSISAPI BindInstallEventToNsisFunc(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();

    TCHAR szEventName[MAX_PATH] = { 0 };
    popstring(szEventName);
    long callbackFuncAddress = popint();

    PluginContext::Instance()->BindInstallEvent(szEventName, callbackFuncAddress);
}

NSISAPI BindButtonClickedEventToNsisFunc(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();

    TCHAR szControlName[MAX_PATH] = { 0 };
    popstring(szControlName);

    long callbackFuncAddress = popint();

    PluginContext::Instance()->BindButtonClickedEvent(szControlName, callbackFuncAddress);
}

NSISAPI NsisExtractFilesFinished(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();

    if (PluginContext::Instance()->GetSetupPage()) {
        PluginContext::Instance()->GetSetupPage()->NsisExtractFilesFinished();
    }
}


NSISAPI SetInstallDirectory(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    TCHAR szDir[MAX_PATH] = { 0 };
    popstring(szDir);

    if (PluginContext::Instance()->GetSetupPage()) {
        PluginContext::Instance()->GetSetupPage()->SetInstallDirectory(szDir);
    }
}

NSISAPI GetInstallDirectory(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    tstring strDir;
    if (PluginContext::Instance()->GetSetupPage()) {
        strDir = PluginContext::Instance()->GetSetupPage()->GetInstallDirectory();
    }
    pushstring(strDir.c_str());
}


NSISAPI SetInstallStepDescription(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    TCHAR szDescription[MAX_PATH] = { 0 };
    long percent = -1;

    popstring(szDescription);
    percent = popint();

    if (PluginContext::Instance()->GetSetupPage()) {
        PluginContext::Instance()->GetSetupPage()->SetInstallStepDescription(szDescription, percent);
    }
}

NSISAPI IsCreateDesktopShortcutEnabled(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    long enabled = 0;

    if (PluginContext::Instance()->GetSetupPage()) {
        enabled = PluginContext::Instance()->GetSetupPage()->IsCreateDesktopShortcutEnabled() ? 1 : 0;
    }
    pushint(enabled);
}

NSISAPI IsAutoStartupOnBootEnabled(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    long enabled = 0;

    if (PluginContext::Instance()->GetSetupPage()) {
        enabled = PluginContext::Instance()->GetSetupPage()->IsAutoStartupOnBootEnabled() ? 1 : 0;
    }
    pushint(enabled);
}

NSISAPI IsRunNowEnabled(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    NSMETHOD_INIT();
    long enabled = 0;

    if (PluginContext::Instance()->GetSetupPage()) {
        enabled = PluginContext::Instance()->GetSetupPage()->IsRunNowEnabled() ? 1 : 0;
    }
    pushint(enabled);
}

NSISAPI BackgroundRun(HWND hwndParent, int stringSize, TCHAR *variables, stack_t **stacktop, ExtraParameters *extra) {
    NSMETHOD_INIT();
    long nsisFuncAddress = popint();

    std::thread t = std::thread([nsisFuncAddress]() {
        PluginContext::Instance()->ExecuteNsisFunction(nsisFuncAddress - 1);
    });
    t.detach();
}