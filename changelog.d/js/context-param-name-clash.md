- Fixed `slice2js` to rename the parameters it synthesizes for generated proxy and skeleton methods when an operation
  already has an in-parameter with the same name. An operation with an in-parameter named `context` (proxy) or
  `current` (skeleton) previously produced a TypeScript declaration with two parameters of that name, which failed to
  compile.
