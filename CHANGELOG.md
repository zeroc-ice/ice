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

## Changes in Ice 3.9.0

These are the changes since the [Ice 3.8.1] release.

### General Changes

- Removed the 'ice2slice' compiler

- Fixed the WebSocket transport to enforce the RFC 6455 limits on control frames.

- Fixed an unbounded memory allocation when unmarshaling a proxy with a large endpoint count.

### Slice Language Changes

- Added the `["oneway"]` metadata directive for Slice operations. This directive can only be applied to operations that
  do not return data (no return type, out parameters, or exception specification). `OnewayOnlyException` is thrown
  if a `["oneway"]` operation is invoked using a twoway proxy. In languages where exceptions can be thrown synchronously
  or asynchronously, this exception is always thrown synchronously.

### C++ Changes

- Changed the mapping of `@p [NAME]` tags which reference out parameters in Slice.
  These now generate `` `[NAME]` `` instead of `@p [NAME]`.

### C# Changes

- Changed the mapping of `@p [NAME]` tags which reference out parameters in Slice.
  These now generate `<c>[NAME]</c>` instead of `<paramref name="[NAME]" />`.

- Added the `--icerpc` flag to `slice2cs`. When set, `slice2cs` generates C# code for
  [IceRPC](https://github.com/icerpc/icerpc-csharp) instead of Ice. The `ZeroC.Ice.Slice.Tools` MSBuild integration
  exposes this flag via the `IceRpc` boolean item metadata on `SliceCompile` items.

- Added the `["cs:internal"]` metadata directive. When applied to a Slice definition, the generated C# type is emitted
  with the `internal` access modifier instead of `public`.

- Added the `["cs:readonly"]` metadata directive. When applied to a Slice struct, the generated C# struct is emitted
  as a `readonly` struct.

[Ice 3.8.1]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
