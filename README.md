# OpenFunscripter
Can be used to create `.funscript` files. (NSFW)  
The project is based on OpenGL, SDL3, ImGui, libmpv, & all these other great [libraries](/3rdParty/).

![OpenFunscripter Screenshot](/docs/images/OpenFunscripter.jpg)

### Build
1. Clone the repository 
2. From the root directory of the repository run `git submodule update --init --recursive`
3. Run CMake and compile

Known linux dependencies to just compile are `build-essential libmpv-dev libglvnd-dev`.  

### Windows libmpv binaries used
Currently using: [mpv-dev-x86_64-v3-20241229-git-56e24d5.7z](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/)

### Platforms
Tested on Windows. 
In theory it should work on Linux and macOS but no guarantees.
If it doesn't work on your platform, please submit a bug request or even better, a pull request. 
