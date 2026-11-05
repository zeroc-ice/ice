- Fixed `Ice::createProperties` in Ice for Ruby to accept `nil` as the defaults argument (equivalent to omitting it).
  A `nil` defaults previously bypassed the type check and crashed the interpreter.
