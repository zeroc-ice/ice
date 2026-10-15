- Fixed a crash when unmarshaling a dictionary whose value type is a class (for example `dictionary<string, SomeClass>`)
  contained in a class or exception marshaled with the sliced format. With the 1.0 encoding, the crash occurred for any
  such dictionary.
