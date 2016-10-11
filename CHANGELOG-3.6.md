The entries below contain brief descriptions of the changes in a release, in no
particular order. Some of the entries reflect significant new additions, while
others represent minor corrections. Although this list is not a comprehensive
report of every change we made in a release, it does provide details on the
changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your
applications to this release, and the manual for complete details on a
particular aspect of Ice.

- [Changes in Ice 3.6.3](#changes-in-ice-363)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [Objective-C Changes](#objective-c-changes)
  - [PHP Changes](#php-changes)
  - [Python Changes](#python-changes)
- [Changes in Ice 3.6.2](#changes-in-ice-362)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-1)
  - [C# Changes](#c-changes-2)
  - [Java Changes](#java-changes)
  - [Python Changes](#python-changes-1)
  - [Ruby Changes](#ruby-changes)
- [Changes in Ice 3.6.1](#changes-in-ice-361)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-3)
  - [JavaScript Changes](#javascript-changes)
  - [PHP Changes](#php-changes-1)
- [Changes in Ice 3.6.0](#changes-in-ice-360)
  - [General Changes](#general-changes-3)
  - [C++ Changes](#c-changes-4)
  - [C# Changes](#c-changes-5)
  - [Java Changes](#java-changes-1)
  - [JavaScript Changes](#javascript-changes-1)
  - [Objective-C Changes](#objective-c-changes-1)
  - [PHP Changes](#php-changes-2)
  - [Python Changes](#python-changes-2)
  - [Ruby Changes](#ruby-changes-1)


# Changes in Ice 3.6.3

These are the changes since Ice 3.6.2.

## General Changes

- Fixed IceGrid bug where deployment of an application would fail if a well-
  known object was registered with an adapter whose adapter ID contained spaces.

- Added support for limiting the number of events queued for a given subscriber.
  This is useful to prevent IceStorm from consuming too much memory when a
  subscriber is too slow to consume published events. The queue maximum size is
  configured with the IceStorm.Send.QueueSizeMax property. You can use the
  property IceStorm.Send.QueueSizeMaxPolicy=RemoveSubscriber|DropEvents to
  configure the behavior of IceStorm when the limit is reached. By default,
  IceStorm will queue events indefinitely.

- Speed-up the update of IceStorm replicas after the election of a new IceStorm
  coordinator.

- Fixed a bug in the IceLocatorDiscovery plug-in that could occasionally
  trigger an infinite loop.

- Fixed a bug in the unmarshalling code where passing optional input
  parameters to an operation with no required input parameters would
  cause an Ice::EncapsulationException to be thrown if the receiver
  didn't expect the optional input parameters. The same applies for
  passing optional output parameters to operations without required
  output parameters.

- Fixed a bug in icegridadmin and IceGridGUI which was preventing to get
  properties for IceBox services using the IceBox shared communicator.

- Fixed a bug in IceGrid Admin (IceGridGUI) that resulted in an incorrect
  state when you removed and then re-added the same IceGrid application.

- The Slice compilers now support non-ASCII paths on all platforms.

- General clean up in slice2html, including a fix for broken link paths.

## C++ Changes

- Added support for archiving log files. The property Ice.LogFile.SizeMax
  controls the maximum size in bytes of log files; when a log file reaches
  this size, the log file is renamed and a new log file is started.
  The Ice.LogFile.SizeMax property is set to 0 by default, which means the
  log file size is unlimited and a single log file is created.

## Objective-C Changes

- Added identityToString and stringToIdentity non-member functions, which
  were missing from previous releases.

## PHP Changes

- Added support for PHP 7.0.

## Python Changes

- Fixed a bug in the Slice compiler in which a Slice comment that begins or
  ends with a double quote could generate a bogus Python docstring.

- Fixed bug where Ice::UnknownUserException would sometimes be raised in C++
  extension instead of being passed to Python.

# Changes in Ice 3.6.2

These are the changes since Ice 3.6.1.

## General Changes

- Added two new tools, icegriddb and icestormdb, used to import/export the
  IceGrid and IceStorm databases.

- Fixed a bug that affects Java and C# generated code. The generated patcher
  for reading class data members was bogus when the class had more than one
  class data member and derived from a class that contained class data
  members. The same issue was true for exceptions with class data members
  deriving from exceptions with class data members.

- Fixed a bug that prevented scripting languages (Python, Ruby, Javascript and
  PHP) from marshaling NaN or Infinity as a floating point value.

- Fixed an IceGrid bug where resolving endpoints of dynamically registered
  replica groups would fail unless the client was using an encoding superior
  to the encoding of the dynamically registered object adapters.

- Added missing functions Ice::identityToString and Ice::stringToIdentity
  (C++, Objective-C, PHP, Python and Ruby).

- Added support for universal character names (\uNNNN and \UNNNNNNNN) in Slice
  string constants.

- Fixed Glacier2 router bug where requests from client to server could be lost
  if forwarded as batch requests with the _fwd=O context.

- Fixed Glacier2 bug which would occur in un-buffered mode and when forwarding
  one-way requests. The Glacier2 router could eventually leak and hang on
  shutdown.

## C++ Changes

- Fixed El Capitan build issues caused by a new security feature that no longer
  exports DYLD_LIBRARY_PATH to child processes.

- Fixed potential deadlock that could occur when using collocation optimization
  and serialized server thread pools.

- Fixed IceSSL bug that would only show up with WSS servers running on OS X
  and Linux. The WSS server could stop reading requests if the client sent
  multiple requests within the same SSL record.

## C# Changes

- Minor fixes to release the memory used by the transports to send and receive
  data sooner. The garbage collector can therefore collect this memory sooner
  as well.

## Java Changes

- Fixed a bug where unmarshaling Ice objects was really slow when using
  compact type IDs.

- Fixed an IceGridGUI bug where metrics attributes for Glacier2 and IceStorm
  were not displayed.

- Fixed an IceGridGUI bug where the GUI started minimized in OS X.

## Python Changes

- Added compatibility with Python 3.5.

## Ruby Changes

- Added compatibility with Ruby 2.3.

# Changes in Ice 3.6.1

These are the changes since Ice 3.6.0.

## General Changes

- Fixed IceStorm performance issue when creating a large number of topics.

- Added support for Visual Studio 2015 and Universal Windows Platform (UWP)
  apps.

- Connection and transport buffers are now released when the connection is
  closed rather than waiting for the collection or destruction of the Ice
  connection object.

- Calling `destroy()` on the `Glacier2::SessionHelper` during connection
  establishment will now immediately cause the termination of the connection
  attempt and `SessionCallback::connectFailed()` will be called with a
  `CommunicatorDestroyedException`. Previously, the helper would wait until
  the connection attempt either succeeded or failed before calling
  `SessionCallback::disconnected()` or `SessionCallback::connectFailed`.

- Fixed a bug in Slice compilers that would incorrectly reject valid Slice
  where a parameter with the same name as a type would prevent further use of
  that type in the same scope. For example:
  ```
  module Test
  {
  sequence<int> Seq;
  struct S1
  {
    Seq seq;
    Seq seq2; // This would fail with 'Seq is not a type'
  };

  interface Intf
  {
    void method(Seq seq, Seq seq2); // This would fail with 'Seq is not a type' for second parameter
  };
  };
  ```

- Added `--validate` option to Slice compilers. This option validates the
  options that are passed to the Slice compiler and emits any errors but does
  not create the generated files.

## C++ Changes

- Added registration function for Ice plug-ins (Ice::registerIceSSL, ...). Use
  these functions if you don't want to rely on plug-in dynamic loading at run
  time. Your application will need to link with the plug-in's dynamic library
  if it uses the register function.

- C++ source builds in Windows now use third-party packages from Nuget
  (https://www.nuget.org).

- Windows C++ builds now install the required third-party dependencies to the
  install directory.

## JavaScript Changes

- Fixed an issue in connection code that would cause an unexpected exception to
  be raised in some browsers when connection caching was enabled.

- Fixed an issue where JavaScript browser code was trying to use TCP endpoints.

- Fixed an issue that prevents Ice for JavaScript from running in a Web Worker.

- JavaScript browser libraries no longer include a sourceMappingURL. To use
  source maps you must enable them manually.

## PHP Changes

- Update PHP builds for Linux and OS X to use php-config from PATH. It is no
  longer necessary to set PHP_HOME.

# Changes in Ice 3.6.0

These are the changes since Ice 3.5.1.

## General Changes

- Exceptions raised by the `Ice::Plugin` initialize method are now caught by the
  communicator, which raises `Ice::PluginInitializationException` instead.

- Fixed IceGrid bug where an application update could fail and leave the
  registry in an invalid state if some allocation requests were pending while
  the application was updated.

- Deprecated the `IceSSL.PersistKeySet` which is now only used when importing
  certificates with the deprecated `IceSSL.ImportCert` property.

- Deprecated the `IceSSL.KeySet` property, use the `IceSSL.CertStoreLocation`
  property instead.

- Added `IceSSL.CertStoreLocation` property. This is a Windows specific property
  to specify the location of the certificate store or key set for IceSSL
  operation. The LocalMachine and CurrentUser locations are supported.

- Added `IceSSL::WSSConnectionInfo` local Slice class to provide information on
  a WSS connection. This class extends the `IceSSL::ConnectionInfo` class. The
  `IceSSL::WSSNativeConnectionInfo` was also added to provide the language
  mapping specific certificates associated with the SSL connection.

- Added `IceSSL::WSSEndpointInfo` local Slice class to provide WSS endpoint
  information. This class extends the `IceSSL::EndpointInfo` class.

- Updated `Ice::WSEndpointInfo` to extend `Ice::TCPEndpointInfo` and
  `Ice::WSConnectionInfo` to extend `Ice::TCPConnectionInfo`

- IceSSL now supports using the platform Root Certificate Authorities to
  validate remote certificates. The use of the platform Root CAs can be enabled
  with the `IceSSL.UsePlatformCAs` property. The platform Root CAs are only
  checked if `IceSSL.CAs` is not set.

- The `IceSSL.CertAuthFile` and `IceSSL.CertAuthDir` properties have been
  deprecated. You should now use the `IceSSL.CAs` property to configure the path
  of the PEM file containing the Root Certificate Authorities or the path of a
  directory containing the certificates (OpenSSL only).

- The `IceSSL.KeyFile` property has been deprecated. You should instead use
  `IceSSL.CertFile` and PKCS12 files to configure the IceSSL identity.

- The default value of the `IceSSL.VerifyDepthMax` property is now 3 (it was
  previously 2). This allows certificate chains of 3 certificates (e.g., a Peer,
  CA and Root certificate chain).

- The certificate chain provided in the `IceSSL::ConnectionInfo` should now
  always include the root certificate if the chain could successfully be
  verified.

- Added `verified` member to the `IceSSL::ConnectionInfo` class. This member
  indicates whether or not the peer certificate was successfully verified. This
  member is useful for clients which set `IceSSL.VerifyPeer=0` to check if the
  server certificate could be verified or not. For server connections, the
  member should always be `true` since servers always reject invalid client
  certificates.

- The Ice distribution now supports the Objective-C mapping on OS X.

- The Glacier2 `SessionHelper` class now creates the callback object adapter
  automatically unless the application calls
  `SessionFactoryHelper.setUseCallbacks(false)`.

- Added the `headers` member to the `Ice::WSConnectionInfo` class. This member
  contains the HTTP headers provided during WebSocket connection establishment.

- Significantly changed the behavior of batch requests:

    - Invocations on batch fixed proxies are still queued by the Ice connection
      object as in previous releases.

    - Invocations on batch direct or indirect proxies are now queued directly
      by the proxy object.

    - As a result, calling `flushBatchRequests` on the Ice communicator or
      connection will no longer flush batch requests queued with proxies.

    - You should instead call `flushBatchRequests` on the proxy to flush its
      batch request queue. The flushing of those batch requests is handled like
      a regular one-way request (e.g., the Ice run time will retry if the
      connection fails).

- Added support for batch request interceptors (C++, Java, C#, Python,
  Objective-C). You can assign a batch request interceptor on communicator
  initialization using the `InitializationData` structure.

- IPv6 is now disabled by default if the operating system doesn't support the
  creation of IPv6 sockets.

- Added new `Ice.BatchAutoFlushSize` property to allow controlling how much data
  (in kilobytes) is queued for batching before the batch is sent over the
  connection. Previously, this was controlled by the `Ice.MessageSizeMax`
  property. The `Ice.BatchAutoFlush` property has also been deprecated. To
  disable auto flush you can set `Ice.BatchAutoFlushSize=0`. The default value
  is 1024.

- We have relaxed the semantics of `Ice.MessageSizeMax`. The check for the
  maximum message size is now only performed on the receiver side. If the
  size of the message is greater than the limit, Ice raises
  `Ice::MemoryLimitException` and closes the connection. Previously, it was also
  used to limit the amount of data sent to a peer (either for a request or
  reply). This is no longer the case. A zero or negative value now also disables
  the check and allows an unlimited amount of data to be received (this should
  only be used in a trusted environment). Finally, support for the
  `objectAdapter.MessageSizeMax` property was added to allow setting this limit
  on specific object adapters.

- The Ice run time now raises `Ice::IllegalServantException` if a null servant
  is provided to a method that requires a non-null servant (such as the
  `Ice::ObjectAdapter::add` methods or the Freeze evictor `add` methods).

- It's now possible to specify an IPv6 address for the UDP `--interface` local
  option; previously it only accepted an interface name or index.

- Ice now guarantees that invocations made on different proxy objects will be
  serialized if the proxies are equal and if connection caching is enabled.
  Previously, invocations could be sent out of order if the connection wasn't
  established.

- IceGrid now permits removing or moving servers when saving an application
  "without restart" (to ensure the update doesn't cause any server shutdown).
  The servers to be removed or moved must be stopped or the saving will fail.
  Previously, IceGrid prevented the saving of the application even if the
  servers were stopped.

- It's now possible to pass indirect proxies to
  `IceGrid::AdminSession::setObservers`. Previously, the IceGrid registry would
  fail to resolve the endpoints and the observer updates weren't sent.

- The IceGrid implementation of `Ice::Locator::getRegistry` now returns a
  replicated proxy if IceGrid replication is used. This way, if an IceGrid
  replica becomes unavailable during a server's activation, the server will
  still be able to register its endpoints with one of the other replicas.

- Added a `cancel` method to `AsyncResult`. Calling it prevents a queued
  asynchronous request from being sent, or ignores the reply if the request has
  already been sent. It does not cancel the request in the server. Canceled
  requests throw `Ice::InvocationCanceledException`.

- Removed the "old" asynchronous invocation API that was deprecated in Ice 3.4.

- Removed the following deprecated Slice operations:
    - `Ice::Router::addProxy`
    - `IceGrid::Admin::writeMessage`
    - `IceStorm::Topic::subscribe`

- Added the WebSocket transports to the Ice core, including new implementations
  in Java and C#. This means it's no longer necessary to use Glacier2 as an
  intermediary when a JavaScript client needs to communicate with a Java or C#
  server.

- Added a "Logger" facet to the admin object, which allows remote applications
  to attach `RemoteLogger` objects and receive log messages. Both the
  icegridadmin command-line utility and IceGrid Admin GUI use this new facet
  to retrieve the logs of Ice servers, IceBox services, IceGrid nodes and
  IceGrid registries.

- Added `getNodeAdmin` and `getRegistryAdmin` operations to the `IceGrid::Admin`
  interface. They return the admin object of a given IceGrid registry or node.

- Added new `getPrefix` operation to `Ice::Logger`; it returns the prefix of the
  logger.

- The admin object can now be hosted in an application-supplied object adapter
  using the new property `Ice.Admin.Enabled` and the new operation
  `Ice::Communicator::createAdmin`.

- Added methods to proxies to asynchronously get the connection associated with
  the proxy. The current synchronous methods can block as they may cause the
  connection to be established.

- Added support for HTTP proxies to the Ice core in C++, C# and Java. This
  allows outgoing TCP & SSL connections to be mediated by an HTTP network proxy
  service that supports HTTP CONNECT tunneling. The new properties
  `Ice.HTTPProxyHost` and `Ice.HTTPProxyPort` provide addressing information
  for the proxy.

- Added the `IceStorm::InvalidSubscriber` exception, which is thrown by the
  `subscribe/unsubscribe` operations when an invalid subscriber (such as a null
  proxy) is passed. IceStorm would previously either abort (if built in debug
  mode) or throw an `Ice::UnknownLocalException` if such a subscriber proxy was
  provided.

- Added `Ice.Default.SourceAddress` property and `--sourceAddress` endpoint
  setting to control the interface used for an outgoing connection. The
  configured source address can be retrieved with the `sourceAddress` member
  of the `Ice::IPEndpointInfo` class.

- The signatures of `Ice::OutputStream startSize` and `endSize` have been
  modified. It is now necessary for the caller of `startSize` to save the
  returned position and pass it into `endSize`.

- Added `IceStormAdmin.Host` and `IceStormAdmin.Port` properties. You can set
  these properties to connect to an IceStorm instance running on a given host
  and port.

- Added IceStorm finder interface (defined in `IceStorm/IceStorm.ice`) to enable
  clients to easily find the proxy of the topic manager with just the endpoint
  information of the IceStorm service. The client doesn't need to know the
  instance name. IceStorm provides an Ice object with the identity
  `IceStorm/Finder` that implements this interface.

- Added IceLocatorDiscovery plug-in, which enables Ice applications to discover
  locators (such as IceGrid registries) running on the network using UDP
  multicast.

- Added support for IceLocatorDiscovery to the IceGrid administrative utilities.
  Also added `IceGridAdmin.Host` and `IceGridAdmin.Port` properties; you can set
  these properties to connect to an IceGrid registry running on a given host and
  port.

- Added "finder" interfaces for locators and routers to enable clients to easily
  find proxies for the locator or router (defined in `Ice/Locator.ice` and
  `Ice/Router.ice`). Glacier2 and IceGrid implement these interfaces and provide
  finder Ice objects with the identities `Ice/RouterFinder` and
  `Ice/LocatorFinder`, respectively. Clients can use these objects to connect
  to Glacier2 and IceGrid using only the endpoint information; they don't need
  to know the instance name of the Glacier2 or IceGrid service.

- Ice metrics now support collocation optimization. The
  `Ice::Instrumentation::CollocatedObserver` local interface and the
  `IceMX::CollocatedMetrics` class have been added to monitor collocated
  requests.

- Collocation optimization has been significantly improved to remove
  limitations. It now works with AMI, AMD, blobjects and batching. Collocated
  requests are now always marshaled and unmarshaled but they don't go through
  any transport. The `Ice::CollocationOptimizationException` is no longer used
  and as a result is now deprecated.

- Fixed a bug where retries might fail if the request failed right after
  connection establishment.

- Added callbacks for connection closure and heartbeats. The callback is invoked
  when the connection is closed or when a heartbeat is received. Callbacks are
  installed on the connection with the `Ice::Connection::setCallback` method and
  implement the `Ice::ConnectionCallback` interface.

- Active connection management improvements:

    - Active connection management now supports sending heartbeats over the Ice
      connection. Depending on the configuration, heartbeats are sent when there
      are long invocations in progress, when the connection is idle, or at
      regular time intervals.

    - Connection closure by active connection management can be configured to
      close connections under certain conditions. For example, a connection can
      be closed gracefully or forcefully if it's idle for the specified timeout
      period.

    - The ACM properties to configure the ACM timeout have been deprecated. You
      should now use the `prefix.Timeout` property where prefix can be
      `Ice.ACM`, `Ice.ACM.Client`, `Ice.ACM.Server` or `objectAdapter.ACM`.

- IceGrid and Glacier2 now support the new ACM mechanism to keep sessions alive.
  The `Glacier2::Router` and `IceGrid::Registry` interfaces support a new
  `getACMTimeout` operation. To keep the connection alive the client should
  configure its connection to always send heartbeats using the timeout returned
  by `getACMTimeout`. A client can also install a connection callback to be
  notified when the connection is unexpectedly closed.

- Added support for invocation timeouts. Invocation timeouts can be set on a
  per-proxy basis with the `ice_invocationTimeout` proxy method. By default,
  there's no invocation timeout meaning that an invocation will never timeout
  if the connection between the client and server is alive. An invocation
  timeout does not cause a connection to be closed. If the timeout is triggered
  the invocation will raise `Ice::InvocationTimeoutException`.

- With the addition of invocation timeouts, the existing connection timeout
  mechanism no longer applies to invocations. Connection timeouts are now
  enabled by default using the new property `Ice.Default.Timeout`, which
  specifies the timeout for stringified proxy endpoints that do not have a
  timeout explicitly set. The default setting for the property is 60 seconds.
  It is also now possible to set an infinite timeout using `-t infinite` in
  the endpoint configuration.

- Added IceDiscovery plug-in, which provides a UDP multicast-based Ice locator
  implementation. This can be used as an alternative to IceGrid when running a
  few clients and servers on the same network and you don't want to deploy a
  central service such as IceGrid.

- Added support for IceGrid custom load balancing with the addition of replica
  group and type filters. These filters allow you to select the set of replicas
  that are sent back to the client when it resolves replica group endpoints.
  It's also possible to filter the objects returned to the client when it uses
  the `IceGrid::Query` interface.

- This addition requires the upgrade of IceGrid registry databases if you
  deployed replica groups for your IceGrid applications. See the release notes
  for information on how to upgrade your IceGrid databases.

- Fixed a marshaling bug where the marshaling of a type with nested optionals
  of variable size could trigger an assertion.

- Fixed an IceGrid registry replication bug where database synchronization could
  fail on startup or connection establishment with the master if multiple
  applications referred to a replica group defined in another application that
  wasn't yet added.

- FreezeScript can now transform classes to structures and structures to
  classes. It also supports the use of compact type IDs for classes.

- The `CryptPasswords` file support for Glacier2 and IceGrid is now implemented
  in a separate C++ plug-in, the `Glacier2CryptPermissionsVerifier` plug-in. On
  Windows, this plug-in is the only remaining Ice run time dependency on
  OpenSSL.

- Removed the `Ice::Stats` interface, which was deprecated in Ice 3.5 and
  replaced by the new metrics facility.

- The `IceBox.ServiceManager` object adapter has been deprecated. Use
  `Ice.Admin` and the `IceBox.ServiceManager` facet instead.

- Removed the following deprecated properties:
    - `Glacier2.AddSSLContext`
    - `IcePatch2.ChunkSize`
    - `IcePatch2.Directory`
    - `IcePatch2.Remove`
    - `IcePatch2.Thorough`

- Fixed a bug where optional class data members were not correctly marshaled and
  unmarshaled when non optional class data members were defined after the
  optional class data members or the optional tag were not in ascending order.
  This bugs affects Java, CSharp and JavaScript language mappings.

## C++ Changes

- Calling Glacier2::SessionFactoryHelper::destroy() is now required to ensure
  resources allocated for session helpers are correctly released.

- Fixed a bug where the object adapter default servant map wasn't cleared on
  object adapter destruction. This could potential lead to leaks if the default
  servant had a reference to the object adapter.

- IceStorm is now faster when creating persistent topics and subscribers.

- Ice for C++ now supports the GCC and clang symbol visibility options: we build
  by default with `-fvisibility=hidden` and we define `ICE_DECLSPEC_EXPORT` and
  `ICE_DECLSPEC_IMPORT` for these compilers.

- Refactored the capture and printing of exception stack traces to be more
  efficient when the stack traces are not printed. The signature of
  `IceUtil::Exception::ice_stackTrace` changed to:
  `std::string ice_stackTrace() const;`

- Fixed a bug where a private Windows Runtime delegate type could be published
  in Windows Store Components metadata files (WINMD).

- New garbage collection support for object graphs with cycles. Ice no longer
  uses a background thread to collect objects. Instead, `Ice::Object` now has a
  new `ice_collectable` method to indicate whether or not the object graph can
  be collected. When a graph is marked as collectable with this method, you
  shouldn't update the structure of the object graph. Ice tries to collect the
  graph each time a reference count is dropped for an object in the cycle. By
  default, object collection is turned off. When it's turned on with
  `Ice.CollectObjects=1`, the Ice run time calls `ice_collectable(true)` on all
  the objects it unmarshals. The properties `Ice.GC.Interval` and `Ice.Trace.GC`
  are no longer supported, nor is the `Ice::collectGarbage` function.

- Significant changes to the IceSSL plug-in:

    - Now uses the native SecureTransport API on OS X
    - Now uses the native SChannel API on Windows
    - OpenSSL is only used in IceSSL on Linux

- Added support for the `IceSSL.FindCert` property on Windows and OS X.

- Added the new metadata tag `cpp:view-type:Type`, where `Type` is a type that
  can safely provide a "view" into the Ice unmarshaling buffer and thereby avoid
  extra copies. This tag can only be applied to operation parameters of type
  string, sequence and dictionary, and only has an effect in situations where
  the Ice data buffer is guaranteed to still exist.

- It is no longer necessary to provide import libraries such as `IceD.lib`,
  `IceUtilD.lib` or `IceGridD.lib` when linking on Windows with Visual C++. The
  linker finds the import library names automatically through
  `pragma comment(lib, ...)` in the Ice header files.

- Ice header files no longer disable or downgrade any warning reported by the
  C++ compiler in your code. In Ice 3.5, all header files indirectly included
  the following:
  ```
  #ifdef _MSC_VER
  //
  // Move some warnings to level 4
  //
  # pragma warning( 4 : 4250 ) // ... : inherits ... via dominance
  # pragma warning( 4 : 4251 ) // class ... needs to have
  // dll-interface to be used by clients of class...
  #endif
  ```
  When upgrading your Visual C++ application to Ice 3.6, you may need to fix,
  disable or downgrade these warnings in your C++ code.

- Fixed a bug where communicator destruction could raise
  `Ice::NullHandleException` if a custom observer was present.

- Fixed a bug where the selector wouldn't check for EPOLLERR on Linux. This
  could cause the selector thread to spin if this condition occurred (which is
  the case with UDP connected sockets).

- Added per-process string converters for narrow and wide strings. The functions
  `setProcessStringConverter` and `getProcessStringConverter` in the IceUtil
  namespace can be used to establish and access the process narrow string
  converter. Likewise `setProcessWstringConverter` and
  `getProcessWstringConverter` can be used to establish and access the process
  wide string converter.

- Removed the `stringConverter` and `wstringConverter` data members of
  `Ice::InitializationData`.

- Moved the `StringConverter` template and classes from the Ice namespace to the
  IceUtil namespace.

- Added `Ice.LogStdErr.Convert` property which controls whether or not the
  default StdErr logger converts messages from native narrow encoding to the
  console code page. The default value is 1, meaning by default messages are
  converted to the console code page encoding. The property is only used on
  Windows.

- Added `ice_staticId` to the ProxyHandle template.

- Removed deprecated `Ice::Object::ice_getHash` function.

## C# Changes

- Fixed the Java IceSSL implementation to behave like the C++ and Java
  implementation when `IceSSL.VerifyPeer=1` is set for servers. The server will
  now request a certificate to the client and not abort the connection if the
  client doesn't provide a certificate.

- The default constructor for a Slice structure, exception or class now
  initializes string, structure or enumerator data members. A string data member
  is initialized to the empty string, a structure data member is initialized
  with a new default-constructed structure, and an enumerator is initialized
  with the first enumerator value.

- Fixed a bug in the Slice-to-C# compiler that would generate invalid code if an
  interface inherited a method that had an optional value as a return value.

- Deprecated the `clr:collection` Slice metadata for sequences and dictionaries
  as well as the `CollectionBase` and `DictionaryBase` classes.

- Fixed a security vulnerability in IceSSL in which an untrusted self-signed CA
  certificate was accepted.

- Added `IceSSL.CertAuthFile` property, which allows you to specify a CA
  certificate without having to import it into a certificate store.

- Deprecated the `IceSSL.ImportCert` property. Certificates should be imported
  using standard Windows tools.

- Fixed a bug where under certain circumstances the thread pool could serialize
  dispatch.

- The `Ice.Communicator` type now implements `IDisposable`, which allows
  communicator objects to be used as resources in `using()` statements.

- Added the metadata tag `clr:implements:Base`, which adds the specified base
  type to the generated code for a Slice structure, class or interface.

- Most Slice data types now support .NET serialization, which allows Ice
  exceptions to cross AppDomain boundaries. The only limitation is that data
  members whose types are proxies cannot be serialized; these members will be
  set to null after deserialization. Note that the `Ice.Optional` type now
  implements `ISerializable` and the .NET serialization format in Ice 3.6 is
  not backward-compatible with Ice 3.5.

- It is now possible for users to provide a base class for a Slice-generated
  class with .NET partial classes.

- Fixed leak in C# transport. The leak occurred for each Ice connection.

- Mono is no longer an officially-supported platform for Ice for .NET.

## Java Changes

- Fixed the Java IceSSL implementation to behave like the C++ and C#
  implementation when `IceSSL.VerifyPeer=1` is set for servers. The server will
  now reject the connection if the client provides a certificate and this
  certificate can't be verified.

- Passing null for a Slice structure or enumerator is now tolerated. Ice
  marshals an empty structure or the first enumerator value.

- The default constructor for a Slice structure, exception or class now
  initializes string, structure or enumerator data members. A string data member
  is initialized to the empty string, a structure data member is initialized
  with a new default-constructed structure, and an enumerator is initialized
  with the first enumerator value.

- The `clone` method provided by Java classes generated from Slice definitions
  is now returning the generated class, instead of `java.lang.Object`. For
  example, the `Ice.Identity` class generated from the `Ice::Identity` Slice
  structure contains:
  ```
  // Prior to Ice 3.6
  public java.lang.Object clone() { ... }

  // Starting with Ice 3.6
  public Identity clone() { ... }
  ```
  This way, you typically no longer need to cast after cloning an Ice structure or other Ice object.

- Added new Slice metadata `java:buffer` to enable the use of `java.nio.Buffer`
  types in the generated code for sequences of byte, short, int, long, float and
  double. For example, `sequence<byte>` maps to `java.nio.ByteBuffer` instead of
  `byte[]`.

- Changed the Ice core to allow applications to safely interrupt threads that
  are calling Ice APIs. The new property `Ice.ThreadInterruptSafe` must be
  enabled in order to use thread interrupts. Ice raises the new exception
  `Ice.OperationInterruptedException` if an API call is interrupted.

- Added support for Java 8.

- Java 8 lambda expressions can be used as callbacks for asynchronous proxy
  invocations.

- Removed deprecated `IceUtil.Version` class.

## JavaScript Changes

- JavaScript is now an officially-supported language mapping.

- Passing null for a Slice structure or enumerator is now tolerated. Ice
  marshals an empty structure or the first enumerator value.

- The default constructor for a Slice structure, exception or class now
  initializes string, structure or enumerator data members. A string data member
  is initialized to the empty string, a structure data member is initialized
  with a new default-constructed structure, and an enumerator is initialized
  with the first enumerator value.

- Added the following methods to `AsyncResult` for consistency with the other
  language mappings: `cancel`, `isCompleted`, `isSent`, `throwLocalException`,
  `sentSynchronously`.

- Changed the NodeJS packaging such that existing Ice for JavaScript programs
  will need to modify their require statements. For example, to use Ice you
  would now write:
  `var Ice = require("icejs").Ice;`

## Objective-C Changes

- The Objective-C headers are now installed under the `include/objc` directory.
  You will need to change the `#import` in your code to import `objc/Ice.h`
  where you were previously importing `Ice/Ice.h`. You will need to make similar
  changes as well if you include headers for Ice services:
  `IceStorm/IceStorm.h`, `IceGrid/IceGrid.h`, `Glacier2/Glacier2.h` should be
  changed to `objc/IceStorm.h`, `objc/IceGrid.h`, `objc/Glacier2.h`
  respectively.

- The IceSSL types are no longer included with the `IceObjC` library. You should
  now import `objc/IceSSL.h` in your source code and link with the `IceSSLObjC`
  library if you need to use IceSSL types (`ICESSLEndpointInfo` and
  `IceSSLConnectionInfo`).

- The Ice run time local interfaces (such as `ICECommunicator`,
  `ICEObjectAdapter`, etc.) are now generated from the Slice definitions.

- Added support for the `[objc:param:ID]` metadata to allow changing the
  generated identifier for the parameter in the generated interface selector for
  a Slice operation. For example, the Slice operation
  `void op(int a, ["objc:param:withB"] int b)` will be generated as the selector
  `op:(int)a withB:(int)b`.

- Added support for servant locators.

- The default constructor for a Slice structure, exception or class now
  initializes string, structure or enumerator data members. A string data member
  is initialized to the empty string, a structure data member is initialized
  with a new default-constructed structure, and an enumerator is initialized
  with the first enumerator value.

## PHP Changes

- Passing null for a Slice structure is now tolerated, an empty structure will
  be marshaled.

- Added support for PHP 5.6.

- Added `ice_staticId` to generated proxy helper classes.

- Generated interfaces now extend the `Object` base interface.

- Added `ObjectPrxHelper` class.

## Python Changes

- Passing `None` for a Slice structure is now tolerated, an empty structure will
  be marshaled.

- Added support for Python 3.4.

- Changed the semantics of `Ice.Unset` so that it evaluates as false in a
  boolean context.

- Fixed a bug in the Slice-to-Python translator that eliminates the use of
  implicit relative imports for nested Slice modules.

- Added `ice_staticId` to generated proxy classes.

## Ruby Changes

- Passing null for a Slice structure is now tolerated, an empty structure will
  be marshaled.

- Added support for Ruby 2.1.

- Added `ice_staticId` to generated proxy classes.
