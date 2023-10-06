# Ice for Java Compat

[Getting started] | [Examples] | [Maven packages] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications, including RPC, pub/sub, server deployment, and more.

Ice for Java Compat is the Ice Java implementation that is backward-compatible with prior Ice releases.

## Sample Code

```slice
// Slice definitions (Hello.ice)

#pragma once

module Demo
{
    interface Hello
    {
        void sayHello();
    }
}
```

```java
// Client implementation (Client.java)

import Demo.*;

public class Client
{
    public static void main(String[] args)
    {
        try(Ice.Communicator com = Ice.Util.initialize(args))
        {
            HelloPrx hello = HelloPrxHelper.checkedCast(
                com.stringToProxy("hello:default -h localhost -p 10000"));
            hello.sayHello();
        }
    }
}
```

```java
// Server implementation (Server.java)

public class Server
{
    static class ShutdownHook extends Thread
    {
        @Override
        public void run()
        {
            _communicator.destroy();
        }

        ShutdownHook(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        private final Ice.Communicator _communicator;
    }

    public static void main(String[] args)
    {
        try(Ice.Communicator communicator = Ice.Util.initialize(args))
        {
            // Install shutdown hook to (also) destroy communicator during JVM shutdown.
            // This ensures the communicator gets destroyed when the user interrupts the
            // application with Ctrl-C.
            Runtime.getRuntime().addShutdownHook(new ShutdownHook(communicator));

            Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(
                "Hello",
                "default -h localhost -p 10000");
            adapter.add(new Printer(), Ice.Util.stringToIdentity("hello"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
```

```java
// Printer implementation (Printer.java)

import Demo.*;

public class Printer extends _HelloDisp
{
    @Override
    public void sayHello(Ice.Current current)
    {
        System.out.println("Hello World!");
    }
}
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-java-compat
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/java-compat
[Maven packages]: https://central.sonatype.com/namespace/com.zeroc
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/java-compat/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
