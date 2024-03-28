The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

We recommend that you use the release notes as a guide for migrating your applications to this release, and the manual
for complete details on a particular aspect of Ice.

# Changes in Ice 3.8.0

These are the changes since the Ice 3.7.10 release in [CHANGELOG-3.7.md](./CHANGELOG-3.7.md).

## Slice Language Changes

- Removed support for the `["preserve-slice"]` metadata. Slice classes encoded in the Sliced-format are now always
  full-preserved.

- Exceptions are now always encoded in the Sliced-format and no longer support preservation of unknown type slices.

- Removed local Slice. `local` is no longer a Slice keyword.

- Slice classes can no longer define operations or implement interfaces, and `implements` is no longer a Slice keyword.
This feature has been deprecated since Ice 3.7.

- Slice classes no longer represent remote Ice objects; the syntax `MyClass*` (a proxy to a class) is now invalid.

- An interface can no longer be used as a type. This feature, known as "interface by value", has been deprecated since
Ice 3.7. You can still define proxies with the usual syntax, `Greeter*`, where `Greeter` represents an interface.

## C++ Changes

- There is now a single C++ mapping, based on the C++11 mapping provided by Ice 3.7. This new C++ mapping requires a
C++ compiler with support for std=c++17 or higher.

## Objective-C Changes

- The Objective-C mapping was removed.

## PHP Changes

- Removed the flattened mapping deprecated in 3.7.

## IceGrid Changes

- Removed deprecated server and application distributions in IceGrid. These distributions relied on the IcePatch2
service.

## IcePatch2 Changes

- The IcePatch2 service was removed.
