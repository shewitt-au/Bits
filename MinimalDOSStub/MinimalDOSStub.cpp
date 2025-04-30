// MinimalDOSStub.cpp
// 
// Generate the smallest DOS stub we can.
// We link in into Bits using the /STUB linker option.
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h> 
#include <Windows.h>

using namespace std;

const IMAGE_DOS_HEADER g_DH =
{
    IMAGE_DOS_SIGNATURE, // WORD   e_magic;    // Magic number
    0,                   // WORD   e_cblp;     // Bytes on last page of file
    0,                   // WORD   e_cp;       // Pages in file
    0,                   // WORD   e_crlc;     // Relocations
    0,                   // WORD   e_cparhdr;  // Size of header in paragraphs
    0,                   // WORD   e_minalloc; // Minimum extra paragraphs needed
    0,                   // WORD   e_maxalloc; // Maximum extra paragraphs needed
    0,                   // WORD   e_ss;       // Initial (relative) SS value
    0,                   // WORD   e_sp;       // Initial SP value
    0,                   // WORD   e_csum;     // Checksum
    0,                   // WORD   e_ip;       // Initial IP value
    0,                   // WORD   e_cs;       // Initial (relative) CS value
    0,                   // WORD   e_lfarlc;   // File address of relocation table
    0,                   // WORD   e_ovno;     // Overlay number
    0,                   // WORD   e_res[4];   // Reserved words
    0,                   // WORD   e_oemid;    // OEM identifier (for e_oeminfo)
    0,                   // WORD   e_oeminfo;  // OEM information; e_oemid specific
    0,                   // WORD   e_res2[10]; // Reserved words
    0,                   // LONG   e_lfanew;   // File address of new exe header
};

int main()
{
    FILE* out = fopen("dosstub.exe", "wb");
    fwrite(&g_DH, 1, sizeof(g_DH), out);
}
