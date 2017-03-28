The entries below contain brief descriptions of the changes in a release, in no
particular order. Some of the entries reflect significant new additions, while
others represent minor corrections. Although this list is not a comprehensive
report of every change we made in a release, it does provide details on the
changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your
applications to this release, and the manual for complete details on a
particular aspect of Ice.

- [Changes in Ice 3.3.1](#changes-in-ice-331)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [Java Changes](#java-changes)
  - [C# Changes](#c#-changes)
  - [Python Changes](#python-changes)
  - [Ruby Changes](#ruby-changes)
  - [PHP Changes](#php-changes)
- [Changes in Ice 3.3.0](#changes-in-ice-330)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-1)
  - [Java Changes](#java-changes-1)
  - [.NET Changes](#net-changes)
  - [Python Changes](#python-changes-1)
  - [Ruby Changes](#ruby-changes-1)
  - [PHP Changes](#php-changes-1)

# Changes in Ice 3.3.1

These are changes since Ice 3.3.0.

## General Changes

These entries apply to all relevant language mappings unless otherwise
noted.

- Fixed a bug where `ice_getCachedConnection` could throw if called
  shortly after connection establishment failed.

- Fixed an IceGrid bug where an application update could trigger the
  deactivation of all session-activated servers even if these servers
  were not updated.

- The frequency at which Ice checks for connections that can be reaped
  by active connection management (ACM) is now set by default to 10%
  of the smallest, non-zero ACM setting (`Ice.ACM.Client` and
  `Ice.ACM.Server`), with a minimum value of 5 seconds and a maximum
  value of 5 minutes. This default can be overridden by setting
  `Ice.MonitorConnections`, which is no longer deprecated, and whose
  value is the check frequency in seconds.

- Fixed a bug in IceBox where services using the shared communicator
  would be passed the wrong communicator if re-started with the IceBox
  service manager interface.

- Improved Glacier2 to automatically destroy the client session if the
  forwarding of a request from a back-end server to the client fails
  with an unrecoverable error. This is particularly useful for clients
  that receive requests from back-end servers on a regular basis and
  disable the Glacier2 session timeout. If for some reason the client
  becomes unreachable, Glacier2 will eventually destroy the client
  session (assuming timeouts are properly configured on the Glacier2
  client endpoints).

- Added support for updating locator cache endpoints in the background
  if the new `Ice.BackgroundLocatorCacheUpdates` property is set to 1.

  Locator cache updates are triggered if the proxy locator cache
  timeout is configured and the cached endpoints are older than the
  configured timeout.

  By default, locator cache updates are performed before making an
  invocation, delaying the invocation until the new endpoints are
  retrieved from the locator. With this feature enabled, the locator
  cache updates are performed in the background and the invocation
  that triggered the update uses the previously cached endpoints.

- Optimized the Ice client run time to minimize locator requests when
  multiple concurrent or AMI requests need to resolve the endpoints of
  the same well-known object, object adapter, or replica group.

- Fixed a bug in IceGrid that would cause it to generate incorrect
  configuration files for servers whose `ice-version` was set to a
  version previous to Ice 3.3.0.

- Fixed a bug in the Slice translators that prevented them from being
  run from a directory without write permissions, even if no output
  files were being written to the directory.

- Fixed a bug where retried AMI calls could cause the timer thread to
  print an error message if the retry failed and if `Ice.RetryIntervals`
  was configured with non-zero time intervals.

- All error output from the Slice compilers now goes to stderr.

- Made unmarshaling code robust against messages containing invalid
  indexes for class instances.

- Fixed IcePatch2 to handle non-ASCII file names properly.

- Fixed a bug in IceGrid that could cause the registry to fail with an
  assertion if an application is synchronized concurrently by multiple
  administrative clients (or by nodes started with the `--deploy`
  option).

- IceGrid locator implementation changes:

  - Fixed a bug where the registry could crash when resolving the
    endpoints of a replica group if some servers were activating
    concurrently.

  - Fixed a bug where the registry could hang when resolving the
    endpoints of a round-robin replica group if a member of the
    replica group was removed concurrently.

  - Improved the scalability of the locator implementation to better
    handle a high volume of concurrent requests. The registry now
    limits the number of invocations to the IceGrid nodes to get the
    object adapter endpoints.

- A Slice compiler now removes any files it has generated during the
  current execution if it fails or is interrupted.

- The Slice compilers that support the `--depend` option now use an exit
  status of 1 if a failure occurs while generating dependencies. In
  previous releases the compilers used an exit status of 0.

- Fixed a bug in the iceca script that prevented the command `init --overwrite`
  from working properly on Windows.

- Fixed a bug where the IceBox shared communicator ignored Ice
  communicator properties specified in an IceBox service configuration
  file.

- Fixed a bug where Ice plug-ins could be destroyed more than once.

- Fixed several issues in the IceGrid GUI.

## C++ Changes

- Fixed a bug in IceSSL plug-in which could cause a crash on
  communicator destruction.

- Changed the thread pool to ensure that references to AMI callback
  objects are released as soon as possible.

- Fixed bug in `slice2cpp` that caused bad code to be generated for
  classes and modules whose name was a C++ keyword.

- Fixed a bug in IceSSL where the `IceSSL.CheckCertName` property did
  not have any effect.

- Fixed `icegridadmin` `service list` command crash.

- Fixed a bug where `slice2cpp` and `slice2freeze` would generate absolute
  include paths if the command line option `-I.` was used.

- Fixed a Freeze transactional evictor bug that could cause the
  transaction of an asynchronous method dispatch to be rolled back if
  rollback on user exception was enabled.

- Fixed a bug that could cause the first invocation on a proxy to
  incorrectly be sent compressed or not compressed.

- Fixed a bug on Windows that could cause an interrupted application
  to spin for a while before terminating.

## Java Changes

- Slice types now implement `java.io.Serializable`.

- If a servant raises `OutOfMemoryError` while dispatching an operation,
  the Ice run time now traps it and treats it as a hard error, meaning
  Ice logs an error message and closes the connection on which the
  request was received.

- Added object serialization that allows native Java objects to be
  sent as operation parameters. See the Slice chapter and the Java
  language mapping chapter in the manual for more information.

- Fixed bug in `slice2java` that caused bad code to be generated for a
  class whose name was a Java keyword if the class contained data
  members.

- The path names specified by the `IceSSL.Keystore`, `IceSSL.Truststore`,
  and `IceSSL.Random` properties can now refer to class path resources.

- Added methods to the IceSSL plug-in to allow keystores, truststores,
  and RNG seeds to be specified using input streams.

- Fixed the `IceSSL.ConnectionInfo` type so that the "incoming" and
  `adapterName` members are public.

- Added support for using Ice in an applet, along with a new demo.

## C# Changes

- Added object serialization that allows native C# objects to be sent
  as operation parameters. See the Slice chapter and the C# language
  mapping chapter in the manual for more information.

- Changed the Ice run time to no longer rely on .NET worker threads
  to start asynchronous I/O operations.

- Fixed a bug in `slice2cs` that caused a core dump if a dictionary had
  a value type of Object.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  if an operation had a parameter named `ex`.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  if a Slice class used the property mapping and also applied
  a `["cs:attribute:..."]` metadata directive to a data member.

- Fixed an issue that could cause an application to fail to start if
  Ice was unable to load an assembly that was referenced (but not
  required) by the program and not present on the system.

- Fixed a bug in marshaling code that caused a trailing zero byte
  to be marshaled for user exceptions without class data members.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  for a dictionary that used a structure as the key type.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  for a sequence whose element type is a structure that uses the
  class mapping.

## Python Changes

- Implemented the `Ice.Trace.Slicing` property for user exceptions.

- It is now possible to pass a sequence of 64-bit integer values
  using an object that supports the buffer protocol.

- Fixed a bug in the constructors of user-defined types that have
  structures as data members. Consider this example:

  ```
  // Slice
  struct Point {
      int x;
      int y;
  };
  struct Event {
      Point location;
      ...
  };
  ```

  The Event constructor supplied a default instance of the Point
  member if the caller did not supply one. However, this default
  instance was unintentionally shared by all Event objects that
  omitted a value for the Point member during construction. The
  generated code now ensures that each instance of Event assigns a
  new instance of Point.

- Fixed bugs in `getImplicitContext` related to using it when an
  implicit context had not been set.

- Fixed bugs in `stringToProxy` and `propertyToProxy` related to null
  proxies.

- `Ice.getSliceDir()` now only searches for the `slice` directory in a
  standard Ice installation.

- Improved the way the build system locates the Python framework on
  macOS.

## Ruby Changes

- The translator now generates an `eql?` method for Slice structures.

- Implemented the `Ice.Trace.Slicing` property for user exceptions.

- Added helper `Ice.getSliceDir`, which locates the `slice` directory in
  a standard Ice installation.

## PHP Changes

- It is now possible to specify multiple Slice files in your profile
  configuration that have common include files. For example, if `A.ice`
  and `B.ice` both include `Common.ice`, you can now do the following:

  ```
  ice.slice=-I. A.ice B.ice
  ```

  In previous releases this would have caused a redefinition error,
  which you could work around by creating a single Slice file that
  included both `A.ice` and `B.ice`. This workaround is no longer
  necessary.

- Fixed bugs in the `ice_router` and `ice_locator` proxy methods.

# Changes in Ice 3.3.0

These are changes since Ice 3.2.1.

## General Changes

These entries apply to all relevant language mappings unless otherwise
noted.

- It is now possible to use UNC paths on Windows in the configuration
  of Ice services, such as in the value of the `IceGrid.Node.Data`
  property for an IceGrid node.

- Fixed bug in `slice2html` that caused incorrect hyperlinks to be
  generated if nested modules were used.

- Added support for non-blocking AMI and batch requests. Activities
  such as DNS lookups, endpoint resolution, connection establishment,
  and sending the request are now performed in the background if
  necessary.

- AMI requests now return a boolean to indicate if the request is
  sent synchronously. If not sent synchronously and the AMI callback
  implements the `Ice::AMISentCallback` interface, the Ice run time
  calls the `ice_sent()` method once the request is sent.

- AMI timeouts are no longer triggered by the connection monitor
  thread at regular time intervals. Instead, timeouts are now
  triggered on time just like synchronous invocations. As a result,
  the setting of the `Ice.MonitorConnections` is no longer useful and
  this property has been deprecated.

- Added the proxy methods `ice_flushBatchRequests()` and
  `ice_flushBatchRequests_async()` to flush the batch requests of the
  connection associated with the proxy. Like AMI requests, the
  `ice_flushBatchRequests_async()` method is guaranteed to not block.

- Added support for oneway AMI requests. The `ice_response()` method of
  the AMI callback is never called for oneway requests sent with AMI.
  Unlike regular oneway requests which might block until the request
  is passed to the TCP/IP stack, oneway AMI requests can't block.

- Removed the thread-per-connection (TPC) concurrency model.
  Applications that were using TPC for its ordering guarantees can use
  the new thread pool serialization feature.

- The Ice thread pool now supports a new serialization mode that
  serializes the processing of messages received over a connection.
  For example, this is useful when you want to ensure the orderly
  dispatching of oneway requests from a client even if the thread pool
  has multiple threads. Serialization is enabled by setting the thread
  pool's `Serialize` property to 1.

- An Ice server will now `abort()` if there are no more file descriptors
  available to accept incoming connections. Note that this behavior is
  subject to change in the final release.

- An IceGrid node is more conservative when removing the directory of
  a server. The node only removes the directory when the server is
  explicitly removed from a deployed application or if the directory
  contains only files and directories that were created by the node.

- IceGrid's round-robin load balancing policy better handles servers
  that are unreachable.

- Fixed an IceGrid bug where patching servers deployed with the
  attribute `application-distrib` set to false wouldn't work.

- Added an object adapter to the IceGrid registry. The adapter is
  named `IceGrid.Registry.AdminSessionManager` and is responsible for
  Glacier2 administrative sessions.

- When resolving the endpoints of a replica group, the IceGrid locator
  no longer waits for an object adapter to complete its activation if
  another adapter is already active.

- Fixed an IceGrid descriptor bug where re-opening an unnamed property
  set would cause multiple property definitions.

- Added a fix to prevent the IceGrid node from printing an annoying
  thread pool size warning on startup.

- Fixed IceGrid bug where updates to IceBox services were not
  correctly sent to the IceGrid GUI or to IceGrid registry slaves.

- The IceGrid node now unblocks the `SIGHUP`, `SIGINT` and `SIGTERM` signals
  from forked servers.

- The IceGrid node now prints a warning if it can't reach the IceGrid
  registry when it starts. This warning can be disabled with `--nowarn`.

- An IceBox service can now recursively start and stop other IceBox
  services from within its start and stop methods. The level of
  recursion is limited to the threads in the administrative object
  adapter's thread pool.

- Configuration properties for IceBox services can now be defined on
  the command line.

- IceBox services that share a communicator now use a dedicated
  communicator instance that is not used by the IceBox server. This
  dedicated communicator only inherits properties from the server's
  communicator if `IceBox.InheritProperties` is set to a non-zero value.

- It is no longer necessary to define a value for the property
  `IceBox.ServiceManager.Endpoints`. If this property is not defined,
  the `ServiceManager` interface is not available.

- Changes related to Highly-Available IceStorm

  - IceStorm now supports master/slave replication with automatic
    failover.

  - IceStorm also supports a transient mode that uses no databases.
    This mode is enabled using the new property `<service>.Transient.Replication`
    is not supported in this mode.

  - Subscriptions are now persistent (except in transient mode).

  - Added a new Quality of Service (QoS) parameter, `retryCount`, to
    control when subscribers are removed. IceStorm automatically
    removes a subscriber after the specified number of unsuccessful
    event deliveries. The default value of `retryCount` is 0, meaning
    the subscriber is removed immediately upon any failure. A
    subscriber is always removed on a hard failure, which is defined
    as the occurrence of `ObjectNotExistException` or
    `NotRegisteredException`.

  - The IceStorm database format has changed. Use the utility
    icestormmigrate to migrate from the database formats of Ice 3.1.1
    and Ice 3.2.1. The old migration script (`updateicestorm.py`) has
    been removed.

  - Added the new C++ example `demo/IceStorm/replication2`, which
    demonstrates how to manually configure IceStorm replication.

  - Added the `replica` command to `icestormadmin` to get debugging
    information on IceStorm replication.

  - The `TopicManager` object adapter now uses its own thread pool in
    the replicated case to ensure that ordering is guaranteed by
    default even if a publisher uses a oneway proxy.

  - Upon shutdown IceStorm now guarantees that all received events
    are delivered to all subscribers.

  - Added `Topic::getNonReplicatedPublisher`, which always return a
    non-replicated publisher proxy.

  - All IceStorm properties must be prefixed with the service name.
    For example, consider the following IceBox service configuration:

    `IceBox.Service.Foo=IceStormService,33:createIceStorm ...`

    In this case the IceStorm configuration properties must use the
    `Foo` prefix, such as

    `Foo.Discard.Interval=10`

  - Removed the `icestormadmin` Slice checksum check. This avoids
    warnings when using `icestormadmin` against an older version of
    IceStorm.

- It is now possible to specify properties in a config file with
  `space`, `#` or `=` in either the key or value string. It is necessary
  to escape `#` and `=` as `\#` and `\=`. Leading or trailing spaces
  are only allowed in keys and need to be escaped as `\ `.

- Properties set programmatically can no longer have leading or
  trailing whitespace in the key.

- There is no longer a limit on the allowable length of a property
  line in an Ice config file. Previously the limit was 1024.

- The new property `Ice.Warn.UnusedProperties` causes the communicator
  to display a warning during its destruction that lists all
  properties that were set but whose values were never read.

- The way that the Ice run time determines whether a proxy invocation
  is eligible for collocation optimization has changed somewhat. The
  Ice run time no longer performs a DNS lookup; instead, invocations
  on a direct proxy will use the optimization only if the host and
  port of one of its endpoints match the host and port of an endpoint
  or published endpoint of an object adapter from the same
  communicator.

- Exceptions thrown from collocation-optimized invocations are now
  fully transparent. If an operation throws an exception that is not
  in the operation's exception specification, or throws a non-Ice
  exception, the client receives `UnknownUserException` or
  `UnknownException`, exactly as if the servant for a remote invocation
  had thrown the same exception. (In earlier versions, the client
  received the original exception, rather than an unknown exception.)

  For Ice run-time exceptions, all run-time exceptions are passed
  to the client as `UnknownLocalException`, except for

  - `ObjectNotExistException`
  - `FacetNotExistException`
  - `OperationNotExistException`
  - `OperationNotExistException`
  - `UnknownException`
  - `UnknownLocalException`
  - `UnknownUserException`
  - `CollocationOptimizationException`

- The property `Ice.Default.CollocationOptimization` and the proxy
  property `<proxy>.CollocationOptimization` have been deprecated and
  replaced by `Ice.Default.CollocationOptimized` and
  `<proxy>.CollocationOptimized`, respectively.

- Most proxy factory methods now return a proxy of the same type as
  the original and no longer require the use of a checked or unchecked
  cast. For example, in C++ you can write

  ```
  HelloPrx hello = ...;
  hello = hello->ice_oneway();
  ```

  Previously you would have needed a cast, such as

  ```
  hello = HelloPrx::uncheckedCast(hello->ice_oneway());
  ```

  In Java and .NET, you must use a type cast:

  ```
  hello = (HelloPrx)hello.ice_oneway();
  ```

- If a proxy contains a host that is multihomed, the client will now
  try all the available IP addresses. Previously, only the first in
  the address list returned by the DNS was used and others were
  ignored.

- It's now possible to change the compression setting for a fixed
  proxy. In previous releases, calling `ice_compress` on a fixed proxy
  would raise `Ice::FixedProxyException`.

- Added a new `skipEncapsulation` method to the `Ice::InputStream`
  interface. This method can be used to skip an encapsulation when
  reading from a stream.

- The `endEncapsulation` method from the `Ice::InputStream` interface will
  now throw `Ice::EncapsulationException` if not all the data from the
  encapsulation has been read.

- The marshaling code now throws an exception if an enumerator that is
  out of range for its enumeration is sent or received.

- Network tracing now also includes failures to connect if
  the `Ice.Trace.Network` property is set to 2.

- The property `Ice.Trace.Location` has been deprecated and replaced
  by `Ice.Trace.Locator`.

- The Ice locator tracing (enabled with the `Ice.Trace.Locator`
  property) now traces calls to locator registry when an object
  adapter updates its endpoints or the server process proxy is
  registered.

- The new property `Ice.TCP.Backlog` allows you to specify the size of
  incoming connection backlog for TCP/IP sockets. This setting is
  also used for SSL. In C++ the default value is `SOMAXCONN` or 511 if
  that macro is not defined. In Java and .NET the default value is
  511.

- Added support for IPv6. It is disabled by default but can be enabled
  using the configuration property `Ice.IPv6`. The property `Ice.IPv4` can
  be used to disable IPv4 support.

- Added support for UDP multicast.

- A new object adapter property, `<adapter>.ProxyOptions`, lets you
  customize the proxies that the adapter creates.

- Added a new operation to the `ObjectAdapter` interface named
  `refreshPublishedEndpoints()`. This operation allows you to update an
  adapter's published endpoints after a change in the available local
  interfaces or after a change to the `PublishedEndpoints` property.

- The `-h *` endpoint option is now only valid for object adapter
  endpoints. It can no longer be used in proxy endpoints or object
  adapter published endpoints.

- Ice now listens on `INADDR_ANY` for object adapter endpoints that do
  not contain a host name (or are set to use `-h *` or `-h 0.0.0.0`).
  Previously, Ice would only listen on the local interfaces that were
  present when the adapter was created.

- Accepting incoming connections can no longer block a thread from a
  server thread pool. This includes activities such as connection
  validation and SSL handshaking.

- Changed servant locators so both `locate()` and `finished()` can throw
  user exceptions.

- Improved Glacier2 to take advantage of the new non-blocking aspects
  of Ice. Glacier2 now uses the thread pool concurrency model and, in
  buffered mode, Glacier2 only uses a single thread to forward queued
  requests. As a result, Glacier2 now requires a fixed number of
  threads regardless of the number of connected clients.

- Glacier2 filters are now disabled by default for IceGrid client
  and administrative sessions created with the IceGrid session
  managers. If you rely on these filters being enabled, you must now
  explicitly set the property `IceGrid.Registry.SessionFilters` or
  `IceGrid.Registry.AdminSessionFilters`.

- The use of sequences (and structs containing sequences) as valid
  dictionary keys has been deprecated and will be removed in a future
  release.

- `slice2vb` has been removed from the distribution and is no longer
  supported. To use Visual Basic .NET with Ice, use `slice2cs` to
  generate C# code from Slice definitions, and then use a C# compiler
  to create a DLL from the generated code. Then link against that DLL
  in your Visual Basic project.

- Fixed a variety of bugs with `slice2cpp` where incorrect code
  would be generated when C++ reserved words are used as Slice
  identifiers.

- If a class, interface, or exception has a base, `slice2html` now
  generates a hyperlink to the base. (Previously, only the name of the
  base was shown, without a hyperlink.)

- `slice2html` now generates documentation for Slice constant
  definitions.

- `slice2html` now generates the scoped name for the `<h1>` element of
  each page instead of the unqualified name. For example, it now shows
  `Ice::DNSException` instead of `DNSException`.

- The ability to install a logger via the `Ice.LoggerPlugin` property
  has been removed. The new mechanism requires that you create an
  instance of the `Ice::LoggerPlugin` class in your plug-in factory.
  Please see the manual for more details.

- It is no longer possible to provide input files on the command line
  for `icestormadmin` or `icegridadmin`.

- Freeze evictor update:

  - the existing evictor was renamed `BackgroundSaveEvictor`
  - added new `TransactionalEvictor`

- Fixed a bug in FreezeScript that caused a failure when a script
  attempted to access the `length` member of a string value.

- Added support for string concatenation in FreezeScript using the
  `+` operator.

- `dumpdb` output now goes to stdout.

## C++ Changes

- Fixed `icepatch2server` crash on exit bug.

- Fixed race condition in `IceUtil::Cache`.

- Changed the string methods in `InputStream` and `OutputStream` to accept
  an optional boolean argument. If true (the default), strings are
  processed by the string converter (if any) before marshaling or
  unmarshaling. If false, the string converter is bypassed. No source
  code changes are necessary for existing programs.

- The new `Ice::StringConverterPlugin` class allows you to install a
  custom string converter and/or wstring converter during communicator
  initialization.

- The Ice shared library now provides a string converter plug-in
  factory: `createStringConverter`. See the Ice manual for details.

- `IceUtil::stringToWstring` and `IceUtil::wstringToString` now take
  a second argument of type `ConversionFlags`. It can be either
  `strictConversion` or `lenientConversion`. The default is
  `lenientConversion` for compatibility with previous releases. See
  the Ice manual for further details.

- Added IceGrid secure demo to demonstrate how to setup a secure
  IceGrid deployment.

- Fixed a Glacier2 assert that would occur if the configuration
  defined only the SSL permissions verifier.

- The `IceUtil::Timer` destroy method no longer joins with the timer
  execution thread if it's called from a timer task. Instead, the
  thread is detached.

- The `IceUtil::Timer` `schedule` and `scheduleRepeated` methods now throw
  `IceUtil::IllegalArgumentException` if the timer is destroyed.

- Fixed potential Slice parser assert.

- Added an HA IceStorm template to `config/templates.xml`.

- Removed the index parameter from the IceStorm template.

- It is now possible to manually configure the node proxies when
  using HA IceStorm in conjunction with IceGrid.

- iceca and associated `ImportKey.class` are now copied into bin and lib
  respectively.

- iceca did not correctly support the `--verbose` flag with the import
  command.

- iceca request has been simplified. The command now takes the name of
  the file to generate, the common name, and an optional email
  address.

- The iceca `ImportKey.class` utility is now installed in
  `/usr/share/Ice-3.3.0` for an RPM install or `${prefix}/lib` otherwise.

- The following APIs were deprecated in Ice 3.1 and have been removed:

  - `Application::main(int, char*[], const char*, const Ice::LoggerPtr&)`
  - `initializeWithLogger`
  - `initializeWithProperties`
  - `initializeWithPropertiesAndLogger`
  - `stringToIdentity`
  - `identityToString`
  - `ObjectPrx::ice_hash`
  - `ObjectPrx::ice_communicator`
  - `ObjectPrx::ice_newIdentity`
  - `ObjectPrx::ice_newContext`
  - `ObjectPrx::ice_newFacet`
  - `ObjectPrx::ice_newAdapterId`
  - `ObjectPrx::ice_newEndpoints`
  - `ObjectPrx::ice_collocationOptimization`
  - `ObjectPrx::ice_connection`

- Added `public` keyword to the definition of `IceSSL::PublicKey`.

- Fixed a bug in `slice2freeze` in which it was not properly handling
  the `cpp:type:wstring` metadata, which would result in narrow
  strings being used in generated code when wstring should have been
  used instead.

- `IceInternal::GCShared` now derives from `IceUtil::Shared`. This now
  makes it possible to have a servant class derived from
  `IceUtil::Thread` (which, previously, was impossible).

- The thread stack size specified with the `IceUtil::Thread::start()`
  method parameter is now adjusted to `PTHREAD_STACK_MIN` if it's
  inferior to it. On macOS, it's also adjusted to the next closest
  multiple of the page size (4KB).

- Added a new overloading of the Ice::createInputStream method:

  ```
  Ice::InputStreamPtr createInputStream(
      const CommunicatorPtr&,
      const pair< const Byte*, const Byte*>&);
  ```

- Fixed a bug in the encoding of user exceptions with a class
  attribute and returned by AMD-dispatched invocations. An extra byte
  was encoded at the end. This fix doesn't affect on the wire
  compatibility with old Ice clients.

- Added `what()` member to `IceUtil::Exception`, so a single catch handler
  can be used for Ice exceptions:

  ```
  try
  {
      // Call something that may throw an Ice exception
  }
  catch(const ::std::exception& ex)
  {
      cerr << ex.what() << endl; // Works for any ::std::exception,
                                 // including Ice exceptions
  }
  ```

  `what()` is implemented in terms of `ice_print()`, so overriding
  `ice_print()` also changes the string returned by `what()`.

## Java Changes

- Fixed the `slice2java` ant task to redirect stderr output so that
  messages from the preprocessor are not interpreted as dependencies.

- Fixed the `slice2java` ant task to recompile all of the files in the
  file list if at least one of the files has changed and checksums are
  being generated.

- Deprecated the class `IceUtil.Version`.

- Added the methods `stringVersion` and `intVersion` to `Ice.Util` for
  obtaining the Ice version.

- Fixed a `NullPointerException` thrown when allocating an `InputStream`
  with an empty buffer.

- Fixed a `NullPointerException` thrown when calling `startEncapsulation`
  on an `OutputStream`.

- The following APIs were deprecated in Ice 3.1 and have been removed:

  - `Application.main(String, String[], String, Logger)`
  - `Util.initializeWithLogger`
  - `Util.initializeWithProperties`
  - `Util.initializeWithPropertiesAndLogger`
  - `ObjectPrx.ice_hash`
  - `ObjectPrx.ice_communicator`
  - `ObjectPrx.ice_newIdentity`
  - `ObjectPrx.ice_newContext`
  - `ObjectPrx.ice_newFacet`
  - `ObjectPrx.ice_newAdapterId`
  - `ObjectPrx.ice_newEndpoints`
  - `ObjectPrx.ice_collocationOptimization`
  - `ObjectPrx.ice_connection`

- Ice for Java now supports the `ICE_CONFIG` environment variable.

- The Java2 language mapping is now deprecated.

- Added support for a password callback in IceSSL. Also added new
  properties for specifying the name of a password callback class or
  certificate verifier class.

- `LocalObject` is now mapped to `java.lang.Object`; an `out LocalObject`
  parameter is still mapped to an `Ice.LocalObjectHolder` object.
  `Ice.LocalObjectHolder` now holds a `java.lang.Object`. The interface
  `Ice.LocalObject` and the abstract class `Ice.LocalObjectImpl` are now
  deprecated.

- `Ice.LocalException` and `Ice.UserException` now implement
  `java.lang.Cloneable`.

- Fixed the generated hashCode method to prevent `NullPointerException`.

## .NET Changes

- Added the methods `stringVersion` and `intVersion` to `Ice.Util` for
  obtaining the Ice version.

- Fixed a `NullReferenceException` thrown when allocating an `InputStream`
  with an empty buffer.

- Fixed a `NullReferenceException` thrown when calling
  `startEncapsulation` on an `OutputStream`.

- Changed `Ice.Exception` to derive from `System.Exception` rather than
  `System.ApplicationException` in accordance with Microsoft's
  recommendations for writing custom exceptions.

- The following APIs were deprecated in Ice 3.1 and have been removed:

  - `Application.main(string, string[], string, Logger)`
  - `Util.initializeWithLogger`
  - `Util.initializeWithProperties`
  - `Util.initializeWithPropertiesAndLogger`
  - `ObjectPrx.ice_hash`
  - `ObjectPrx.ice_communicator`
  - `ObjectPrx.ice_newIdentity`
  - `ObjectPrx.ice_newContext`
  - `ObjectPrx.ice_newFacet`
  - `ObjectPrx.ice_newAdapterId`
  - `ObjectPrx.ice_newEndpoints`
  - `ObjectPrx.ice_collocationOptimization`
  - `ObjectPrx.ice_connection`

- The `Ice.Application` class now supports signal handling on Mono.

- Added the build macro `MANAGED`, which can be enabled if you want to
  build a version of the Ice run time that uses only managed code.

- Improved marshaling performance for Ice for .NET. This particularly
  affects programs running under Mono on big-endian machines, where
  marshaling performance of sequences improves by up to a factor of
  10. For little-endian machines (both Windows and Mono), marshaling
  performance is also improved.

- Added `ice_name()` method to exceptions, in keeping with the C++
  and Java convention. The method returns the Slice type ID of
  exception, without a leading `::` qualifier.

- Fixed a bug that caused a `NullPointer` exception for sequences
  of structs if the structs used the class mapping and the sequence
  contained null elements.

- Fixed a bug that caused a `NullPointer` exception when marshaling
  a struct containing reference types if that was nested inside
  a data type, such as another exception or struct.

- Added support for a password callback in IceSSL. Also added new
  properties for specifying the name of a password callback class or
  certificate verifier class. See manual for more details.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  for data members of classes and structures that used the
  `["clr:property"]` metadata directive.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated
  for Slice exceptions with data members that had the same name as one
  of the data members of `System.ApplicationException`.

- The metadata directive `cs:collection` is no longer valid.
  Use `["clr:collection"]` instead.

- Added a new mapping for Slice sequences to use C# 2.0
  generics:

  ```
  // Slice
  ["clr:generic:List"] sequence<int> S;
  ```

  maps to:

  ```
  // C#
  System.Collections.Generic.List<int> S;
  ```

  This also works for types other than `List`. Please see the manual for
  more details.

- Added a new mapping for Slice dictionaries to use C# 2.0
  generics:

  ```
  // Slice
  dictionary<string, int> D;
  ```

  maps to:

  ```
  // C#
  System.Collections.Generic.Dictionary<string, int> D;
  ```

  The previous mapping to the .NET 1.1 `DictionaryBase` class is still
  supported with the `clr:collection` metadata directive.

- `LocalObject` is now mapped to `System.Object`. The interface
  `Ice.LocalObject` and the abstract class `Ice.LocalObjectImpl` are now
  deprecated.

- The `Ice.Util.proxyIdentityCompare` and `proxyIdentityAndFacetCompare`
  methods are now using `string.CompareOrdinal` to compare the identity
  category, name and the facet instead of string.Compare with the
  `CultureInfo.InvariantCulture` culture-specific information.

- `Ice.LocalException` and `Ice.UserException` now implement
  `System.ICloneable`.

- Ice for C# has been renamed Ice for .NET and the assemblies have
  been renamed as well. For example `icecs.dll` is now just `Ice.dll`.

## Python Changes

- Fixed a bug that occurred when converting an `Ice.Identity` object to
  a string via `str()`. In prior releases, this conversion used the
  global method Ice.identityToString, but that method is no longer
  supported. As a result, the output now resembles that of other
  user-defined types.

- Added the methods `Ice.stringVersion` and `Ice.intVersion`.

- Unicode objects are now accepted as arguments (or data members of
  arguments) to remote Slice operations. The local Ice API still uses
  8-bit strings.

- Added new `Ice/converter` demo to illustrate how to use the new Ice
  for C++ string converter plug-in. This demo is very similar to the
  C++ demo.

- Fixed a deadlock bug that could occur when a multithreaded client
  executes Slice operations that return abstract objects by value.

- Removed `#!/usr/bin/env python` where unnecessary.

- Added `ice_staticId()` method to generated Slice classes.

- The following APIs were deprecated in Ice 3.1 and have been removed:

  - `initializeWithLogger`
  - `initializeWithProperties`
  - `initializeWithPropertiesAndLogger`
  - `identityToString`
  - `stringToIdentity`
  - `ObjectPrx.ice_communicator`
  - `ObjectPrx.ice_newIdentity`
  - `ObjectPrx.ice_newContext`
  - `ObjectPrx.ice_newFacet`
  - `ObjectPrx.ice_newAdapterId`
  - `ObjectPrx.ice_newEndpoints`
  - `ObjectPrx.ice_connection`

- Fixed a bug where returning an AMD servant from a `ServantLocator`
  would cause a deadlock in the IcePy extension.

- Added support for blobjects.

- Fixed a code generation bug with `slice2py` where the proxy type
  for a Python reserved word would be incorrectly named. For example,
  interface def would generate a proxy class named `_defPrx`, not the
  correct class name `defPrx`.

- `LocalObject` is now mapped to the Python base `object` type. The
  class `Ice.LocalObject` is now deprecated.

- Fixed a bug in `Ice.initialize` that would cause a crash if the
  configuration file specified by `--Ice.Config` was not found.

## Ruby Changes

- Added the methods `Ice::stringVersion` and `Ice::intVersion`.

- Removed `#!/usr/bin/env ruby` where unnecessary.

- Added `ice_staticId()` method to generated Slice classes.

- Fixed a code generation bug with `slice2rb` where the reserved words
  `BEGIN` and `END` would generate classes, modules and constants named
  `_BEGIN`, and `_END` which is illegal. They now generate `BEGIN_` and
  `END_`.

- Fixed a bug where marshaling a plain object proxy fails with an
  uninitialized constant exception.

## PHP Changes

- Constructors are now generated for Slice structure, exception, and
  class types. Constructor parameters supply default values for each
  data member.

- A null value is now allowed where a sequence or dictionary value is
  expected.

- Added the methods `Ice_stringVersion` and `Ice_intVersion`.

- Removed the undocumented PHP constants `ICE_STRING_VERSION` and
  `ICE_INT_VERSION`.

- LocalObject is now mapped to PHP's base `object` type. The interface
  `Ice.LocalObject` and the abstract class `Ice.LocalObjectImpl` are now
  deprecated.

- Added support for an optional context argument to `ice_checkedCast`
  and `ice_uncheckedCast`.

- Added the following proxy methods:

  - `ice_getRouter/ice_router`
  - `ice_getLocator/ice_locator`

- Added the translator option `-w` to suppress warnings during Slice
  translation.
