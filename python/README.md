# Ice for Python

[Getting started] | [Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Python is the Python implementation of the Ice framework.

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

```python
// Client application (client.py)
import sys
import Ice

Ice.loadSlice('Hello.ice')
import Demo

# Ice.initialize returns an initialized Ice communicator, the communicator is destroyed
# once it goes out of scope.
with Ice.initialize(sys.argv) as communicator:
    hello = Demo.HelloPrx.checkedCast(
        communicator.stringToProxy("hello:default -h localhost -p 10000"))
    hello.sayHello()
```

```python
// Server application (server.py)

import signal
import sys
import Ice

Ice.loadSlice('Hello.ice')
import Demo

class Printer(Demo.Hello):
    def sayHello(self, current):
        print("Hello World!")

# Ice.initialize returns an initialized Ice communicator, the communicator is destroyed
# once it goes out of scope.
with Ice.initialize(sys.argv) as communicator:

    # Install a signal handler to shutdown the communicator on Ctrl-C
    signal.signal(signal.SIGINT, lambda signum, frame: communicator.shutdown())
    if hasattr(signal, 'SIGBREAK'):
        signal.signal(signal.SIGBREAK, lambda signum, frame: communicator.shutdown())
    adapter = communicator.createObjectAdapterWithEndpoints("Hello", "default -h localhost -p 10000")
    adapter.add(Printer(), Ice.stringToIdentity("hello"))
    adapter.activate()
    communicator.waitForShutdown()
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-python
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/python
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/python/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
