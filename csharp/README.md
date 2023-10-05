# Ice for .NET

[Getting started] | [Examples] | [NuGet packages] | [Documentation] | [Building from source]

Ice is a full featured framework for creating networked applications with RPC, pub/sub, server deployment
and more.

## Sample Code

```slice
module Demo
{
    interface Hello
    {
        idempotent void sayHello(int delay);
        void shutdown();
    }
}
```

```csharp
// Client application
using var communicator = Ice.Util.initialize(ref args);
var hello = HelloPrxHelper.uncheckedCast(
    communicator.stringToProxy("hello:default -h localhost -p 10000"));
hello.sayHello();
```

```csharp
// Server application
using var communicator = Ice.Util.initialize(ref args);

// Destroy the communicator on Ctrl+C or Ctrl+Break
Console.CancelKeyPress += (sender, eventArgs) => communicator.destroy();
var adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default -h localhost -p 10000");
adapter.add(new HelloI(), Ice.Util.stringToIdentity("hello"));
adapter.activate();
communicator.waitForShutdown();
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-c-sharp
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/csharp
[NuGet packages]: https://www.nuget.org/profiles/ZeroC
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: BUILDING.md