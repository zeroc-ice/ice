# IceBox

IceBox allows you to develop Ice servers as collections of services that can be dynamically loaded by the
[IceBox server] through configuration.

The ZeroC.IceBox NuGet package provides the IceBox assembly required to develop IceBox services. It also includes the
Slice definitions that define the `IceBox::ServiceManager` interface, used to remotely administer an IceBox server. The
generated code for these definitions is included in the IceBox assembly.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs] | [API reference][api]

[api]: https://code.zeroc.com/ice/3.8/api/csharp/api/IceBox.html
[docs]:https://docs.zeroc.com/ice/3.8/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/3.8/csharp
[package]: https://www.nuget.org/packages/ZeroC.IceBox
[IceBox server]: https://www.nuget.org/packages/iceboxnet
[source]: https://github.com/zeroc-ice/ice/tree/3.8/csharp/src/IceBox
