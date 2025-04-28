/*
Bits is a Windows utility that determines whether a PE file is 32 or 64-bit.
It is designed to be as small as possible and to have no dependencies beyond
that which is guaranteed to be on the system. It adds an Explorer right-click
menu item.

Author: Stephen Hewitt
*/

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#ms-dos-stub-image-only

#include "framework.hpp"
#include "MemFile.hpp"
#include "Shell.hpp"

int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
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
    // e_lfanew is the last member of IMAGE_DOS_HEADER
    if (!mf.check(&pDOS->e_lfanew))
    {
        pMsg = L"File too small to fit an IMAGE_DOS_HEADER!";
        bError = true;
        goto bail;
    }
    if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
    {
        pMsg = L"No DOS header signature (e_magic)!";
        bError = true;
        goto bail;
    }

    // So far there have been no structural differences between 32-bit and
    // 64-bit PE files. In IMAGE_NT_HEADERS, specifically IMAGE_OPTIONAL_HEADER
    // contained within it, there are. IMAGE_OPTIONAL_HEADER has two versions:
    //  IMAGE_OPTIONAL_HEADER32 and IMAGE_OPTIONAL_HEADER64.
    // Consequently IMAGE_NT_HEADERS has two flavours:
    //  IMAGE_NT_HEADERS32 and IMAGE_NT_HEADERS64
    //
    // typedef struct _IMAGE_NT_HEADERS {
    //     DWORD Signature;
    //     IMAGE_FILE_HEADER FileHeader;
    //     IMAGE_OPTIONAL_HEADER32 OptionalHeader;
    // } IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;
    //
    // typedef struct _IMAGE_NT_HEADERS64 {
    //     DWORD Signature;
    //     IMAGE_FILE_HEADER FileHeader;
    //     IMAGE_OPTIONAL_HEADER64 OptionalHeader;
    // } IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;
    
    pNT = (PIMAGE_NT_HEADERS32)((char*)pDOS + pDOS->e_lfanew);
    if (!mf.check(pNT))
    {
        pMsg = L"Can't fit an IMAGE_NT_HEADERS32 at the offset pointed to by";
               L"e_lfanew (from IMAGE_DOS_HEADER)!";
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
        pMsg = L"32-bit";
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        pMsg = L"64-bit";
        break;
    default:
        pMsg = L"Unknown";
        break;
    }

bail:
    MessageBox(NULL, pMsg, lpCmdLine, MB_OK|(bError?MB_ICONERROR:0));

    return 0;
}
