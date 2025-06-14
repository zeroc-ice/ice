The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

We recommend that you use the release notes as a guide for migrating your applications to this release, and the manual
for complete details on a particular aspect of Ice.

# Changes in Ice 3.8.0

These are the changes since the Ice 3.7.10 release in [CHANGELOG-3.7.md](./CHANGELOG-3.7.md).

## General Changes

- Replaced ACM and connection timeouts by idle, inactivity, connect, and close timeouts.
  - Idle timeout\
  Once a connection is established, this connection is considered healthy as long as it does not wait for more than idle
  timeout to read a byte. If a connection waits for more than idle timeout to read a byte, it is considered idle and
  aborted by the idle check mechanism.
  Idle is never a good state. To prevent connections from becoming idle, Ice ensures there is regular write activity on
  established connections: if there is no write on a connection for idle timeout / 2, Ice sends a heartbeat on this
  connection. A heartbeat is a one-way, unacknowledged, ValidateConnection message. The default idle timeout is
  60 seconds. You can change this value with the configuration property `Ice.Connection.Client.IdleTimeout` (for client
  connections) or `Ice.Connection.Server.IdleTimeout` (for server connections). The unit for this timeout is seconds.
   You can also override this value for a specific object adapter with the configuration property
  `AdapterName.Connection.IdleTimeout`. We recommend keeping things simple: use the same value (typically the default)
  for all your Ice applications. In particular, the idle timeout is not negotiated during connection establishment and
  an idle timeout mismatch may result in untimely connection aborts.
    - **Interop with Ice 3.7 and earlier releases**\
    If your Ice 3.8 application connects to an older Ice server or accepts a connection from an older Ice client, you
    need to change the configuration of your older Ice application to send regular heartbeats. Otherwise, your Ice 3.8
    application can consider the connection idle (after idle timeout) and abort this connection. With Ice 3.7 and Ice
    3.6, you can set the property `Ice.ACM.Heartbeat` to 3, and make sure the `Ice.ACM.Timeout` property matches your
    idle timeout (the default for the ACM timeout is 60 seconds, just like the default idle timeout). If you are unable
    to change the configuration of your older Ice application, you can switch off the idle check on the Ice 3.8 side
    by setting `Ice.Connection.Client.EnableIdleCheck` or `Ice.Connection.Server.EnableIdleCheck` to 0. You can also
    switch off the idle check for just a specific object adapter by setting `AdapterName.Connection.EnableIdleCheck`
    to 0.
    - **Interop with IceRPC**\
    [IceRPC] uses the same idle timeout and idle check mechanism for connections that use the `ice` protocol.
  - Inactivity timeout\
  A connection is considered inactive when there is no application-level activity on this connection:
  there is no outstanding invocation (we're not waiting for a response for a request we've sent), there is no
  outstanding dispatch, and no request or response is being sent or received. Sending or receiving a heartbeat (see
  above) does not count as application-level activity. When a connection remains inactive for more than the inactivity
  timeout, this connection is closed gracefully. Inactive is a good state: the connection is healthy but unused, so we
  shut it down to reclaim resources. The default inactivity timeout is 300 seconds. You can change this default by
  setting `Ice.Connection.Client.InactivityTimeout` (for client connections) or
  `Ice.Connection.Server.InactivityTimeout` (for server connections). The unit for this timeout is seconds. You
  can also override this value for a specific object adapter with the configuration property
  `AdapterName.Connection.InactivityTimeout`.\
  Note: make sure your inactivity timeout is greater than your idle timeout, as the implementation of the inactivity
  timeout relies on a smaller idle timeout value. If you disable your idle timeout by setting it to 0 or very large
  value (not something we recommend), you will effectively disable your inactivity timeout as well.
  - Connect timeout\
  A connection establishment fails if it takes more than connect timeout to complete. The default connect timeout is 10
  seconds. You can change this value by setting `Ice.Connection.Client.ConnectTimeout` (for client connections) or
  `Ice.Connection.Server.ConnectTimeout` (for server connections). The unit for this timeout is seconds. You can also
  override this value for a specific object adapter with the configuration property
  `AdapterName.Connection.ConnectTimeout`.
  - Close timeout\
  A graceful connection closure transitions to connection abort if it takes more than close timeout to complete. The
  default close timeout is 10 seconds. You can change this value by setting `Ice.Connection.Client.CloseTimeout` (for
  client connections) or `Ice.Connection.Server.CloseTimeout` (for server connections). The unit for this timeout is
  seconds, as usual. You can also override this value for a specific object adapter with the configuration
  property `AdapterName.Connection.CloseTimeout`.

- Add new properties for flow control.
  - Max dispatches\
  The maximum number of concurrent dispatches for each connection is now 100. You can change this value by setting
  `Ice.Connection.Client.MaxDispatches` (for client connections) or `Ice.Connection.Server.MaxDispatches` (for server
  connections). You can also override this value for a specific object adapter with the configuration
  property `AdapterName.Connection.MaxDispatches`.
  When the limit is reached, Ice stops reading from the connection, which applies back pressure on the peer.
  - Max connections\
  The property _adapter_.MaxConnections limits the number of incoming connections accepted by an object adapter. The
  default is 0, which means no limit.

- Simplify proxy creation.
  You can now create a typed proxy directly from a communicator and a string in all languages. For example:

  ```cpp
  // C++
  GreeterPrx greeter{communicator, "greeter:tcp -h localhost -p 4061"};
  ```

  ```csharp
  // C#
  var greeter = GreeterPrxHelper.createProxy(communicator, "greeter:tcp -h localhost -p 4061");
  ```

  ```java
  // Java
  var greeter = GreeterPrx.createProxy(communicator, "greeter:tcp -h localhost -p 4061");
  ```

   ```js
  // JavaScript
  const greeter = new GreeterPrx(communicator, "greeter:tcp -h localhost -p 4061");
  ```

  ```matlab
  % MATLAB
  greeter = GreeterPrx(communicator, 'greeter:tcp -h localhost -p 4061');
  ```

  ```php
  // PHP
  $greeter = GreeterPrxHelper::createProxy($communicator, 'greeter:tcp -h localhost -p 4061');
  ```

  ```python
  # Python
  greeter = GreeterPrx(communicator, "greeter:tcp -h localhost -p 4061")
  ```

  ```ruby
  # Ruby
  greeter = GreeterPrx.new(communicator, "greeter:tcp -h localhost -p 4061")
  ```

  ```swift
  // Swift
  let greeter = try makeProxy(
    communicator: communicator,
    proxyString: "greeter:tcp -h localhost -p 4061",
    type: GreeterPrx.self)
  ```

  ```ts
  // TypeScript
  const greeter = new GreeterPrx(communicator, "greeter:tcp -h localhost -p 4061");
  ```

  The existing `stringToProxy` operation on `Communicator` remains available. However, the new syntax is now the
  preferred way to create a proxy from a string.

- Add new dispatcher API in C++, C#, Java, JavaScript, and Swift.
  - Replaced dispatch interceptors by middleware. See the new forwarder and middleware demos.
  - Changed the name of the Ice 3.7 dispatcher API: it's now called executor.

- Add `setDefaultObjectAdapter` operation on Communicator to simplify the creation of bidir connections. See the updated
  Ice/bidir demo.

- Rework the published endpoints of object adapters
  - The published endpoints of an object adapter are the endpoint(s) included in the proxies returned by the `add` and
`createProxy` operations on an object adapter. For indirect object adapters, the published endpoints are the endpoints
registered with the Ice Locator (typically the IceGrid registry).
  - Improved the algorithm for computing the default published endpoints.
  - Add new _adapter_.PublishedHost property, used to compute the default published endpoints.
  - Removed the `refreshPublishedEndpoints` operation on `ObjectAdapter`.

- The local exceptions that can be marshaled now have a common base class (`DispatchException`), and are no longer
  limited to 6 exceptions. The reply status of a dispatch exception can have any value between 2 and 255. A dispatch
  exception with reply status >= 5 is marshaled as its reply status (one byte) followed by its message (a Slice-encoded
  string).

```mermaid
classDiagram
    class ReplyStatus {
        <<enumeration>>
        Ok = 0
        UserException
        ObjectNotExist
        FacetNotExist
        OperationNotExist
        ...
    }
    LocalException <|-- DispatchException
    LocalException: +string message
    DispatchException : +ReplyStatus replyStatus
    DispatchException <|-- RequestFailedException
    class RequestFailedException{
        +Identity id
        +string facet
        +string operation
    }
    RequestFailedException <|-- ObjectNotExistException
    ObjectNotExistException : replyStatus = ObjectNotExist
    RequestFailedException <|-- FacetNotExistException
    FacetNotExistException : replyStatus = FacetNotExist
    RequestFailedException <|-- OperationNotExistException
    OperationNotExistException : replyStatus = OperationNotExist

    DispatchException <|-- UnknownException
    UnknownException : replyStatus = UnknownException
    UnknownException <|-- UnknownLocalException
    UnknownLocalException : replyStatus = UnknownLocalException
    UnknownException <|-- UnknownUserException
    UnknownUserException : replyStatus = UnknownUserException
```

- Consolidate and refactor the exceptions derived from LocalException.

  | Local exception in Ice 3.7          | Replacement                                                                              | Notes    |
  | ----------------------------------- | ---------------------------------------------------------------------------------------- | -------- |
  | BadMagicException                   | ProtocolException (base)                                                                 |          |
  | CFNetworkException                  | SocketException (base)                                                                   |          |
  | CloneNotImplementedException        | std::logic_error                                                                         | C++ only |
  | CompressionException                | ProtocolException (base)                                                                 |          |
  | ConnectionManuallyClosedException   | ConnectionAbortedException, ConnectionClosedException                                    |          |
  | ConnectionNotValidatedException     | ProtocolException (base)                                                                 |          |
  | EncapsulationException              | MarshalException (base)                                                                  |          |
  | EndpointParseException              | ParseException                                                                           |          |
  | EndpointSelectionTypeParseException | ParseException                                                                           |          |
  | IllegalIdentityException            | ArgumentException (C#), std::invalid_argument (C++), IllegalArgumentException (Java)     |          |
  | IllegalMessageSizeException         | MarshalException                                                                         |          |
  | IllegalServantException             | ArgumentNullException (C#), std::invalid_argument (C++), IllegalArgumentException (Java) |          |
  | IdentityParseException              | ParseException                                                                           |          |
  | MemoryLimitException                | MarshalException (base)                                                                  |          |
  | NoValueFactoryException             | MarshalException (base)                                                                  |          |
  | ProxyParseException                 | ParseException                                                                           |          |
  | ProxyUnmarshalException             | MarshalException (base)                                                                  |          |
  | StringConversionException           | MarshalException (base)                                                                  |          |
  | UnexpectedObjectException           | MarshalException (base)                                                                  |          |
  | UnknownMessageException             | ProtocolException (base)                                                                 |          |
  | UnknownReplyStatusException         | None: all values are now valid                                                           |          |
  | UnmarshalOutOfBoundsException       | MarshalException (base)                                                                  |          |
  | UnsupportedEncodingException        | MarshalException                                                                         |          |
  | UnsupportedProtocolException        | MarshalException, FeatureNotSupportedException                                           |          |
  | VersionMismatchException            | InitializationException                                                                  |          |
  | VersionParseException               | ParseException                                                                           |          |

  base = was existing base class

  New local exceptions:\
  ConnectionAbortedException, ConnectionClosedException, ParseException

- Refactored the unmarshaling of Slice-defined classes and exceptions.
  When Ice unmarshals a Slice-defined class or exception, it first needs to locate and create an instance of the mapped
  C++/C#/Java (...) class, using the default parameter-less constructor of the mapped class. The new abstraction for
  this process is the Slice loader. Its API varies slightly from language to language, for example:

  ```cpp
  // C++
  class SliceLoader
  {
  public:
        [[nodiscard]] virtual ValuePtr newClassInstance(std::string_view typeId) const;
        [[nodiscard]] virtual std::exception_ptr newExceptionInstance(std::string_view typeId) const;
  };
  ```

  ```java
  // Java
  @FunctionalInterface
  public interface SliceLoader {
      java.lang.Object newInstance(String typeId);
  }
  ```

  ```typescript
  // TypeScript
  interface SliceLoader {
      newInstance(typeId: string): Ice.Value | Ice.UserException | null;
  }
  ```

  ```matlab
  % MATLAB
  classdef (Abstract) SliceLoader < handle
      methods(Abstract)
          r = newInstance(obj, typeId)
      end
  end
  ```

  You can implement `SliceLoader` and install your own custom Slice loader on a communicator by setting the
  `sliceLoader` field in `InitializationData`. This custom Slice loader is always in addition to an internal Slice
  loader that Ice uses when you don't set a custom Slice loader or when your Slice loader returns null. This new
  `InitializationData` field replaces the `ValueFactory` and `ValueFactoryManager` provided in previous Ice releases.

  In most languages, generated classes for Slice classes and exceptions register themselves at startup with a default
  Slice loader implemented by Ice, and you don't need to do anything to help Ice locate these generated classes.
  However, in Java and MATLAB, there is no such registration at startup, and you need to help Ice locate these generated
  classes when:
  - you remap either the class name or an enclosing module using the `java:identifier`, `java:package`, or
    `matlab:identifier` metadata; or
  - you assign a compact ID to your class

  You help Ice locate these classes by installing a Slice loader in `InitializationData`, just like when you provide a
  custom Slice loader. Ice for Java and Ice for MATLAB provide implementations of `SliceLoader` for this purpose. For
  example, you can use the `ClassSliceLoader` implementation to create a Slice loader for one or more generated classes
  (typically classes with remapped names or compact IDs).

  Limitation: in Python, Ruby, and PHP, a custom Slice loader can only create class instances. The creation of custom
  user exceptions is currently ignored.

- The plug-ins provided by Ice now have fixed names: IceIAP, IceBT, IceUDP, IceWS, IceDiscovery, IceLocatorDiscovery.
  This fixed name is the only name you can use when loading/configuring such a plug-in with the Ice.Plugin.name property.

- The Windows MSI installer is now built using the WiX Toolset. The WiX project files are included in the packaging/msi
  directory.

- The RPM packaging files, previously distributed in the ice-packaging repository, are now included in the packaging/rpm
  directory.

- The DEB packaging files, previously distributed in the ice-packaging repository, are now included in the packaging/deb
  directory.

- Removed support for setting per-language plug-in entry points. In Ice 3.7 and earlier, it was possible to specify
  plug-in entry points on a per-language basis using the `Ice.Plugin.<name>.<lang>` syntax. This feature was rarely used
  and discouraged, as configuration files should not be shared across language mappings.

- The default value for Ice.ClassGraphDepthMax is now `10`. In Ice 3.7, the default was `0`, which meant the class
  graph depth was unlimited.

- Removed the `stringToIdentity` method from the Communicator class. This method was deprecated in Ice 3.7.

## Slice Language Changes

- Removed local Slice. `local` is no longer a Slice keyword.

- Added new metadata for customizing the mapped names of Slice definitions in each language.
  This metadata is of the form: `["<lang>:identifier:<identifier>"]`, where `<lang>` can be any of the standard language
  prefixes, and that definition's identifier will be `<identifier>` in the specified language.

  For example:

  ```slice
  ["cs:identifier:MyNamespace"]
  ["java:identifier:com.example.mypackage"]
  module MyModule {}
  ```

  The argument is used as a drop-in replacement for the Slice identifier, with no additional processing.
  For the above example, `slice2cs` will generate `namespace MyNamespace {}` and `slice2java` will generate
  `package com.example.mypackage;`. All other compilers will map the module using its Slice-provided identifier,
  as usual.

  This metadata can be applied to any Slice definition with an identifier, and is available for all languages.

  Note that this only affects the _mapped_ name of Slice definitions.
  It has no effect on Slice type IDs, or a definition's on-the-wire representation.

- Deprecated the `cs:namespace`, `java:package`, `python:package`, and `swift:module` metadata.
  `<lang>:identifier` metadata is now the preferred way to change how Slice modules are mapped.

- Removed automatic escaping of Slice identifiers. Previously, the Slice compilers had a list of each language's
  keywords and reserved identifiers, and would automatically escape conflicting identifiers during code-generation.
  Now that this has been removed, conflicting identifiers should be fixed using `<lang>:identifier` metadata.

- Added a shorthand syntax for defining nested modules. For example, the following two definitions are equivalent:

  ```slice
  module Foo { module Bar { module Baz { /*...*/ } } }

  module Foo::Bar::Baz { /*...*/ }
  ```

  Note: metadata cannot be applied to modules using this syntax, since it's ambiguous which module it would apply to.

- Added support for triple-slash doc comments, in addition to the already supported JavaDoc comment syntax.
  For example, the following two definitions are equivalent:

  ```slice
  /// Sends a request.
  /// @param message the message.
  /// @return a response code.
  int sendRequest(string message);

  /**
   * Sends a request.
   * @param message the message.
   * @return a response code.
   */
  int sendRequest(string message);
  ```

- Lists of metadata can be split into separate brackets now, allowing for longer metadata to be placed on separate lines
  or for metadata to be grouped by functionality. For example, you can now write:

  ```slice
  ["deprecated:This operation should no longer be called"]
  ["amd"] ["format:sliced"]
  void myOperation(MyClass c);
  ```

  Previously, all metadata needed to be in a single comma-separated list (note that this syntax is still supported):

  ```slice
  ["deprecated:This operation should no longer be called", "amd", "format:sliced"]
  void myOperation(MyClass c);
  ```

- Metadata can now be applied to Slice enumerators.

- Added `["deprecated"]` as an alias for the `["deprecate"]` metadata.

- Removed the `["protected"]` metadata. This was primarily for classes with operations, which are no longer allowed.

- Removed the `["preserve-slice"]` metadata. Slice classes marshaled in the sliced format are now always preserved when
  unmarshaled.

- Exceptions are now always marshaled in the sliced format and no longer support preservation of unknown slices during
  unmarshaling.

- Slice classes can no longer define operations or implement interfaces, and `implements` is no longer a Slice keyword.
  This feature has been deprecated since Ice 3.7.

- Slice classes can no longer represent remote Ice objects; the syntax `MyClass*` (a proxy to a class) is now invalid.

- An interface can no longer be used as a type. This feature, known as "interface by value", has been deprecated since
  Ice 3.7. You can still define proxies with the usual syntax, `Greeter*`, where `Greeter` represents an interface.

- The type of an optional field or parameter can no longer be a class or contain a class.

- `:` is now an alias for the `extends` keyword.

- Removed Slice checksums.

- Sequences can no longer be used as dictionary key types.
  This feature has been deprecated since Ice 3.3.0.

- Improved validation of metadata and doc-comments.

## Slice Tools

- Add new `ice2slice` compiler that converts Slice files in the `.ice` format (used by Ice) into Slice files in the
`.slice` format (used by IceRPC).

- Removed the `slice2html` compiler, which was previously used to convert Slice documentation comments to HTML. Doxygen
  should be used to generate Slice API documentation.

- Removed the `--impl` argument from the Slice compilers.

- You can now use identifiers with underscores or with the Ice prefix without any special compiler option.

## IceSSL Changes

The ssl transport is no longer a plug-in. It is now built into the main Ice library and always available.

### Integration with Platform SSL Engines

Ice 3.8 introduces new IceSSL configuration APIs that allow you to configure the ssl transport using platform-native
SSL engine APIs. This provides significantly greater flexibility for advanced use cases.

- The ssl transport can now be fully configured programmatically, without relying on IceSSL properties.
- Separate configurations for outgoing and incoming SSL connections are supported.
- Per-ObjectAdapter configuration is also possible.

> These APIs are platform-dependent. A good starting point is the `Ice/secure` demo for your target platform and
> language mapping.

### Removed Support for OpenSSL on Windows

In Ice 3.7, IceSSL on Windows could be built with either Schannel or OpenSSL. In Ice 3.8, since IceSSL is now built-in,
it always uses the platform’s native SSL APIs. On Windows, this means **Schannel is always used**;
**OpenSSL is no longer supported** on Windows.

### Removed IceSSL APIs

- **Certificate API**\
  The `IceSSL::Certificate` type and related APIs have been removed. Applications that require access to certificate
  data must now use platform-native certificate APIs.

- **Certificate Verifiers**\
  Custom certificate verifiers have been replaced with new configuration APIs that allow applications to install
  validation callbacks that directly use the underlying SSL engine APIs.

- **Password Callbacks**\
  Password callback support has been removed. Applications can now provide certificates and keys directly through the
  new configuration APIs.

### Updated IceSSL Properties

- **IceSSL.CertFile**\
  This property no longer accepts multiple files. In Ice 3.7, IceSSL with OpenSSL or Schannel allowed specifying two
  files—one for RSA and one for DSA certificates. This uncommon use case is no longer supported. Applications requiring
  more flexibility should use the new configuration APIs, which support selecting certificates on a per-connection
  basis.

- **IceSSL.CheckCertName**\
  In Ice 3.7, this property controlled two unrelated features:
  (1) matching the target host name against the peer's certificate Subject Alternative Name or Common Name, and
  (2) enabling SNI (Server Name Indication).
  In Ice 3.8, **SNI is always enabled** for outgoing connections when the target endpoint uses a DNS name. The property
  now only controls certificate name matching.

- **IceSSL.Truststore**\
  The `IceSSL.Keystore` property is no longer used as a fallback for `IceSSL.Truststore`.

- **IceSSL.VerifyPeer**\
  Setting `IceSSL.VerifyPeer=0` no longer suppresses verification errors. In Ice 3.7, this was often used alongside
  the now-removed certificate verifier APIs. Applications requiring custom trust logic should use the new validation
  callbacks.

### Removed IceSSL Properties

Several IceSSL properties have been removed in Ice 3.8, either because better alternatives are now available, they are
no longer useful, or they go against best practices:

- **IceSSL.CertVerifier**\
  Previously used to dynamically load custom certificate verifiers. This mechanism and the property have been removed.

- **IceSSL.Ciphers**\
  Used to configure the list of allowed SSL ciphers. Ice 3.8 now uses system-wide defaults by default. For advanced use
  cases, the new configuration APIs provide direct access to the underlying SSL engine.

- **IceSSL.DH.bits**, **IceSSL.DHParams**\
  Used to configure Diffie-Hellman parameters for DH cipher suites. Since DH-based ciphers are no longer recommended,
  these properties have been removed.

- **IceSSL.EntropyDaemon**\
  Relevant only for legacy OpenSSL versions that are no longer supported.

- **IceSSL.InitOpenSSL**\
  Controlled whether IceSSL performed global OpenSSL initialization. This is no longer needed with the OpenSSL versions
  supported in Ice 3.8.

- **IceSSL.Random**\
  Used to provide seed data to the SSL engine. Modern SSL platforms handle this internally, so the property is no longer
  necessary.

- **IceSSL.SchannelStrongCrypto**\
  Enabled the `SCH_USE_STRONG_CRYPTO` flag on Windows to disable weak cryptographic algorithms. This flag is now always
  enabled by default, making the property redundant.

- **IceSSL.PasswordCallback**, **IceSSL.PasswordRetryMax**\
  Supported dynamic password callbacks, which are no longer supported. These properties and their underlying mechanism
  have been removed.

- **IceSSL.Protocols**, **IceSSL.ProtocolVersionMax**, **IceSSL.ProtocolVersionMin**\
  Controlled which SSL/TLS protocol versions were allowed for connections. Ice 3.8 now uses the system defaults
  (typically TLS 1.2 and TLS 1.3). Applications needing precise control can either adjust system settings or use the
  new configuration APIs.

- **IceSSL.VerifyDepthMax**\
  Previously used to set the maximum certificate chain length. This feature was rarely used. Applications requiring this
  functionality should implement a custom certificate validation callback.

## C++ Changes

- There is now a single C++ mapping, based on the C++11 mapping provided by Ice 3.7. This new C++ mapping requires a
C++ compiler with support for C++17 or higher.

- Generated proxy classes are now concrete classes with public constructors.

- Nullable proxies are represented using `std::optional`.

- All functions that create proxies, including `Communicator::stringToProxy`, `ObjectAdapter::add`,
  `Connection::createProxy` and more, are now template functions that allow you to choose the type of the returned
  proxy. The default proxy type is `Ice::ObjectPrx` for backwards compatibility. We recommend you always specify the
  desired proxy type explicitly. For example:

  ```cpp
  // widget is a std::optional<WidgetPrx>
  auto widget = communicator->propertyToProxy<WidgetPrx>("MyWidget");
  ```

- The C++ structs, classes, exception classes, and enumerations generated by the Slice compiler can now be printed using
`operator<<(ostream&, const T&)`. For structs, classes, and exceptions, this operator prints the type name and all the
field names and values; see the [Slice/print] test for examples.
  - You can also implement your own custom printing by applying metadata `["cpp:custom-print"]` to your Slice type.

- When unmarshaling an array of short, int, long, float, or double, you can now choose to use unaligned unmarshaling by
defining ICE_UNALIGNED when building your application. This optimization requires the non-default array mapping for
Slice sequences, enabled by the `cpp:array` metadata directive, and a compatible little endian platform. In Ice 3.7 and
earlier releases, this unaligned unmarshaling was turned on automatically on x86 and x64 CPUs, and turned off on all
other CPUs.

- Replaced the `Ice::registerPluginFactory` mechanism (with `Ice::registerIceUDP`, `Ice::registerIceDiscovery`, etc.)
by plug-in factories on InitializationData. The corresponding plug-ins are created during communicator initialization.
See `InitializationData::pluginFactories`.

- Removed StringConverterPlugin and ThreadHookPlugin.

- Removed the `--nowarn` option, which was used to suppress warnings in `icegridnode`, `glacier2router`,
  `icegridregistry`, and `icebox`.

- The C++ NuGet package has been renamed to `ZeroC.Ice.Cpp`. This package replaces the `zeroc.ice.vXXX` packages from
  Ice 3.7. It includes the Slice tools for C++ and no longer requires the `zeroc.icebuilder.msbuild` package.
  Additionally, it provides CMake support files in the cmake directory.

- Added overloads for the `ice_invocationTimeout` and `ice_locatorCacheTimeout` proxy methods that accept
  `std::chrono::duration` values. The corresponding `ice_getInvocationTimeout` and `ice_getLocatorCacheTimeout` methods
  now return `std::chrono::milliseconds`.

## C# Changes

- Added full support for nullable types:
  - Both the Ice C# API and the code generated by the Slice compiler are `#nullable enable`.
  - Ice now uses the standard `?` notation for all nullable types.

- The thread pools created by Ice no longer set a synchronization context. As a result, the continuation from an async
invocation made from an Ice thread pool thread executes in a .NET thread pool thread; previously, this continuation
was executed in a thread managed by the same Ice thread pool unless you specified `.ConfigureAwait(false)`.

- The `cs:attribute` Slice metadata is now limited to enums, enumerators, fields, and constants.

- Replaced the `Ice.Util.registerPluginFactory` mechanism by plug-in factories on InitializationData. The corresponding
plug-ins are created during communicator initialization. See `InitializationData.pluginFactories`.

- Slice structs are now mapped to record structs or record classes:
  - a Slice struct with only numeric, bool, enum, or record struct fields is mapped to a record struct.
  - a Slice struct with any other field type is mapped to a sealed record class.

- Updated Ice.Communicator to implement IAsyncDisposable. The preferred way to create and dispose of a communicator is
  now:

  ```cs
  await using Ice.Communicator communicator = Ice.Util.initialize(ref args);
  ```

- The preferred way to wait for communicator shutdown in an async context is now:

  ```cs
  await communicator.shutdownCompleted;
  ```

- Removed ThreadHookPlugin.

- Removed support for serializable objects (the `cs:serializable` metadata directive).

- The monolithic `zeroc.ice.net` package has been replaced with modular NuGet packages:

  ### New C\# Packages

    | Package                   | Description                                                                                             |
    | ------------------------- | ------------------------------------------------------------------------------------------------------- |
    | iceboxnet                 | The IceBox server for .NET, packaged as a dotnet tool.                                                  |
    | ZeroC.Glacier2            | The Glacier2 assembly, used by Glacier2 client applications.                                            |
    | ZeroC.Ice                 | The main Ice assembly.                                                                                  |
    | ZeroC.Ice.Slice.Tools     | The Slice compiler (slice2cs) and MSBuild integration. Replaces the `zeroc.icebuilder.msbuild` package. |
    | ZeroC.IceBox              | The IceBox assembly.                                                                                    |
    | ZeroC.IceDiscovery        | The IceDiscovery plug-in.                                                                               |
    | ZeroC.IceGrid             | The IceGrid assembly, used by IceGrid client applications.                                              |
    | ZeroC.IceLocatorDiscovery | The IceLocatorDiscovery plug-in.                                                                        |
    | ZeroC.IceStorm            | The IceStorm assembly, used by publishers and subscribers for IceStorm.                                 |

- Added overloads for the `ice_invocationTimeout` and `ice_locatorCacheTimeout` proxy methods that accept `TimeSpan`
  values. The corresponding `ice_getInvocationTimeout` and `ice_getLocatorCacheTimeout` methods now return a `TimeSpan`.

- Removed support for using `clr` as an alias for `cs` in metadata declarations.

## Java Changes

- Removed the Java-Compat mapping.

- Add plug-in factories to InitializationData. The corresponding plug-ins are created during communicator
initialization. See `InitializationData.pluginFactories`.

- Removed the `com.zeroc.Ice.Exception` base class. The common base class for all Ice exceptions is now
  `java.lang.Exception`.

- Removed the `Ice.ThreadInterruptSafe` property. You no longer need to set a property to use Ice for Java with
  interrupts.

- Removed ThreadHookPlugin.

- Reworked IceMX to avoid creating split packages.

- Added overloads for the `ice_invocationTimeout` and `ice_locatorCacheTimeout` proxy methods that accept
  `java.time.Duration` values. The corresponding `ice_getInvocationTimeout` and `ice_getLocatorCacheTimeout` methods
  now return a `java.time.Duration`.

- Added support for the `Ice.ClassGraphDepthMax` property, which controls the maximum depth allowed when unmarshaling a
  graph of Slice class instances.

## JavaScript Changes

- The Ice for JavaScript NPM package has been converted to a scoped package named `@zeroc/ice`.

- The `slice2js` Slice compiler is now included in the `@zeroc/ice` package, so there is no longer a need to install a
  separate `slice2js` NPM package.

- Added support for `Symbol.asyncDispose` on `Ice.Communicator`. TypeScript applications can now use the communicator in
  `await using` expressions.

  ```ts
  await using communicator = Ice.initialize(process.argv);
  ```

- Ice for JavaScript now uses `console.assert` with the label `DEV`. You must configure your build to strip out these
  `DEV` assertions in release builds.

- Fixed a bug where a reply could be processed before the corresponding request was marked as sent. This could break
  at-most-once semantics.

- Added support for `Ice.ClassGraphDepthMax` to control the maximum unmarshaling depth for graphs of Slice class
  instances.

- Removed support for the Internet Explorer browser.

- Slice modules are now always mapped to JavaScript ES6 modules. The `js:es6-module` metadata has been removed, as a
  single module mapping is now used by default.

- **Slice `long` is now mapped to JavaScript `BigInt`.** For input parameters, both `number` and `BigInt` are accepted.
  The `Ice.Long` class has been removed.

- The WebSocket transport is now supported with Node.js 24 and higher. In Node.js 23 and earlier, WebSocket connections
  do not reliably report errors during connection establishment. We advise against using the WebSocket transport on
  these versions.

## MATLAB Changes

- Changed the mapping for `sequence<string>`. A `sequence<string>` now maps to a MATLAB string array. This new mapping
  remains highly compatible with the previous mapping (cell array of char arrays).

- Changed the mapping for Slice dictionaries. A Slice dictionary now always maps to a MATLAB dictionary; the old
  `containers.Map` are no longer used.
  The dictionary key type is the mapped key type. The dictionary value type is as per the table below:

  | Slice type T mapped to MATLAB MT    | Value type of MATLAB dictionary | Mapping for `sequence<T>`                 |
  |-------------------------------------|---------------------------------|-------------------------------------------|
  | bool, numeric, struct, enum         | MT                              | array of MT                               |
  | string                              | string (1)                      | array of string                           |
  | sequence, dictionary, class, proxy  | cell holding a MT               | cell array of MT                          |

  (1) A single Slice string maps to a MATLAB char array, not to a MATLAB string.

- All fields are not mapped to typed MATLAB properties except optional fields and fields whose type is a class or uses
  a class.
  - In such properties, a null proxy is represented by an empty array of the proxy type, for example `GreeterPrx.empty`.
    Likewise, an empty sequence (array) is represented by an empty array of the correct type, such as `string.empty` or
    `int32.empty`.
  - `[]` is no longer a valid value for proxy and sequence properties. `[]` is still accepted as an operation argument
    for proxy and sequence parameters.

- Changed the default value for properties mapped from a Slice struct type. It's now an empty array of the mapped class;
  it was previously a new instance of the mapped class created with no argument.

- The default value for property `Ice.ProgramName` is now `matlab-client`. It used to be the first element in the args
  cell given to `Ice.initialize`.

- The `slice2matlab` function has been updated to accept multiple arguments, which are passed directly to the
  `slice2matlab` compiler. In Ice 3.7, all arguments had to be provided as a single string, which was less convenient.

## Objective-C Changes

- The Objective-C mapping was removed.

## PHP Changes

- Removed the flattened mapping deprecated in 3.7.

- Removed support for Windows PHP builds.

- Removed support for PHP 5 builds.

## Python Changes

- Added `Ice.EventLoopAdapter` for async event loop integration. This adapter enables integration of Ice with the
  application’s event loop, eliminates the need for manual use of `Ice.wrap_future`, and supports alternative event
  loops beyond the default asyncio loop.

- Added async context manager support to `Ice.Communicator`. You can now use the communicator in `async with`
  expressions.

  ```python
  async def main():
      async with Ice.initialize(
          sys.argv,
          eventLoop=asyncio.get_running_loop()) as communicator:
          ...

  if __name__ == "__main__":
      asyncio.run(main())
  ```

- Added a `checkedCastAsync` method to proxy objects to support asynchronous checked casts.

- Added a `Communicator.shutdownCompleted` method to asynchronously wait for communicator to shut down.

- Removed the copy parameter from the memory view factory. In Ice 3.8, the factory always receives a memory view over
  the input stream data, making the copy parameter unnecessary.

- Ice no longer uses `Ice.Unset` to represent unset optional parameters. Starting with Ice 3.8, `None` is used
  instead. The Ice.Unset global has been removed. Furthermore, there is no distinction between a non-set
  optional parameter and an optional parameter set to `None`.

- Removed the `Ice.generateUUID` helper method. You should use Python’s built-in uuid module instead.

- Added support for building pip packages directly from the Ice source distribution. In previous releases, the pip
  package was built from a separate tarball that included pre-generated source code.

## Ruby Changes

## Swift Changes

- Ice for Swift now requires Swift 6.1.

- Removed support for Carthage. Ice for Swift now uses the Swift Package Manager (SwiftPM) for dependency management.

- Updated to use async/await and fully support Swift's Structured Concurrency model.

- Removed the dependency on `PromiseKit`. Functions which previously returned a `Promise` are now async functions.

- Functions in dispatch protocols generated from Slice interfaces are now always async. The `amd` metadata directive has
  no effect on the generated code.

- Servants must now conform to the `Sendable` protocol.

- Proxy methods generated from Slice interfaces are now always async. The `Async` suffixed functions have been
  removed.

- Simplified the server-side mapping by removing the generated Disp structs. You can now use the generated server-side
  protocols as servants / dispatchers, like in other language mappings.

- Added a SwiftPM plugin, `CompileSlice`, that lets you compile Slice files as part of SwiftPM and Xcode builds.

## DataStorm Changes

- The DataStorm publisher/subscriber framework has been integrated into the Ice distribution, and is no longer a
  separate product.

- Added support for running DataStorm callbacks using a custom executor. The executor can be set using the
  `customExecutor` parameter of the Node's constructor.

- Added `DataStorm.Node.Name` property to configure the name of a DataStorm node. The node names must be unique within
  a DataStorm deployment.

- Fixed a bug in DataStorm that can result in unexpected samples received after a session was recovered after
  disconnection.

- Fixed a bug in filter initialization that can result in segmentation fault when using a key or a sample filter.

## Glacier2 Changes

- Removed buffered mode. As a result, Glacier2 has now a single mode, the previous "unbuffered" mode.

- Removed support for request overrides (the `_ovrd` request context).

- Removed support for creating batches of requests (`Glacier2.Client.AlwaysBatch` and `Glacier2.Server.AlwayBatch`).

- Removed Glacier2 helper classes.

## IceGrid Changes

- Removed deprecated server and application distributions in IceGrid. These distributions relied on the IcePatch2
service.

## IcePatch2 Changes

- The IcePatch2 service was removed.

[IceRPC]: https://github.com/icerpc
[Slice/print]: cpp/test/Slice/print
