- Fixed a bug in `slice2java` that emitted broken code for dictionaries using `java:type:<instance-type>:<formal-type>`
  metadata to specify a formal type. This bug affected dictionaries used in classes or exceptions, or as the
  return type of operations with `["marshaled-result"]`.
