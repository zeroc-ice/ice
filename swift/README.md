# Ice for Swift

[Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server
deployment, and more.

Ice for Swift is the Swift implementation of the Ice framework.

## Usage

To add Ice to your project, add the following URL as a dependency in your Package.swift:

```swift
dependencies: [.package(url: "https://github.com/zeroc-ice/ice.git", .upToNextMinor(from: "3.8.0"))],
```

Next, add the products you need to your target's dependencies:

```swift
targets: [
    .executableTarget(
        name: "MyTarget",
        dependencies: [.product(name: "Ice", package: "ice")],
    ),
 ]
```

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

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.8/swift
[Documentation]: https://docs.zeroc.com/ice/3.8/swift/
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
