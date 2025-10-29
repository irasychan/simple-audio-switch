//
// Created by irasy on 29/10/2025.
//

#include "AudioSwitch.h"
#include <functiondiscoverykeys_devpkey.h>

AudioSwitch::AudioSwitch() {
  CoInitialize(nullptr);
  CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                   __uuidof(IMMDeviceEnumerator),
                   reinterpret_cast<void **>(&enumerator_));
}
AudioSwitch::~AudioSwitch() { CoUninitialize(); }

static std::wstring propString(IPropertyStore *ps, REFPROPERTYKEY key) {
  PROPVARIANT v;
  PropVariantInit(&v);
  std::wstring s;
  if (SUCCEEDED(ps->GetValue(key, &v)) && v.vt == VT_LPWSTR)
    s = v.pwszVal;
  PropVariantClear(&v);
  return s;
}

std::vector<Device> AudioSwitch::listPlaybackDevices() const {
  std::vector<Device> out;
  CComPtr<IMMDeviceCollection> coll;
  if (FAILED(enumerator_->EnumAudioEndpoints(eRender, DEVICE_STATEMASK_ALL,
                                             &coll)))
    return out;
  UINT count = 0;
  coll->GetCount(&count);
  for (UINT i = 0; i < count; ++i) {
    CComPtr<IMMDevice> dev;
    if (FAILED(coll->Item(i, &dev)))
      continue;
    LPWSTR id = nullptr;
    if (FAILED(dev->GetId(&id)))
      continue;
    CComPtr<IPropertyStore> ps;
    if (FAILED(dev->OpenPropertyStore(STGM_READ, &ps))) {
      CoTaskMemFree(id);
      continue;
    }
    std::wstring name = propString(ps, PKEY_Device_FriendlyName);
    out.push_back({id, name});
    CoTaskMemFree(id);
  }
  return out;
}

Device AudioSwitch::getDefaultPlayback(ERole role) const {
  CComPtr<IMMDevice> dev;
  if (FAILED(enumerator_->GetDefaultAudioEndpoint(eRender, role, &dev)))
    return {};
  LPWSTR id = nullptr;
  dev->GetId(&id);
  CComPtr<IPropertyStore> ps;
  dev->OpenPropertyStore(STGM_READ, &ps);
  std::wstring name = propString(ps, PKEY_Device_FriendlyName);
  Device d{id ? id : L"", name};
  if (id)
    CoTaskMemFree(id);
  return d;
}

HRESULT AudioSwitch::queryPolicy(IPolicyConfig **pc) {
  CComPtr<IUnknown> unknown;
  HRESULT hr =
      CoCreateInstance(CLSID_PolicyConfigClient, nullptr, CLSCTX_ALL,
                       IID_IUnknown, reinterpret_cast<void **>(&unknown));
  if (FAILED(hr))
    return hr;
  return unknown->QueryInterface(__uuidof(IPolicyConfig),
                                 reinterpret_cast<void **>(pc));
}

HRESULT AudioSwitch::setDefaultPlayback(const std::wstring &deviceId,
                                        ERole role) {
  CComPtr<IPolicyConfig> pc;
  if (SUCCEEDED(queryPolicy(&pc)))
    return pc->SetDefaultEndpoint(deviceId.c_str(), role);
  return E_NOINTERFACE;
}

HRESULT AudioSwitch::setVisibility(const std::wstring &id, const bool visible) {
  CComPtr<IPolicyConfig> pc;
  if (SUCCEEDED(queryPolicy(&pc)))
    return pc->SetEndpointVisibility(id.c_str(), visible ? TRUE : FALSE);
  return E_NOINTERFACE;
}
