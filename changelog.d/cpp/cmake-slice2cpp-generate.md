- Fixed the CMake `slice2cpp_generate` function to compile Slice files that live in subdirectories. It previously
  wrote every generated file into one flat directory, so the `#include` directives `slice2cpp` emits did not resolve,
  and two Slice files with the same name collided.
- `slice2cpp_generate` now accepts `INCLUDE_DIRS`, `OPTIONS`, `HEADER_OUTPUT_DIR`, `INCLUDE_DIR` and `INCLUDE_SCOPE`,
  to pass additional include directories and options to `slice2cpp`, write the generated headers to their own
  directory, and export them from a library. The Slice dependency scan runs with the same options as the compile, so
  `-D` conditional includes produce correct dependencies.
