/*******************************************************************************************************************************************************
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

const QString appUniqueId = "67e55044-10b1-4ca4-9ba2-77f889847cd7";

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

static bool isAnotherInstanceRunning(QSharedMemory& sharedMemory) {
    // 尝试附加到已存在的共享内存
    if (sharedMemory.attach()) {
        // 如果附加成功，说明有其他实例正在运行
        return true;
    }
    // 如果附加失败，但错误是共享内存已存在，也说明有其他实例正在运行
    if (sharedMemory.error() == QSharedMemory::AlreadyExists) {
        return true;
    }
    // 创建新的共享内存，大小为1字节
    sharedMemory.create(1);
    // 这里可以添加一些数据到共享内存，用于标识或通信
    return false;
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

    TCHAR szNsUpdateInstall[MAX_PATH] = { 0 };
    popstring(szNsUpdateInstall);

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
    mainPage->SetPluginsDir(szNsisPluginDir);
    PluginContext::Instance()->SetSetupPage(mainPage);
    QSharedMemory sharedMemory(appUniqueId);
    if (isAnotherInstanceRunning(sharedMemory)) {
        std::wstring msgTitle = _T("警告");
        std::wstring msgContent = _T("安装程序正在运行");
        QMessageBox::warning(0, tstringToQString(msgTitle), tstringToQString(msgContent));
        return;
    }
    while (true)
    {
        std::wstring szExeName = std::wstring(szAppName) + std::wstring(L".exe");
        bool isRuning = IsProcessRunning(szExeName.c_str());
        if (isRuning) {
            static int iTryTime = 0;
            if (iTryTime <= 5) {
                ++iTryTime;
                Sleep(100);
                continue;
            }

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
    QString strUpdateInstall = tstringToQString(szNsUpdateInstall);
    if (strUpdateInstall == "1") {
        mainPage->StartInstall(true);
    }
    mainPage->setAppName(szAppName);
    mainPage->show();
    app.exec();
    sharedMemory.detach();
}

NSISAPI ParseUpdateInstall(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    NSMETHOD_INIT();
    TCHAR szCMDLINE[MAX_PATH] = { 0 };
    popstring(szCMDLINE);

    tstring strUpdateInstall = _T("0");
    std::wstring strCmdLine = szCMDLINE;
    printLog("ParseUpdateInstall...");
    std::wstring key = _T("/UpdateInstall=");
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
        printLog("UpdateInstall paramValue: ");
        printLogW(paramValue.c_str());
        if (paramValue == _T("1")) {
            printLog(("---- found UpdateInstall"));
            strUpdateInstall = _T("1");
        }
    }
    pushstring(strUpdateInstall.c_str());
}

NSISAPI KillProcess(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    NSMETHOD_INIT();
    TCHAR szProcessName[MAX_PATH] = { 0 };
    popstring(szProcessName);

    QProcess::execute("taskkill", QStringList() << "/f" << "/im" << tstringToQString(szProcessName));
}

NSISAPI VersionCompare(HWND hwndParent, int stringSize, TCHAR* variables, stack_t** stacktop, ExtraParameters* extra) {
    NSMETHOD_INIT();
    //Sleep(10 * 1000);
    TCHAR szString1[MAX_PATH] = { 0 };
    popstring(szString1);

    TCHAR szString2[MAX_PATH] = { 0 };
    popstring(szString2);

    QString version1 = tstringToQString(szString1);
    QString version2 = tstringToQString(szString2);
    if (version1 == version2) {
        pushint(0);
    }
    else {
        // 分割版本号字符串，按照点号（.）分割
        QStringList v1Parts = version1.split('.');
        QStringList v2Parts = version2.split('.');

        // 获取版本号的最大长度（两者中最大的部分数）
        int maxLength = std::max(v1Parts.size(), v2Parts.size());

        // 对比每个部分
        for (int i = 0; i < maxLength; ++i) {
            // 取出每个部分，若版本号不够长，缺失的部分视为0
            int v1 = (i < v1Parts.size()) ? v1Parts[i].toInt() : 0;
            int v2 = (i < v2Parts.size()) ? v2Parts[i].toInt() : 0;

            // 如果发现不相等，返回比较结果
            if (v1 > v2) {
                pushint(1); // version1 大于 version2
                return;
            }
            if (v1 < v2) {
                pushint(-1); // version1 小于 version2
                return;
            }
        }
    }

    pushint(0);
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