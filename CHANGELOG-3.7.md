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
  - [C++ Changes](#c-changes)
  - [C# Changes](#c-changes-1)
  - [Java Changes](#java-changes)
  - [JavaScript Changes](#javascript-changes)
  - [Objective-C Changes](#objective-c-changes)
  - [PHP Changes](#php-changes)
  - [Python Changes](#python-changes)

# Changes in Ice 3.7.0

These are the changes since the Ice 3.6 release or snapshot described in
[CHANGELOG-3.6.md](./CHANGELOG-3.6.md).

## General Changes

- Semicolons are now optional after braces in Slice definitions. For example
  ```
  module M
  {
      enum { A, B, C , D }

      interface Intf
      {
          void op();
      }
  }
  ```
  is equivalent to
  ```
  module M
  {
      enum { A, B, C , D };

      interface Intf
      {
          void op();
      };
  };
  ```

- Added `Ice::ObjectAdapter::setPublishedEndpoints` to allow updating the
  published endpoints programmatically.

- The server runtime will now bind to all the addresses associated with a DNS
  name specified in an endpoint of the object adapter (with the endpoint -h
  option). You must make sure the DNS name resolves to local addresses only.

  If no `PublishedEndpoints` property is specified for the object adapter, the
  published endpoints for an endpoint with a DNS name will either be, if the
  endpoint doesn't specifies a fixed port, a list of endpoints with each of
  the addresses associated with the DNS name or, if it specifies a fixed port,
  the endpoint with the DNS name.

- Added the IceBridge service, which acts as a bridge between a client and
  server to relay requests and replies in both directions.

- Added new `ice_id` method or member function to all Ice exceptions; `ice_id`
  returns the Slice type ID of the exception. It replaces the now deprecated
  `ice_name` method or member function.

- Added `Ice::Connection::throwException`. When the connection is closed, this
  method throws an exception indicating the reason of the connection closure.

- Changed the Slice definition of the `Connection::close` operation to take an
  enumerator instead of a boolean. The new enumeration, `ConnectionClose`,
  defines three enumerators for controlling how the connection is closed:

  - `Forcefully` - Closes the connection immediately. Equivalent to the boolean
  value true in previous releases.

  - `Gracefully` - Closes the connection gracefully without waiting for pending
  invocations to complete.

  - `GracefullyWithWait` - Closes the connection gracefully after all pending
  invocations have completed. Equivalent to the boolean value false in previous
  releases.

  The `Ice::ForcedCloseConnectionException` exception has also been replaced
  with `Ice::ConnectionManuallyClosedException`. This exception is set on the
  connection when `Connection::close` is called.

- Added new operation metadata, `marshaled-result`, in C++11, C++98, C#, Java,
  and Java Compat. When this metadata is specified, the generated code for
  the servant dispatch returns a generated struct that contains the marshaled
  values for the return and out parameters. See the Ice Manual for additional
  details on this metadata.

- Added new overloads to `Ice::initialize` in C++11, C++98, C#, Java, Java Compat,
  Python and Ruby. They accept a `configFile` string parameter as an alternative
  to the `InitializationData` parameter of several existing `Ice::initialize`
  overloads.

- Added support for a new `Ice.ClassGraphDepthMax` property to prevent stack
  overflows in case a sender sends a very large graph.

  The unmarshaling or destruction of a graph of Slice class instances is a
  recursive operation. This property limits the amount of stack size required to
  perform these operations. This property is supported with all the language
  mappings except Java and JavaScript where it's not needed (the run time
  environment allows graceful handling of stack overflows).

  The default maximum class graph depth is 100. If you increase this value, you
  must ensure the thread pool stack size is large enough to allow reading graphs
  without causing a stack overflow.

- Added support for IceStorm subscriber `locatorCacheTimeout` and
  `connectionCached` QoS settings. These settings match the proxy settings and
  allow configuring per-request load balancing on the subscriber proxy.

- Implementations of the `Ice::Router` interface can now indicate whether or not
  they support a routing table through the optional out parameter `hasRoutingTable`
  of the `getClientProxy` operation. The Ice run time won't call the `addProxies`
  operation if the router implementation indicates that it doesn't manage a routing
  table.

- The `findObjectByType`, `findAllObjectsByType`, `findObjectByTypeOnLeastLoadedNode`
  operations from the `IceGrid::Query` interface and the `allocateObjectByType`
  operation from the `IceGrid::Session` interfaces now only return proxies for
  Ice objects from enabled servers. If a server is disabled, its well-known or
  allocatable Ice objects won't be returned anymore to clients.

- A Slice enumeration (enum) now creates a new namespace scope for its
  enumerators. In previous releases, the enumerators were in the same
  namespace scope as the enumeration. For example:
  ```
     enum Fruit { Apple, Orange, Pear };
     enum ComputerBrands { Apple, Dell, HP }; // Ok as of Ice 3.7, error in prior releases
  ```
  The mapping of enum to C++, C#, Java etc. is not affected by this
  change. Slice constants and data member default values that reference
  enumerators should be updated to use only the enumerator's name when the
  enclosing enum is in a different module. For example:
  ```
  module M1
  {
      enum Fruit { Apple, Orange, Pear };
      enum ComputerBrands { Apple, Dell, HP };

      const Fruit a = Apple; // Recommended syntax for all Ice releases
  };

  module M2
  {
      const M1::Fruit a1 = Apple;             // The recommended syntax as of Ice 3.7
      const M1::Fruit a2 = M1::Fruit::Apple;  // Ok as well
      const M1::Fruit a3 = M1::Apple;         // Supported for backwards compatibility
                                              // with earlier Ice releases
  };

  ```

- The Communicator and Connection `flushBatchRequests` operations now require
  an argument to specify whether or not the batch requests to flush should be
  compressed. See the documentation of the `Ice::CompressBatch` enumeration
  for the different options available to specify when the batch should be compressed.

- The UDP server endpoint now supports specifying `--interface *` to join the
  multicast group using all the local interfaces. It's also now the default
  behavior if no `--interface` option is specified.

- Ice no longer halts a program if it can't accept new incoming connections when
  the system runs out of file descriptors. Instead, it rejects queued pending
  connections and temporarily stops accepting new connections. An error message
  is also sent to the Ice logger.

- Added Bluetooth transport plug-in for C++ and Android. The C++ plug-in
  requires BlueZ 5.40 or later.

- Dispatch interceptors and `ice_dispatch` can now catch user exceptions. User
  exceptions raised by a servant dispatch are propagated to `ice_dispatch` and
  may be raised by the implementation of `Ice::DispatchInterceptor::dispatch`.
  As a result, the `Ice::DispatchStatus` enumeration has been removed. See the
  Ice manual for details on the new dispatch interceptor API.

- The `ice_getConnection` operation now correctly returns a connection if
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
  Freeze/Berkeley DB.

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

- Replaced `Ice::NoObjectFactoryException` with `Ice::NoValueFactoryException`.

- The Slice compiler options `--ice` and `--underscore` are now deprecated, and
  replaced by the global Slice metadata `ice-prefix` and `underscore`.

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

- Fixed proxies stringification: `Communicator::proxyToString` and equivalent
  "to string" methods on fixed proxies no longer raise a `FixedProxyException`;
  the proxy is just stringified without endpoints.

- An empty endpoint in an Object Adapter endpoint list is now rejected with an
  `EndpointParseException`; such an endpoint was ignored in previous releases.

- IcePatch2 and IceGrid's distribution mechanism (based on IcePatch2) are now
  deprecated.

- Updated IceSSL hostname verification (enabled with `IceSSL.CheckCertName`) to
  use the native checks of the platform's SSL implementation.

- Removed `IceSSL::NativeConnectionInfo`. `IceSSL::ConnectionInfo`'s `certs` data
  member is now mapped to the native certificate type in C++, Java and C#. In other
  languages, it remains mapped to a string sequence containing the PEM encoded
  certificates.

- Freeze has been moved to its own source repository, https://github.com/zeroc-ice/freeze.
  Freeze is no longer included with Ice binary distributions.

- Added support for suppressing Slice warnings using the` [["suppress-warning"]]`
  global metadata directive. If one or more categories are specified (for
  example `"suppress-warning:invalid-metadata"` or
  `"suppress-warning:deprecated, invalid-metadata"`) only warnings matching these
  categories are suppressed, otherwise all warnings are suppressed.

## C++ Changes

- Added a new C++11 mapping that takes advantage of C++11 language features. This
  new mapping is very different from the Slice-to-C++ mapping provided in prior
  releases. The old mapping, now known as the C++98 mapping, is still supported so
  that existing applications can be migrated to Ice 3.7 without much change.

- Added support for Visual Studio 2010 (C++98 only)

- The `Ice::Communicator` and `Ice::ObjectAdapter` `destroy` functions are now
  declared as `noexcept` (C++11) or `throw()` (C++98).

- Added new class `Ice::CommunicatorHolder`. `CommunicatorHolder` creates a `Communicator`
  in its constructor and destroys it in its destructor.

- The `--dll-export` option of `slice2cpp` is now deprecated, and replaced by the global
  Slice metadata `cpp:dll-export:SYMBOL`.

- Added `cpp:scoped` metadata for enums in the C++98 mapping. The generated C++
  enumerators for a "scoped enum" are prefixed with the enumeration's name. For
  example:
  ```
     // Slice
     ["cpp:scoped"] enum Fruit { Apple, Orange, Pear };
  ```
  corresponds to:
  ```
     // C++98
     enum Fruit { FruitApple, FruitOrange, FruitPear };
  ```

- Upgrade the UWP IceSSL implementation to support client side certificates and custom
  certificate verification.

- Added `getOpenSSLVersion` function to `IceSSL::OpenSSL::Plugin` to retrieve
  the OpenSSL version used by the Ice runtime.

- Added `getAuthorityKeyIdentifier` and `getSubjectKeyIdentifier` functions to
  `IceSSL::Certificate`. These functions are not supported on iOS or UWP.

- Improved the IceSSL Certificate API to allow retrieving X509v3 extensions.
  This feature is currently only available with OpenSSL and SChannel.

- Refactored the IceSSL Plug-in API to allow loading multiple implementations of
  the plug-in in the same process. Each communicator can load a single
  implementation, but separate communicators in the same process can load
  different implementations.

- Added ability to build IceSSL with OpenSSL on Windows. The resulting library
  is named `icesslopenssl`. An application can load this plug-in with the
  `IceSSLOpenSSL:createIceSSLOpenSSL` entry point.

- Added `IceSSL.SchannelStrongCrypto` property: when set to a value greater than
  0, the IceSSL SChannel implementation sets the `SCH_USE_STRONG_CRYPTO` flag,
  which instructs SChannel to disable weak cryptographic algorithms. The default
  values for this property is 0 for increased interoperability.

- Improve Linux stack traces by using libbacktrace when available.

- Fixed IceGrid PlatformInfo to report the correct Windows release and version
  versions greater than Windows 8 were reported as Windows 8.

- IceSSL has been updated to support OpenSSL 1.1.0 version.

- Add IceBridge Ice service that acts as a bridge between one or more clients and
  a server.

## C# Changes

- The `batchRequestInterceptor` data member of `Ice.InitializationData` is now
  defined as a `System.Action<Ice.BatchRequest, int, int>` delegate. You will
  need to update your code accordingly if you were using the now removed
  `Ice.BatchRequestInterceptor` interface.

- The `Ice.PropertiesUpdateCallback` interface is deprecated, use the
  `System.Action<Dictionary<string, string>>` delegate instead to receive
  property updates.

- The `threadHook` member of `InitializationData` is now deprecated. We have
  added `threadStart` and `threadStop` members for consistency with the C++11
  and Java mappings. A program should set these members to a `System.Action`
  delegate.

- The `Ice.ClassResolver` delegate has been replaced with the
  `System.Func<string, Type>` delegate. The `Ice.CompactIdResolver` delegate
  has been replaced with the `System.Func<int, string>` delegate. The
  `Ice.Dispatcher` delegate has been replaced with the
  `System.Action<System.Action, Ice.Connection>` delegate.

- Added new interface/class metadata `cs:tie`. Use this metadata to generate a tie
  class for a given interface or class.

- `cs:` and `clr:` are now interchangeable in metadata directives.

- Add support to preload referenced assemblies. The property
  `Ice.PreloadAssemblies`   controls this behavior. If set to a value greater than
  0 the Ice run-time will try   to load all the assemblies referenced by the
  process during communicator initialization,   otherwise the referenced
  assemblies will be initialized when the Ice run-time needs   to lookup a C#
  class. The default value is 0.

- Added a new C# AMI mapping based on TAP (Task-based Asynchronous Pattern) using this
  mapping allow applications to take advantage of C# async/away keywords.

- Update the AMD mapping to be Task-based, this greatly improved the interoperability
  of AMI and AMD making straightforward to chain AMD and AMI calls.

- Added support for thread safe marshalling annotating operations with ["marshaled-result"]
  metadata will change the servant operation signature to return a marshalled result avoiding
  thread safety issues derived from returning references to mutable objects. Refer to the
  "Parameter Passing in C-Sharp" section of the manual for more details about this feature.

- Update C# proxy implementation to implement ISerializable.

## Java Changes

- Added a new Java mapping that takes advantage of Java 8 language features. The new
  mapping is significantly different than prior releases in many ways, including the
  package name (com.zeroc) as well as APIs such as AMI, AMD, out parameters and optional
  values. The prior mapping, now known as Java Compat, is still supported so that
  existing applications can be migrated to Ice 3.7 without much change.

- The Ice Communicator interface now implements `java.lang.AutoCloseable`. This enables
  the code to initialize the communicator within a `try-with-resources` statement.
  The communicator will be destroyed automatically at the end of this statement.

- Fixed a bug where unmarshaling Ice objects was very slow when using
  compact type IDs.

- (Java Compat) Added new interface/class metadata `java:tie`. Use this metadata
  to generate a tie class for a given interface or class.

## JavaScript Changes

- Improved the `Ice.Long` class to allow creating `Ice.Long` instance from
  JavaScript `Numbers`.

- The `Ice.Promise` class is now extending the standard JavaScript `Promise`
  class.

- The `Ice.Class` helper function used to create classes has been removed, the
  Ice runtime and the generated code now use the JavaScript `class` keyword to
  define the classes.

- `Ice.HashMap` usage is now limited to dictionaries with mutable keys, for all
  other cases the standard JavaScript `Map` type is used.

- `Ice.HashMap` API has been aligned with the API of JavaScript `Map` type.

- Added support to map Slice modules to JavaScript native modules this requires
  using the global metadata `[["js:es6-module"]]`.

- The `["amd"]` metadata is now ignored in JavaScript. An operation can now be
  be dispatched asynchronously by just returning a JavaScript Promise object.

- `sequence<byte>` is now always mapped to the `Uint8Array` JavaScript type. It
  used to be mapped to the `Buffer` type for NodeJS and to `Uint8Array` for
  browsers.

- The helper method `Ice.Buffer.createNative` has been removed and replaced by
  the use of `Uint8Array`.

## Objective-C Changes

- Fixed a bug where optional object dictionary parameters would
  trigger an assert on marshaling.

- The `--dll-export` option of `slice2objc` is now deprecated, and replaced by the
  global Slice metadata `objc:dll-export:SYMBOL`.

- Added `objc:scoped` metadata for enums. The generated Objective-C enumerators
  for a "scoped enum" are prefixed with the enumeration's name. For example:
  ```
  // Slice
  module M
  {
     ["objc:scoped"] enum Fruit { Apple, Orange, Pear };
  };

  ```
  corresponds to:
  ```
  // Objective-C
  typedef enum : ICEInt
  {
      MFruitApple,
      MFruitPear,
      MFruitOrange
  } MFruit;
  ```

## PHP Changes

- Added support for PHP 7.0 and PHP 7.1.

- The symbol used to indicate an unset optional value for the PHP namespace
  mapping is `Ice\None`. The symbol for the flattened mapping remains `Ice_Unset`,
  but since `unset` is a PHP keyword, we could not use `Ice\Unset`.

## Python Changes

- Added support for the Dispatcher facility. The `dispatcher` member of
  `InitializationData` can be set to a callable that Ice invokes when it
  needs to dispatch a servant invocation or an AMI callback. This facility
  is useful for example in UI applications where it's convenient to
  schedule Ice activity for execution on the main UI thread.

- The `threadHook` member of `InitializationData` is now deprecated. We have
  added `threadStart` and `threadStop` members for consistency with the C++11
  and Java mappings. A program should set these members to a callable, such as
  a lambda function.

- The `batchRequestInterceptor` member of `InitializationData` can now be set
  to a callable. For backward compatibility, a program can also continue to supply
  an instance of the deprecated class `Ice.BatchRequestInterceptor`.

- The Ice Communicator now implements the context manager protocol. This enables
  the code to initialize the communicator within a `with` statement. The communicator
  is destroyed automatically at the end of the `with` statement.

- Added a new AMI mapping that returns `Ice.Future`. The Future class provides an API
  that is compatible with `concurrent.futures.Future`, with some additional Ice-specific
  methods. Programs can use the new mapping by adding the suffix `Async` to operation
  names, such as `sayHelloAsync`. The existing `begin_/end_` mapping is still supported.

- Changed the AMD mapping. AMD servant methods must no longer append the `_async` suffix to
  their names. Additionally, an AMD callback is no longer passed to a servant method.
  Now a servant method always uses the mapped name, and it can either return the results
  (for a synchronous implementation) or return an `Ice.Future` (for an asynchronous
  implementation).

  With Python 3, a servant method can also be implemented as a coroutine. Ice will start
  the coroutine, and coroutines can `await` on `Ice.Future` objects. Note that because Ice
  is multithreaded, users who also want to use the asyncio package must make sure it's
  done in a thread-safe manner. To assist with this, the `Ice.wrap_future` function accepts
  an `Ice.Future` and returns an `asyncio.Future`.

- Renamed optional invocation context parameter to `context` for consistency with other
  language mappings (was `_ctx` in previous versions).

- Fixed a bug where Ice.Application Ctrl-C handler was installed even if Ice.Application.NoSignalHandling
  was set.

## Ruby Changes

- Ice for Ruby is no longer supported on Windows.

- Fix Application Ctrl-C handling to be compatible with Ruby 2.x signal handler restrictions.
