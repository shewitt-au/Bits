#pragma once
/*
MemFile.hpp

A memory mapped file class.
This shit ain't meant to be globally useful. Locally useful will do just fine.
*/

#include <stdint.h>
#pragma intrinsic(wcslen)

class MemFile
{
public:
    MemFile(LPCWSTR pFile)
    {
        m_pBegin = NULL;
        m_pOnePastEnd = NULL;

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
        m_pOnePastEnd = m_pBegin + size.QuadPart;
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
        return (uint8_t*)p>=m_pBegin && (uint8_t*)p+sizeof(T)<m_pOnePastEnd;
    }

private:
    uint8_t *m_pBegin;
    uint8_t *m_pOnePastEnd;
};
