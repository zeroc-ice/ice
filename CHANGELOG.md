# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)
  - [Slice Language Changes](#slice-language-changes)
  - [C++ Changes](#c-changes)
  - [C# Changes](#c-changes-1)
  - [Swift Changes](#swift-changes)
  - [Ice Service Changes](#ice-service-changes)

## Changes in Ice 3.9.0

These are the changes since the [Ice 3.8.1] release.

### General Changes

- Removed the 'ice2slice' compiler

- Fixed the WebSocket transport to enforce the RFC 6455 limits on control frames.

- Fixed an unbounded memory allocation when unmarshaling a proxy with a large endpoint count.

- Fixed an integer overflow in the sequence-size validation performed while unmarshaling.

- Fixed the unmarshaling of classes and exceptions to reject a malformed sliced-format slice header.

- Fixed the unmarshaling of batch requests to reject a request count larger than the message can hold.

- Fixed the unmarshaling of IP endpoints to reject a port value outside the 0..65535 range.

- Fixed the unmarshaling of class indirection tables to reject a zero-valued entry.

### Slice Language Changes

- Added the `["oneway"]` metadata directive for Slice operations. This directive can only be applied to operations that
  do not return data (no return type, out parameters, or exception specification). `OnewayOnlyException` is thrown
  if a `["oneway"]` operation is invoked using a twoway proxy. In languages where exceptions can be thrown synchronously
  or asynchronously, this exception is always thrown synchronously.

### C++ Changes

- Fixed alignment-unsafe 16-bit reads and writes in the WebSocket transport.

- Changed the macOS SSL transport to require TLS 1.2 or later.

- Changed the macOS SSL transport to enable only forward-secret (ECDHE) cipher suites.

- Fixed a resource leak in the SSL engine. The Schannel and OpenSSL engines now release their
  certificate stores, chain engines, imported key sets, and `SSL_CTX` when the communicator is
  destroyed.

- Changed the macOS SSL transport so that, when `IceSSL.Keychain` is not set, the certificate configured with
  `IceSSL.CertFile` is imported into a temporary keychain (removed when the communicator is destroyed) instead of
  the user's login keychain.

- Rejected peer-initiated TLS renegotiation in the OpenSSL SSL engine and on the server side of Schannel-based SSL
  connections. This applies to all SSL-based transports (`ssl`, `wss`, `bts`).

- Changed the mapping of `@p [NAME]` tags which reference out parameters in Slice. These now generate `` `[NAME]` ``
  instead of `@p [NAME]`.

### C# Changes

- Added the `--icerpc` flag to `slice2cs`. When set, `slice2cs` generates C# code for
  [IceRPC](https://github.com/icerpc/icerpc-csharp) instead of Ice. The `ZeroC.Ice.Slice.Tools` MSBuild integration
  exposes this flag via the `IceRpc` boolean item metadata on `SliceCompile` items.

- Added the `["cs:internal"]` metadata directive. When applied to a Slice definition, the generated C# type is emitted
  with the `internal` access modifier instead of `public`.

- Added the `["cs:readonly"]` metadata directive. When applied to a Slice struct, the generated C# struct is emitted
  as a `readonly` struct.

- Changed the mapping of `@p [NAME]` tags which reference out parameters in Slice. These now generate `<c>[NAME]</c>`
  instead of `<paramref name="[NAME]" />`.

- Fixed a resource leak in the SSL engine. The certificates loaded from `IceSSL.CertFile` and `IceSSL.CAs` are now
  disposed when the communicator is destroyed, instead of waiting for the GC to finalize them.

### Swift Changes

- Fixed `InputStream.readSize` to reject a negative size.

### Ice Service Changes

- Updated the creation of SSL-based sessions in IceGrid and Glacier2: a connection without a client certificate, or with
  an empty DN, is now rejected before reaching the permissions verifier.

- Updated the Glacier2CryptPermissionsVerifier plug-in to issue a warning when the configured password file contains one
  or more DES passwords.

[Ice 3.8.1]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
