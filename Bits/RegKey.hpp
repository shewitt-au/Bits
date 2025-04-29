#pragma once
/*
RegKey.hpp

Registry manipulation.
This shit ain't meant to be globally useful. Locally useful will do just fine.
*/

#include <stdint.h>
#include <string.h>
#pragma intrinsic(wcslen)

class RegKey
{
public:
    RegKey(HKEY hkParent, LPCSTR pSubKey = NULL)
    {
        LSTATUS st = RegOpenKeyExA(
                        hkParent,       // HKEY   hKey
                        pSubKey,        // LPCSTR lpSubKey
                        0,              // DWORD  ulOptions
                        KEY_ALL_ACCESS, // REGSAM samDesired
                        &m_hKey         // PHKEY  phkResult
                        );
        if (st!=ERROR_SUCCESS)
            m_hKey = NULL;
    }

    RegKey(const RegKey& parent, LPCSTR pSubKey = NULL)
        : RegKey(parent.m_hKey, pSubKey)
    {
    }

    operator bool() const
    {
        return m_hKey != NULL;
    }

    ~RegKey()
    {
        if (m_hKey)
            RegCloseKey(m_hKey);
    }

    RegKey CreateKey(LPCSTR pName)
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
        if (st!=ERROR_SUCCESS)
            sub.m_hKey = NULL;

        return sub;
    }

    bool DeleteKey(LPCSTR pName)
    {
        LRESULT st = RegDeleteKeyA(
                        m_hKey, // HKEY hKey
                        pName   // LPCSTR lpSubKey
                        );

        return st == ERROR_SUCCESS;
    };

    bool SetValue(LPCWSTR pValue, LPCWSTR pKey=NULL)
    {
        LRESULT st = RegSetKeyValue(
                        m_hKey,     // HKEY hKey
                        pKey,       // LPCSTR lpSubKey
                        NULL,       // LPCSTR lpValueName
                        REG_SZ,     // DWORD dwType
                        pValue,     // LPCVOID lpData
                        (wcslen(pValue)+1)*sizeof(WCHAR) // DWORD cbData
                        );

        return st == ERROR_SUCCESS;
    }

private:
    RegKey() : m_hKey(NULL)
    {
    }

    HKEY m_hKey;
};
