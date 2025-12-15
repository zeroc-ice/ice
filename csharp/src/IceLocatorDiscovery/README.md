# IceLocatorDiscovery

IceLocatorDiscovery provides an Ice location service based on UDP multicast that allows Ice applications to automatically discover and configure locators.
The ZeroC.IceLocatorDiscovery NuGet package includes the IceLocatorDiscovery plugin for Ice for C#.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs]

```csharp
// Configure the communicator to load the IceLocatorDiscovery plug-in during initialization.
var initData = new Ice.InitializationData
{
    properties = new Ice.Properties(ref args),
    pluginFactories = [new IceLocatorDiscovery.PluginFactory()]
};

await using var communicator = new Ice.Communicator(initData);
```

[docs]:https://docs.zeroc.com/ice/3.8/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/3.8/csharp
[package]: https://www.nuget.org/packages/ZeroC.IceLocatorDiscovery
[source]: https://github.com/zeroc-ice/ice/tree/3.8/csharp/src/IceLocatorDiscovery
