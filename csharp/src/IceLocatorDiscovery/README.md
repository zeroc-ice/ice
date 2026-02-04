# IceLocatorDiscovery Plug-in

IceLocatorDiscovery is an Ice location service based on UDP multicast that allows Ice applications to automatically
discover and configure locators. The ZeroC.IceLocatorDiscovery NuGet package provides the IceLocatorDiscovery plug-in
for C#.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs]

```csharp
// Configure the communicator to load the IceLocatorDiscovery plug-in during
// initialization.
var initData = new Ice.InitializationData
{
    properties = new Ice.Properties(ref args),
    pluginFactories = [new IceLocatorDiscovery.PluginFactory()]
};

await using var communicator = new Ice.Communicator(initData);
```

[docs]:https://docs.zeroc.com/ice/latest/csharp
[examples]: https://github.com/zeroc-ice/ice-demos/tree/main/csharp
[package]: https://www.nuget.org/packages/ZeroC.IceLocatorDiscovery
[source]: https://github.com/zeroc-ice/ice/tree/main/csharp/src/IceLocatorDiscovery
