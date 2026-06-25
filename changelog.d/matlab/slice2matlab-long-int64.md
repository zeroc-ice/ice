- Fixed `slice2matlab` to map all `long` constants and default values to MATLAB `int64`. They were previously
  emitted as bare numeric literals, which MATLAB interprets as `double`, silently losing precision for values
  with magnitude greater than 2^53.
