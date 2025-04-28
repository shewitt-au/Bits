// Bits.cpp : Defines the entry point for the application.
//

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#ms-dos-stub-image-only

#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text") // REFUSES TO WORK

#include "framework.h"
#include <string.h>
#pragma intrinsic(wcslen)
#include <stdint.h>
#include <shellapi.h>

// To install or uninstall the Explorer context menu we need administrator
// privileges. We spawn another instance of ourself requesting elevation.
// Was pass a single ‘:’ character, which is illegal in filenames, as the
// parameter to signal that we want to manage the context menu.
void ElevateClone()
{
    WCHAR MyPath[MAX_PATH];
    GetModuleFileName(NULL, MyPath, sizeof(MyPath)/sizeof(WCHAR));

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

class RegKey
{
public:
    RegKey(HKEY hkParent, LPCSTR pSubKey = NULL)
    {
        m_hKey = NULL;
        LSTATUS st = RegOpenKeyExA(
            hkParent,       // HKEY   hKey
            pSubKey,        // LPCSTR lpSubKey
            0,              // DWORD  ulOptions
            KEY_ALL_ACCESS, // REGSAM samDesired
            &m_hKey         // PHKEY  phkResult
        );
    }

    RegKey(const RegKey& parent, LPCSTR pSubKey = NULL)
        : RegKey(parent.m_hKey, pSubKey)
    {
    }

    operator bool() const
    {
        return m_hKey != NULL;
    }

    ~RegKey()
    {
        if (m_hKey)
            RegCloseKey(m_hKey);
    }

    RegKey CreateKey(LPCSTR pName)
    {
        RegKey sub;
        RegCreateKeyExA(
            m_hKey,             // HKEY hKey
            pName,              // LPCSTR lpSubKey
            0,                  // DWORD Reserved
            NULL,               // LPSTR lpClass
            0,                  // DWORD dwOptions
            KEY_ALL_ACCESS,     // REGSAM samDesired
            NULL,               // const LPSECURITY_ATTRIBUTES lpSecurityAttributes
            &sub.m_hKey,        // PHKEY phkResult
            NULL                // LPDWORD lpdwDisposition
        );

        return sub;
    }
    bool DeleteKey(LPCSTR pName)
    {
        LRESULT st = RegDeleteKeyA(
            m_hKey,     // HKEY  hKey
            pName       // LPCSTR lpSubKey
        );

        return st != ERROR_SUCCESS;
    };

    bool SetValue(LPCWSTR pValue, LPCWSTR pKey=NULL)
    {
        LRESULT st = RegSetKeyValue(
            m_hKey,             // HKEY hKey
            pKey,               // LPCSTR lpSubKey
            NULL,               // LPCSTR lpValueName
            REG_SZ,             // DWORD dwType
            pValue,             // LPCVOID lpData
            (wcslen(pValue)+1)*sizeof(WCHAR)    // DWORD cbData
        );

        return st != ERROR_SUCCESS;
    }

private:
    RegKey() : m_hKey(NULL)
    {
    }

    HKEY m_hKey;
};

void Install()
{
    RegKey star(HKEY_CLASSES_ROOT, "*");
    RegKey shell(star, "shell");
    RegKey menu_entry = shell.CreateKey("32 or 64 bit?");

    WCHAR MyPath[MAX_PATH];
    GetModuleFileName(NULL, MyPath, sizeof(MyPath)/sizeof(WCHAR));
    //                       "       "   %   1
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

    menu_entry.SetValue(Command, L"command");
}

void Uninstall()
{
    RegKey star(HKEY_CLASSES_ROOT, "*");
    RegKey shell(star, "shell");
    RegKey menu_entry(shell, "32 or 64 bit?");
    menu_entry.DeleteKey("command");
    shell.DeleteKey("32 or 64 bit?");
}

void ManageShellIntegration()
{
    int req = MessageBoxA(
                NULL,
                "Do you want an Explorer context menu?\n"
                    "Selecting \"Yes\" adds one, and \"No\" removes it.",
                "Explorer context menu?",
                MB_YESNOCANCEL | MB_ICONQUESTION
                );
    switch (req)
    {
    case IDYES:
        Install();
        break;
    case IDNO:
        Uninstall();
    }
}

LPWSTR WithoutExe(LPWSTR pCmdLine)
{
    if (*pCmdLine == L'"')
    {
        for (++pCmdLine; *pCmdLine != L'"'; ++pCmdLine) {}
        for (++pCmdLine; *pCmdLine == L' '; ++pCmdLine) {}
        return pCmdLine;
    }
    else
    {
        for (; *pCmdLine != L' '; ++pCmdLine) {}
        for (; *pCmdLine == L' '; ++pCmdLine) {}
        return pCmdLine;
    }
}

extern "C" int MyStartup()
{
    // We're trying to be small, so we don't want an embedded manifest.
    // On modern systems if DPI awareness isn't enabled things look
    // somewhat "retro". We want to get as fancy as the client system
    // supports but still run on DPI-unaware systems. To that end we use
    // runtime linking and try for the newest-fangledness we can get.
    HMODULE hU32 = GetModuleHandleA("user32.dll");
    if (hU32)
    {
        typedef DPI_AWARENESS_CONTEXT (WINAPI *PDPIAC)(DPI_AWARENESS_CONTEXT);
        PDPIAC pFn = (PDPIAC)GetProcAddress(hU32, "SetThreadDpiAwarenessContext");
        if (pFn)
        {
            auto old = (*pFn)(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            if (!old)
                (*pFn)(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
        }
    }

    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
    LPWSTR lpCmdLine = WithoutExe(GetCommandLine());
    STARTUPINFO si;
    GetStartupInfo(&si);
    ExitProcess(wWinMain(hInst, NULL, lpCmdLine, si.wShowWindow));

    return 0;
}

class MemFile
{
public:
    MemFile(LPCWSTR pFile)
    {
        m_pBegin = NULL;

        HANDLE hFile = CreateFileW(
            pFile,                 // LPCWSTR lpFileName
            GENERIC_READ,          // DWORD   dwDesiredAccess
            FILE_SHARE_READ,       // DWORD   dwShareMode
            NULL,                  // LPSECURITY_ATTRIBUTES lpSecurityAttributes
            OPEN_EXISTING,         // DWORD   dwCreationDisposition
            FILE_ATTRIBUTE_NORMAL, // DWORD   dwFlagsAndAttributes
            NULL                   // HANDLE  hTemplateFile
        );
        if (hFile == INVALID_HANDLE_VALUE)
            return;

        LARGE_INTEGER size;
        if (!GetFileSizeEx(hFile, &size))
        {
            CloseHandle(hFile);
            return;
        }

        HANDLE hMapping = CreateFileMapping(
            hFile,         // HANDLE hFile
            NULL,          // LPSECURITY_ATTRIBUTES lpFileMappingAttributes
            PAGE_READONLY, // DWORD  flProtect
            0,             // DWORD  dwMaximumSizeHigh
            0,             // DWORD  dwMaximumSizeLow
            NULL           // LPCSTR lpName
        );
        CloseHandle(hFile);
        if (hMapping == NULL)
            return;

        m_pBegin = (uint8_t*)MapViewOfFile(
            hMapping,      // HANDLE hFileMappingObject
            FILE_MAP_READ, // DWORD  dwDesiredAccess
            0,             // DWORD  dwFileOffsetHigh
            0,             // DWORD  dwFileOffsetLow
            0              // SIZE_T dwNumberOfBytesToMap
        );
        CloseHandle(hMapping);
        m_pEnd = m_pBegin + size.QuadPart;
    }

    ~MemFile()
    {
        if (m_pBegin)
            UnmapViewOfFile(m_pBegin);
    }

    uint8_t* view() const
    {
        return m_pBegin;
    }

    template <typename T>
    bool check(T *p) const
    {
        return (uint8_t*)p >= m_pBegin && (uint8_t*)p + sizeof(T) <= m_pEnd;
    }

private:
    uint8_t *m_pBegin;
    uint8_t *m_pEnd;

};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    if (*lpCmdLine == 0)
    {
        ElevateClone();
        return 0;
    }
    else if (*lpCmdLine==L':')
    {
        ManageShellIntegration();
        return 0;
    }

    WORD magic;
    PIMAGE_DOS_HEADER pDOS;
    PIMAGE_NT_HEADERS pNT;
    LPCWSTR pMsg = NULL;
    bool bError = false;

    MemFile mf(lpCmdLine);
    LPCVOID pView = mf.view();
    if (!pView)
    {
        pMsg = L"Can't map file!";
        bError = true;
        goto bail;
    }

    pDOS = (PIMAGE_DOS_HEADER)pView;
    if (!mf.check(&pDOS->e_lfanew))
    {
        pMsg = L"File too small!";
        bError = true;
        goto bail;
    }
    if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
    {
        pMsg = L"No DOS header!";
        bError = true;
        goto bail;
    }

    pNT = (PIMAGE_NT_HEADERS)((char*)pDOS + pDOS->e_lfanew);
    if (!mf.check(pNT))
    {
        pMsg = L"e_lfanew in DOS header out of range!";
        bError = true;
        goto bail;
    }

    if (!mf.check(&pNT->OptionalHeader.Magic))
    {
        pMsg = L"File too small!";
        bError = true;
        goto bail;
    }

    if (pNT->Signature != IMAGE_NT_SIGNATURE)
    {
        pMsg = L"No NT header!";
        bError = true;
        goto bail;
    }

    magic = pNT->OptionalHeader.Magic;

    switch (magic)
    {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        pMsg = L"32 bit";
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        pMsg = L"64 bit";
        break;
    default:
        pMsg = L"Unknown";
        break;
    }

bail:
    MessageBox(NULL, pMsg, lpCmdLine, MB_OK|(bError?MB_ICONERROR:0));

    return 0;
}
