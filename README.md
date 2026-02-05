<p align="center">
  <img src="https://raw.githubusercontent.com/zeroc-ice/ice/3.7/.github/assets/ice-banner.svg" height="150" width="150" />
</p>

[![CI](https://github.com/zeroc-ice/ice/actions/workflows/ci.yml/badge.svg)][ci-home]
[![GPLv2](https://img.shields.io/github/license/zeroc-ice/ice?color=red)][license]
[![Static Badge](https://img.shields.io/badge/license-Commercial-blue)][Commercial]

# The Ice framework

The Internet Communications Engine (Ice) is a complete RPC framework that helps you build networked applications with
minimal effort. Ice allows you to focus your efforts on your application logic, and it takes care of all interactions
with low-level network programming interfaces. With Ice, there is no need to worry about details such as opening
network connections, serializing and deserializing data for network transmission, or retrying failed connection
attempts.

[Downloads][downloads] | [Examples][examples] | [Documentation][docs]

## RPCs with Ice

Remote procedure calls ([RPCs][rpcs]) are at the heart of the Ice framework.

You create RPCs with an easy 2-step process:

1. Define the contract between your client and your server with the [Slice][slice] language—Ice's [IDL][idl].
2. Run the Slice compiler on these Slice definitions to generate stubs in the programming language(s) of your choice.

For example:

```slice
// The contract specified using Slice.

/// Represents a simple greeter.
interface Greeter
{
    /// Creates a personalized greeting.
    /// @param name The name of the person to greet.
    /// @return The greeting.
    string greet(string name);
}
```

```shell
# Compile the Slice contract with the Slice compiler for C++ (slice2cpp)
slice2cpp Greeter.ice
```

```c++
// C++ client

// Call operation greet on a remote object that implements
// interface Greeter using the generated proxy class (GreeterPrx).
GreeterPrx greeter{communicator, "greeter:tcp -h localhost -p 4061"};
string greeting = greeter.greet("alice");
```

```c++
// C++ server

// Implements the Greeter interface by deriving from the generated
// Greeter abstract base class.
class Chatbot : public Greeter
{
public:
    std::string greet(std::string name, const Ice::Current&) override
    {
        std::ostringstream os;
        os << "Hello, " << name << "!";
        return os.str();
    }
};
```

You can use any supported programming language for your client and server. For example, a Python client could call a C++
server; neither side knows the programming language used by the other side.

## Complete solution with a uniform API

The Ice framework provides everything you need to build networked applications:

- RPCs with a compact binary [protocol][protocol] over a variety of network transports (TCP, SSL, UDP, WebSocket,
Bluetooth...)
- Configuration ([Ice Properties][properties])
- Logging ([Ice Logger][logger])
- Instrumentation and metrics ([IceMX][icemx])
- Pub-sub ([IceStorm][icestorm], [DataStorm][datastorm])
- Server deployment, replication and monitoring ([IceGrid][icegrid])
- Application gateway ([Glacier2][glacier2])

## Language Support

[C++](cpp/README.md) | [C#](csharp/README.md) | [Java](java/README.md) | [JavaScript/TypeScript](js/README.md) | [MATLAB](matlab/README.md) | [PHP](php/README.md) | [Python](python/README.md) | [Ruby](ruby/README.md) | [Swift](swift/README.md)

## Copyright and license

Ice is a single-copyright project: all the source code in this [ice repository][ice-repo] is
Copyright &copy; ZeroC, Inc., with very few exceptions.

As copyright owner, ZeroC can license Ice under different license terms, and offers the following licenses for Ice:

- [GPLv2][license], a popular open-source license with strong [copyleft][copyleft] conditions (the default license)
- Commercial or closed-source licenses

If you license Ice under GPLv2, there is no license fee or signed license agreement: you just need to comply with the
GPLv2 terms and conditions. ZeroC also grants a few [exceptions](ICE_LICENSE) to the GPLv2 terms and conditions.

If you purchase a commercial or closed-source license for Ice, you must comply with the terms and conditions listed in
the associated license agreement; the GPLv2 terms and conditions do not apply.

The Ice software itself remains the same: the only difference between an open-source Ice and a commercial Ice are the
license terms.

[ci-home]: https://github.com/zeroc-ice/ice/actions/workflows/ci.yml
[Commercial]: https://zeroc.com/ice/pricing
[copyleft]: https://en.wikipedia.org/wiki/Copyleft
[datastorm]: https://docs.zeroc.com/ice/latest/cpp/datastorm
[docs]: https://docs.zeroc.com/ice/
[downloads]: ./NIGHTLY.md
[examples]: https://github.com/zeroc-ice/ice-demos
[glacier2]: https://docs.zeroc.com/ice/latest/cpp/glacier2
[ice-repo]: https://github.com/zeroc-ice/ice
[icegrid]: https://docs.zeroc.com/ice/latest/cpp/icegrid
[icemx]: https://docs.zeroc.com/ice/latest/cpp/the-metrics-facet
[icestorm]: https://docs.zeroc.com/ice/latest/cpp/icestorm
[idl]: https://en.wikipedia.org/wiki/Interface_description_language
[license]: LICENSE
[logger]: https://docs.zeroc.com/ice/latest/cpp/logger-facility
[properties]: https://docs.zeroc.com/ice/latest/cpp/properties-and-configuration
[protocol]: https://docs.zeroc.com/ice/latest/cpp/ice-protocol
[rpcs]: https://en.wikipedia.org/wiki/Remote_procedure_call
[slice]: https://docs.zeroc.com/ice/latest/cpp/the-slice-language
