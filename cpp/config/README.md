# Ice CMake Package

The Ice CMake package lets CMake projects consume an Ice for C++ installation: `find_package(Ice CONFIG)` provides
imported targets for the Ice libraries, and the `slice2cpp_generate` function compiles the Slice (`.ice`) files in a
target's sources as part of the build.

[Ice documentation][docs] | [slice2cpp options][slice2cpp] | [Slice documentation][slice]

## Requirements

- CMake 3.21 or later
- An Ice for C++ installation (system package, Homebrew, NuGet, or installed from source with `make install`; the
  package cannot be used from a source or build tree)

## Usage

```cmake
find_package(Ice REQUIRED CONFIG)

add_executable(client Client.cpp Greeter.ice)
slice2cpp_generate(client)
target_link_libraries(client PRIVATE Ice::Ice)
```

> [!IMPORTANT]
> The `CONFIG` (or `NO_MODULE`) keyword is required. It selects config mode, which loads the `IceConfig.cmake`
> installed by this package. Without it, `find_package` uses module mode and runs the `FindIce` module bundled with
> CMake. This applies to every call without either keyword, whatever else you pass — `REQUIRED`, `COMPONENTS`, a
> version.

When Ice is not installed in a standard location, point CMake at it with `CMAKE_PREFIX_PATH` (or set `Ice_DIR` to the
directory containing `IceConfig.cmake`).

A versioned request is answered by the installed version: `find_package(Ice 3.9 CONFIG)` accepts any 3.9 patch release
that is not older than the request, since Ice treats every x.y as a major release line and only patch releases within a
line are compatible. Version ranges are honored on both endpoints.

### Windows

On Windows, Ice for C++ ships as the `ZeroC.Ice.Cpp` NuGet package rather than as an installation. Restore the
package, then point CMake at the directory it was extracted to:

```shell
nuget install ZeroC.Ice.Cpp -OutputDirectory packages
```

```cmake
set(Ice_ROOT "${CMAKE_CURRENT_LIST_DIR}/packages/ZeroC.Ice.Cpp.<version>")
find_package(Ice REQUIRED CONFIG)
```

The package ships both Debug and Release binaries, and the `Ice_WIN32_PLATFORM` cache variable (`x64` or `Win32`)
selects the platform within it. It defaults to the build architecture, so it only needs setting to cross the two.

The Ice DLLs are not on the `PATH`, so copy them next to the executable at build time. The `ICE_RUNTIME_DLLS` property
on `Ice::Ice` lists the ones Ice needs that CMake does not track itself, such as bzip2:

```cmake
if(WIN32)
  add_custom_command(TARGET client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
      $<TARGET_RUNTIME_DLLS:client>
      $<GENEX_EVAL:$<TARGET_PROPERTY:Ice::Ice,ICE_RUNTIME_DLLS>>
      $<TARGET_FILE_DIR:client>
    COMMAND_EXPAND_LISTS
  )
endif()
```

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

Components present in the installation are always available; listing them in `find_package(Ice CONFIG COMPONENTS ...)`
additionally makes them required.

The package also defines two imported executables, neither of which is a component: `Ice::slice2cpp`, the compiler
`slice2cpp_generate` runs, and on Windows `Ice::icebox_EXE`. Neither can be listed in `COMPONENTS`.

## Result Variables

| Variable                   | Description                                                             |
| -------------------------- | ----------------------------------------------------------------------- |
| `Ice_VERSION`              | The Ice version, including any pre-release suffix, e.g. `3.9.0-alpha.0` |
| `Ice_SO_VERSION`           | The shared library version, e.g. `39a0`                                 |
| `Ice_PREFIX`               | The installation prefix                                                 |
| `Ice_INCLUDE_DIR`          | The directory containing the Ice header files                           |
| `Ice_SLICE_DIR`            | The directory containing the Ice Slice files                            |
| `Ice_SLICE2CPP_EXECUTABLE` | Path to the `slice2cpp` compiler                                        |

`Ice_VERSION` carries the pre-release suffix so it matches what Ice reports everywhere else. CMake derives
`Ice_VERSION_MAJOR`, `MINOR` and `PATCH` from the numeric part, and its version comparisons ignore the suffix
altogether: with `3.9.0-alpha.0` installed, `Ice_VERSION VERSION_EQUAL 3.9.0` is true. A `find_package` request with
`EXACT` is the exception, since this package compares it as a string: a pre-release does not satisfy `EXACT` for the
release it precedes.

`Ice_SLICE_DIR` is always passed to slice2cpp by `slice2cpp_generate`, so you never need to add it to `INCLUDE_DIRS`
yourself.

Use `Ice_SLICE2CPP_EXECUTABLE` where generator expressions are unavailable, such as at configure time. A build-time
custom command should prefer `$<TARGET_FILE:Ice::slice2cpp>` in its `COMMAND`, and list it in `DEPENDS` so the
command re-runs when the compiler changes.

## Compiling Slice Files

`slice2cpp_generate` compiles the `.ice` files already present in a target's sources and adds the generated C++ to the
target. Call it from the directory that created the target, after all of the target's `.ice` sources have been added.

```cmake
slice2cpp_generate(<target>
  [INCLUDE_DIRS <dir>...]     # extra -I directories; also drive the generated file layout (see below)
  [OPTIONS <option>...]       # extra slice2cpp options, e.g. -DFOO or --header-ext hpp
  [HEADER_OUTPUT_DIR <dir>]   # put the headers here instead of with the sources
  [INCLUDE_DIR <dir>]         # slice2cpp --include-dir
  [INCLUDE_SCOPE <scope>]     # PRIVATE (default) or PUBLIC
)
```

`slice2cpp_generate` always passes `Ice_SLICE_DIR` to slice2cpp, before the `INCLUDE_DIRS` directories, so
`#include <Ice/...>` directives resolve without any configuration.

Only the `.ice` files in the target's own sources are compiled. `INCLUDE_DIRS` merely resolves includes, so a Slice
file included from a shared directory needs a target of its own that compiles it, which this target then links to pick
up its headers.

Generated files are placed under `<current binary dir>/generated/<target>`; with `HEADER_OUTPUT_DIR`, the headers are
placed under `HEADER_OUTPUT_DIR[/INCLUDE_DIR]` instead. When a Slice file includes another, e.g.
`#include <Demo/Weather.ice>`, the generated code includes the corresponding generated header by the same path,
`Demo/Weather.h`. To make these directives resolve, each generated file is placed at its `.ice` file's path relative to
the `INCLUDE_DIRS` directory containing it: with `INCLUDE_DIRS ../slice`, the file `../slice/Demo/Weather.ice`
generates `Demo/Weather.h` under the header output directory.

The directories containing the generated headers are added to the target's include directories with the scope given by
`INCLUDE_SCOPE`. With `PRIVATE` (the default), only this target's own sources can include the generated headers. With
`PUBLIC`, the directories become part of the target's usage requirements: when another target depends on this one
through `target_link_libraries`, they go on that target's include path as well, so its sources can include the
generated headers. `PUBLIC` applies within the build tree only: these directories are omitted from the target's
installed interface, and installing the generated headers is the caller's responsibility.

For example, the following defines a library whose Slice files include files from a shared directory, and publishes
its generated headers to its own consumers under a `Demo/` prefix:

```cmake
add_library(weather_api Weather.ice)
slice2cpp_generate(weather_api
  INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../slice
  OPTIONS -DENABLE_EXTRAS
  HEADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/include
  INCLUDE_DIR Demo
  INCLUDE_SCOPE PUBLIC)
```

[docs]: https://docs.zeroc.com/ice/latest/cpp
[slice2cpp]: https://docs.zeroc.com/ice/latest/cpp/using-the-slice-compilers
[slice]: https://docs.zeroc.com/ice/latest/cpp/the-slice-language
