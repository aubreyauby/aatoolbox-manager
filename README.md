# aaToolbox Manager
aaToolbox Manager is a program I made to host all downloads and resources for my projects. I found it much more efficient (and sometimes cost/time-effective) to go with this method rather than just hosting it all in a website. This was inspired by Creative Cloud's and Visual Studio's own installers, and is one of my first major C++ projects, as I am willing to drive away from the dependency of Electron (which uses Chromium, yuck!) to host it.

This project was fully developed with Win32 as a start, with plans to potentially rewrite in Qt once I figure out how the whole shenanigans work and feel more confident in using it for future C++ projects. This was completely written in VSCode.

## Test it out yourself!
This project uses CMake to build the entire project into an executable file. There are future plans to make this cross-platform, but for now it is Windows exclusive.

### 1. Create a `build` folder within the root directory.
In PowerShell, use this CMake command. You might need to [install](https://cmake.org/download/) CMake first if PowerShell doesn't recognize it:

```cmake -S . -B build```

### 2. Build the project inside the `build` folder you just made.
Have CMake build the entire project using this command:

```cmake --build build```

### 3. Run the app.
Head to the `build` folder, then go to `Debug`, and you'll find the executable `aaToolboxManager.exe` file.
