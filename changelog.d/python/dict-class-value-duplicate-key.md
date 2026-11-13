- Fixed a crash when unmarshaling a dictionary whose value type is a class (for example
  `dictionary<string, SomeClass>`) and whose encoded entries repeat a key.
