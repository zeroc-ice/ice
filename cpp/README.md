# Ice for C++

[Getting started C++11] | [Examples C++11] | [Getting started C++98] | [Examples C++98] | [NuGet packages] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for C++ is the C++ implementation of Ice.

## Sample Code with the Ice C++11 Mapping

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

```cpp
// Client application (Client.cpp)

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

int
main(int argc, char* argv[])
{
    try
    {
        const Ice::CommunicatorHolder ich(argc, argv);
        auto hello = Ice::checkedCast<HelloPrx>(ich->stringToProxy("hello:default -h localhost -p 10000"));
        hello->sayHello();
    }
    catch(const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
```

```cpp
// Server application (Server.cpp)

#include <Ice/Ice.h>
#include <Printer.h>

using namespace std;

int main(int argc, char* argv[])
{
    try
    {
        // CtrlCHandler must be created before the communicator or any other threads
        // are started
        Ice::CtrlCHandler ctrlCHandler;

        const Ice::CommunicatorHolder ich(argc, argv);
        const auto& communicator = ich.communicator();

        ctrlCHandler.setCallback(
            [communicator](int)
            {
                communicator->shutdown();
            });

        auto adapter = communicator->createObjectAdapterWithEndpoints(
            "Hello",
            "default -h localhost -p 10000");
        adapter->add(make_shared<HelloI>(), Ice::stringToIdentity("hello"));
        adapter->activate();
        communicator->waitForShutdown();
    }
    catch(const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
```

```cpp
// Printer implementation (Printer.h)

#include <Ice/Ice.h>
#include <Hello.h>

class Printer : public Demo::Hello
{
public:
    /**
     * Prints a message to the standard output.
     **/
    virtual void sayHello(const Ice::Current&) override
    {
        std::cout << "Hello World!" << std::endl;
    }
}
```

## Sample Code with the Ice C++98 Mapping

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

```cpp
// Client application (Client.cpp)

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

int main(int argc, char* argv[])
{
    try
    {
        Ice::CommunicatorHolder ich(argc, argv);
        HelloPrx hello = HelloPrx::checkedCast(
            ich->stringToProxy("hello:default -h localhost -p 10000"));
        hello->sayHello();
    }
    catch(const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
```

```cpp
// Server application (Server.cpp)

#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

int main(int argc, char* argv[])
{
    try
    {
        Ice::CommunicatorHolder ich(argc, argv);
        Ice::ObjectAdapterPtr adapter = ich->createObjectAdapterWithEndpoints(
            "Hello",
            "default -h localhost -p 10000");
        adapter->add(new HelloI, Ice::stringToIdentity("hello"));
        adapter->activate();
        ich->waitForShutdown();
    }
    catch(const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return 1;
    }
    return 0;
}
```

```cpp
// Printer implementation (Printer.h)

#include <Ice/Ice.h>
#include <Hello.h>

class Printer : public Demo::Hello
{
public:
    /**
     * Prints a message to the standard output.
     **/
    virtual void sayHello(const Ice::Current&)
    {
        std::cout << "Hello World!" << std::endl;
    }
}
```

[Getting started C++11]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-c++-c++11
[Examples C++11]: https://github.com/zeroc-ice/ice-demos/tree/3.7/cpp11
[Getting started C++98]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-c++-c++98
[Examples C++98]: https://github.com/zeroc-ice/ice-demos/tree/3.7/cpp98
[NuGet packages]: https://www.nuget.org/packages?q=zeroc.ice.v
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/cpp/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
