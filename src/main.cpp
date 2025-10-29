#include "AudioSwitch.h"
#include "WinError.h"
#include <algorithm>
#include <iostream>

/**
 * @brief Entry point of the application that allows switching or querying audio
 * device settings.
 *
 * Processes command-line arguments to perform operations such as listing
 * playback devices, getting the default playback device, setting a default
 * playback device, and changing device visibility.
 *
 * @param argc The count of command-line arguments.
 * @param argv An array of command-line arguments provided by the user.
 * @return Returns 0 on successful execution. In case of invalid input or no
 * arguments, displays usage information and returns 0.
 */
int wmain(int argc, wchar_t **argv) {
  AudioSwitch audioSwitch;

  auto usage = [] {
    std::wcout << L"Usage:\n"
                  L"simple-audio-switch.exe --list\n"
                  L"simple-audio-switch.exe --get [role 0|1|2]\n"
                  L"simple-audio-switch.exe --set <device id> [role 0|1|2]\n"
                  L"simple-audio-switch.exe --visible <device id> <0|1>\n";
  };

  if (argc < 2) {
    usage();
    return 0;
  }
  std::wstring cmd = argv[1];

  if (cmd == L"--list") {
    auto devices = audioSwitch.listPlaybackDevices();
    for (const auto &d : devices) {
      std::wcout << L"ID: " << d.id << L"\n"
                 << L"Name: " << d.name << L"\n"
                 << L"-------------------\n";
    }
  }

  if (cmd == L"--get") {
    auto device = audioSwitch.getDefaultPlayback();
    std::wcout << L"ID: " << device.id << L"\n"
               << L"Name: " << device.name << L"\n";
  }

  if (cmd == L"--set" && argc >= 3) {
    std::wstring pattern = argv[2];
    int roleIdx = 0; // 0 console, 1 multimedia, 2 communications
    if (argc >= 4)
      roleIdx = std::clamp(_wtoi(argv[3]), 0, 2);

    auto devs = audioSwitch.listPlaybackDevices();
    // match by substring on friendly name first, then try full id
    auto it = std::ranges::find_if(devs, [&](const Device &d) {
      return d.name.find(pattern) != std::wstring::npos || d.id == pattern;
    });
    if (it == devs.end()) {
      std::wcerr << L"No device matches pattern: " << pattern << L"\n";
      CoUninitialize();
      return 2;
    }
    auto hr =
        audioSwitch.setDefaultPlayback(it->id, static_cast<ERole>(roleIdx));
    if (SUCCEEDED(hr)) {
      std::wcout << L"Switched default (" << roleIdx << L") to: " << it->name
                 << L"\n";
    } else {
      _com_error err(hr);
      std::wcerr << L"SetDefaultEndpoint failed: " << err.ErrorMessage()
                 << L"\n";
      CoUninitialize();
      return 3;
    }
  }
  return 0;
}
