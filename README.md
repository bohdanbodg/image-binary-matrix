# Image Binary Matrix

A small application to fetch an image mask based on a specified color range and generate a binary matrix corresponding to the mask.

## Requirements

- C++17 compiler
- [CMake](https://cmake.org/download) >= 3.15 (>= 23 for Ubuntu)
- [Python](https://www.python.org/downloads) 3
- [Conan](https://conan.io/downloads) 2

## Build & Run

### Build

> If you are building for the first time, please run `conan profile detect` and `conan profile show` to make sure that the `compiler.cppstd` option includes the `17` version of C++.

```
make build
```

OR

```
python ./scripts/build.py
```

OR

```
python3 ./scripts/build.py
```

> P.S. The initial build may take a considerable amount of time.

### Run

```
make run
```

OR

```
python ./scripts/run.py
```

OR

```
python3 ./scripts/run.py
```

> P.S. Tested on MacOS 14, Windows 11 and Ubuntu 22.

## Technologies

- [C++17](https://isocpp.org)
- [GLEW](https://glew.sourceforge.net)
- [GLFW](https://www.glfw.org)
- [ImGui](https://github.com/ocornut/imgui)
- [OpenCV](https://opencv.org)
- [CMake](https://cmake.org)
- [Python](https://www.python.org)
- [Conan](https://conan.io)

## Screenshots

### Application GUI

![Application GUI](./img/app_gui.png)

### Binary Matrix Output

![Binary Matrix Output](./img/bm_output.png)
