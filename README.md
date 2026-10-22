# SystemHook

SystemHook is a lightweight, command-line utility for Windows that extracts icons from system executables and dynamic-link libraries (DLLs). It also includes functionality to launch applications and convert extracted icons into Base64-encoded HTML tags for web use.

## Features

- Extract all embedded icons from any given executable or DLL.
- Batch extract icons from all DLLs within the `System32` directory.
- Convert extracted PNG icons into Base64 HTML image tags.
- Launch Windows applications or custom executables directly via the command line.
- Single-file executable with embedded resources (icon and PowerShell script).
- Generate "GodMode" shortcuts (system CLSIDs, MS-Settings, URL Protocols) with detailed CSV statistics.

## Prerequisites

- Windows 10 or Windows 11.
- MinGW 64-bit (Required for compilation).
- PowerShell (Included by default in Windows).

## Compilation [OTHER]

If you have MinGW installed, it is highly recommended to use it for compilation as it is the standard toolchain for this project.
[NOTE: THIS PROGRAM IS COMMAND-LINE ONLY. IN CMD/POWERSHELL, PLEASE WRITE "SystemHook".]

## GodMode Feature

Extracts system shortcuts (CLSIDs, MS-Settings, URL Protocols), creates a dedicated folder with a custom icon, and saves detailed CSV statistics.

```cmd
SystemHook.exe run --GodMode "<Output_Directory>"
