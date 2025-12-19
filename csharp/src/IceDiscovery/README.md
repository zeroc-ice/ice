# IceDiscovery Plug-in

IceDiscovery is an Ice location service based on UDP multicast. The ZeroC.IceDiscovery NuGet package provides the
IceDiscovery plug-in for C#.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs]

## Sample Code

```csharp
// Configure the communicator to load the IceDiscovery plug-in during initialization.
// This plug-in installs a default locator on the communicator.
var initData = new Ice.InitializationData
{
    properties = new Ice.Properties(ref args),
    pluginFactories = [new IceDiscovery.PluginFactory()]
};

// Create an Ice communicator.
await using var communicator = new Ice.Communicator(initData);
```

[docs]:https://docs.zeroc.com/ice/latest/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/main/csharp
[package]: https://www.nuget.org/packages/ZeroC.IceDiscovery
[source]: https://github.com/zeroc-ice/ice/tree/main/csharp/src/IceDiscovery
