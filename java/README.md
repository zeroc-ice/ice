# Ice for Java

[Examples] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Java is the Java implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

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

class Client {
    public static void main(String[] args) {
        try (var communicator = new Communicator(args)) {
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

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;

class Server {
    public static void main(String[] args) {
        try (var communicator = new Communicator(args)) {
            ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("GreeterAdapter", "tcp -p 4061");
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

class Chatbot implements Greeter {
    public String greet(String name, Current current) {
        System.out.println("Dispatching greet request { name = '" + name + "' }");
        return "Hello, " + name + "!";
    }
}
```

## Protocol Compression

Ice for Java implements protocol compression with the bzip2 classes of [Apache Commons Compress][commons-compress].
Ice loads these classes reflectively at run time, and the Ice JAR does not declare a dependency on this library. To
enable compression, add `org.apache.commons:commons-compress` to your application's runtime class path, for example
with Gradle:

```groovy
dependencies {
    runtimeOnly "org.apache.commons:commons-compress:1.28.0"
}
```

When Ice cannot find these classes, it sends all messages uncompressed and throws an exception when it receives a
compressed message.

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/java
[Documentation]: https://docs.zeroc.com/ice/latest/java
[API Reference]: https://code.zeroc.com/ice/main/api/java/index.html
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
[commons-compress]: https://commons.apache.org/proper/commons-compress/
