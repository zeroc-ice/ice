# Ice for Python

[Examples] | [Ice Manual] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Python is the Python implementation of the Ice framework.

## Sample Code

```slice
// Slice definitions (Greeter.ice)

#pragma once

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

```python
# Client application (main.py)
import Ice
import asyncio
import getpass
import sys

import VisitorCenter

async def main():
    async with Ice.initialize(sys.argv, eventLoop=asyncio.get_running_loop()) as communicator:
        greeter = VisitorCenter.GreeterPrx(communicator, "greeter:tcp -h localhost -p 4061")
        greeting = await greeter.greetAsync(getpass.getuser())
        print(greeting)
```

```python
# Server application (main.py)

import Ice
import chatbot
import sys

def main():
    with Ice.initialize(sys.argv) as communicator:
        adapter = communicator.createObjectAdapterWithEndpoints("GreeterAdapter", "tcp -p 4061")
        adapter.add(chatbot.Chatbot(), Ice.Identity(name="greeter"))

        adapter.activate()
        print("Listening on port 4061...")

        try:
            communicator.waitForShutdown()
        except KeyboardInterrupt:
            print("Caught Ctrl+C, exiting...")
```

```python
# Greeter implementation (chatbot.py)
import VisitorCenter

class Chatbot(VisitorCenter.Greeter):
    def greet(self, name: str, current: Ice.Current) -> str:
        print(f"Dispatching greet request {{ name = '{name}' }}")
        return f"Hello, {name}!"
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/python
[Ice Manual]: https://doc.zeroc.com/ice/3.7
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
