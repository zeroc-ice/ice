The entries below contain brief descriptions of the changes in a release, in no particular order. Some of the entries reflect significant new additions, while others represent minor corrections. Although this list is not a comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your applications to this release, and the manual for complete details on a particular aspect of Ice.

- [Changes in Ice 3.5.1](#changes-in-ice-351)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [Java Changes](#java-changes)
  - [C# Changes](#c#-changes)
  - [PHP Changes](#php-changes)
- [Changes in Ice 3.5.0](#changes-in-ice-350)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-1)
  - [Java Changes](#java-changes-1)
  - [C# Changes](#c#-changes-1)
  - [Python Changes](#python-changes)
  - [PHP Changes](#php-changes-1)
  - [Ruby Changes](#ruby-changes)
  - [Visual Studio Add-in Changes](#visual-studio-add-in-changes)

# Changes in Ice 3.5.1

These are changes since Ice 3.5.0.

## General Changes

- Moved instructions for building a source distribution to the Ice manual.

- It's now possible to install an instrumentation observer with the communicator initialization data structure and still have the metrics administrative facet enabled. Previously, installing an observer would disable the metrics facet.

- IceGrid database replication now keeps track of a serial number to compare the slave and master databases on startup. A slave database with a more recent serial number than the master database will no longer be overwritten when the slave connects and synchronizes its database with the master. Instead, an error message will be printed on both the master and slave consoles and the slave connection will be rejected by the master.

  Upon getting this error, the IceGrid administrator can decide which database to keep and either initialize the master database with the slave database or vice-versa. A new `--initdb-from-replica=<replica>` command-line option has been added to allow initializing a registry database from another replica.

  This change is backward compatible with Ice 3.5.0. If a 3.5.0 slave connects to an Ice 3.5.1 master, or vice-versa, no check is performed and the previous behavior applies (the slave database always gets overwritten by the master database when the slave connects to the master).

- Improved IceGrid algorithm to find registry replicas on node or slave startup.

- Fixed bug in `IceGrid::Query::findAllReplicas` implementation that could cause a crash when trying to find the proxies of an empty replica group.

- Added support for SOCKSv4 proxies to the Ice core in C++, C# and Java. This allows outgoing TCP & SSL connections using IPv4 to be mediated by a network proxy service.

- Fixed bug that could cause an unexpected exception or error on connection closure when the connection is closed shortly before sending pending messages.

- Fixed the graphical IceGrid administrative tool to run and build in JVMs without JavaFX support.

- IceGrid node now initializes the process supplementary groups.

- Added man pages for Ice executable tools and commands.

- Added ability to list IceStorm topic subscribers.

- Fixed a bug in the graphical IceGrid administrative tool that caused the "Save" and "Save to File" commands to produce an empty file without reporting any errors.

- The Visual Studio Add-in now supports configuration-specific compiler options.

## C++ Changes

- Reading an optional with the `Ice::InputStream::read` method should now clear the optional if the optional isn't marshaled on the input stream.

- Fixed file descriptor leak that would occur if an object adapter listened on a UDP endpoint (one descriptor leaked per UDP endpoint).

- Fixed a bug that could cause an unexpected exception to be thrown on plug-in loading when a plug-in property specified both a path and version.

- Fixed a bug in `slice2cpp` that generated invalid C++ code for C++11 asynchronous invocations.

- Fixed a bug in `slice2cpp` that generated invalid C++ code when using `cpp:virtual` metadata.

- Fixed a bug in `slice2cpp` that generated invalid C++ code when a Slice operation has more than ten output parameters.

## Java Changes

- Fixed an SSL issue with anonymous ciphers on SLES 11 SP3 when using the IBM JDK.

- To simplify the use of optional values with the stream API, an `optionalFormat()` method is now generated in Helper classes when the `--stream` option is enabled.

- Fixed an assertion that would occur if a memory limit exception was raised while marshaling the response of an AMD callback.

## C# Changes

- To simplify the use of optional values with the stream API, an `optionalFormat()` method is now generated in Helper classes when the `--stream` option is enabled.

- Fixed an assertion that would occur if a memory limit exception was raised while marshaling the response of an AMD callback.

- Fixed a bug in `slice2cs` that caused incorrect code to be generated for a Slice structure that declares default values for its members.

## PHP Changes

- Fixed PHP namespace builds.

# Changes in Ice 3.5.0

These are the changes since Ice 3.4.2.

## General Changes

- Fixed a bug where the `CtrlCHandler` destruction would cause a segfault on some recent Linux distributions with GCC 4.7.

- IPv6 is now enabled by default. A server listening on an endpoint with the wildcard address will therefore now accept connections from both IPv4 and IPv6 clients. It will also publish endpoints with IPv6 addresses if some network interfaces are configured with IPv6 addresses. Ice will still prefer IPv4 addresses when resolving hostnames from endpoints and the result includes both types of addresses. This behavior can be changed by setting `Ice.PreferIPv6Address` to 1. In this case, Ice prefers the IPv6 address over the IPv4 address if the hostname resolves to both IPv4 and IPv6 addresses. IPv6 can be disabled by setting `Ice.IPv6` to 0.

- Fixed a bug where an erroneous connection warning could be printed on the server side if the client closed the connection before receiving the connection validation message.

- Fixed a Freeze map issue where the clear method could throw an exception if active cursors were still opened.

- Added support for `wrapInputStream` function. Unlike the `createInputStream` function, the returned input stream wraps the provided encoded data rather than copying it. While it avoids copying the data, care must be taken to not modify the data while the input stream is being used.

- The IceBox server no longer configures a service's logger if the service defines `Ice.LogFile` or `Ice.UseSyslog`.

- IceGrid should now interoperate with IceGrid node versions >= 3.2.

- Fixed issue where defining an `IceUtil::Handle` for a specialization of a generated Slice class would fail to compile because the Slice class privately inherited from `IceInternal::GCShared`.

- Fixed client side checks for encoding version. An invocation on a proxy with a compatible encoding (same major version) will now succeed even if the encoding minor version is not supported. In this case, the Ice run time uses the greatest supported encoding.

- Fixed bug where the retry of an AMI invocation could trigger an assertion.

- Improved the dispatch and remote invocation metrics classes to support additional `size` and `replySize` data members.

- Improved the 1.1 encoding to further reduce the size of encoded objects. Objects are no longer encoded at the end of the encapsulation. Instead, an object is encoded as soon as it's written and it has not already been encoded within the encapsulation. IMPORTANT: This change implies that the 1.1 class encoding from Ice 3.5.0 is incompatible with the encoding from the 3.5 beta version.

- Added support for Slice class compact IDs. A compact ID is defined as a numeric value in the Slice class definition. For example:
```
// Slice
class Compact(1)
{
    string v;
};
```
  When using the 1.1 encoding, Ice will marshal this numeric value instead of the string type ID of the class.

- Fixed bugs in the Slice parser related to optional values:

    - The parser was too strict when validating tags of optional data members. It is legal to reuse the tag of an inherited data member but the parser treated it as an error.

    - The parser did not detect duplicate tags among the optional parameters in an operation.

- The properties `IcePatch2.ChunkSize`, `IcePatch2.Directory`, `IcePatch2.Remove` and `IcePatch2.Thorough` have been deprecated for IcePatch2 clients. The replacements are `IcePatch2Client.ChunkSize`, `IcePatch2Client.Directory`, `IcePatch2Client.Remove` and `IcePatch2Client.Thorough`, respectively.

- The `Ice::ConnectionInfo` class supports an additional `connectionId` field. This ID matches the connection ID set on a proxy with the `ice_connectionId` proxy method.

- The `Ice::UDPEndpointInfo` class no longer supports encoding` and protocol version members. These members have been deprecated with the 1.1 encoding.

- Added support for TLS 1.1 and TLS 1.2 in IceSSL for C++ and Java.

- IceGrid now supports a new `proxy-options` attribute on the `<adapter>`, `<replica-group>`, `<object>` and `<allocatable>` XML descriptor elements. This attribute allows to specify the proxy options to use for well-known objects or allocatable objects. For adapters, it also generates the object adapter `ProxyOptions` property in the server or service configuration file. This change requires an IceGrid database upgrade.

- Improved Glacier2 permission verifiers to permit throwing `Glacier2::PermissionDeniedException` exceptions. Glacier2 now forwards permission denied exceptions to the client.

- The Slice compilers now define the `ICE_VERSION` preprocessor macro so that you can use the Ice version to conditionally compile your Slice definitions.

- Fixed bug where the UDP client endpoint `--interface` option would be ignored on some platforms (Windows and macOS).
IceGrid now enables active connection management for its object adapters to allow client connections to be closed automatically when inactive.

- Improved `Ice.Trace.Network` tracing for transport acceptors. The tracing now shows when the object adapter starts listening for incoming connections and when it starts accepting them.

- Improved IceStorm batch subscriber sending to wait for the batch to be sent before sending another batch.

- Added a new "compact" encoding for Slice classes and exceptions. This format significantly reduces the "on-the-wire" size of class and exception instances, but sacrifices the ability for the receiver to "slice" an unknown derived type to a known type. The compact format is now the default encoding. Applications can selectively enable the traditional "sliced" format using metadata and the property `Ice.Default.SlicedFormat`.

- When using the "sliced" format for Slice classes and exceptions, Ice now has the ability to preserve the slices of unknown derived types and forward the original value intact. This allows an intermediary without knowledge of all derived types to forward a value while retaining the slices of unknown types. Types must be annotated with the "preserve-slice" metadata to support this feature.

- Added support for optional data members in classes and exceptions, and optional parameters in operations.
The changes necessary to support the compact encoding, preserved slices, and optional members and parameters required a revision to the Ice encoding. The default encoding version is now 1.1. Ice remains fully backward-compatible with existing applications that use version 1.0 of the Ice encoding, however these new features cannot be used in mixed environments.

- Slice enumerators can now be assigned arbitrary values:
```
// Slice
enum Color
{
    red = 1,
    green, // Value is 2
    blue = 9
};
```

- IceGrid and the IceGrid administrative clients have been enhanced to support updating server configuration properties at run time without requiring the servers to be restarted.

- Added support for updating Ice properties at run time using the Ice administrative "Properties" facet. The facet also allows user code to register a callback to be notified of the updates.

- Fixed a bug where batched requests could cause invalid `Ice::UnmarshalOutOfBoundsException` exceptions if a request from the batch failed because no servants could be found to dispatch the request.

- Fixed issue where IceGrid round-robin load balancing policy returned the same adapter endpoints multiple times if some IceGrid nodes were down.

- Fixed a bug where, under certain circumstances, Ice could assert when a connection was being closed.

- Improved server disable to prevent the IceGrid locator from returning endpoints of disabled servers.

- Fixed a per-session memory leak in Glacier2 that occurred when request buffering was enabled.

- Fixed IceStorm bug that could cause a crash when using the Freeze database backend and multiple threads were trying to write to the database.

- Fixed minor performance issue with bi-directional proxies.

- Added support for replica and node name verification when securing the IceGrid node and registries. The names are matched against the certificate common name. The connection is rejected if the certificate CN does not match the replica or node name. The properties `IceGrid.Registry.RequireNodeCertCN` and `IceGrid.Registry.RequireReplicaCertCN` must be set to 1 to enable this verification.

- Fixed IceGrid bug where the server deactivation timeout would be ignored if the server was not deactivated using the IceGrid administrative facility.

- Fixed IceSSL bug in all language mappings that could cause a server to crash if a failure occurred during connection establishment and network tracing was enabled.

## C++ Changes

- `operator==` and `operator<` are now only generated by default for Slice structures that can be used as dictionary keys. For other structures you will need to add the metadata `["cpp:comparable"]` if you need these operators. For example:
```
["cpp:comparable"]
struct MyStruct {
    Ice::StringSeq ids;
};
```

- Added support for a zero-copy `Ice::OutputStream::finished()` method. If supported by the stream implementation, the memory returned by this method points to the output stream's internal buffer. Special care must be taken to ensure the memory is not accessed after the de-allocation of the output stream object.

- Added support for zero-copy on x86_64 processors when using the sequence C++ array mapping.

- Added support for WinRT.

- Added new AMI mapping with support for C++11 lambda functions. This mapping is only available on platforms supporting the new C++11 features.

- Fixed a Windows bug where a connection failure would be reported with `Ice::SocketException` instead of `Ice:ConnectFailedException`.

- Added the property `IceSSL.InitOpenSSL`, which allows an application to disable the OpenSSL startup and shutdown activities in the IceSSL plug-in.

- Fixed a bug that could cause an assertion failure when flushing batch requests.

- Added support for clang compiler on macOS.

- Fixed issue where a connection to a remote address would always fail with an assertion on Windows 8.

- Declaring `ice_print` for local exceptions is now optional. This method is not declared by default, but you can use the metadata tag `["cpp:ice_print"]` to enable the method declaration.

- The `slice2cpp --depend` option now includes the generated header file in Makefile dependencies.

- Deprecated `ice_getHash`. Users should create their own hash implementations when needed.

## Java Changes

- Added support for the `Ice.UseApplicationClassLoader` configuration property. When set to 1, the Ice core will set the thread context class loader to the class loader of the servant or AMI callback before dispatching the call.

- Fixed a Freeze map index marshaling bug. The encoding of the key would write an additional byte if the map value used classes. This bug fix requires that you recreate your indices if your Freeze map value uses classes.

- Fixed a bug in IceSSL that could cause delays due to DNS lookups.

- Fixed a bug in the generated code for a Slice exception with no data members: constructors are now generated correctly.

- Fixed an IceSSL bug in which the `IceSSL.Alias` property was not working correctly.

- The mapping for Slice classes, exceptions and structures now defines a `serialVersionUID` member. You can override the default UID using the metadata `[java:serialVersionUID:<value>]`.

- Ice services are now built in separate JAR files.

## C# Changes

- Fixed potential leak which could eventually result in an `OutOfMemoryException` for applications creating proxies on a regular basis.

- Added the properties `IceSSL.KeySet` and `IceSSL.PersistKeySet` to control how and where to import private keys of X.509 certificates.

- Changed the marshaling code for sequences of user-defined types that are mapped to `System.Collections.Generic.Stack<T>`. In previous releases the elements were marshaled in bottom-to-top order; now they are marshaled in top-to-bottom order. Stacks of primitive types have always been marshaled in top-to-bottom order.

- Fixed a bug that could cause an infinite loop while visiting all referenced assemblies of an executable.

- The generated constructors for an abstract Slice class now use protected visibility instead of public visibility.

- Fixed a bug in `Ice.Application` for Mono that could cause the VM to crash when a signal occurred.

- Minor marshaling/un-marshaling performance improvement (removed virtual specifier from streaming class).

- Added missing throwLocalException method to `Ice.AsyncResult` interface.

- Struct types that contains default values are mapped to a class.

## Python Changes

- Fixed a bug in the `Glacier2.Application` class where the code was using the old asynchronous API.

- Added "self" to the Slice compiler's list of Python keywords.

- Added support for Python 3.

- Fixed a bug in the `checkedCast` operation when using a non-existent facet. In previous releases this operation raised a `FacetNotExistException`; now it returns a nil proxy as in other language mappings.

- Fixed a bug that caused IceGrid activation of a Python server to hang when tracing was enabled for the server.

## PHP Changes

- SLES 11 RPMs now require PHP 5.3.

- Added compatibility with PHP 5.4.

- Fixed a bug in the `checkedCast` operation when using a non-existent facet. In previous releases this operation raised a `FacetNotExistException`; now it returns a nil proxy as in other language mappings.

- Fixed a bug in `Ice_initialize`: if an array was passed representing the command-line arguments, the function was not updating that array to remove Ice-specific options.

- It is no longer necessary to use call-time reference arguments when passing the command-line argument array to `Ice_initialize` and `Ice_createProperties`, as these functions now force their arguments to be passed by reference. For example, where you used to call a function like this:
  `$communicator = Ice_initialize(&$argv);`
  you can now use this:
  `$communicator = Ice_initialize($argv);`

## Ruby Changes

- Fixed a bug in the `checkedCast` operation when using a non-existent facet. In previous releases this operation raised a `FacetNotExistException`; now it returns a nil proxy as in other language mappings.

## Visual Studio Add-in Changes

- Fixed a bug with C++ projects that build static libraries.

- Fixed a bug that could cause Visual Studio to hang while opening or building solutions containing a large number of projects.

- Fixed a bug during project cleaning in which an exception was thrown when trying to remove generated files and the user is not authorized to remove the files.

- Fixed a bug with solutions that contain unloaded projects.
