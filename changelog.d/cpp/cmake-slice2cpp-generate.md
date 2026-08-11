- Fixed the CMake `slice2cpp_generate` function to compile Slice files that live in subdirectories: generated
  files now mirror the layout of the `.ice` files found through the new `INCLUDE_DIRS` argument.
- `slice2cpp_generate` now accepts `INCLUDE_DIRS`, `OPTIONS`, `HEADER_OUTPUT_DIR`, `INCLUDE_DIR`,
  `INCLUDE_SCOPE`, `DEPENDS`, `GENERATED_HEADERS` and `GENERATED_SOURCES`, to pass include directories and
  options to `slice2cpp`, write the generated headers to their own directory, export them to the target's
  consumers, order generation after the build steps that produce a `.ice` source, and report the generated
  files' paths.
