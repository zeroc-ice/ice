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

These are the changes since Ice 3.6.2.

## General Changes

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

## Java Changes

- Fixed a bug where unmarshaling Ice objects was really slow when using
  compact type IDs.

## PHP Changes

- Added support for PHP 7.
