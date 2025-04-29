#include "framework.hpp"
#include "RegKey.hpp"
#include <string.h>
#pragma intrinsic(wcslen)

RegKey::RegKey(HKEY hkParent, LPCSTR pSubKey)
{
    LSTATUS st = RegOpenKeyExA(
        hkParent,       // HKEY   hKey
        pSubKey,        // LPCSTR lpSubKey
        0,              // DWORD  ulOptions
        KEY_ALL_ACCESS, // REGSAM samDesired
        &m_hKey         // PHKEY  phkResult
    );
    if (st != ERROR_SUCCESS)
        m_hKey = NULL;
}

RegKey RegKey::CreateKey(LPCSTR pName)
{
    RegKey sub;
    LSTATUS st = RegCreateKeyExA(
        m_hKey,         // HKEY hKey
        pName,          // LPCSTR lpSubKey
        0,              // DWORD Reserved
        NULL,           // LPSTR lpClass
        0,              // DWORD dwOptions
        KEY_ALL_ACCESS, // REGSAM samDesired
        NULL,           // const LPSECURITY_ATTRIBUTES lpSecurityAttributes
        &sub.m_hKey,    // PHKEY phkResult
        NULL            // LPDWORD lpdwDisposition
    );
    if (st != ERROR_SUCCESS)
        sub.m_hKey = NULL;

    return sub;
}

bool RegKey::SetValue(LPCWSTR pValue, LPCWSTR pKey)
{
    LRESULT st = RegSetKeyValue(
        m_hKey,     // HKEY hKey
        pKey,       // LPCSTR lpSubKey
        NULL,       // LPCSTR lpValueName
        REG_SZ,     // DWORD dwType
        pValue,     // LPCVOID lpData
        (wcslen(pValue) + 1) * sizeof(WCHAR) // DWORD cbData
    );

    return st == ERROR_SUCCESS;
}
