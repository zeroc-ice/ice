# Ice for Java Compat

[Getting started] | [Examples] | [Maven packages] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Java Compat is a Java implementation of the Ice framework that is largely source
compatible with Ice for Java 3.6 and prior releases.

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
import Ice.*;

public class Client
{
    public static void main(String[] args)
    {
        try(Communicator com = Util.initialize(args))
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

import Ice.*;

public class Server
{
    public static void main(String[] args)
    {
        try(Communicator communicator = Util.initialize(args))
        {
            // Install a shutdown hook to ensure the communicator gets shut down when
            // the user interrupts the application with Ctrl-C.
            Runtime.getRuntime().addShutdownHook(new ShutdownHook(communicator));

            ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(
                "Hello",
                "default -h localhost -p 10000");
            adapter.add(new Printer(), Util.stringToIdentity("hello"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }

    static class ShutdownHook extends Thread
    {
        @Override
        public void run()
        {
            _communicator.shutdown();
        }

        ShutdownHook(Communicator communicator)
        {
            _communicator = communicator;
        }

        private final Communicator _communicator;
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
