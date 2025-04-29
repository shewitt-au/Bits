#pragma once
/*
RegKey.hpp

Registry manipulation.
This shit ain't meant to be globally useful. Locally useful will do just fine.
*/

#include "framework.hpp"

class RegKey
{
public:
    RegKey(HKEY hkParent, LPCSTR pSubKey = NULL);

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

    RegKey CreateKey(LPCSTR pName);

    bool DeleteKey(LPCSTR pName)
    {
        LRESULT st = RegDeleteKeyA(
                        m_hKey, // HKEY hKey
                        pName   // LPCSTR lpSubKey
                        );

        return st == ERROR_SUCCESS;
    };

    bool SetValue(LPCWSTR pValue, LPCWSTR pKey = NULL);

private:
    RegKey() : m_hKey(NULL)
    {
    }

    HKEY m_hKey;
};
