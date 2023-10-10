# Ice for .NET

[Getting started] | [Examples] | [NuGet package] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server deployment, and more.

Ice for .NET is the C# / .NET implementation of the Ice framework.

## Sample Code

```slice
#pragma once

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```csharp
// Client application
using(var communicator = Ice.Util.initialize(ref args))
var hello = HelloPrxHelper.checkedCast(
    communicator.stringToProxy("hello:default -h localhost -p 10000"));
hello.sayHello();
```

```csharp
// Server application
using(var communicator = Ice.Util.initialize(ref args))

// Shut down the communicator on Ctrl+C or Ctrl+Break.
Console.CancelKeyPress += (sender, eventArgs) =>
{
    eventArgs.Cancel = true;
    communicator.shutdown();
};

var adapter = communicator.createObjectAdapterWithEndpoints(
    "Hello",
    "default -h localhost -p 10000");
adapter.add(new Printer(), Ice.Util.stringToIdentity("hello"));
adapter.activate();
communicator.waitForShutdown();

public class Printer : HelloDisp_
{
    /// <summary>Prints a message to the standard output.</summary>
    public override void sayHello(Ice.Current current)
    {
        Console.WriteLine("Hello World!");
    }
}
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-c-sharp
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/csharp
[NuGet package]: https://www.nuget.org/packages/zeroc.ice.net
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/csharp/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
