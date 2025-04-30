// "Shell.cpp"

/*
Manage Explorer context menu integration.
*/

#define VERSIONSTRING "1.1"

#include "framework.hpp"
#include <shellapi.h>
#include "RegKey.hpp"

const char g_AppName[] = "Bits";
const char g_MenuItemName[] = "32 or 64-bit?";

/*
To install or uninstall the Explorer context menu we need administrator
privileges. We spawn another instance of ourself requesting elevation.
Was pass a single ‘:’ character, which is illegal in filenames, as the
parameter to signal that we want to manage the context menu.
*/
void ElevateClone()
{
    WCHAR MyPath[MAX_PATH];
    GetModuleFileName(NULL, MyPath, sizeof(MyPath) / sizeof(WCHAR));

    SHELLEXECUTEINFO Info = {
        sizeof(SHELLEXECUTEINFO),   // DWORD cbSize
        0,                          // ULONG fMask
        NULL,                       // HWND hwnd
        L"runas",                   // LPCWSTR lpVerb
        MyPath,                     // LPCWSTR lpFile
        L":",                       // LPCWSTR lpParameters
        NULL,                       // LPCWSTR lpDirectory
        SW_SHOWNORMAL,              // int nShow
        NULL,                       // HINSTANCE hInstApp
        NULL,                       // void* lpIDList
        NULL,                       // LPCWSTR lpClass
        NULL,                       // HKEY hkeyClass
        0,                          // DWORD dwHotKey
        NULL,                       //  union {
                                    //     HANDLE hIcon;
                                    //     HANDLE hMonitor;
                                    //     } DUMMYUNIONNAME;
        NULL                        // HANDLE hProcess
    };

    if (!ShellExecuteEx(&Info))
        MessageBoxA(NULL, "Failed to elevate!", NULL, MB_OK | MB_ICONERROR);
}

void InstallUninstallProblem()
{
    MessageBoxA(
        NULL,
        "Error installing/uninstalling context menu!",
        g_AppName,
        MB_OK | MB_ICONERROR
    );
}

void Install()
{
    RegKey star(HKEY_CLASSES_ROOT, "*");
    if (!star)
    {
        InstallUninstallProblem();
        return;
    }
    RegKey shell(star, "shell");
    if (!shell)
    {
        InstallUninstallProblem();
        return;
    }
    RegKey menu_entry = shell.CreateKey(g_MenuItemName);
    if (!menu_entry)
    {
        InstallUninstallProblem();
        return;
    }

    WCHAR MyPath[MAX_PATH];
    GetModuleFileName(NULL, MyPath, sizeof(MyPath) / sizeof(WCHAR));
    //                       "   "  sp   %   1
    WCHAR Command[MAX_PATH + 1 + 1 + 1 + 1 + 1];
    LPCWSTR pSource = MyPath;
    LPWSTR  pDest = Command;
    *(pDest++) = L'"';
    for (; *pSource; ++pSource, ++pDest)
        *pDest = *pSource;
    *(pDest++) = L'"';
    *(pDest++) = L' ';
    *(pDest++) = L'%';
    *(pDest++) = L'1';
    *(pDest++) = 0;

    bool ok = menu_entry.SetValue(Command, L"command");
    if (!ok)
    {
        InstallUninstallProblem();
        return;
    }
}

void Uninstall()
{
    RegKey star(HKEY_CLASSES_ROOT, "*");
    if (!star)
    {
        InstallUninstallProblem();
        return;
    }
    RegKey shell(star, "shell");
    if (!shell)
    {
        InstallUninstallProblem();
        return;
    }
    RegKey menu_entry(shell, g_MenuItemName);
    if (!menu_entry)
    {
        InstallUninstallProblem();
        return;
    }
    bool ok = menu_entry.DeleteKey("command");
    if (!ok)
    {
        InstallUninstallProblem();
        return;
    }
    ok = shell.DeleteKey(g_MenuItemName);
    if (!ok)
    {
        InstallUninstallProblem();
        return;
    }
}

void ManageShellIntegration()
{
    int req = MessageBoxA(
        NULL,
        "Bits v" VERSIONSTRING " \n"
        "By Stephen Hewitt\n\n"
        "Do you want an Explorer context menu?\n"
        "Selecting \"Yes\" adds one, and \"No\" removes it.",
        g_AppName,
        MB_YESNOCANCEL | MB_ICONQUESTION
    );
    switch (req)
    {
    case IDYES:
        Install();
        break;
    case IDNO:
        Uninstall();
        break;
    }
}
