// "Bits.cpp"

/*
Bits is a Windows utility that determines whether a PE file is 32 or 64-bit.
It is designed to be as small as possible and to have no dependencies beyond
that which is guaranteed to be on the system. It adds an Explorer right-click
menu item.

Author: Stephen Hewitt
*/

// Links:
//
// https://docs.microsoft.com/en-us/archive/msdn-magazine/2002/february/inside-windows-win32-portable-executable-file-format-in-detail
// https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#ms-dos-stub-image-only
//

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
    PIMAGE_NT_HEADERS32 pNT;
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

    // typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    //     WORD   e_magic;                     // Magic number
    //     WORD   e_cblp;                      // Bytes on last page of file
    //     WORD   e_cp;                        // Pages in file
    //     WORD   e_crlc;                      // Relocations
    //     WORD   e_cparhdr;                   // Size of header in paragraphs
    //     WORD   e_minalloc;                  // Minimum extra paragraphs needed
    //     WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    //     WORD   e_ss;                        // Initial (relative) SS value
    //     WORD   e_sp;                        // Initial SP value
    //     WORD   e_csum;                      // Checksum
    //     WORD   e_ip;                        // Initial IP value
    //     WORD   e_cs;                        // Initial (relative) CS value
    //     WORD   e_lfarlc;                    // File address of relocation table
    //     WORD   e_ovno;                      // Overlay number
    //     WORD   e_res[4];                    // Reserved words
    //     WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    //     WORD   e_oeminfo;                   // OEM information; e_oemid specific
    //     WORD   e_res2[10];                  // Reserved words
    //     LONG   e_lfanew;                    // File address of new exe header
    // } IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;

    pDOS = (PIMAGE_DOS_HEADER)pView;
    if (!mf.check(pDOS))
    {
        pMsg = L"File too small to fit an IMAGE_DOS_HEADER!";
        bError = true;
        goto bail;
    }
    if (pDOS->e_magic != IMAGE_DOS_SIGNATURE)
    {
        pMsg = L"Wrong IMAGE_DOS_HEADER signature!";
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

    if (pNT->Signature != IMAGE_NT_SIGNATURE)
    {
        pMsg = L"Wrong IMAGE_NT_HEADERS signature!";
        bError = true;
        goto bail;
    }

    // 'Magic' is the first member of IMAGE_OPTIONAL_HEADER (32 & 64) and thus
    // is before any differences between the 32 & 64-bit versions.
    magic = pNT->OptionalHeader.Magic;

    switch (magic)
    {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        pMsg = L"32-bit";
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        if (!mf.check((PIMAGE_NT_HEADERS64)pNT))
        {
            pMsg = L"Looks like an invalid 64 bit file!\n";
                   L"Can't fit an IMAGE_NT_HEADERS64 at the offset pointed to by";
                   L"e_lfanew (from IMAGE_DOS_HEADER).";
            bError = true;
        }
        else
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
