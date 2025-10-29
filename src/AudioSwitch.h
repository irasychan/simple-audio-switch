//
// Created by irasy on 29/10/2025.
//

#pragma once
#include "Device.h"
#include "PolicyConfig.h"
#include <atlcomcli.h>
#include <mmdeviceapi.h>
#include <vector>

class AudioSwitch {
public:
  AudioSwitch();
  ~AudioSwitch();
  std::vector<Device> listPlaybackDevices() const;
  Device getDefaultPlayback(ERole role = eConsole) const;
  static HRESULT setDefaultPlayback(const std::wstring &deviceId, ERole role);
  static HRESULT setVisibility(const std::wstring &id, bool visible);

private:
  CComPtr<IMMDeviceEnumerator> enumerator_;
  static HRESULT queryPolicy(IPolicyConfig **pc);
};