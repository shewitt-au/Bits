// "App.cpp"

/*
Application scaffolding

This is where a lot of the hoops that must be jumped through to keep the app
small are jumped through. If the app were more elaborate this hoop-jumping
would be pointless and counterproductive. As an app gets bigger and bigger,
the space saved with such shenanigans becomes more and more insignificant,
not to mention the fact that the hoop-jumping limits the language features
at our disposal. But Bits does little more than wade ankle deep through
some 1's and 0's and display a rudimentary UI (MessageBox).

So, what makes a small app so big?
 	So, you write a "Hello, World!" console app, build a 32-bit release
version, and it's 11K! What gives?

Well, probably the biggest one is the runtime libraries. Many language features
require runtime support. Like for calling the constructors and destructors of
global objects; exception handling; the implementation of various standard
library functions; code to handle stuff like new and delete; the list goes on.
A lot of this code is implemented in DLLs. Even so, every function that's
imported from a DLL must have bookkeeping information in the importing module.
What function are we importing? From what DLL? Then there's code/data that, for
various reasons, can’t be imported from a DLL (or is, perhaps, just better in
the consumer) and must be statically linked. The entry point of an app is an
example of code that can't be imported from a DLL. One of the headers of a PE
file specifies the address of the entry point. An exe can't know the addresses
DLLs will be loaded at so the entry point must be in the code of the app itself.
And I'm talking about the real entry point here. "WinMain" (or "main" for a
console app) is't it. A runtime function is the real entry point. It
initialises the runtime libraries, calls the constructors of global objects,
does other stuff, and then calls what a day-to-day programmer rightfully
considers the entry point. Then it does other stuff afterwards to clean up, like
calling the destructors of global object for example.

So how do we get rid of this overhead? Write your own entry point and tell the
linker to call your version using the "/ENTRY" linker switch. You can use this
function directly, or, as I've done in Bits, set things up and then call a more
tradition entry point with the expected parameters and environment set up. After
doing this and compiling you'll probably get problems. For example, if exception
handling is enabled the app is going to need runtime support. You're going have
to disable exceptions (see the "/EH" compiler switches).

The next tick employed is section merging. When you build an app it has multiple
sections for various types of code and data. Each section as its own memory
region and memory protection settings. Some common sections are:
   - ".text":  this section is for code and is readable and executable,
               but not writable.
   - ".data":  this section contains initialised data and is readable, writable
               but not executable.
•  - ".idata": this section contains DLL import data.

Sections have an overhead. There's the cost of the bookkeeping and then there's
the allocation granularity of the section data itself. The linker enables
sections to be merged using the "MERGE" option. You used to be able to merge the
".idata" section but current versions of the MSVC toolchain don’t allow it. Care
must be taken when merging sections with different memory protections.
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
//  the .data section is where initalised data is stored and it's
//  writable.
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
