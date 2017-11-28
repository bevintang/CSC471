
# CSC 471 Lab 1


## Prerequisites

### Linux

You must install `g++` (or `clang` if you prefer) and `cmake`.
On debian-like systems like Ubuntu, this will look like:

```
sudo apt install g++ cmake
```

### OSX

You can use homebrew/macports or install these manually.

- Xcode developer tools. You'll need to log in with your Apple ID.
- CMake ([http://cmake.org/download/](http://cmake.org/download/))

### Windows

- Visual Studio Community Edition
- CMake ([http://cmake.org/download/](http://cmake.org/download/))
  - Make sure to add CMake to the system path when asked to do so.


## Build

```
mkdir build
cd build

cmake .. -G 'YOUR_GENERATOR_HERE'
```

The generator depends on your platform.

For **Linux** you probably don't need to specify a generator.

For **OSX**, use `Xcode`.

For **Windows**, use `Visual Studio 15 2017`.

For additional generators, see [CMake documentation](http://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

