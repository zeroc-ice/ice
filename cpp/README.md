# Ice for C++

[Examples] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for C++ is the C++ implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

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

```cpp
// Client application (Client.cpp)

#include "Greeter.h"

#include <Ice/Ice.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    Ice::CommunicatorPtr communicator = Ice::initialize(argc, argv);
    Ice::CommunicatorHolder communicatorHolder{communicator};

    VisitorCenter::GreeterPrx greeter{communicator, "greeter:tcp -h localhost -p 4061"};

    string greeting = greeter->greet("alice");
    cout << greeting << endl;
    return 0;
}
```

```cpp
// Server application (Server.cpp)

#include "Chatbot.h"

#include <Ice/Ice.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    Ice::CtrlCHandler ctrlCHandler;

    Ice::CommunicatorPtr communicator = Ice::initialize(argc, argv);
    Ice::CommunicatorHolder communicatorHolder{communicator};

    auto adapter =
        communicator->createObjectAdapterWithEndpoints("GreeterAdapter", "tcp -p 4061");
    adapter->add(make_shared<Server::Chatbot>(), Ice::Identity{"greeter"});

    adapter->activate();
    cout << "Listening on port 4061..." << endl;

    ctrlCHandler.setCallback(
        [communicator](int signal)
        {
            cout << "Caught signal " << signal << ", shutting down..." << endl;
            communicator->shutdown();
        });

    communicator->waitForShutdown();
    return 0;
}
```

```cpp
// Greeter implementation (Chatbot.h)

#include "Greeter.h"

#include <iostream>
#include <sstream>

namespace Server
{
    class Chatbot : public VisitorCenter::Greeter
    {
    public:
        std::string greet(std::string name, const Ice::Current&) override
        {
            std::cout
                << "Dispatching greet request { name = '" << name << "' }"
                << std::endl;

            std::ostringstream os;
            os << "Hello, " << name << "!";
            return os.str();
        }
    };
}
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/cpp
[Documentation]: https://docs.zeroc.com/ice/latest/cpp/
[API Reference]: https://code.zeroc.com/ice/main/api/cpp/index.html
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
