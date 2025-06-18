# Laptop Keyboard Problem Solver

A lightweight, no-install Windows utility **to disable specific keys on your keyboard**. Ideal for situations where a laptop key is **malfunctioning, broken, or continuously pressing itself.**

## The Problem

Have you ever had a key on your laptop keyboard go haywire? It might get stuck, start typing on its own, or become overly sensitive. This can make your computer unusable, especially if it's a critical key like a letter, number, or modifier. This tool provides a simple and effective software solution to this common hardware problem by letting you completely block any problematic keys.

# Features

**Selective Key Blocking:** Disable only the keys that are causing problems.

**Configuration via Text File:** Easily add or remove keys to block by editing a simple .cfg file.

**System Tray Control:** Runs silently in the system tray. Right-click the icon to access all options.

**Toggle On/Off:** Quickly enable or disable the key blocking without closing the application.

**Reload Configuration:** Update the list of blocked keys on-the-fly without a restart.

**Portable:** No installation required. Just run the .exe file.

**Lightweight:** Minimal resource consumption.

# How to Use

## Get the Executable:
        Download the latest main.exe from the Releases page of this repository.
        OR build it from source.

## Run the Application:
        Double-click main.exe.
        You will see a confirmation message, and a new icon will appear in your system tray (notification area).

## Configure the Blocked Keys:
        The application looks for a configuration file named Nader.cfg on your Desktop.
        If the file doesn't exist, the application will automatically create a default one for you on your Desktop.
        Open Nader.cfg with any text editor (like Notepad).
        Add the names of the keys you want to block, one key per line.
        Use # to add comments.

## Example Nader.cfg:
    Code snippet

    # This is a comment.
    # The keys below are broken on my laptop.
    7
    8
    -
    F5

## Control from the System Tray:
        Right-click the tray icon to open the context menu:
            Enable/Disable Blocking: Toggles the key blocking functionality. The icon's tooltip will update to show the current status ("Active" or "Inactive").
            Reload Config: Applies any changes you've made to Nader.cfg instantly.
            About: Shows version info and a list of currently blocked keys.
            Exit: Closes the application.
        Double-click the tray icon as a shortcut to the "About" dialog.

Available Keys to Block

You can block a wide variety of keys. Use the following names in your Nader.cfg file (they are not case-sensitive).

    Letters: A - Z
    Numbers: 0 - 9
    Function Keys: F1 - F12
    Numpad: NUMPAD0 - NUMPAD9, *, +, NUMPAD-, NUMPAD., NUMPAD/
    Control Keys: SPACE, ENTER, TAB, BACKSPACE, DELETE, INSERT, HOME, END, PAGEUP, PAGEDOWN, UP, DOWN, LEFT, RIGHT
    Modifier Keys: ESCAPE, CAPSLOCK, SHIFT, CTRL, ALT, LWIN, RWIN
    Special Characters: -, =, [, ], \, ;, ', ,, ., /, `

Building from Source

To compile the application yourself, you'll need a C++ compiler that supports the Win32 API, such as MinGW-w64 (part of the MSYS2 toolchain).

    Prerequisites:
        Install MinGW-w64.
        Ensure g++ is in your system's PATH.

    Clone the Repository:
    Bash

git clone https://github.com/your-username/Laptop-Keyboard-Problem.git
cd Laptop-Keyboard-Problem

Compile:
Open your terminal or command prompt in the project directory and run the following command:
Bash

    g++ -o main.exe main.cpp -luser32 -lshell32 -lole32 -mwindows -static-libgcc -static-libstdc++

    This will create main.exe in the current directory.

Dependencies

    OS: Windows
    Libraries (linked during build): user32, shell32, ole32

License

This project is licensed under the MIT License. See the LICENSE file for details.
