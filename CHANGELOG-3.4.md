The entries below contain brief descriptions of the changes in a release, in no
particular order. Some of the entries reflect significant new additions, while
others represent minor corrections. Although this list is not a comprehensive
report of every change we made in a release, it does provide details on the
changes we feel Ice users might need to be aware of.

We recommend that you use the release notes as a guide for migrating your
applications to this release, and the manual for complete details on a
particular aspect of Ice.

- [Changes in Ice 3.4.2](#changes-in-ice-342)
  - [General Changes](#general-changes)
  - [C++ Changes](#c-changes)
  - [Java Changes](#java-changes)
  - [C# Changes](#c#-changes)
  - [Python Changes](#python-changes)
  - [Ruby Changes](#ruby-changes)
  - [PHP Changes](#php-changes)
  - [Visual Studio Add-in Changes](#visual-studio-add-in-changes)
- [Changes in 3.4.1](#changes-in-341)
  - [General Changes](#general-changes-1)
  - [C++ Changes](#c-changes-1)
  - [Java Changes](#java-changes-1)
  - [C# Changes](#c#-changes-1)
  - [Python Changes](#python-changes-1)
- [Changes for 3.4.0](#changes-for-340)
  - [General Changes](#general-changes-2)
  - [C++ Changes](#c-changes-2)
  - [Java Changes](#java-changes-2)
  - [C# Changes](#c#-changes-2)
  - [Python Changes](#python-changes-2)
  - [Ruby Changes](#ruby-changes-1)
  - [PHP Changes](#php-changes-1)
  - [Visual Studio Plugin Changes](#visual-studio-plugin-changes)

# Changes in Ice 3.4.2

These are changes since Ice 3.4.1.

## General Changes

- The operation mode sent over the wire for the Object operations
  `ice_ping`, `ice_isA`, `ice_ids`, and `ice_id` should be Nonmutating, but
  the language mappings were inconsistent in this respect. All
  language mappings now send the correct mode.

- Fixed a bug where under certain circumstances, Ice would
  indefinitely re-try to add a proxy to the Glacier2 routing table.

- Improved queuing of Glacier2 requests to the client to not invoke
  requests if the client connection already has requests pending for
  send. Instead, Glacier2 waits for the pending requests to be
  sent. This allow request overriding to occur more often when the
  client connection is slow.

- Fixed bug where an IceGrid node or registry could hang for a long
  time on startup if some IceGrid registry replicas weren't reachable.

- Added `throwLocalException` method to the `Ice::AsyncResult` class. If
  the request fails with a local exception, this method throws the
  local exception.

- Fixed a bug that caused a crash if a timeout occurs while sending
  an AMI request and if automatic retry is enabled (the default).

- It is now possible to use a constant as the default value for a data
  member. For example:

  ```
  const int MIN = 1;

  struct Info
  {
      int size = MIN;
  };
  ```

  It is also possible for a constant to refer to another constant:

  ```
  const int MAXIMUM_SIZE = 100;
  const int DEFAULT_SIZE = MAXIMUM_SIZE;
  ```

## C++ Changes

- Fixed a bug in the Linux epoll selector implementation that could
  cause hangs when using the thread pool serialization mode.

- Fixed a memory leak in IceSSL on Windows.

- Fixed a bug in the generated code for a Slice structure containing
  at least one data member that declares a default value. The
  constructors were not being exported correctly in this case.

- Fixed a bug where the `IceUtil::Timer` thread could die in case very
  large delays were specified.

- Removed extra `include/IceGrid/Perf.h` file.

## Java Changes

- Added support for Android.

- Added overloaded constructors for user and system exceptions that
  accept a `Throwable` argument.

- Added an `ice_staticId` method to generated `PrxHelper` classes.

- Fixed a bug in `slice2java` in which a floating point default value
  could cause a compile error.

- Fixed a bug where `Ice.ServerIdleTime` wouldn't consistently shutdown
  servers after the given period of inactivity.

## C# Changes

- Added support for the .NET Compact Framework.

- Added `demo/Ice/compact`, which is a simple graphical client that uses
  Ice for .NET Compact Framework.

- Fixed bugs in the generated code that caused naming conflicts when
  using a Slice module named `System`.

- Added an `ice_staticId` method to generated `PrxHelper` classes.

- Fixed a bug in the generated code when a Slice class uses the
  property mapping and declares default values for its members.

## Python Changes

- Fixed bug where `None` was not recognized as a keyword.

## Ruby Changes

- Fixed a bug in the Ice extension that would cause an `ArgumentError`
  to be raised when attempting to print a byte sequence whose value is
  a string that contains null bytes.

## PHP Changes

- Fixed a bug that caused `undefined symbol IcePHP__t_XXX` errors when
  `Ice.php` or a generated file is included from a non-global scope.

- `slice2php` now generates `require_once` statements corresponding to
  Slice `#include` directives, replacing the `require` statements used
  in previous releases.

- Fixed a bug in the Ice extension that caused Ice-related INI
  settings to be ignored.

## Visual Studio Add-in Changes

- Fixed a bug that prevented the Add-in from automatically compiling
  Slice files when using a non-English version of Visual Studio.

- Improved the layout and wording of the Ice Configuration dialogs.

- Added `$(IceHome)` macro to VS 2008 project configurations.

- Changed C++ project configurations to use `$(IceHome)` macro in
  configuration settings.

- Removed Ice Home field from `Ice Configuration` dialog. Projects
  now always use the Ice installation corresponding to the location
  of the add-in DLL.

- Fixed issues that prevented the use of solution folders in Ice
  projects.

- Fixed an issue that caused the add-in to not work when using
  Visual Studio 2010 and `Solution Navigator`.

- Added a check to prevent incorrect Runtime library usage in C++
  projects.

- Added support for .NET Smart Device project types used by the .NET
  Compact Framework.

- Improved add-in output messages.

- Fixed an issue that produced a deadlock in Visual Studio when
  Slice compiler output was large enough to fill the output buffer.

- Fixed support for parallel builds. The Slice compiler was run
  several times, once for each parallel build. Now the Slice
  compiler is run only once, before all parallel builds begin.

- Fixed an issue that could cause errors with Visual Studio if a
  Slice file was saved while the project was building.

- Fixed the add-in to work properly when `--header-ext` and
  `--source-ext` are present in Extra Compiler Options.

- Fixed the add-in to support building Freeze demos without the need
  to set environment variables.

- Fixed the add-in to always open generated files as read only;
  previously the generated files were not opened read only until the
  first project build.

- Added support for setting the output directory for generated files,
  making the add-in more VCS friendly.

- Added support for .NET `DEVPATH`.

- Added support for canceling changes in configuration dialogs. All
  the configuration dialogs now have `OK`, `Cancel`, and `Apply` buttons with
  the standard semantics.

- Added warnings to prevent incorrect options from being entered in
  Extra Compiler Options.

- Added a timestamp to Slice build output, which is only printed when
  the trace level is Debug.

- The add-in now supports command-line builds.

- The add-in now avoids making extraneous changes to projects that
  could result in merge issues when projects are in a shared
  development.

- The add-in no longer overrides the debug environment setting for
  `PATH`, but rather appends appropriately.

- On the loss of focus when adding an include directory in the Slice
  Include Path, the addition is now saved rather than lost.

- Fixed an issue where moving a Slice-generated file to a filter so
  that it would be ignored by source control did not always work
  because the generated files could be moved back into the regular
  Source Files list by a build.

- Improved the tracing output generated by the add-in and changed
  the tracing options.

# Changes in 3.4.1

These are changes since Ice 3.4.0.

## General Changes

- Added the new property `Freeze.DbEnv.<db>.LockFile` and changed the
  `dumpdb` and `transformdb` utilities so that they can be run safely on
  a database environment that is currently open in another process.

- Changed the Glacier2 helper classes to cache the client category.

- Added the ability to use underscores in Slice identifiers. See the
  release notes for important information on this change.

- Added the `setConnectContext` method to `Glacier2.SessionFactoryHelper`,
  which allows an application to provide a request context to be used
  when creating a Glacier2 session. (Java and C#)

- Fixed IceGrid bug where node observers would not be notified when a
  server is re-enabled after it has been updated and if the server was
  disabled following an activation failure.

- Fixed a bug in the `Glacier2.SessionFactoryHelper` class (Java/C#) in
  which the `getPort()` method would return an incorrect default port if
  no port was configured.

## C++ Changes

- Fixed a bug in `slice2cpp` in which streaming code was not generated
  properly for types in nested modules.

- `slice2cpp` now generates a "one-shot" constructor for a Slice
  structure if at least one of its members has a default value.

- Fixed a bug in `slice2cpp` so that asynchronous `end_op` methods are
  now exported properly.

- Fixed a bug in `slice2cpp` that would generate invalid code for a
  Slice exception when the `--stream` option was not used.

- Fixed compatibility issues with OpenSSL 1.0.

- Fixed a bug in `slice2cpp` that would cause invalid code to be
  generated for a nested Slice definition when the `--stream` option was
  used.

- Fixed a bug in Freeze that disabled automatic log deletion.

## Java Changes

- Fixed an issue in which intensive use of Freeze or the stream API
  could cause an `OutOfMemoryError`. Note that Freeze maps must be
  regenerated with `slice2freezej` to incorporate this fix.

- Fixed a bug in the Slice2Java ant task in which duplicate Slice
  files could be passed to `slice2java`. Also fixed a bug in `slice2java`
  that caused the translator to fail if a duplicate file was passed
  on the command line.

- Fixed race condition which could cause the thread pool selector
  thread to go away (causing the thread pool to not dispatch further
  incoming messages).

- Deprecated `Ice.AsyncCallback`. To use the generic asynchronous
  callback facility, applications should derive their classes from
  `Ice.Callback`.

- The `size` method on a `Freeze.Map` and on the value of an indexed
  submap now use the current transaction associated with the
  connection, if any.

## C# Changes

- Fixed a bug in the generated code that prevented sequences from
  being marshaled using the stream API if the sequences used generic
  collection types.

- Fixed a bug in the IceSSL plug-in that could incorrectly report a
  certificate verification failure when the `IceSSL.CheckCertName`
  property is enabled.

## Python Changes

- Fixed a bug in which destroyed Communicator instances were not being
  garbage collected.

- Fixed a bug in `Ice.Application` in which the exit status was not
  properly returned from `main()`.

# Changes in 3.4.0

These are changes since Ice 3.3.1.

## General Changes

- Added ability to define default values for Slice data members. For
  example, you can write:

  ```
  class C
  {
      int i = 2;
      string name = "John";
  };
  ```

- Fixed IceGrid bug where the IceGrid node would disconnect from the
  registry if the clock was moved backward.

- Fixed hang in `slice2cs` and `slice2py` that would occur if a comment
  contained a `<` with no closing `>`.

- Fixed a bug in the Java and C# stream classes where `readString()` and
  `readBlob()` were not checking the size before allocating memory.

- Fixed source files to have consistent end of line terminators.

- Fixed the `Ice.initialize()`, `Ice.initialize(Ice.InitializationData)`
  and `Ice.createProperties()` overloads in Python, Ruby and PHP to
  match the C++, .NET and Java behavior. These overloads no longer
  read the `ICE_CONFIG` environment variable.

- Fixed IceGrid bug where updating an application could cause a
  temporary hang if a server deactivation hanged during the update.

- Added a new asynchronous method invocation (AMI) facility for C++,
  C#, Java, and Python. The previous API is now deprecated.

- Added the ability to invoke `flushBatchRequests` asynchronously on a
  communicator or connection.

- The Ice extension for Visual Studio is now included in the Ice
  distribution. The source code for the extension can be found in the
  `vsplugin` subdirectory of the source distribution. Note that the
  extension no longer supports Visual Studio 2005.

- Added a `cloneWithPrefix` operation to the `Logger` interface.

- Added a `removeServantLocator` operation to the `ObjectAdapter`
  interface.

- The translators now include a comment at the beginning of each
  generated file to indicate that the file is machine-generated.
  This is particularly useful for tools such as StyleCop.

- The IceGrid graphical administration tool now allows you to filter
  the live deployment for a particular application.

- Added the ability to query an IceGrid node to determine the number
  of CPU sockets on its host machine. Currently this only works for
  Windows Vista (or later) and native (non-VM) Linux. You can also
  manually configure the number of CPU sockets.

- Freeze now uses file locks to prevent multiple processes from
  opening the same database environment.

- Added new utility classes to simplify the use of Glacier2. The
  `Glacier2::Application` class extends `Ice::Application` to add support
  for keeping a Glacier2 session alive and automatically restarting it
  when necessary. For Java and C#, helper classes are also provided
  for use in graphical programs. See the Ice manual for more
  information.

- Added new `InputStream::readAndCheckSeqSize` method to read and check
  the size of a sequence. The check ensures the sequence size is
  consistent with the stream buffer size. This is useful to prevent
  over-allocating memory for the sequence if the input stream data
  cannot be trusted.

- When using the system logger (enabled when `Ice.UseSyslog` is set), it
  is now possible to set the facility via the `Ice.SyslogFacility`
  property. The default value is `LOG_USER`.

- It is now legal to pass an empty name to `createObjectAdapterWithRouter`
  and `createObjectAdapterWithEndpoints`. If so, a UUID will be used for
  the object adapter's name.

- Added support for dispatching servant invocations and AMI callbacks
  in application-specific threads (e.g., in a GUI-safe thread). The
  application needs to implement the `Ice::Dispatcher` interface and
  initialize the communicator appropriately to use this dispatcher
  implementation. See the Ice manual for more information.

- `icegridadmin` now clears the user-supplied password after use.

- `icegridadmin` now correctly handles EOF when reading username and
  password.

- The `Connection::createProxy` method will now create a secure
  proxy if the connection is secure and a datagram proxy if the
  connection is a datagram connection.

- Added `Communicator::proxyToProperty` which converts a proxy to a
  property set.

- Calling `Glacier2::Router::destroySession()` no longer raises an
  `Ice::ConnectionLostException` exception. Glacier2 no longer
  forcefully closes the connection of a destroyed session but instead
  allows active connection management (ACM) for client connections to
  close it (see below).

- Glacier2 has been changed to support the use of active connection
  management. Active connection management is enabled by default for
  Glacier2 client connections if `Glacier2.SessionTimeout` is set.
  The ACM timeout is set to twice the value of the session timeout. If
  the session timeout is disabled, ACM is also disabled for client
  connections.

- Added support for a per-object adapter ACM timeout configured via
  the `<adapter>.ACM property`. If not set, the property defaults to
  value of `Ice.ACM.Server` set for the adapter's communicator.

- Improved the Ice run time's invocation retry facility to always
  retry at least once on `Ice::CloseConnectionException` failures even
  if the retry limit has been reached. If retries are disabled, this
  ensures that an invocation is retried if it fails with an
  `Ice::CloseConnectionException`.

- Added `Glacier2.AddConnectionContext` property to enable forwarding
  connection information into the context of routed invocations. See
  the manual for more information. `Glacier2.AddSSLContext` has been
  deprecated, this new property should be used instead.

- Added `Ice.Override.CloseTimeout` property. This property overrides
  timeout settings used to close connections.

- Ice connections are now forcefully closed after sending a close
  connection message and once the connection timeout
  expires. Previously, the connection would only be closed when the
  object adapter or communicator was destroyed.

- The new `Ice::ThreadHookPlugin` class allows you to install thread
  notification hooks during communicator initialization via plug-ins.

- Added `Glacier2::Router::refreshSession`, which keeps the client's
  Glacier2 session alive. This is useful in the case that the
  application architecture does not use a Glacier2 session
  manager. Modified `demo/Glacier2/callback` to demonstrate the use of
  this method.

- Fixed bug in `icegridadmin` where invalid XML could cause a crash.

- Fixed a bug where calling the object adapter `waitForHold()` method
  would cause other calls on the object adapter to hang until the
  `waitForHold()` method returned.

- Improved connection establishment scalability. Creating or accepting
  a new connection should now be done in constant time regardless of
  the number of connections already established or accepted.

- Fixed Glacier2 router bug which would cause the `getCategoryForClient`
  method to throw `Ice::NullHandleException` if the server endpoints
  were not set; instead it now returns an empty string.

- `Ice.NegativeSizeException` has been removed. The run time now throws
  `MarshalException` or `UnmarshalOutOfBoundsException` instead.

- The Ice thread pool now supports receiving and sending data over Ice
  connections using multiple threads. Allowing multiple connections to
  concurrently send and receive data improves CPU usage on machines
  with multiple cores. The maximum number of threads allowed to
  concurrently send/receive data is capped by the number of available
  cores.

- The Ice run time now uses Windows completion ports and overlapped IO
  to accept, connect, and receive and send data over connections. This
  improves scalability of server applications handling many
  connections on Windows.

- Ice connections now support read timeouts. A connection will now
  eventually timeout once the connection timeout is reached if no more
  data is available for reading after the message header was received.
  ACM no longer closes a connection for which a message is being
  received.

- The `IceSSL.TrustOnly` properties support a new syntax that allows you
  to reject a peer whose distinguished name matches certain criteria.

- IceSSL now compares the host name or IP address in a proxy endpoint
  (if any) against the common name of the server's certificate when
  the property `IceSSL.CheckCertName` is enabled. This is in addition to
  the existing behavior that compared the host name or IP address
  against the DNS names and IP addresses in the server certificate's
  subject alternative name extension.

- `slice2docbook` is no longer supported and has been removed from
  the distribution.

- `slice2html` comments now use a syntax that matches the javadoc
  syntax. Instead of

      module::interface::operation

  `slice2html` now expects

      module.interface#operation

  Similarly, the link syntax now follows the javadoc syntax. Instead
  of

      [module::interface::operation]

  `slice2html` now expects

      {@link module.interface#operation}

  The old syntax is still supported; `slice2html` prints a warning for
  each source file in which it encounters the old syntax.

- A batch invocation now only throws if the connection associated
  with the proxy failed and there were batch requests queued at the
  time of the failure. If there were no batch requests queued, the
  batch invocation does not throw but instead tries to obtain a new
  connection.

- Added the ability to read properties from the Windows registry. This
  is done by setting the Ice.Config property to a value such as the
  following:

    ```
    HKLM\Software\MyCompany\IceProperties
    ```

  This will read all the string values from the specified key in
  `HKEY_LOCAL_MACHINE` and set them as Ice properties.

- `iceserviceinstall`, the Windows service installation utility, now
  supports reading its configuration settings from HKLM in the Windows
  registry.

- Added ability to get information from endpoints such as host and
  port without having to parse the stringified endpoint.

- Added ability to get addressing information from a connection.

- The `IceSSL::ConnectionInfo` type is now generated from a local Slice
  definition. You can downcast this type to `NativeConnectionInfo` if
  you require access to native certificates.

- Slice files are now allowed to be in UTF-8 format and include UTF-8
  BOM at start of file and include UTF-8 characters in comments.

- Fixed bug in Slice compilers that would cause a crash if a Slice
  file had double slashes in its path.

- The following properties were deprecated in Ice 3.2 and have been
  removed:

  `Glacier2.AddUserToAllowCategories`

  `Glacier2.AllowCategories`

  `Ice.UseEventLog`

- The following APIs were deprecated in Ice 3.2 and have been removed:

  `Communicator::setDefaultContext`

  `Communicator::getDefaultContext`

  `ObjectPrx:ice_defaultContext`

- Support for the deprecated Slice keyword `nonmutating` has been
  removed. As a result the following property has also been removed:

  `Freeze.UseNonmutating`

- HP-UX is no longer a supported platform for Ice.

- Added extra network tracing to show the list of local interfaces
  currently available when an endpoint is using a wildcard host, as
  well as the published endpoints for an object adapter. Both of these
  traces will be shown when `Ice.Trace.Network` >= 1.

- Added new property, `Ice.Trace.ThreadPool`, that when set >= 1 enables
  tracing of thread pool creation as dynamic thread pools grow and
  shrink.

- Changed the format of the default logger output. Traces are now
  prepended by '--', warnings by '-!' and errors by '!!'.

- The value of the thread pool `SizeWarn` property is now 0 by default
  rather than 80% of Size. This means that by default warnings about
  thread pool growth will now be disabled.

- Added example `demo/Ice/plugin` to show how to write a simple Ice
  plug-in as well as a Logger plug-in.

- Added `getPlugins` method to `PluginManager` which returns a list of the
  names of all installed plug-ins.

- The reason member of `MemoryLimitException` now contains the size of
  the request that caused the exception as well as the value of the
  maximum message size.

- The `Ice::Application` helper classes now use the process logger to
  emit errors rather than just writing directly to stderr.

- A Slice class that implements an interface but does not define or
  inherit any operations is now mapped to a concrete class. This
  change affects the Java, C#, Python, and Ruby language mappings.

- It is now possible to use an UNC path as an include directive for
  the Slice compilers.

- Fixed a bug in `slice2html` that caused incorrect hyperlinks to be
  generated for the summary of the index sections of a module page.

- Added a new property, `Ice.LogFile`, which causes the default Ice
  logger to write to a file rather than stderr. The file name is the
  property's value.

- Added the following operations to the `ObjectAdapter` API:

  `addDefaultServant`

  `removeDefaultServant`

  `findDefaultServant`

  These methods provide a simpler way of using default servants in
  Ice. Please refer to the manual for more information.

- Using the `--depend` option with Slice compilers that support it no
  longer outputs dependency info even if the Slice file itself
  contains errors.

## C++ Changes

- Renamed the `createIceStringConverter` function back to
  `createStringConverter` for compatibility with previous releases.

- Added a new demo named `interleaved` that shows how to use
  asynchronous invocation and dispatch to achieve maximum throughput
  with Ice.

- Fixed a bug that prevented an application from using Unicode path
  names for Freeze databases on Windows.

- Enhanced the portable streaming API to provide better support for
  template programming.

- The C++ class generated for a derived Slice class no longer uses
  virtual inheritance by default. The metadata `cpp:virtual` can be
  defined for a Slice class to force the translator to use virtual
  inheritance when necessary.

- The following functions have new overloaded versions on Windows that
  accept a Unicode argument vector:

  `Ice::Application::main`

  `Ice::Service::main`

  `Ice::argsToStringSeq`

- Added support for specifying the priority of threads in a thread
  pool. See the Ice manual for more information.

- Added new demo `book/map_filesystem`.

- `IceUtil:: Shared` now uses the native atomic functions provided by gcc
  4.1 and later on supported platforms. For details see
  http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html

- Fixed a bug in `slice2cpp` that could cause incorrect includes to be
  generated if a Slice file was included that was a symbolic link.

- On Windows it is now possible to build the Ice DLLs with unique names
  for each supported compiler. See `UNIQUE_DLL_NAMES` setting in
  `cpp/config/Make.rules.mak`.

- The Ice DLLs no longer all use the default base load address.

- Changed the signature of the `Ice::Service` start method.

- Added new static methods to initialize an `IceUtil::Time` class from
  a double:

  `IceUtil::Time::secondsDouble(double)`

  `IceUtil::Time::milliSecondsDouble(double)`

  `IceUtil::Time::microSecondsDouble(double)`

- Added the ability to use alternative database storage for IceStorm
  and IceGrid other than Freeze. The following SQL databases are
  supported: SQLite and PostgreSQL. Please see the manual for more
  information.

- Fixed a bug where globally-scoped garbage collected class pointers
  could cause a crash on application termination.

- Fixed a bug where an SSL connection was rejected because of a
  certificate validation failure even if `IceSSL.VerifyPeer=0`.

- Added `ice_stackTrace()` method to `Exception` classes, which returns
  the exception stack trace on platforms/compilers that support it.
  Currently this is only supported with the GCC compiler.

- Added new property, `Ice.PrintStackTraces`, which controls whether the
  Ice logger utility classes (`Ice::Trace`, `Ice::Warning`, etc) print the
  stack trace for Ice exceptions. By default it is enabled for debug
  builds and disabled for release builds.

- Removed `AMD_Array_Object_ice_invoke` class and instead added an
  ice_response method to `AMD_Object_ice_invoke` that uses the array
  type.

- The system logger (enabled when `Ice.UseSyslog` is set) now uses the
  value of `Ice.ProgramName` as the log identifier prefix.

- Fixed crash if `Application::main()` or Service::main() were called
  with `argc == 0`.

## Java Changes

- The Freeze map API now uses Java5 generic type conventions.

- Ice now tests for the presence of the Bzip2 classes only if an
  application attempts to use compression.

- Added new demo `book/map_filesystem`.

- Changed IceSSL to use the keystore as the truststore if no
  truststore is explicitly defined.

- Added new `ice_dispatch` method to `Object` that does not take a
  `DispatchInterceptorAsyncCallback` parameter to be used for
  synchronous dispatch only.

- Added support for using direct buffers in the transport layer to
  minimize copying. The semantics of the `Ice.CacheMessageBuffers`
  property have been extended as follows:

  0 = no buffer caching
  1 = buffer caching using non-direct buffers
  2 = buffer caching using direct buffers

  If this property is not defined, the new default value is 2.

- The Ice run time now re-throws exceptions using fillInStackTrace to
  ensure the exception stack trace includes the calling point.

- Added the `classLoader` member to `Ice.InitializationData`, which
  allows you to supply a custom class loader that Ice uses to load
  plug-ins, user exceptions, concrete Slice classes, etc.

- Fixed a bug in the implementation of `ice_invoke_async`.

- The Freeze classes are now stored in a separate JAR file named
  `Freeze.jar`.

- The translator no longer generates one-shot constructors for class,
  struct and exception types if doing so would exceed Java's limit of
  255 parameters for a method.

- Fixed a bug where an outgoing SSL connection was allowed even though
  certificate validation failed.

- A holder class generated for a Slice class or interface now extends
  the generic base class `Ice.ObjectHolderBase` and implements the
  streaming interface `Ice.ReadObjectCallback`.

- `slice2java` no longer generates a `<name>OperationsNC` interface for
  local interfaces and classes. This makes the code easier to browse
  with IDEs.

- `slice2java` now preserves javadoc comments that appear in the Slice
  source and writes them through to the corresponding APIs in the
  generated code.

- `Ice.jar` and `Freeze.jar` now include source files. This allows IDEs
  such as Eclipse to browse the Ice source code and to display javadoc
  comments.

- Fixed a bug that caused `UnmarshalOutOfBoundsException` during an
  attempt to unmarshal an exception or class if `Ice.jar` is installed
  in the JVM's extension directory.

- The following APIs are deprecated and will be removed in a future
  release.

  `Ice.Object.ice_hash()`

  `Ice.ObjectPrx.ice_getHash()`

  `Ice.ObjectPrx.ice_toString()`

- The `StackSize` thread pool configuration property is now used.

- The generated classes for Slice structs are no longer declared final.

- `Ice.Util.generateUUID` has been deprecated. Use
  `java.util.UUID.randomUUID().toString()` instead.

- Ice now supports the ability to load a configuration file as a class
  path resource. For a path name specified in the `ICE_CONFIG`
  environment variable, in the `--Ice.Config` command-line option, or
  passed to `Properties::load()`, Ice first attempts to open the file as
  a class path resource and, if that fails, tries to open it as a file
  in the local file system. See the manual for more information.

- Removed the Java2 mapping. The release notes contain instructions
  for migrating an application to the Java5 mapping.

- Changed the Slice compiler to emit `@SuppressWarning` annotations only
  when necessary.

- Fixed a discrepancy between the documented behavior of the property
  `IceSSL.CheckCertName` and its actual behavior. The documented
  behavior states that IceSSL compares the host name as it appears in
  the proxy's endpoint against the server certificate's alternative
  subject names. The actual behavior in Ice 3.3 differed slightly in
  that the host name may have undergone a translation that could
  result in IceSSL using a different host name than that of the proxy.
  For example, the proxy may have contained "127.0.0.1" but IceSSL
  used "localhost". This could cause the SSL connection attempt to
  fail if `IceSSL.CheckCertName` is enabled. IceSSL now uses the host
  name from the proxy for this validation step.

## C# Changes

- The Ice DLLs no longer all use the default base load address.

- Both Ice Plugin and IceBox Service configuration now allow you to
  enter a full path as the assembly DLL name.

- Added new `ice_dispatch` method to Object that does not take a
  `DispatchInterceptorAsyncCallback` parameter to be used for
  synchronous dispatch only.

- IceSSL's behavior with respect to the `IceSSL.CheckCertName` property
  is now consistent with that of C++ and Java. Specifically, IceSSL
  compares the host name or IP address in a proxy endpoint (if any)
  against the common name and subject alternative names of the
  server's certificate. In prior releases, IceSSL relied on .NET to
  perform the validation of the common name but no comparison was made
  with the subject alternative names.

- Fixed bug where random endpoint selection would only work for
  proxies with more than two endpoints.

- Fixed thread safety issue in random endpoint selection.

- `slice2cs` no longer generates a `<name>OperationsNC` interface for
  local interfaces and classes. This makes the code easier to browse
  with IDEs.

- `slice2cs` now converts javadoc comments in Slice files into Visual C#
  XML documentation comments. This allows you to generate
  an `<assembly>.xml` file from the documentation comments by passing
  the `/doc` option to Visual C#. The comments in the <assembly>.xml
  file are shown as Visual C# tooltips if the `<assembly>.xml` file is
  installed in the same directory as the `<assembly>.dll` file.

  The build for Ice now generates the `<assembly>.xml` file for each
  assembly and installs it, so Visual C# displays tooltips for Ice
  APIs.

- The generated types for Slice classes, interfaces, structs and
  exceptions are now declared as partial.

- The following APIs are deprecated and will be removed in a future
  release.

  `Ice.Object.ice_hash()`

  `Ice.ObjectPrx.ice_getHash()`

  `Ice.ObjectPrx.ice_toString()`

- The `StackSize` thread pool configuration property is now used.

- The default Ice logger now uses `System.Diagnostics.Trace` to
  output messages. Please see manual and Trace documentation for
  more information.

- `Ice.Util.generateUUID` has been deprecated. Use
  `System.Guid.NewGuid.ToString()` instead.

## Python Changes

- Fixed a code generation bug that could lead to undefined Slice
  types, depending on the order in which the generated code was
  imported.

- Changed the Python mapping for long integer constants so that 64-bit
  values are no longer generated as strings.

- The extension now restores the pre-existing dlopen flags.

- The `__cmp__` method generated for slice structures and enumerations
  has been replaced by the rich operators `__lt__`, `__le__`, `__eq__`,
  `__ne__`, `__gt__`, `__ge__`.

- Slice comments are now converted into Python docstrings in the
  generated code.

- Added the administrative facet API.

- Integer values are now accepted where floating point values are
  expected.

- Objects that support the number protocol are now accepted as legal
  values for Slice integer and floating-point types.

- 64-bit integers are now supported in buffer types.

## Ruby Changes

- Fixed a compilation error when using Ruby 1.8.7 (patch level >=
  248).

- The following API as been deprecated and will be removed in a
  future release.

  `Ice::ObjectPrx::ice_getHash()`

## PHP Changes

- Enumerations are now supported as dictionary key types.

- Integer values are now accepted where floating point values are
  expected.

- Added `slice2php` and removed dynamic Slice translation.

- Added support for PHP namespaces (requires PHP 5.3 or later).

- Added ability to create and destroy communicators. Use the function
  `Ice_initialize` (`\Ice\initialize`) to create a communicator. By
  default, all communicators created during a page request are
  destroyed automatically at the end of the request.

- Added ability to register a communicator for use in a subsequent
  page request. Three new functions are provided:

  - `Ice_register`   (`\Ice\register`)

  - `Ice_unregister` (`\Ice\unregister`)

  - `Ice_find`       (`\Ice\find`)

- Added a demo (in `Glacier2/hello`) that shows how to register and
  use the same communicator instance for multiple page requests in a
  PHP session.

- Added support for all Slice local exceptions.

- The INI settings are now used to configure property sets that a
  script can use to initialize a communicator. The `ice.slice` directive
  is no longer supported.

- Added the function `Ice_createProperties()`.

- Now generating minimal proxy classes to simplify downcasting.

- Removed the `$ICE` global variable.

- Removed the following methods:

  `$ICE->setProperty()`

  `$ICE->getProperty()`

- Removed the following functions:

  `Ice_stringToIdentity()`

  `Ice_identityToString()`

  `Ice_loadProfile()`

  `Ice_loadProfileWithArgs()`

  `Ice_dumpProfile()`

## Visual Studio Add-in Changes

- Added support for Visual Basic projects. For Visual Basic projects
  the plug-in only takes care of managing Ice references and not slice
  compilation.

- It is now possible to install the plug-in when `My Documents` is
  linked to a network drive.

- The plug-in will no longer modify projects for which it is not
  enabled.

- The plug-in will now compile slice files for C++ projects even if
  the project does not have `C++ Options`.
