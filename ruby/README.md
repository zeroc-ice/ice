# Ice for Ruby

[Examples] | [Documentation] | [API Reference] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Ruby is the Ruby implementation of the Ice framework. As of this version, it
supports only clients: you need to implement the server portion of your application in
another programming language such as C++, C# or Java.

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

```ruby
# Client application (client.rb)
require 'etc'
require 'Ice'
require_relative 'Greeter.rb'

Ice::initialize(ARGV) do |communicator|
    greeter = VisitorCenter::GreeterPrx.new(communicator, "greeter:tcp -h localhost -p 4061")
    greeting = greeter.greet(Etc.getlogin)
    puts greeting
end
```

[Examples]: https://github.com/zeroc-ice/ice-demos/tree/main/ruby
[Documentation]: https://doc.zeroc.com/ice/3.7
[API Reference]: https://code.zeroc.com/ice/main/api/ruby/index.html
[Building from source]: ./BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
