[![Join the chat at https://gitter.im/zeroc-ice/ice](https://badges.gitter.im/zeroc-ice/ice.svg)][gitter]

# The Ice framework

Ice helps you build networked applications with minimal effort. By taking care of all interactions with low-level
network programming interfaces, Ice allows you to focus your efforts on your application logic. You don't need to
worry about details such as opening network connections, encoding and decoding data for network transmission,
or retrying failed connection attempts (to name just a few of dozens of such low-level details).

[Downloads][downloads] | [Examples][examples] | [Documentation][docs]

## RPCs with Ice

Remote procedure calls ([RPCs][rpcs]) are at the heart of the Ice framework.

You create RPCs with an easy 2-step process:
1. Define the contract between your client and your server with the [Slice][slice] language—Ice's [IDL][idl].
2. Run the Slice compiler on these Slice definitions to generate stubs in the programming language(s) of your choice.

For example:

```slice
// The contract specified using Slice.
interface Hello
{
    // The caller says "hello".
    void sayHello();
}
```

```shell
# Compile the Slice contract with the Slice compiler for C++ (slice2cpp)
slice2cpp Hello.ice
```

```c++
// C++ client

// Call operation sayHello on a remote object that implements
// interface Hello using the generated proxy class (HelloPrx).
helloPrx->sayHello();
```

```c++
// C++ server

// Implement the Hello interface by deriving from the generated
// Hello abstract base class.
class HelloImpl : public Hello
{
public:

    virtual string sayHello(const Ice::Current&) override
    {
        cout << "Hello World!" << endl;
    }
};
```

You can use any supported programming language for your client and server. For example, a Python client could call a C++
server; neither side knows the programming language used by the other side.

## Complete solution with a uniform API

The Ice framework provides everything you need to build networked applications:
- RPCs with a compact binary [protocol][protocol] over a variety of network transports (TCP, UDP, WebSocket,
Bluetooth...)
- Secure communications ([IceSSL][icessl])
- Configuration ([Ice Properties][properties])
- Logging ([Ice Logger][logger])
- Instrumentation and metrics ([IceMX][icemx])
- Pub-sub ([IceStorm][icestorm])
- Server deployment, replication and monitoring ([IceGrid][icegrid])
- Session management and firewall traversal ([Glacier2][glacier2])

The Ice API is defined almost entirely using Slice; as a result, it is essentially the same in all programming
languages.

## Building Ice from source

[C++](cpp) | [C#](csharp) | [Java](java) | [JavaScript/TypeScript](js) | [MATLAB](matlab) | [Objective-C](objective-c) | [PHP](php) | [Python](python) | [Ruby](ruby) | [Swift](swift)

## Copyright and license

Ice is a single-copyright project: all the source code in this [ice repository][ice-repo] is
Copyright &copy; ZeroC, Inc., with very few exceptions.

As copyright owner, ZeroC can license Ice under different license terms, and offers the following licenses for Ice:
- GPL v2, a popular open-source license with strong [copyleft][copyleft] conditions (the default license)
- Commercial or closed-source licenses

If you license Ice under GPL v2, there is no license fee or signed license agreement: you just need to comply with the
GPL v2 terms and conditions. See [ICE_LICENSE](ICE_LICENSE) and [LICENSE](LICENSE) for further information.

If you purchase a commercial or closed-source license for Ice, you must comply with the terms and conditions listed in
the associated license agreement; the GPL v2 terms and conditions do not apply.

The Ice software itself remains the same: the only difference between an open-source Ice and a commercial Ice are the
license terms.

[copyleft]: https://en.wikipedia.org/wiki/Copyleft
[docs]: https://doc.zeroc.com/ice/3.7
[downloads]: https://zeroc.com/downloads/ice
[examples]: https://github.com/zeroc-ice/ice-demos
[gitter]: https://gitter.im/zeroc-ice/ice?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge
[glacier2]: https://doc.zeroc.com/ice/3.7/ice-services/glacier2
[ice-repo]: https://github.com/zeroc-ice/ice
[icegrid]: https://doc.zeroc.com/ice/3.7/ice-services/icegrid
[icemx]: https://doc.zeroc.com/ice/3.7/administration-and-diagnostics/administrative-facility/the-metrics-facet
[icessl]: https://doc.zeroc.com/ice/3.7/ice-plugins/icessl
[icestorm]: https://doc.zeroc.com/ice/3.7/ice-services/icestorm
[idl]: https://en.wikipedia.org/wiki/Interface_description_language
[logger]: https://doc.zeroc.com/ice/3.7/administration-and-diagnostics/logger-facility
[properties]: https://doc.zeroc.com/ice/3.7/properties-and-configuration
[protocol]: https://doc.zeroc.com/ice/3.7/ice-protocol-and-encoding
[rpcs]: https://en.wikipedia.org/wiki/Remote_procedure_call
[slice]: https://doc.zeroc.com/ice/3.7/the-slice-language
