// "RegKey.hpp"

/*
Registry manipulation.
This shit ain't meant to be globally useful. Locally useful will do just fine.
*/

#pragma once

#include "framework.hpp"

class RegKey
{
public:
    RegKey(HKEY hkParent, LPCSTR pSubKey = NULL);
    RegKey(const RegKey& parent, LPCSTR pSubKey = NULL);
    ~RegKey();

    operator bool() const;

    RegKey CreateKey(LPCSTR pName);
    bool DeleteKey(LPCSTR pName);
    bool SetValue(LPCWSTR pValue, LPCWSTR pKey = NULL);

private:
    RegKey() : m_hKey(NULL)
    {
    }

    HKEY m_hKey;
};

// Inline functions

inline RegKey::RegKey(const RegKey& parent, LPCSTR pSubKey)
    : RegKey(parent.m_hKey, pSubKey)
{
}

inline RegKey::operator bool() const
{
    return m_hKey != NULL;
}

inline RegKey::~RegKey()
{
    if (m_hKey)
        RegCloseKey(m_hKey);
}

inline bool RegKey::DeleteKey(LPCSTR pName)
{
    LRESULT st = RegDeleteKeyA(
        m_hKey, // HKEY hKey
        pName   // LPCSTR lpSubKey
    );

    return st == ERROR_SUCCESS;
};
