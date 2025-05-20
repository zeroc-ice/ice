# Ice for C-Sharp

[Examples] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for C# is the C# (and .NET) implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

#pragma once

module VisitorCenter
{
    /// Represents a simple greeter.
    interface Greeter
    {
        /// Creates a personalized greeting.
        /// @param name The name of the person to greet.
        /// @return The greeting.
        ["cs:identifier:Greet"] // We prefer PascalCase for C# methods.
        string greet(string name);
    }
}
```

```csharp
// Client application

using VisitorCenter;

await using Ice.Communicator communicator = Ice.Util.initialize(ref args);

GreeterPrx greeter = GreeterPrxHelper.createProxy(
    communicator,
    "greeter:tcp -h localhost -p 4061");

string greeting = await greeter.GreetAsync(Environment.UserName);
Console.WriteLine(greeting);
```

```csharp
// Server application

await using Ice.Communicator communicator = Ice.Util.initialize(ref args);

Ice.ObjectAdapter adapter =
    communicator.createObjectAdapterWithEndpoints("GreeterAdapter", "tcp -p 4061");

adapter.add(new Server.Chatbot(), new Ice.Identity { name = "greeter" });

adapter.activate();
Console.WriteLine("Listening on port 4061...");

Console.CancelKeyPress += (sender, eventArgs) =>
{
    eventArgs.Cancel = true;
    Console.WriteLine("Caught Ctrl+C, shutting down...");
    communicator.shutdown();
};

await communicator.shutdownCompleted;
```

```csharp
// Greeter implementation

using VisitorCenter;

namespace Server;

internal class Chatbot : GreeterDisp_
{
    public override string Greet(string name, Ice.Current current)
    {
        Console.WriteLine($"Dispatching greet request {{ name = '{name}' }}");
        return $"Hello, {name}!";
    }
}
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/csharp
[Documentation]: https://doc.zeroc.com/ice/3.7
[API Reference]: https://code.zeroc.com/ice/main/api/csharp/index.html
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
