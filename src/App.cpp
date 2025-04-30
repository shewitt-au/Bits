// "App.cpp"

/*
Application scaffolding
*/

#include "framework.hpp"
#include <shellapi.h>
#include "RegKey.hpp"

// Merging the .rdata (read-only data) into .text is a no-brainer.
#pragma comment(linker, "/merge:.rdata=.text")

// This one is more complicated. It gives the following linker warning:
//  "warning LNK4254: section '.data' (C0000040) merged into '.text'
//   (60000020) with different attributes"
// We're going to use it anyway, but buyer beware:
//  The .data section is where uninitalised data is stored and it's
//  writable. The .text section in non-writable. So uninitialized
//  globals will not be modifiable without generating an access
//  violation.
#pragma comment(linker, "/merge:.data=.text")

//#pragma comment(linker, "/merge:.idata=.text") // REFUSES TO WORK

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

/*
I want Bits to be as small as possible and to have no dependencies beyond what
is guaranteed to be present on the system. To that end I’m not using any C/C++
runtimes. Normally the entry point in a Windows "/SUBSYSTEM:WINDOWS" process
is _WinMainCRTStartup (which calls WinMain). _WinMainCRTStartup is dependent on
the C/C++ runtime library. We bypass _WinMainCRTStartup and provide our own.
This has costs beyond the obvious (can’t use C/C++ runtime functions),
SOME of which are:
  - No constructors/destructors are called on global objects
  - No exception handling
  - No new and delete
*/
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
            {
                (*pFn)(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
                if (!old)
                    (*pFn)(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
            }
        }
    }

    HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
    LPWSTR lpCmdLine = WithoutExe(GetCommandLine());
    STARTUPINFO si;
    GetStartupInfo(&si);
    ExitProcess(wWinMain(hInst, NULL, lpCmdLine, si.wShowWindow));

    return 0;
}
