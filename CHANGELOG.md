# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [C# Changes](#c-changes-1)
  - [JavaScript Changes](#javascript-changes)
  - [MATLAB Changes](#matlab-changes)
  - [PHP Changes](#php-changes)
  - [Python Changes](#python-changes)
  - [Ruby Changes](#ruby-changes)
  - [Ice Service Changes](#ice-service-changes)
    - [Ice Service installed as a Windows Service](#ice-service-installed-as-a-windows-service)

## Changes in Ice 3.9.0

### General Changes

- Fixed the server-side WebSocket opening handshake to reject messages that are not `GET` requests, as required by
  RFC 6455. A peer could previously trip an assertion by sending a response-shaped handshake.

### C++ Changes

- Fixed a crash in the iOS (SecureTransport) SSL transport: using `IceSSL.FindCert` to select a keychain
  certificate that has no label attribute could abort the process during communicator initialization. Ice now reports
  a clear error instead.

- Fixed a crash in the macOS (SecureTransport) SSL transport: configuring `IceSSL.CertFile` together with
  `IceSSL.KeyFile` using a certificate that has no Subject Key Identifier extension aborted the process during
  communicator initialization. Such certificates are now rejected with a `CertificateReadException`.

### C# Changes

- Fixed a bug in `slice2cs --icerpc` where the generated request decoder read in-parameters in declaration order
  instead of the marshal order. As a result, an operation with an optional parameter declared before a required
  parameter (or optional parameters declared out of tag order) could fail to decode or decode incorrect values.

- Fixed a bug in `slice2cs --icerpc` where the generated proxy built the response tuple in marshal order while
  declaring it in declaration order. As a result, an operation whose out-parameters were declared in an order
  different from their marshal order could return values in the wrong tuple slots or fail to compile.

- Fixed thread-safety bugs in the C# metrics (IceMX) implementation that could produce incorrect metrics or
  throw under concurrent updates.

- Fixed `iceboxnet` rejecting valid per-service command-line options (`--<service>.*`) with "unknown option" and
  failing to start: the option validation iterated the original arguments instead of the filtered list.

- The per-thread `ImplicitContext.getContext` in C# now returns a snapshot of the context instead of the live
  internal dictionary (matching the `Shared` implementation). Code that mutated the returned dictionary to
  update the implicit context must now use `put` or `setContext`.

- Fixed a bug in `slice2cs` handling of `cs:namespace`: a nested module received the namespace prefix twice
  (e.g. `Foo.A.Foo.B` instead of `Foo.A.B`), producing C# that did not compile.

### JavaScript Changes

- Assigning an out-of-range or non-integer value to an `InputStream` or `OutputStream` position now throws a
  `RangeError` instead of being silently ignored, matching the buffer-position behavior of the other language
  mappings.

### MATLAB Changes

- Fixed `ice_getCachedConnection` to return an empty `Ice.Connection` array, rather than an empty
  `double` array, when the proxy has no cached connection.

- Fixed the unmarshaling of unknown optional values with tags greater than or equal to 30. These no
  longer desynchronize the input stream causing spurious `MarshalException`.

- Fixed the `ice_getConnectionAsync` proxy method. Retrieving the result of the returned future failed on every
  successful call because the `Ice.Connection` was constructed without its communicator.

- Fixed `Ice.Future.wait('sent')`, which could block indefinitely or time out even after the request had been
  sent. The wait now completes as soon as the invocation reaches or passes the requested state.

- Fixed `slice2matlab` to map all `long` constants and default values to MATLAB `int64`. They were previously
  emitted as bare numeric literals, which MATLAB interprets as `double`, silently losing precision for values
  with magnitude greater than 2^53.

### PHP Changes

- Fixed Ice for PHP mis-marshaling a non-empty optional `sequence<string>`.

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

- Fixed Ice for Ruby to correctly marshal an empty `sequence<byte>` supplied as a string: it now writes a single size
  byte instead of a raw 4-byte integer, which previously de-synchronized the rest of the message.

### Ice Service Changes

#### Ice Service installed as a Windows Service

- Fixed a crash on shutdown. When an Ice service running as a Windows service was stopped using an Ice admin tool —
  for example, an IceGrid node shut down with `icegridadmin` or the IceGrid GUI — instead of through the Windows
  Service Control Manager, the process terminated abnormally rather than stopping cleanly (and never reported
  `SERVICE_STOPPED`). This affected the IceGrid registry and node, the Glacier2 router, and IceBox.

These are the changes since the [Ice 3.8.2] release.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
