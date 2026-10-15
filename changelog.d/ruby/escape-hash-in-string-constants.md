- Fixed `slice2rb` and `Ice::loadSlice` to escape `#` in the Ruby string literals they generate for Slice string
  constants. A Slice string containing Ruby interpolation syntax such as `#{...}`, `#@`, or `#$` previously produced a
  double-quoted Ruby literal that was interpreted at load time, corrupting the value or executing code.
