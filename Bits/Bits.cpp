// Bits.cpp : Defines the entry point for the application.
//

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#ms-dos-stub-image-only

#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text") // REFUSES TO WORK

#include "framework.h"

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
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);
	LPWSTR lpCmdLine = WithoutExe(GetCommandLine());
	STARTUPINFO si;
	GetStartupInfo(&si);
	return wWinMain(hInst, NULL, lpCmdLine, si.wShowWindow);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

	MessageBox(NULL, lpCmdLine, L"Bits", MB_OK);

	PIMAGE_DOS_HEADER pDOS = (PIMAGE_DOS_HEADER)hInstance;
	if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(NULL, L"Crap!", L"Bits", MB_OK);
		return 1;
	}

	return 0;
}
