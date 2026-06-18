# ViBoard
Fast Free & Open-Source soundboard.

## Key Features
* Minimum resource usage, especially in background.
* Linux (X11 and XWayland) and Windows support.
* Supports .mp3, .wav, .ogg, and .flac.
* Create any number of named soundboards and organize them into dedicated windows or tabs.
* Assign hotkeys to your sound effects to trigger them in other apps or games (if app or game allows).
* Play using up to 2 output devices simultaneously (such as both your headphones and microphone).
    * Windows requires [VB Cable](https://vb-audio.com/Cable/) for microphone input.
* Ability to trigger a game's push-to-talk when a sound is being played.
* Themes, with support for custom user themes.

![Screenshot](https://github.com/goodguyartem/ViBoard/blob/main/screenshots/screenshot0.png?raw=true)

## Installation
Get the latest build from the [Releases](https://github.com/goodguyartem/VIBoard/releases) section for your system and extract them to your installation directory of choice (such as C:\\Program Files\\ViBoard or /opt/viboard). Currently only x64 Windows and Linux binaries are provided.

Note: on Windows you'll likely get a "Windows Protected Your PC" prompt the first time you run the program. It's nothing to fear, it just means the program isn't signed. Just click "More info" then "Run anyway". You may also instead inspect the source code and build the binaries yourself.

## Build From Source
The project uses [CMake](https://cmake.org/) to generate build files. Clone the repo into your folder of choice and navigate to it with:
```
git clone https://github.com/goodguyartem/viboard.git
cd ViBoard
```
### Install Scripts
If you're building for non-development use, automated install scripts are provided for your convinience if you'd like to use them:

**Linux:**
```bash
bash

chmod +x Install # Enable execute permission.
sudo ./Install   # Run the install script.

# Clean uninstalling is also possible.
chmod +x Uninstall
./Uninstall
```
**Windows:**
```bat
cmd

:: Run the install script
Install.bat

:: Clean uninstalling is also possible.
Uninstall.bat
```

### Manual
Configure CMake and create a build directory with:
``` bash
cmake -B build
```
To build the project run:
```bash
cmake --build build
```
This will build with the default configuration. To disable debug symbols and enable optimizations, pass `--config Release`. To use a specific generator, use `-G`. For example, to create an x64 Visual Studio project, run:
```bat
cmake --build build -G "Visual Studio 17 2022" -A x64
```
Optionally, install the program with:
```bash
cmake --install build
```

## Supported OSs
Currently, there is only support for
* Windows (only Windows 11 has been tested)
* Linux (X11 or XWayland).

Native Wayland is currently not supported for Linux builds as implementing system-wide hotkey support is not as trivial as it is in X11 or Win32. I'll likely add at least basic native Wayland support if I don't implement hotkey support as the rest should (hopefully) be trivial, but no promises!

Building on other platforms should be possible, but is untested. Since the project uses only minimal dependencies, like GLFW for cross-platform window creation, features outside the scope of these libraries have to be implemented per-platform, such as:
* System-wide hotkey support (uses Win32 and X11). GLFW will not listen to key presses when the window is not active.
* Sending faked key presses to the OS to trigger push-to-talk (uses Win32 and X11). GLFW can read but not send input to the best of my knowledge.
* Opening file browsers and URLs (uses system shell and nativefiledialog-extended). NFD only allows prompting the user to pick folders/files.

If you build without implementing these features, it should simply fall back to the dummy API but I haven't tested this yet.

## Contributions
If you think you can help make this better, feature requests and contributions are welcomed!