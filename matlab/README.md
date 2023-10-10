# Ice for MATLAB

[Getting started] | [Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for MATLAB is the MATLAB implementation of the Ice framework.

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

```matlab
// Client application (client.m)
function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto);
    end

    import Demo.*

    if nargin == 0
        args = {};
    end

    try
        communicator = Ice.initialize(args);
        cleanup = onCleanup(@() communicator.destroy());
        hello = Demo.HelloPrx.checkedCast(
            communicator.stringToProxy('hello:default -h localhost -p 10000'));
        hello.sayHello();
    catch ex
        fprintf('%s\n', getReport(ex));
    end
    rmpath('generated');
end
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-matlab
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/matlab
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/matlab/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
