#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <comdef.h>
#include <functiondiscoverykeys_devpkey.h> // PKEY_Device_FriendlyName

// Add ATL for CComPtr
#ifndef ATL_NO_AUTOMATIC_NAMESPACE
#define ATL_NO_AUTOMATIC_NAMESPACE
#endif
#include <atlbase.h>

// --- Undocumented but standard in the field ---
static const CLSID CLSID_PolicyConfigClient =
{0x870af99c,0x171d,0x4f9e,{0xaf,0x0d,0xe6,0x3d,0xf4,0x0c,0x2b,0xc9}};

MIDL_INTERFACE("f8679f50-850a-41cf-9c72-430f290290c8")
IPolicyConfig : public IUnknown {
    public:
    virtual HRESULT STDMETHODCALLTYPE GetMixFormat(LPCWSTR, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(LPCWSTR, INT, WAVEFORMATEX**) = 0;
    virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(LPCWSTR, WAVEFORMATEX*, WAVEFORMATEX*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(LPCWSTR, INT, PINT64, PINT64) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(LPCWSTR, PINT64) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShareMode(LPCWSTR, struct DeviceShareMode*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShareMode(LPCWSTR, struct DeviceShareMode*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(LPCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(LPCWSTR, const PROPERTYKEY&, PROPVARIANT*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(LPCWSTR, ERole) = 0;     // <- 11th
    virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(LPCWSTR, INT) = 0;    // <- 12th
};

// --- helpers ---
std::wstring getPropString(IPropertyStore* store, const PROPERTYKEY& key)
{
    PROPVARIANT v;
    PropVariantInit(&v);
    if (SUCCEEDED(store->GetValue(key, &v)) && v.vt == VT_LPWSTR)
    {
        std::wstring s = v.pwszVal;
        PropVariantClear(&v);
        return s;
    }
    PropVariantClear(&v);
    return L"";
}

struct DeviceInfo
{
    std::wstring id;
    std::wstring name;
};

std::vector<DeviceInfo> enumeratePlaybackDevices(IMMDeviceEnumerator* e)
{
    std::vector<DeviceInfo> out;
    CComPtr<IMMDeviceCollection> coll;
    if (FAILED(e->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL, &coll))) return out;
    UINT count = 0;
    coll->GetCount(&count);
    for (UINT i = 0; i < count; ++i)
    {
        CComPtr<IMMDevice> dev;
        if (FAILED(coll->Item(i, &dev))) continue;
        LPWSTR id = nullptr;
        if (FAILED(dev->GetId(&id))) continue;
        CComPtr<IPropertyStore> ps;
        if (FAILED(dev->OpenPropertyStore(STGM_READ, &ps)))
        {
            CoTaskMemFree(id);
            continue;
        }
        std::wstring name = getPropString(ps, PKEY_Device_FriendlyName);
        out.push_back({id, name});
        CoTaskMemFree(id);
    }
    return out;
}

DeviceInfo getDefaultPlayback(IMMDeviceEnumerator* e, ERole role = eConsole)
{
    CComPtr<IMMDevice> dev;
    if (FAILED(e->GetDefaultAudioEndpoint(eRender, (ERole)role, &dev))) return {};
    LPWSTR id = nullptr;
    dev->GetId(&id);
    CComPtr<IPropertyStore> ps;
    dev->OpenPropertyStore(STGM_READ, &ps);
    std::wstring name = getPropString(ps, PKEY_Device_FriendlyName);
    DeviceInfo d{id ? id : L"", name};
    if (id) CoTaskMemFree(id);
    return d;
}

HRESULT setDefaultPlayback(const std::wstring& deviceId, ERole role)
{
    CComPtr<IPolicyConfig> pc;
    HRESULT hr = CoCreateInstance(CLSID_PolicyConfigClient, nullptr, CLSCTX_ALL,
                                  __uuidof(IPolicyConfig), (void**)&pc);
    if (FAILED(hr)) return hr;
    return pc->SetDefaultEndpoint(deviceId.c_str(), role);
}

int wmain(int argc, wchar_t** argv)
{
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr))
    {
        std::wcerr << L"COM init failed\n";
        return 1;
    }

    CComPtr<IMMDeviceEnumerator> en;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&en);
    if (FAILED(hr))
    {
        std::wcerr << L"MMDeviceEnumerator failed\n";
        CoUninitialize();
        return 1;
    }

    std::wstring cmd = argc > 1 ? argv[1] : L"--help";
    if (cmd == L"--list")
    {
        auto devs = enumeratePlaybackDevices(en);
        for (auto& d : devs)
        {
            std::wcout << L"[ID] " << d.id << L"\n[Name] " << d.name << L"\n\n";
        }
        CoUninitialize();
        return 0;
    }
    else if (cmd == L"--get")
    {
        auto def = getDefaultPlayback(en);
        if (def.id.empty()) { std::wcout << L"No default device\n"; }
        else
        {
            std::wcout << L"Default: " << def.name << L"\nID: " << def.id << L"\n";
        }
        CoUninitialize();
        return 0;
    }
    else if (cmd == L"--set" && argc >= 3)
    {
        std::wstring pattern = argv[2];
        int roleIdx = 0; // 0 console, 1 multimedia, 2 communications
        if (argc >= 4) roleIdx = std::clamp(_wtoi(argv[3]), 0, 2);

        auto devs = enumeratePlaybackDevices(en);
        // match by substring on friendly name first, then try full id
        auto it = std::ranges::find_if(devs, [&](const DeviceInfo& d)
        {
            return d.name.find(pattern) != std::wstring::npos || d.id == pattern;
        });
        if (it == devs.end())
        {
            std::wcerr << L"No device matches pattern: " << pattern << L"\n";
            CoUninitialize();
            return 2;
        }
        hr = setDefaultPlayback(it->id, static_cast<ERole>(roleIdx));
        if (SUCCEEDED(hr))
        {
            std::wcout << L"Switched default (" << roleIdx << L") to: " << it->name << L"\n";
        }
        else
        {
            _com_error err(hr);
            std::wcerr << L"SetDefaultEndpoint failed: " << err.ErrorMessage() << L"\n";
            CoUninitialize();
            return 3;
        }
        CoUninitialize();
        return 0;
    }

    std::wcout << L"Usage:\n"
        L"  SimpleAudioSwitch --list\n"
        L"  SimpleAudioSwitch --get\n"
        L"  SimpleAudioSwitch --set \"partial device name\" [role]\n"
        L"Roles: 0=Console 1=Multimedia 2=Communications\n";
    CoUninitialize();
    return 0;
}
