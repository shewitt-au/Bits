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

LPVOID Map(LPCWSTR pFile)
{
	HANDLE hFile = CreateFileW(
		pFile,                 // LPCWSTR lpFileName
		GENERIC_READ,	       // DWORD   dwDesiredAccess
		FILE_SHARE_READ,       // DWORD   dwShareMode
		NULL,                  // LPSECURITY_ATTRIBUTES lpSecurityAttributes
		OPEN_EXISTING,         // DWORD   dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL, // DWORD   dwFlagsAndAttributes
		NULL                   // HANDLE  hTemplateFile
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	HANDLE hMapping = CreateFileMapping(
		hFile,         // HANDLE hFile
		NULL,          // LPSECURITY_ATTRIBUTES lpFileMappingAttributes
		PAGE_READONLY, //DWORD  flProtect
		0,             // DWORD  dwMaximumSizeHigh
		0,             // DWORD  dwMaximumSizeLow
		NULL           // LPCSTR lpName
	);
	CloseHandle(hFile);
	if (hMapping == NULL)
		return NULL;

	LPVOID pView = MapViewOfFile(
		hMapping,      // HANDLE hFileMappingObject
		FILE_MAP_READ, // DWORD  dwDesiredAccess
		0,             // DWORD  dwFileOffsetHigh
		0,             // DWORD  dwFileOffsetLow
		0              // SIZE_T dwNumberOfBytesToMap
	);
	CloseHandle(hMapping);

	return pView;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);
	
	WORD magic;
	PIMAGE_DOS_HEADER pDOS;
	PIMAGE_NT_HEADERS pNT;
	LPCWSTR pMsg = NULL;

	LPCVOID pView = Map(lpCmdLine);
	if (!pView)
	{
		pMsg = L"Can't map file!";
		goto bail;
	}

	pDOS = (PIMAGE_DOS_HEADER)pView;
	if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
	{
		pMsg = L"No DOS header!";
		goto bail;
	}
	pNT = (PIMAGE_NT_HEADERS)((char*)pDOS + pDOS->e_lfanew);
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
	UnmapViewOfFile(pView);
	MessageBox(NULL, pMsg, L"Bits", MB_OK);

	return 0;
}
