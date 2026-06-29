- Fixed a crash in Ice for PHP when unmarshaling a non-empty dictionary whose value type is a class (for example
  `dictionary<string, SomeClass>`). The dictionary key could be freed before the deferred class value was inserted,
  causing a use-after-free.
