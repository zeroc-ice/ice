# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)
  - [Slice Language Changes](#slice-language-changes)

## Changes in Ice 3.9.0

These are the changes since the [Ice 3.8.1] release.

### General Changes

- Removed the 'ice2slice' compiler

### Slice Language Changes

- Added the `["oneway"]` metadata directive for Slice operations. This directive can only be applied to operations that
  do not return data (no return type, out parameters, or exception specification). `OnewayOnlyException` is thrown
  if a `["oneway"]` operation is invoked using a twoway proxy. In languages where exceptions can be thrown synchronously or asynchronously, this exception is always thrown synchronously.

- Removed support for using `@p` tags in doc-comments to reference out parameters.

[Ice 3.8.1]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
