//
// Created by irasy on 29/10/2025.
//

#pragma once
#include <string>
#include <Windows.h>
#include <comdef.h>
#include <tchar.h>

inline std::basic_string<TCHAR> hresultMessage(HRESULT hr)
{
    using tstring = std::basic_string<TCHAR>;
    _com_error e(hr);
    const TCHAR* msg = e.ErrorMessage();
    if (msg == nullptr)
    {
        return tstring{_T("Unknown error")};
    }

    return tstring{msg};
}
