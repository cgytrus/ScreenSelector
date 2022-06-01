# Screen Selector
A mod that lets you select the screen to run Geometry Dash on

Fully compatible with Mega Hack v7 **(except the "Fullscreen" and "Borderless Fullscreen" options in the "Display" tab, don't touch those!)**

[Video showcase](https://youtu.be/NxBCq04MgMs)

## Features
- Saves the screen you quit GD on and reopens it on the same screen when starting up
- Adds a borderless fullscreen mode
- Lets you select the screen to run GD on (duh)
- Integrates with both vanilla video options and Mega Hack v7

## Installation
### Mega Hack v7
- Put the **downloaded** ScreenSelector.dll file in the `extensions` folder
### QuickLdr
- If there's no `settings.txt` file in the `quickldr` folder, create it
- Add a **new line** (by clicking on the last line and pressing `Enter`)
- Write `ScreenSelector.dll` and save the file
- Put the **downloaded** ScreenSelector.dll file in the `quickldr` folder
### GDDllLoader
- Put the **downloaded** ScreenSelector.dll file in the `adaf-dll` folder

## Compiling
- Clone the repo recursively (with `--recursive`)
- Configure CMake (`cmake -G "Visual Studio 16 2019" -B build -DCMAKE_BUILD_TYPE=Release -T host=x86 -A win32`)
- Build (`cmake --build build --config Release --target ALL_BUILD`)
