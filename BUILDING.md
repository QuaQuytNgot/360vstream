# Building with CMake

## Build

This project doesn't require any special command-line flags to build to keep
things simple.

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

## Build example

```sh
cmake -S . -B build -D BUILD_EXAMPLES=TRUE -D CMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-msvc` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

### CMake package

This project exports a CMake package to be used with the [`find_package`][3]
command of CMake:

* Package name: `vstream`
* Target name: `vstream::vstream`

Example usage:

```cmake
find_package(vstream REQUIRED)
# Declare the imported target as a build requirement using PRIVATE, where
# project_target is a target created in the consuming project
target_link_libraries(
    project_target PRIVATE
    vstream::vstream
)
```

### Note to packagers

The `CMAKE_INSTALL_INCLUDEDIR` is set to a path other than just `include` if
the project is configured as a top level project to avoid indirectly including
other libraries when installed to a common prefix. Please review the
[install-rules.cmake](cmake/install-rules.cmake) file for the full set of
install rules.

[3]: https://cmake.org/cmake/help/latest/command/find_package.html
