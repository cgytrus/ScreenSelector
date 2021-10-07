# Screen Selector
A mod that lets you select the screen to run Geometry Dash on

Fully compatible with Mega Hack v6 **(except the "Fullscreen" and "Borderless Fullscreen" options in the "Display" tab, don't touch those!)**

[Video showcase](https://youtu.be/NxBCq04MgMs)

## Features
- Saves the screen you quit GD on and reopens it on the same screen when starting up
- Adds a borderless fullscreen mode
- Lets you select the screen to run GD on (duh) (doesn't work with exclusive fullscreen)
- Integrates with both vanilla video options and Mega Hack v6

## Installation
Same as any of the [Mat's mods](https://matcool.github.io/mods) (scroll down to the "Install instructions" section)

## Compiling
- Clone the repo recursively (with `--recursive`)
- Configure CMake (`cmake -G "Visual Studio 16 2019" -B build -DCMAKE_BUILD_TYPE=Release -T host=x86 -A win32`)
- Build (`cmake --build build --config Release --target ALL_BUILD`)
