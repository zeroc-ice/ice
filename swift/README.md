# Ice For Swift

[Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server deployment, and more.

Ice for Swift is the swift implementation of the Ice framework.

## Usage

Ice for Swift requires Swift 5.9 or later and the Swift Package Manager (SwiftPM).

To add Ice to your project, add the dependency to your `Package.swift`:

```swift
dependencies: [
    .package(url: "https://github.com/zeroc-ice/ice", from: "3.7.8"),
],
```

Next, add the products you need to your target's dependencies:

```swift
.target(
    name: "MyTarget",
    dependencies: [
        .product(name: "Ice", package: "ice"),
        .product(name: "IceGrid", package: "ice"),
        .product(name: "IceStorm", package: "ice"),
        .product(name: "Glacier2", package: "ice"),
    ]
)
```

### CompileSlice Plugin

The `CompileSlice` plugin is a SwiftPM plugin that compiles Slice files to Swift source files. This allows
compiling Slice files as part of the SwiftPM build process, without having to manually run the `slice2swift`
compiler.

To use the plugin with with a target that relies on generated Swift files, add the plugin to the target's plugins
list:

```swift
.target(
    name: "MyTarget",
    ...,
    plugins: [
        .plugin(name: "CompileSlice", package: "ice"),
    ]
)
```

Next, create a `slice-plugin.json` file in the target's source directory. The file will contain the
configuration for the plugin, including the list of sources to compile.
All paths are relative to the target's source directory.

| Key            | Description                                                             |
| -------------- | ----------------------------------------------------------------------- |
| `sources`      | List of Slice files or directories (non-recursive) to compile.          |
| `search_paths` | List of directories to search for included Slice files. Can be omitted. |

Example `slice-plugin.json`:

```json
{
    "sources": [ "Hello.ice" ],
    "search_paths": [ "../Slice" ]
}
```

> [!NOTE]
> The `slice-plugin.json` file is discovered automatically by the plugin. It must be located in the target's source
> directory.
>
> Only one `slice-plugin.json` file is allowed per target.
>
> To avoid warnings regarding unhandled files, the `slice-plugin.json` file and corresponding Slice files (if included
> in the target source directory) should be explicitly declared as resources or excluded from the target.

## Sample Code

```slice
// Slice definitions (Hello.ice)

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```swift
// Client application
import Foundation
import Ice

do {
    let communicator = try Ice.initialize(CommandLine.arguments)
    defer {
        communicator.destroy()
    }

    let hello = try uncheckedCast(
        prx: communicator.stringToProxy("hello:default -h localhost -p 10000")!,
        type: HelloPrx.self)
    try hello.sayHello()
} catch {
    print("Error: \(error)\n")
    exit(1)
}
```

```swift
// Server application
import Foundation
import Ice

// Automatically flush stdout
setbuf(__stdoutp, nil)

struct Printer: Hello {
    func sayHello(current _: Ice.Current) throws {
        print("Hello World!")
    }
}

do {
    let communicator = try Ice.initialize(CommandLine.arguments)
    defer {
        communicator.destroy()
    }

    let adapter = try communicator.createObjectAdapterWithEndpoints(
        name: "Hello",
        endpoints: "default -h localhost -p 10000")
    try adapter.add(servant: HelloDisp(Printer()), id: Ice.stringToIdentity("hello"))
    try adapter.activate()
    communicator.waitForShutdown()
} catch {
    print("Error: \(error)\n")
    exit(1)
}
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/swift
[Documentation]: https://docs.zeroc.com/ice/latest/swift/
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
