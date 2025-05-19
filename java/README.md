# Ice for Java

[Examples] | [Ice Manual] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Java is the Java implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Hello.ice)

#pragma once

["java:identifier:com.example.visitorcenter"]
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

```java
// Client implementation (Client.java)

import com.example.visitorcenter.GreeterPrx;
import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Util;

class Client {
    public static void main(String[] args) {
        try (Communicator communicator = Util.initialize(args)) {
            GreeterPrx greeter =
                GreeterPrx.createProxy(communicator, "greeter:tcp -h localhost -p 4061");

            String greeting = greeter.greet(System.getProperty("user.name"));
            System.out.println(greeting);
        }
    }
}
```

```java
// Server implementation (Server.java)

import com.zeroc.Ice.*;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

class Server {
    public static void main(String[] args) {
        try (Communicator communicator = Util.initialize(args)) {
            ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints("GreeterAdapter", "tcp -p 4061");
            adapter.add(new Chatbot(), new Identity("greeter", ""));

            adapter.activate();
            System.out.println("Listening on port 4061...");

            communicator.waitForShutdown();
        }
    }
}

```

```java
// Greeter implementation (Chatbot.java)

import com.example.visitorcenter.Greeter;
import com.zeroc.Ice.Current;

class Chatbot implements Greeter
{
    public String greet(String name, Current current) {
        System.out.println("Dispatching greet request { name = '" + name + "' }");
        return "Hello, " + name + "!";
    }
}``

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/java
[Ice Manual]: https://doc.zeroc.com/ice/3.7
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
