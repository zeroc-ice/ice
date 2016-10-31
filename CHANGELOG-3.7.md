The entries below contain brief descriptions of the changes in a release, in no
particular order. Some of the entries reflect significant new additions, while
others represent minor corrections. Although this list is not a comprehensive
report of every change we made in a release, it does provide details on the
changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your
applications to this release, and the manual for complete details on a
particular aspect of Ice.

- [Changes in Ice 3.7.0](#changes-in-ice-370)
  - [General Changes](#general-changes)

# Changes in Ice 3.7.0

These are the changes since Ice 3.6.3.

## General Changes

- Added Bluetooth transport plug-in for C++ and Android. The C++ plug-in
  requires BlueZ 5.40 or later.

- Dispatch interceptors and ice_dispatch can now handle user exceptions. User
  exceptions raised by a servant dispatch are propagated to ice_dispatch and
  can also be raised from the Ice::DispatchInterceptor::dispatch implementation.
  As a result, the Ice::DispatchStatus enumeration has been removed. See the
  Ice manual for details on the new dispatch interceptor API.

- The ice_getConnection() method now correctly returns a connection if
  connection caching is disabled (it previously returned a null connection).

- The iOS SSL transport is now based on the same implementation as macOS. Most
  of the functionality supported on macOS is now also supported on iOS. There
  are still few limitations however:

  - the `checkValidity`, `getNotBefore`, `getNotAfter` methods are not supported
    on the `IceSSL::Certificate` class.

  - only PKCS12 certificates are supported (no support for PEM).

- Added support for iAP transport to allow iOS clients to communicate with
  connected accessories.

- The `Ice::ConnectionInfo` `sndSize` and `rcvSize` data members have been moved
  to the TCP and UDP connection info classes. The `Ice::WSEndpointInfo` and
  `IceSSL::EndpointInfo` classes no longer inherit `Ice::IPConnectionInfo` and
  instead directly extend `Ice::ConnectionInfo`. IP connection information can
  still be retrieved by accessing the connection information object stored with
  the new `underlying` data member.

- IceGrid and IceStorm now use LMDB for their persistent storage instead of
  Freeze/BerkeleyDB.

- Added command line tools, `icegriddb` and `icestormdb`, to import/export the
  IceGrid and IceStorm databases.

- Added support for two additional IceGrid variables: `server.data` and
  `service.data`. These variables point to server and service specific data
  directories created by IceGrid on the node. These data directories are
  automatically removed by IceGrid if you remove the server from the
  deployment.

  For consistency, the `node.datadir` variable has been deprecated, use the
  `node.data` variable instead.

- Added the new metadata tag `delegate` for local interfaces with one operation.
  Interfaces with this metadata will be generated as a `std::function` in C++11,
  `delegate` in C#, `FunctionalInterface` in Java, `function callback` in
  JavaScript, `block` in Objective-C, `function/lambda` in Python. Other language
  mappings keep their default behavior.

- `ObjectFactory` has been deprecated in favor of the new local interface
  `ValueFactory`. Communicator operations `addObjectFactory`and
  `findObjectFactory` have been deprecated in favor of similar operations on the
  new interface `ValueFactoryManager`.

- Renamed local interface metadata `async` to `async-oneway`.

- Replaced `ConnectionCallback` by delegates `CloseCallback` and `HeartbeatCallback`.
  Also replaced `setCallback` by `setCloseCallback` and `setHeartbeatCallback` on
  the `Connection` interface.

- Updating Windows build system to use MSBuild instead of nmake.

- Changed the parsing of hex escape sequences (\x....) in Slice string literals:
  the parsing now stops after 2 hex digits. For example, \x0ab is now read as '\x0a'
  followed by 'b'. Previously all the hex digits where read like in C++.

- Stringified identities and proxies now support non-ASCII characters
  and universal character names (\unnnn and \Unnnnnnnn). See the property
  Ice.ToStringMode and the static function/method identityToString.

- IcePatch2 and IceGrid's distribution mechanism have been deprecated.

## Java Changes

- Fixed a bug where unmarshaling Ice objects was really slow when using
  compact type IDs.

## Objective-C Changes

- Fixed a bug where optional object dictionary parameters would
  trigger an assert on marshaling.
