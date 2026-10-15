- Fixed `Ice.stringToIdentity` and proxy string parsing rejecting the `\a` escape, which `Ice.identityToString`
  produces for the U+0007 (bell) character. Identities containing this character now round-trip.
