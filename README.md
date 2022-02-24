# GIFView
GIFView is a utility to view GIF files. To use it, just run `gifview <filename>`.

## Building
To build GIFView, you will need the SDL2 library and CMake. On Windows, only building with mingw is supported.

### Linux
Create a build directory:
```bash
cd gifview
mkdir build
cd build
```
Configure and build:
```bash
cmake ..
make
```

### Windows with MinGW
Create a build directory:
```powershell
cd gifview
mkdir build
cd build
```
Configure and build:
```powershell
cmake .. -G 'Unix Makefiles'
make
```
