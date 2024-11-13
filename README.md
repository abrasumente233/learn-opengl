## Build

Prerequisites:

- Meson build system
- Ninja build tool
- C/C++ compiler

```bash
# we use clang here, but you can use gcc as well
CC=clang CXX=clang++ meson setup build
meson compile -C build && ./build/main
```

By default, GLFW3 and GLM are automatically fetched from GitHub and built from source. If you prefer to use your system's existing installation, configure the build with: `meson setup build -Dwrap_mode=nofallback`.
