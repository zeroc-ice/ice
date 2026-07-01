- Fixed a `slice2cpp` bug where a `float` constant whose value is rendered in scientific notation (any magnitude
  ≥ 1e6 or < 1e-4, e.g. `1e8`) generated an invalid C++ literal such as `1e+08.0F`, causing the generated header to
  fail to compile.
