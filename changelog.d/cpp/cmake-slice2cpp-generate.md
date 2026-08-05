- Fixed the CMake `slice2cpp_generate` function to compile Slice files that live in subdirectories: generated
  files now mirror the layout of the `.ice` files found through the new `INCLUDE_DIRS` argument.
- `slice2cpp_generate` now accepts `INCLUDE_DIRS`, `OPTIONS`, `HEADER_OUTPUT_DIR`, `INCLUDE_DIR` and
  `INCLUDE_SCOPE`, to pass include directories and options to `slice2cpp`, write the generated headers to their
  own directory, and export them to the target's consumers.
