- [Changes in Ice 3.6.0](#changes-in-ice-360)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [Java Changes](#java-changes)
  - [C# Changes](#c#-changes)
  - [Python Changes](#python-changes)
  - [Ruby Changes](#ruby-changes)
  - [JavaScript Changes](#javascript-changes)
  - [PHP Changes](#php-changes)
  - [Objective-C Changes](#objective-c-changes)
- [Changes in Ice 3.6 beta](#changes-in-ice-36-beta)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-1)
  - [Java Changes](#java-changes-1)
  - [C# Changes](#c#-changes-1)
  - [Python Changes](#python-changes-1)
  - [Ruby Changes](#ruby-changes-1)
  - [PHP Changes](#php-changes-1)
  - [JavaScript Changes](#javascript-changes-1)

The entries below contain brief descriptions of the changes in a release, in no particular order. Some of the entries reflect significant new additions, while others represent minor corrections. Although this list is not a comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your applications to this release, and the manual for complete details on a particular aspect of Ice.

# Changes in Ice 3.6.0

These are changes since Ice 3.6 beta.

## General Changes

* The Ice distribution now supports the Objective-C mapping on OS X.

* The Glacier2 `SessionHelper` class now creates the callback object adapter automatically unless the application calls `SessionFactoryHelper.setUseCallbacks(false)`.

* The IceGridDiscovery plug-in has been renamed. It's now named IceLocatorDiscovery and it can be used to discover locator implementations in general, not just the IceGrid locator implementations. IceGridDiscovery properties now use the IceLocatorDiscovery prefix. The plug-in entry points have changed as follows:

    * C++: `IceLocatorDiscovery:createIceLocatorDiscovery`

    * Java and .NET: `IceLocatorDiscovery:IceLocatorDiscovery.PluginFactory`

* Added support for headers attribute to the `Ice::WSConnectionInfo` class. This attribute gives access to the HTTP headers provided on WS connection establishment.

* Significantly changed the behavior of batch requests:

    * Invocations on batch fixed proxies are still queued with the Ice connection object as in previous releases.

    * Invocations on batch direct or indirect proxies are now queued directly with the proxy object.

    * As a result, calling `flushBatchRequests` on the Ice communicator or connection will no longer flush batch requests queued with proxies.

    * You should instead call `flushBatchRequests` on the proxy to flush its batch request queue. The flushing of those batch requests is handled like a regular one-way request (the Ice run time will for instance retry the sending if the connection fails).

* Added support for batch request interceptors (C++, Java, C#, Python, Objective-C). You can assign a batch request interceptor on communicator initialization using the Ice `InitializationData` structure. See the Ice manual for more information on batch request interceptors.

## C++ Changes

* IceStorm is now faster when creating persistent topics and subscribers.

## Java Changes

* Passing null for a Slice structure or enumerator is now tolerated. Ice marshals an empty structure or the first enumerator value.

* The default constructor for a Slice structure, exception or class now initializes string, structure or enumerator data members. A string data member is initialized to the empty string, a structure data member is initialized with a new default-constructed structure, and an enumerator is initialized with the first enumerator value.

* The `clone` method provided by Java classes generated from Slice definitions is now returning the generated class, instead of `java.lang.Object`. For example, the Ice.Identity class generated from the `Ice::Identity` Slice struct contains:
  ```
// Prior to Ice 3.6
public java.lang.Object clone() { ... }

// Starting with Ice 3.6
public Identity clone() { ... }
  ```
  This way, you typically no longer need to cast after cloning an Ice structure or other Ice object.

## C# Changes

* The default constructor for a Slice structure, exception or class now initializes string, structure or enumerator data members. A string data member is initialized to the empty string, a structure data member is initialized with a new default-constructed structure, and an enumerator is initialized with the first enumerator value.

## Python Changes

* Passing None for a Slice structure is now tolerated, an empty structure will be marshaled.

## Ruby Changes

* Passing null for a Slice structure is now tolerated, an empty structure will be marshaled.

## JavaScript Changes

* Passing null for a Slice structure or enumerator is now tolerated. Ice marshals an empty structure or the first enumerator value.

* The default constructor for a Slice structure, exception or class now initializes string, structure or enumerator data members. A string data member is initialized to the empty string, a structure data member is initialized with a new default-constructed structure, and an enumerator is initialized with the first enumerator value.

## PHP Changes

* Passing null for a Slice structure is now tolerated, an empty structure will be marshaled.

## Objective-C Changes

* The Objective-C headers are now installed under the `include/objc` directory. You will need to change the `#import` in your code to import `objc/Ice.h` where you were previously importing `Ice/Ice.h`. You will need to make similar changes as well if you include headers for Ice services: `IceStorm/IceStorm.h`, `IceGrid/IceGrid.h`, `Glacier2/Glacier2.h` should be changed to `objc/IceStorm.h`, `objc/IceGrid.h`, `objc/Glacier2.h` respectively.

* The IceSSL types are no longer included with the `IceObjC` library. You should now import `objc/IceSSL.h` in your source code and link with the `IceSSLObjC` library if you need to use IceSSL types (`ICESSLEndpointInfo` and `IceSSLConnectionInfo`).

* The Ice run time local interfaces (such as `ICECommunicator`, `ICEObjectAdapter`, etc) are now generated from the Slice definitions.

* Added support for the `[objc:param:ID]` metadata to allow changing the generated identifier for the parameter in the generated interface selector for a Slice operation. For example, the Slice operation `"void op(int a, ["objc:param:withB"] int b)"` will be generated as the selector `"op:(int)a withB:(int)b"`.

* Added support for Servant Locators.

* The default constructor for a Slice structure, exception or class now initializes string, structure or enumerator data members. A string data member is initialized to the empty string, a structure data member is initialized with a new default-constructed structure, and an enumerator is initialized with the first enumerator value.

# Changes in Ice 3.6 beta

These are the changes since Ice 3.5.1.

## General Changes

* IPv6 is now disabled by default if the operating system doesn't support the creation of IPv6 sockets.

* Added new `Ice.BatchAutoFlushSize` property to allow controlling how much data (in kilobytes) is queued for batching before the batch is sent over the connection. Previously, this was controlled by the `Ice.MessageSizeMax` property. The `Ice.BatchAutoFlush` property has also been deprecated. To disable auto flush you can set `Ice.BatchAutoFlushSize=0`. The default value is 1024.

* We have relaxed the semantics of `Ice.MessageSizeMax`. The check for the maximum message size is now only performed on the receiver side. If the size of the message is greater than the limit, Ice raises `Ice::MemoryLimitException` and closes the connection. Previously, it was also used to limit the amount of data sent to a peer (either for a request or reply). This is no longer the case. A null or negative value now also disables the check and allows an unlimited amount of data to be received. This should only be used in a trusted environment. Finally, support for the `objectAdapter.MessageSizeMax` property was added to allow setting this limit on specific object adapters.

* The Ice run time now raises `Ice::IllegalServantException` if a null servant is provided to a method that requires a non-null servant (such as the `Ice::ObjectAdapter::add` methods or the Freeze evictor add methods).

* It's now possible to specify an IPv6 address for the UDP `--interface` local option; previously it only accepted an interface name or index.

* Ice now guarantees that invocations made on different proxy objects will be serialized if the proxies are equal and if connection caching is enabled. Previously, invocations could be sent out of order if the connection wasn't established.

* IceGrid now permits removing or moving servers when saving an application "without restart" (to ensure the update doesn't cause any server shutdown). The servers to be removed or moved must be stopped or the saving will fail. Previously, IceGrid prevented the saving of the application even if the servers were stopped.

* It's now possible to pass indirect proxies to the setObservers method of the `IceGrid::AdminSession` interface. Previously, the IceGrid registry would fail to resolve the endpoints and the observer updates weren't sent.

* The IceGrid implementation of `Ice::Locator::getRegistry` now returns a replicated proxy if IceGrid replication is used. This way, if an IceGrid replica becomes unavailable during a server's activation, the server will still be able to register its endpoints with one of the other replicas.

* Added a `cancel()` method to `AsyncResult`. Calling it prevents a queued asynchronous request from being sent, or ignores the reply if the request has already been sent. It does not cancel the request in the server. Canceled requests throw `Ice::InvocationCanceledException`.

* Removed the "old" asynchronous invocation API that was deprecated in Ice 3.4.

* Removed the following deprecated Slice operations:
  `Ice::Router::addProxy`
  `IceGrid::Admin::writeMessage`
  `IceStorm::Topic::subscribe`

* Added the WebSocket transports to the Ice core, including new implementations in Java and C#. This means it's no longer necessary to use Glacier2 as an intermediary when a JavaScript client needs to communicate with a Java or C# server.

* Added a "Logger" facet to the admin object, which allows remote applications to attach `RemoteLogger` objects and receive log messages. Both the `icegridadmin` command-line utility and `IceGrid Admin GUI` use this new facet to retrieve the logs of Ice servers, IceBox services, IceGrid nodes and IceGrid registries.

* Added `getNodeAdmin and `getRegistryAdmin` operations to the `IceGrid::Admin` interface. They return the admin object of a given IceGrid registry or node.

* Added new `getPrefix` operation to `Ice::Logger`; it returns the prefix of the logger.

* The admin object can now be hosted in an application-supplied object adapter using the new property `Ice.Admin.Enabled` and the new operation `Ice::Communicator::createAdmin`.

* Added methods to proxies to asynchronously get the connection associated with the proxy. The current synchronous methods can block as they may cause the connection to be established.

* Added support for HTTP proxies to the Ice core in C++, C# and Java. This allows outgoing TCP & SSL connections to be mediated by an HTTP network proxy service that supports HTTP CONNECT tunneling. The new properties `Ice.HTTPProxyHost` and `Ice.HTTPProxyPort` provide addressing information for the proxy.

* Added `IceStorm::InvalidSubscriber` exception which is thrown by the `subscribe/unsubscribe` operations when an invalid subscriber (such as a null proxy) is passed. IceStorm would previously either abort (if built in debug mode) or throw an `Ice::UnknownLocalException` if such a subscriber proxy was provided.

* Added `Ice.Default.SourceAddress` property and `--sourceAddress` endpoint setting to control the interface used for an outgoing connection. The configured source address can be retrieved with the `sourceAddress` member of the `Ice::IPEndpointInfo` class.

* The signatures of `Ice::OutputStream startSize()` and `endSize()` have been modified. It is now necessary for the caller of `startSize()` to save the returned position and pass it into `endSize()`.

* Added `IceStormAdmin.Host` and `IceStormAdmin.Port` properties. You can set these properties to connect to an IceStorm instance running on a given host and port.

* Added IceStorm finder interface (defined in `IceStorm/IceStorm.ice`) to enable clients to easily find the proxy of the topic manager with just the endpoint information of the IceStorm service. The client doesn't need to know the instance name. IceStorm provides an Ice object with the identity `IceStorm/Finder` that implements this interface.

* Added support for IceGridDiscovery to the IceGrid administrative utilities. Also added `IceGridAdmin.Host` and `IceGridAdmin.Port` properties; you can set these properties to connect to an IceGrid registry running on a given host and port.

* Added "finder" interfaces for locators and routers to enable clients to easily find proxies for the locator or router (defined in `Ice/Locator.ice` and `Ice/Router.ice`). Glacier2 and IceGrid implement these interfaces and provide finder Ice objects with the identities `Ice/RouterFinder` and `Ice/LocatorFinder`, respectively. Clients can use these objects to connect to Glacier2 and IceGrid using only the endpoint information; they don't need to know the instance name of the Glacier2 or IceGrid service.

* Added IceGrid discovery plug-in, which enables Ice applications to discover IceGrid registries running on the network using UDP multicast.

* Ice metrics now support collocation optimization. The `Ice::Instrumentation::CollocatedObserver` local interface and the `IceMX::CollocatedMetrics` class have been added to monitor collocated requests.

* Collocation optimization has been significantly improved to remove limitations. It now works with AMI, AMD, blobjects and batching. Collocated requests are now always marshaled and unmarshaled but they don't go through any transport. The `Ice::CollocationOptimizationException` is no longer used and as a result is now deprecated.

* Fixed a bug where retries might fail if the request failed right after connection establishment.

* Added callbacks for connection closure and heartbeats. The callback is invoked when the connection is closed or when a heartbeat is received. Callbacks are installed on the connection with the `Ice::Connection::setCallback` method and implement the `Ice::ConnectionCallback` interface.

* Active connection management improvements:

    * Active connection management now supports sending heartbeats over the Ice connection. Depending on the configuration, heartbeats are sent when there are long invocations in progress, when the connection is idle, or at regular time intervals.

    * Connection closure by active connection management can be configured to close connections under certain conditions. For example, a connection can be closed gracefully or forcefully if it's idle for the specified timeout period.

    * The ACM properties to configure the ACM timeout have been deprecated. You should now use the `prefix.Timeout` property where prefix can be `Ice.ACM`, `Ice.ACM.Client`, `Ice.ACM.Server` or `objectAdapter.ACM`.

* IceGrid and Glacier2 now support the new ACM mechanism to keep sessions alive. The `Glacier2::Router` and `IceGrid::Registry` interfaces support a new `getACMTimeout` operation. To keep the connection alive the client should configure its connection to always send heartbeats using the timeout returned by `getACMTimeout`. A client can also install a connection callback to be notified when the connection is unexpectedly closed.

* Added support for invocation timeouts. Invocation timeouts can be set on a per-proxy basis with the `ice_invocationTimeout` proxy method. By default, there's no invocation timeout meaning that an invocation will never timeout if the connection between the client and server is alive. An invocation timeout does not cause a connection to be closed. If the timeout is triggered the invocation will raise `Ice::InvocationTimeoutException`.

* With the addition of invocation timeouts, the existing connection timeout mechanism no longer applies to invocations. Connection timeouts are now enabled by default using the new property `Ice.Default.Timeout`, which specifies the timeout for stringified proxy endpoints that do not have a timeout explicitly set. The default setting for the property is 60 seconds. It is also now possible to set an infinite timeout using `-t infinite` in the endpoint configuration.

* Added IceDiscovery plug-in, which provides a UDP multicast-based Ice locator implementation. This can be used as an alternative to IceGrid when running few clients and servers on the same network and if you don't want to deploy a central service such as IceGrid.

* Added support for IceGrid custom load balancing with the addition of replica group and type filters. These filters allow you to select the set of replicas that are sent back to the client when it resolves replica group endpoints. It's also possible to filter the objects returned to the client when it uses the `IceGrid::Query` interface.

* This addition requires the upgrade of IceGrid registry databases if you deployed replica groups for your IceGrid applications. See the release notes for information on how to upgrade your IceGrid databases.

* Fixed a marshaling bug where the marshaling of a type with nested optionals of variable size could trigger an assertion.

* Fixed an IceGrid registry replication bug where database synchronization could fail on startup or connection establishment with the master if multiple applications referred to a replica group defined in another application that wasn't yet added.

* FreezeScript can now transform classes to structures and structures to classes. It also supports the use of compact type IDs for classes.

* The `CryptPasswords` file support for Glacier2 and IceGrid is now implemented in a separate C++ plug-in, the `Glacier2CryptPermissionsVerifier` plug-in. On Windows, this plug-in is the only remaining Ice run time dependency on OpenSSL.

* Removed the `Ice::Stats` interface, which was deprecated in Ice 3.5 and replaced by the new metrics facility.

* The `IceBox.ServiceManager` object adapter has been deprecated. Use `Ice.Admin` and the `IceBox.ServiceManager` facet instead.

* Removed the following deprecated properties:
  `Glacier2.AddSSLContext`
  `IcePatch2.ChunkSize`
  `IcePatch2.Directory`
  `IcePatch2.Remove`
  `IcePatch2.Thorough`

## C++ Changes

* Ice for C++ now supports the GCC and clang symbol visibility options: we build by default with `-fvisibility=hidden` and we define `ICE_DECLSPEC_EXPORT` and `ICE_DECLSPEC_IMPORT` for these compilers.

* Refactored the capture and printing of exception stack traces to be more efficient when the stack traces are not printed. The signature of `IceUtil::Exception::ice_stackTrace` changed to:
  `std::string ice_stackTrace() const;`

* Fixed a bug where a private Windows Runtime delegate type could be published in Windows Store Components metadata files (WINMD).

* New garbage collection support for object graphs with cycles. Ice no longer uses a background thread to collect objects. Instead, `Ice::Object` now has a new `ice_collectable` method to indicate whether or not the object graph can be collected. When a graph is marked as collectable with this method, you shouldn't update the structure of the object graph. Ice tries to collect the graph each time a reference count is dropped for an object in the cycle. By default, object collection is turned off. When it's turned on with `Ice.CollectObjects=1`, the Ice run time calls `ice_collectable(true)` on all the objects it unmarshals. The properties `Ice.GC.Interval` and `Ice.Trace.GC` are no longer supported, nor is the `Ice::collectGarbage()` function.

* Significant changes to the IceSSL plug-in:

    * Now uses the native SecureTransport API on OS X
    * Now uses the native SChannel API on Windows
    * OpenSSL is only used in IceSSL on Linux.

* Added support for the `IceSSL.FindCert` property on Windows and OS X.

* Added the new metadata tag `cpp:view-type:Type`, where `Type` is a type that can safely provide a "view" into the Ice unmarshaling buffer and thereby avoid extra copies. This tag can only be applied to operation parameters of type string, sequence and dictionary, and only has an effect in situations where the Ice data buffer is guaranteed to still exist.

* It is no longer necessary to provide import libraries such as `IceD.lib`, `IceUtilD.lib` or `IceGridD.lib` when linking on Windows with Visual C++. The linker finds the import library names automatically through `pragma comment(lib, ...)` in the Ice header files.

* Ice header files no longer disable or downgrade any warning reported by the C++ compiler in your code. In Ice 3.5, all header files indirectly included the following:
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
  When upgrading your Visual C++ application to Ice 3.6, you may need to fix, disable or downgrade these warnings in your C++ code.

* Fixed a bug where the communicator destruction could raise `Ice::NullHandleException` if a custom observer was present.

* Fixed a bug where the selector wouldn't check for `EPOLLERR` on Linux. This could cause the selector thread to spin if this condition occurred (which is the case with UDP connected sockets).

* Added per-process string converters for narrow and wide strings. The functions `setProcessStringConverter` and `getProcessStringConverter` in the IceUtil namespace can be used to establish and access the process narrow string converter. Likewise `setProcessWstringConverter` and `getProcessWstringConverter` can be used to establish and access the process wide string converter.

* Removed the `stringConverter` and `wstringConverter` data members of `Ice::InitializationData`.

* Moved the `StringConverter` template and classes from the Ice namespace to the IceUtil namespace.

* Added `Ice.LogStdErr.Convert` property which controls whether or not the default StdErr logger converts messages from native narrow encoding to the console code page. The default value is 1, meaning by default messages are converted to the console code page encoding. The property is only used on Windows.

* Added `ice_staticId()` to the ProxyHandle template.

* Removed deprecated `Ice::Object::ice_getHash` function.

## Java Changes

* Added new Slice metadata `java:buffer` to enable the use of `java.nio.Buffer` types in the generated code for sequences of byte, short, int, long, float and double. For example, `sequence<byte>` maps to `java.nio.ByteBuffer` instead of `byte[]`.

* Changed the Ice core to allow applications to safely interrupt threads that are calling Ice APIs. The new property `Ice.ThreadInterruptSafe` must be enabled in order to use thread interrupts. Ice raises the new exception `Ice.OperationInterruptedException` if an API call is interrupted.

* Added support for Java 8.

* Java 8 lambda expressions can be used as callbacks for asynchronous proxy invocations.

* Removed deprecated `IceUtil.Version` class.

## C# Changes

* Fixed a bug in the Slice-to-C# compiler that would generate invalid code if an interface inherited a method that had an optional value as a return value.

* Deprecated the `clr:collection` Slice metadata for sequences and dictionaries as well as the `CollectionBase` and `DictionaryBase` classes.

* Fixed a security vulnerability in IceSSL in which an untrusted self-signed CA certificate was accepted.

* Added `IceSSL.CertAuthFile` property, which allows you to specify a CA certificate without having to import it into a certificate store.

* Deprecated the `IceSSL.ImportCert` property. Certificates should be imported using standard Windows tools.

* Fixed a bug where under certain circumstances the thread pool could serialize dispatch.

* The `Ice.Communicator` type now implements `IDisposable`, which allows communicator objects to be used as resources in `using()` statements.

* Added the metadata tag `clr:implements:Base`, which adds the specified base type to the generated code for a Slice structure, class or interface.

* Most Slice data types now support .NET serialization, which allows Ice exceptions to cross AppDomain boundaries. The only limitation is that data members whose types are proxies cannot be serialized; these members will be set to null after deserialization.

* It is now possible for users to provide a base class for a Slice-generated class with .NET partial classes.

* Fixed leak in C# transport. The leak occurred for each Ice connection.

* Mono is no longer an officially-supported platform for Ice for .NET.

## Python Changes

* Added support for Python 3.4.

* Changed the semantics of Ice.Unset so that it evaluates as false in a boolean context.

* Fixed a bug in the Slice-to-Python translator that eliminates the use of implicit relative imports for nested Slice modules.

* Added `ice_staticId()` to generated proxy classes.

## Ruby Changes

* Added support for Ruby 2.1.

* Added `ice_staticId()` to generated proxy classes.

## PHP Changes

* Added support for PHP 5.6.

* Added `ice_staticId()` to generated proxy helper classes.

* Generated interfaces now extend the Object base interface.

* Added `ObjectPrxHelper` class.

## JavaScript Changes

* JavaScript is now an officially-supported language mapping.

* Added the following methods to `AsyncResult` for consistency with the other language mappings: `cancel`, `isCompleted`, `isSent`, `throwLocalException`, `sentSynchronously`.

* Changed the NodeJS packaging such that existing Ice for JavaScript programs will need to modify their require statements. For example, to use Ice you would now write:
  `var Ice = require("icejs").Ice;`
