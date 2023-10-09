# Ice for Java

[Getting started] | [Examples] | [Maven packages] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Java is the Java implementation of the Ice framework. It includes the latest
Slice-to-Java mapping introduced in Ice 3.7.

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

import com.zeroc.Ice.*;
import Demo.*;

public class Client
{
    public static void main(String[] args)
    {
        try(Communicator communicator = Util.initialize(args))
        {
            HelloPrx hello = HelloPrx.checkedCast(
                communicator.stringToProxy("hello:default -h localhost -p 10000"));
            hello.sayHello();
        }
    }
}
```

```java
// Server implementation (Server.java)

import com.zeroc.Ice.*;

public class Server
{
    public static void main(String[] args)
    {
        try(Communicator communicator = Util.initialize(args))
        {
            // Install a shutdown hook to ensure the communicator gets shut down when
            // the user interrupts the application with Ctrl-C.
            Runtime.getRuntime().addShutdownHook(new Thread(() -> communicator.shutdown()));
            ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(
                "Hello",
                "default -h localhost -p 10000");
            adapter.add(new Printer(), Util.stringToIdentity("hello"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
```

```java
// Printer implementation (Printer.java)

import Demo.*;

public class Printer implements Hello
{
    @Override
    public void sayHello(com.zeroc.Ice.Current current)
    {
        System.out.println("Hello World!");
    }
}
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-java
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/java
[Maven packages]: https://central.sonatype.com/namespace/com.zeroc
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/java/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
