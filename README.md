[![Join the chat at https://gitter.im/zeroc-ice/ice](https://badges.gitter.im/zeroc-ice/ice.svg)](https://gitter.im/zeroc-ice/ice?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# Ice - Comprehensive RPC Framework

Ice helps you network your software with minimal effort. By taking care of all
interactions with low-level network programming interfaces, Ice allows you to focus
your efforts on your application logic. When using Ice, there is no need to worry
about details such as opening network connections, serializing and deserializing
data for network transmission, or retrying failed connection attempts (to name just
a few of dozens of such low-level details).

You can download Ice releases from [ZeroC's website](https://zeroc.com/downloads/ice).
Use this GitHub repository to build from source; see [branches](#branches) for more
information.

## Languages

Ice supports the following programming languages:

- C#
- C++
- Java
- JavaScript
- MATLAB
- Objective-C
- PHP
- Python
- Ruby
- Swift
- TypeScript

## Platforms

Ice runs on a wide range of platforms:

- Android
- iOS
- Linux
- Linux on embedded devices
- macOS
- Node.js
- Unix systems such as AIX
- Web Browser
- Windows

## Features

- Efficient, high-performance binary protocol
- Supports a wide range of programming languages and platforms
- Easy to use and type-safe API, with your own interfaces and types defined in
  a programming language neutral IDL, Slice
- Supports secure, encrypted communications by taking advantage of your
  platform's native SSL/TLS stack
- Familiar object-oriented programming model, with the ability to transmit
  proxies (references to remote objects) to remote applications
- Supports synchronous and asynchronous calls, for both client-side invocations
  and server-side dispatches
- Automatic discovery of remote objects through UDP multicast
- Comes with a number of optional services:
    - [IceGrid](https://zeroc.com/products/ice/services/icegrid) - a DNS-like
    service for Ice objects, with support for server deployment, replication,
    monitoring, load-balancing and more
    - [IceStorm](https://zeroc.com/products/ice/services/icestorm) - a
    lightweight topic-based pub-sub service
    - [Glacier2](https://zeroc.com/products/ice/services/glacier2) - a
    sysadmin-friendly solution for routing Ice communications through firewalls

## Branches

- `master`
  Primary development branch (unstable, frequently updated)

- `3.7`
  Ice 3.7.x plus various patches (stable, frequently updated)

- `3.6`
  Ice 3.6.x plus various patches (stable)

- `3.5`
  Ice 3.5.1 plus various patches (stable)

- `3.4`
  Ice 3.4.2 plus various patches (stable)

## Copyright and License

Ice is a single-copyright project: all the source code in this [ice
repository](https://github.com/zeroc-ice/ice) is Copyright &copy; ZeroC, Inc.,
with very few exceptions.

As copyright owner, ZeroC can license Ice under different license terms, and
offers the following licenses for Ice:
- GPL v2, a popular open-source license with strong
[copyleft](https://en.wikipedia.org/wiki/Copyleft) conditions (the default
license)
- Commercial or closed-source licenses

If you license Ice under GPL v2, there is no license fee or signed license
agreement: you just need to comply with the GPL v2 terms and conditions. See
[ICE_LICENSE](./ICE_LICENSE) and [LICENSE](./LICENSE) for further information.

If you purchase a commercial or closed-source license for Ice, you must comply
with the terms and conditions listed in the associated license agreement; the
GPL v2 terms and conditions do not apply.

The Ice software itself remains the same: the only difference between an open-source
Ice and a commercial Ice are the license terms.

## Documentation

- [Ice Release Notes](https://doc.zeroc.com/rel/ice-releases/ice-3-7/ice-3-7-3-release-notes)
- [Ice Manual](https://doc.zeroc.com/ice/3.7/)
