# Ice for Ruby

[Getting started] | [Examples] | [Documentation] | [Building from source]

The [Ice framework] provides everything you need to build networked applications,
including RPC, pub/sub, server deployment, and more.

Ice for Ruby is the Ruby implementation of the Ice framework.

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

```ruby
// Client application (client.rb)
require 'Ice'

Ice::loadSlice('Hello.ice')

Ice::initialize(ARGV) do |communicator|
    hello = Demo::HelloPrx::checkedCast(
        communicator.stringToProxy("hello:default -h localhost -p 10000"))
    hello.sayHello()
end
```

[Getting started]: https://doc.zeroc.com/ice/3.7/hello-world-application/writing-an-ice-application-with-ruby
[Examples]: https://github.com/zeroc-ice/ice-demos/tree/3.7/ruby
[Documentation]: https://doc.zeroc.com/ice/3.7
[Building from source]: https://github.com/zeroc-ice/ice/blob/3.7/ruby/BUILDING.md
[Ice framework]: https://github.com/zeroc-ice/ice
