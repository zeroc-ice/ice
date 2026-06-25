- Assigning an out-of-range or non-integer value to an `InputStream` or `OutputStream` position now throws a
  `RangeError` instead of being silently ignored, matching the buffer-position behavior of the other language
  mappings.
