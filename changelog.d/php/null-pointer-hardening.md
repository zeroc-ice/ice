- Fixed a crash where creating Ice extension objects (such as a proxy, communicator, or endpoint) outside of Ice could
  crash the PHP process. Creating such objects via `new`, `unserialize()`, or reflection now raises an error instead.
- Ice extension objects can no longer be serialized or deserialized.
