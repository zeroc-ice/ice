# Ice For Swift

[Getting started] | [Examples] | [NuGet package] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server deployment, and more.

Ice for Swift is the swift implementation of the Ice framework.

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

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-swift
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/swift
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/swift/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice