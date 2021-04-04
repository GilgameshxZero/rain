## Building on Linux/MacOS

Install `make` and `g++`. Use the `makefile` present in this directory.

## Building on Visual Studio Code

Visual Studio Code is set up with the configurations to build natively on Windows, Linux, and MacOS. When building on Windows, requires a standard Visual Studio installation with `cl.exe` and `VsDevCmd.bat`. Ensure that the path to `VsDevCmd.bat` is correct in `../.vscode/tasks.json`. When building on Linux and MacOS, requires `make` and `g++` on the command-line.

## Building on Visual Studio (on Windows)

A project template is available at `rain`. Convert the contents of the directory into a `.zip` (a valid template), and import the template into Visual Studio. Afterwards, add the necessary test files to the project.
