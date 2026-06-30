- Fixed `slice2rb` to escape `#` in generated Ruby string literals (constants and deprecation reasons). A Slice string
  containing Ruby interpolation syntax such as `#{...}`, `#@`, or `#$` previously produced a double-quoted Ruby literal
  that was interpreted at module-load time, corrupting the value or executing code.
