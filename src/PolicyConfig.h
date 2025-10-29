//
// Created by irasy on 29/10/2025.
//

#pragma once
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <propvarutil.h>

static constexpr CLSID CLSID_PolicyConfigClient = {
    0x870af99c,
    0x171d,
    0x4f9e,
    {0xaf, 0x0d, 0xe6, 0x3d, 0xf4, 0x0c, 0x2b, 0xc9}};

// Undocumented interfaces – method order matters
MIDL_INTERFACE("f8679f50-850a-41cf-9c72-430f290290c8")
IPolicyConfig : public IUnknown {
public:
  virtual HRESULT STDMETHODCALLTYPE GetMixFormat(LPCWSTR, WAVEFORMATEX **) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDeviceFormat(LPCWSTR, INT,
                                                    WAVEFORMATEX **) = 0;
  virtual HRESULT STDMETHODCALLTYPE ResetDeviceFormat(LPCWSTR) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDeviceFormat(LPCWSTR, WAVEFORMATEX *,
                                                    WAVEFORMATEX *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetProcessingPeriod(LPCWSTR, INT, PINT64,
                                                        PINT64) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetProcessingPeriod(LPCWSTR, PINT64) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetShareMode(LPCWSTR,
                                                 struct DeviceShareMode *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetShareMode(LPCWSTR,
                                                 DeviceShareMode *) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetPropertyValue(
      LPCWSTR, const PROPERTYKEY &, PROPVARIANT *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPropertyValue(
      LPCWSTR, const PROPERTYKEY &, PROPVARIANT *) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetDefaultEndpoint(LPCWSTR, ERole) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetEndpointVisibility(LPCWSTR, INT) = 0;
};
