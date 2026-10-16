- Fixed a `NullPointerException` when unmarshaling an optional proxy that is set but holds a null proxy.
  Now such a proxy is unmarshaled to an empty `Optional`.
- Fixed a `NullPointerException` when unmarshaling an optional `java:serializable` object that is set but holds an
  empty sequence. Now such an object is unmarshaled to an empty `Optional`.