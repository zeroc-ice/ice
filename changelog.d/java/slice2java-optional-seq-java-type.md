- Fixed a `slice2java` bug with optional sequences. When a parameter, return value, or field applied `java:type`
  metadata that overrode the sequence's default mapping, `slice2java` generated no marshaling code. As a result,
  the generated Java did not compile (parameters and return values) or silently dropped the value on the wire (fields).
