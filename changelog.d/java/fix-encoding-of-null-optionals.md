- The methods for writing optional values now treat `null` as equivalent to 'unset'; meaning they now write nothing
  on the wire for `null` arguments. Previously, they encoded a null proxy or `java:serializable` object as set-but-null,
  a null string, sequence, or dictionary as set-but-empty, and a null struct or enum threw a `NullPointerException`.
