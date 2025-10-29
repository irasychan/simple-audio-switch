# SimpleAudioSwitch

A tiny Windows command‑line tool to list, get, and set the default playback (output) audio device. It uses the Windows Core Audio APIs (MMDevice) and the commonly used, but undocumented, IPolicyConfig interface to switch the default endpoint for a given role.

## Features
- List all playback devices with friendly name and device ID
- Show the current default playback device
- Set the default playback device by partial friendly name match or by full device ID
- Support for Windows audio roles: Console, Multimedia, Communications

## Requirements
- Windows 10/11
- MSVC toolchain (Visual Studio 2019/2022 C++ Build Tools)
- CMake 3.20+ (4.x also works)

## Build
You can build with CMake from a Developer Command Prompt for VS (or any environment where cl.exe is available):

```cmd
cd path\to\simple-audio-switch
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable will be produced as `build\Release\SimpleAudioSwitch.exe` (or under your IDE's build directory if using CLion).

Notes:
- The project is C++20 (`set(CMAKE_CXX_STANDARD 20)`).
- Links against system libs `ole32` and `uuid` (handled by CMakeLists).

## Usage
Run the built executable from a regular Command Prompt. Matching by name is a substring match against the device's friendly name.

```cmd
SimpleAudioSwitch.exe --list
SimpleAudioSwitch.exe --get
SimpleAudioSwitch.exe --set "Headphones" 0
```

- `--list` prints all playback devices as pairs of `[ID]` and `[Name]`.
- `--get` prints the current default playback device.
- `--set <pattern> [role]` sets the default device. `<pattern>` is either a substring of the friendly name or the full device ID.
  - `role` (optional) chooses which role's default is set:
    - `0` = Console
    - `1` = Multimedia
    - `2` = Communications

Examples:

```cmd
:: Set Console default to the first device whose name contains "Speakers"
SimpleAudioSwitch.exe --set "Speakers" 0

:: Set Multimedia default by exact device ID (as shown by --list)
SimpleAudioSwitch.exe --set "{0.0.0.00000000}.{d1e5...}" 1
```

Exit codes:
- `0` on success
- `1` COM or enumerator initialization error
- `2` no device matched the provided pattern
- `3` failure when calling SetDefaultEndpoint

## How it works (brief)
- Enumerates playback devices via `IMMDeviceEnumerator::EnumAudioEndpoints`.
- Reads friendly names via `IPropertyStore` and `PKEY_Device_FriendlyName`.
- Switches the default endpoint using `IPolicyConfig::SetDefaultEndpoint` with the selected Windows audio role.

## Caveats
- Device name matching is a simple case‑sensitive substring match.
- Uses the widely known but undocumented `IPolicyConfig` interface; behavior could change in future Windows releases.
- This tool only manages playback (render) devices, not recording (capture) devices.

## Troubleshooting
- If the executable runs but nothing changes, confirm you selected the correct role. Windows maintains separate defaults for Console, Multimedia, and Communications.
- Some OEM software may override or react to default device changes; try setting all three roles if needed.
- If compilation fails with missing `atlbase.h`, ensure the "C++ ATL for latest v142/v143 build tools" component is installed with Visual Studio. The project uses `CComPtr` from ATL headers only (no ATL libs are linked).

## License
Apache License 2.0. See the `LICENSE` file for the full text. If you distribute binaries or modifications, include a copy of the license and preserve attribution notices as required by the Apache-2.0 terms.

## Acknowledgements
- Windows Core Audio (MMDevice) API documentation
- Community knowledge around `IPolicyConfig` for default endpoint switching
