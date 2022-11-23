
# GIFView

GIFView is a utility to view GIF files. To use it, just run `gifview <filename>`.


## Building

To build GIFView, you will need the SDL2 library and CMake. On Windows, only
building with mingw is supported.

Create a build directory:

```bash
cd gifview
mkdir build
cd build
```

Configure and build:

```bash
cmake ..
cmake --build .
```

Install:

```bash
cmake --install .
```


## Configuration

Keys can be rebound using keys.conf files. GIFView will first try to read
from the local keys.conf file, at `$HOME/.config/gifview/keys.conf`. If this
file isn't found, it will try to read the global config from
`/etc/gifview/keys.conf`.

The format of the keys.conf file is this:

```
ACTION [KEY1 [KEY2 [KEY3]]]
```

`ACTION` is one of the action names from the `actions` list in `main.c`. The
KEYs are key names, as determined by `SDL_GetKeyName`. If the KEYs are left
unspecified then the action is unbound. Otherwise, the KEYs specify the primary,
secondary, and tertiary bindings for the action.
