
# GIFView

GIFView is a utility to view GIF files. To use it, just run `gifview <filename>`.


## Building

To build GIFView, you will need the SDL2 library and CMake.

Create the build directory, then configure and build:

```shell
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```


### Windows (MSVC)

Download the MSVC versions of SDL2 and SDL_ttf, and unzip them somewhere (the gifview root directory is a good choice.) Then, set the CMake cache variables "SDL2_ROOT" and "SDL2_ttf_ROOT" to the absolute paths of the respective unzipped folders. For example:

```shell
$ cmake -D SDL2_ROOT:PATH="C:/users/YourName/gifview/SDL2-2.26.3" -D SDL2_ttf_ROOT:PATH="C:/users/YourName/gifview/SDL2_ttf-2.20.2" ..
$ cmake --build .
```


## Installing

To install `gifview`:

```shell
$ cmake --install .
```


## Configuration

Keys can be rebound using keys.conf files. GIFView will first try to read from the local keys.conf file, at `$HOME/.config/gifview/keys.conf`. If this file isn't found, it will try to read the global config from `/etc/gifview/keys.conf`.

The format of the keys.conf file is this:

```
ACTION [KEY1 [KEY2 [KEY3]]]
```

`ACTION` is one of the action names from the `actions` list in `main.c`. The `KEY`s are key names, as determined by `SDL_GetKeyName`. If the `KEY`s are left unspecified then the action is unbound. Otherwise, the `KEY`s specify the primary, secondary, and tertiary bindings for the action.
