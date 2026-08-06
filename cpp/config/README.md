# Ice CMake Package

The Ice CMake package lets CMake projects consume an Ice for C++ installation: `find_package(Ice)`
provides imported targets for the Ice libraries, and the `slice2cpp_generate` function compiles the
Slice (`.ice`) files in a target's sources as part of the build.

[Ice documentation][docs] | [slice2cpp options][slice2cpp] | [Slice documentation][slice]

## Requirements

- CMake 3.21 or later
- An Ice for C++ installation (system package, Homebrew, NuGet, or built from source)

## Usage

```cmake
find_package(Ice REQUIRED CONFIG)

add_executable(client Client.cpp Greeter.ice)
slice2cpp_generate(client)
target_link_libraries(client PRIVATE Ice::Ice)
```

> [!IMPORTANT]
> The `CONFIG` (or `NO_MODULE`) keyword is required. `find_package` has two modes: module mode,
> which runs a `FindIce` module — and CMake bundles one of its own — and config mode, which loads
> the `IceConfig.cmake` installed by this package. Module mode wins by default, so a plain
> `find_package(Ice)` runs CMake's bundled module instead of this package and provides none of the
> imported targets below, nor `slice2cpp_generate`. `CONFIG` selects config mode.

When Ice is not installed in a standard location, point CMake at it with `CMAKE_PREFIX_PATH` (or
set `Ice_DIR` to the directory containing `IceConfig.cmake`).

## Imported Targets

Each component of the installation gets an `Ice::<component>` imported library:

| Component           | Imported target            | Notes      |
| ------------------- | -------------------------- | ---------- |
| Ice                 | `Ice::Ice`                 |            |
| DataStorm           | `Ice::DataStorm`           |            |
| Glacier2            | `Ice::Glacier2`            |            |
| IceBox              | `Ice::IceBox`              |            |
| IceDiscovery        | `Ice::IceDiscovery`        |            |
| IceGrid             | `Ice::IceGrid`             |            |
| IceLocatorDiscovery | `Ice::IceLocatorDiscovery` |            |
| IceStorm            | `Ice::IceStorm`            |            |
| IceBT               | `Ice::IceBT`               | Linux only |

Components present in the installation are always available; listing them in
`find_package(Ice CONFIG COMPONENTS ...)` additionally makes them required. The package also
defines `Ice::slice2cpp`, the imported executable `slice2cpp_generate` runs.

## Result Variables

| Variable                   | Description                                        |
| -------------------------- | -------------------------------------------------- |
| `Ice_VERSION`              | The Ice version, e.g. `3.9.0`                      |
| `Ice_SO_VERSION`           | The shared library version, e.g. `39`              |
| `Ice_PREFIX`               | The installation prefix                            |
| `Ice_INCLUDE_DIR`          | The directory containing the Ice header files      |
| `Ice_SLICE_DIR`            | The directory containing the Ice Slice files       |
| `Ice_SLICE2CPP_EXECUTABLE` | Path to the `slice2cpp` compiler                   |

On Windows, the cache variable `Ice_WIN32_PLATFORM` (`x64`, the default, or `Win32`) selects the
platform inside the NuGet package, and the `ICE_RUNTIME_DLLS` property on `Ice::Ice` lists the
runtime DLLs to copy next to a consuming executable.

## Compiling Slice Files

`slice2cpp_generate` compiles the `.ice` files already present in a target's sources and adds the
generated C++ to the target:

```cmake
slice2cpp_generate(<target>
  [INCLUDE_DIRS <dir>...]     # extra -I directories
  [OPTIONS <option>...]       # extra slice2cpp options, e.g. -DFOO or --header-ext hpp
  [HEADER_OUTPUT_DIR <dir>]   # put the headers here instead of with the sources
  [INCLUDE_DIR <dir>]         # slice2cpp --include-dir
  [INCLUDE_SCOPE <scope>]     # PRIVATE (default) or PUBLIC
)
```

Call it from the directory that created the target, after all of the target's `.ice` sources have
been added. Generated files mirror the layout of the `.ice` files under the `INCLUDE_DIRS`
directory that reaches them, so the `#include` directives slice2cpp emits resolve; the generated
include directories are added to the target with `INCLUDE_SCOPE` scope (`PUBLIC` covers the build
tree only; installing the headers is the caller's).

A library whose Slice includes files from a shared directory, publishing its generated headers to
its own consumers under a `Demo/` prefix:

```cmake
add_library(demo Greeter.ice)
slice2cpp_generate(demo
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../slice
  OPTIONS -DENABLE_EXTRAS
  HEADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/include
  INCLUDE_DIR Demo
  INCLUDE_SCOPE PUBLIC)
```

[docs]: https://docs.zeroc.com/ice/latest/cpp
[slice2cpp]: https://docs.zeroc.com/ice/latest/cpp/using-the-slice-compilers
[slice]: https://docs.zeroc.com/ice/latest/cpp/the-slice-language
