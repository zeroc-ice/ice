- Fixed the unmarshaling of a malformed encoding 1.0 class reference with the index -2147483648: the Swift runtime now
  throws `MarshalException` instead of crashing.
