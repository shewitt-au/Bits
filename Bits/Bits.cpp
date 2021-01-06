// Bits.cpp : Defines the entry point for the application.
//

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#ms-dos-stub-image-only

#pragma comment(linker, "/merge:.rdata=.text")
//#pragma comment(linker, "/merge:.idata=.text") // REFUSES TO WORK

#include "framework.h"
#include <stdint.h>

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
			GENERIC_READ,	       // DWORD   dwDesiredAccess
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
			PAGE_READONLY, //DWORD  flProtect
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

	//lpCmdLine = (LPWSTR)L"C:\\Users\\steve\\Desktop\\VDS_S4";
	
	WORD magic;
	PIMAGE_DOS_HEADER pDOS;
	PIMAGE_NT_HEADERS pNT;
	LPCWSTR pMsg = NULL;

////////////////////////////////////
//	lpCmdLine = (LPWSTR)LR"(C:\Program Files\WinVICE-3.1-x64\x64.exe)";
////////////////////////////////////

	MemFile mf(lpCmdLine);
	LPCVOID pView = mf.view();
	if (!pView)
	{
		pMsg = L"Can't map file!";
		goto bail;
	}

	pDOS = (PIMAGE_DOS_HEADER)pView;
	if (!mf.check(&pDOS->e_lfanew))
	{
		pMsg = L"File too small!";
		goto bail;
	}
	if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
	{
		pMsg = L"No DOS header!";
		goto bail;
	}

	pNT = (PIMAGE_NT_HEADERS)((char*)pDOS + pDOS->e_lfanew);
	if (!mf.check(pNT))
	{
		pMsg = L"e_lfanew in DOS header out of range!";
		goto bail;
	}

	if (!mf.check(&pNT->OptionalHeader.Magic))
	{
		pMsg = L"File too small!";
		goto bail;
	}

	if (pNT->Signature != IMAGE_NT_SIGNATURE)
	{
		pMsg = L"No NT header!";
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
	MessageBox(NULL, pMsg, lpCmdLine, MB_OK);

	return 0;
}
