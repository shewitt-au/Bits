#pragma once
/*
MemFile.hpp

A memory mapped file class.
This shit ain't meant to be globally useful. Locally useful will do just fine.
*/

#include "framework.hpp"
#include <stdint.h>

class MemFile
{
public:
    MemFile(LPCWSTR pFile);
    ~MemFile();

    uint8_t* view() const;

    template <typename T>
    bool check(T *p) const
    {
        return (uint8_t*)p>=m_pBegin && (uint8_t*)p+sizeof(T)<m_pOnePastEnd;
    }

private:
    uint8_t *m_pBegin;
    uint8_t *m_pOnePastEnd;
};

// Inline functions

inline MemFile::MemFile(LPCWSTR pFile)
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

inline MemFile::~MemFile()
{
    if (m_pBegin)
        UnmapViewOfFile(m_pBegin);
}

inline uint8_t* MemFile::view() const
{
    return m_pBegin;
}
