# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)
  - [C# Changes](#c-changes)
  - [JavaScript Changes](#javascript-changes)
  - [Python Changes](#python-changes)
  - [Ruby Changes](#ruby-changes)

## Changes in Ice 3.9.0

### General Changes

- Fixed the server-side WebSocket opening handshake to reject messages that are not `GET` requests, as required by
  RFC 6455. A peer could previously trip an assertion by sending a response-shaped handshake.

### C# Changes

- Fixed a bug in `slice2cs --icerpc` where the generated request decoder read in-parameters in declaration order
  instead of the marshal order. As a result, an operation with an optional parameter declared before a required
  parameter (or optional parameters declared out of tag order) could fail to decode or decode incorrect values.

- Fixed a bug in `slice2cs --icerpc` where the generated proxy built the response tuple in marshal order while
  declaring it in declaration order. As a result, an operation whose out-parameters were declared in an order
  different from their marshal order could return values in the wrong tuple slots or fail to compile.

- Fixed C# IceLocatorDiscovery hanging a request when locator rediscovery resolved to the same proxy that had
  just failed: the retry threw into a discarded task, leaving the request's `TaskCompletionSource` uncompleted.

### JavaScript Changes

- Assigning an out-of-range or non-integer value to an `InputStream` or `OutputStream` position now throws a
  `RangeError` instead of being silently ignored, matching the buffer-position behavior of the other language
  mappings.

### Python Changes

- Fixed Ice for Python to reliably abort request marshaling when an invalid value is supplied for a sequence
  parameter (such as a non-sequence argument), instead of continuing with a pending Python exception and sending a
  corrupt or truncated request.

### Ruby Changes

- Fixed `Ice::createProperties` in Ice for Ruby to accept `nil` as the defaults argument (equivalent to omitting it).
  A `nil` defaults previously bypassed the type check and crashed the interpreter.

- Fixed Ice for Ruby to keep a custom `SliceLoader` alive for the communicator's lifetime. The wrapper previously
  registered the wrong address with the garbage collector, leaving a dangling root and allowing the loader to be
  collected while still in use.

- Fixed `Ice::Endpoint` comparison in Ice for Ruby. `<=>` compared an endpoint with itself, making `==`/`<=>`
  asymmetric, and the class defined `eql?` without a matching `hash`; equal endpoints now compare consistently and
  can be used as `Hash`/`Set` keys.

These are the changes since the [Ice 3.8.2] release.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
