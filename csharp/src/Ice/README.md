# Ice for C\#

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server
deployment, and more.

Ice for C# is the C# (and .NET) implementation of the Ice framework. The ZeroC.Ice NuGet package provides the Ice
assembly.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs] | [API reference][api]

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
        ["cs:identifier:Greet"] // We prefer PascalCase for C# methods.
        string greet(string name);
    }
}
```

```csharp
// Client application

using VisitorCenter;

await using var communicator = new Ice.Communicator(ref args);

GreeterPrx greeter = GreeterPrxHelper.createProxy(
    communicator,
    "greeter:tcp -h localhost -p 4061");

string greeting = await greeter.GreetAsync(Environment.UserName);
Console.WriteLine(greeting);
```

```csharp
// Server application

await using var communicator = new Ice.Communicator(ref args);

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

[api]: https://code.zeroc.com/ice/3.8/api/csharp/api/Ice.html
[docs]: https://docs.zeroc.com/ice/latest/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/3.8/csharp
[package]: https://www.nuget.org/packages/ZeroC.Ice
[source]: https://github.com/zeroc-ice/ice/tree/3.8/csharp/src/Ice
[Ice framework]: https://github.com/zeroc-ice/ice
