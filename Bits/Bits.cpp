// Bits.cpp : Defines the entry point for the application.
//

#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text") // REFUSES TO WORK

#include "framework.h"

extern "C" int MyStartup()
{
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	LPWSTR lpCmdLine = GetCommandLine();
	STARTUPINFO si;
	GetStartupInfo(&si);
	return wWinMain(hInst, NULL, lpCmdLine, si.wShowWindow);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // TODO: Place code here.

	MessageBox(NULL, L"Hello", lpCmdLine, MB_OK);
	return 0;
}
