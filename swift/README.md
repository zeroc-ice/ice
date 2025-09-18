# Ice for Swift

[Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server
deployment, and more.

Ice for Swift is the Swift implementation of the Ice framework.

## Usage

To add Ice to your project, add the following URL as a dependency in your Package.swift:

```swift
dependencies: [.package(url: "https://github.com/zeroc-ice/ice-swift-nightly.git", branch: "main")],
```

Next, add the products you need to your target's dependencies:

```swift
targets: [
    .executableTarget(
        name: "MyTarget",
        dependencies: [.product(name: "Ice", package: "ice-swift-nightly")],
    ),
 ]
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
    plugins: [.plugin(name: "CompileSlice", package: "ice-swift-nightly"),
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
// Slice definitions (Greeter.ice)

module VisitorCenter
{
    /// Represents a simple greeter.
    interface Greeter
    {
        /// Creates a personalized greeting.
        /// @param name The name of the person to greet.
        /// @return The greeting.
        string greet(string name);
    }
}
```

```swift
// Client application

import Foundation
import Ice

var args = CommandLine.arguments
let communicator = try Ice.initialize(&args)

defer {
    communicator.destroy()
}

let greeter = try makeProxy(
    communicator: communicator, proxyString: "greeter:tcp -h localhost -p 4061",
    type: GreeterPrx.self)

let greeting = try await greeter.greet(NSUserName())

print(greeting)
```

```swift
// Server application

import Ice

let ctrlCHandler = CtrlCHandler()

var args = CommandLine.arguments
let communicator = try Ice.initialize(&args)

defer {
    communicator.destroy()
}

let adapter = try communicator.createObjectAdapterWithEndpoints(
    name: "GreeterAdapter", endpoints: "tcp -p 4061")

try adapter.add(servant: Chatbot(), id: Ice.Identity(name: "greeter"))

try adapter.activate()
print("Listening on port 4061...")

ctrlCHandler.setCallback { signal in
    print("Caught signal \(signal), shutting down...")
    communicator.shutdown()
}

await communicator.shutdownCompleted()
```

```swift
// Greeter implementation

import Ice

struct Chatbot: Greeter {
    func greet(name: String, current _: Ice.Current) -> String {
        print("Dispatching greet request { name = '\(name)' }")
        return "Hello, \(name)!"
    }
}
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/swift
[Documentation]: https://docs.zeroc.com/ice/latest/swift/
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
