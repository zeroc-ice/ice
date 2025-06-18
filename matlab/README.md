# Ice for MATLAB

[Examples] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for MATLAB is the MATLAB implementation of the Ice framework. As of this version, it
supports only clients: you need to implement the server portion of your application in
another programming language such as C++, C# or Java.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

#pragma once

["matlab:identifier:visitorcenter"]
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

```matlab
// Client application (client.m)
function client(args)
    if nargin == 0
        args = {};
    end

    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto);
    end

    communicator = Ice.Communicator(args);
    cleanup = onCleanup(@() communicator.destroy());

    greeter = visitorcenter.GreeterPrx(communicator, 'greeter:tcp -h localhost -p 4061');
    fprintf('%s\n', greeting);
end
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/matlab
[Documentation]: https://doc.zeroc.com/ice/3.7
[API Reference]: https://code.zeroc.com
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
