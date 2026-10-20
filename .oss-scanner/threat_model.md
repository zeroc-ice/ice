# Ice: threat model for the OSS Scanner

Ice is an RPC framework: a binary protocol (the Ice protocol and the Ice encoding), the Slice interface definition
language and its compilers, runtimes for C++, C#, Java, JavaScript, Python, PHP, Ruby, Swift and MATLAB, and a set of
services built on the C++ runtime (Glacier2, IceGrid, IceStorm, IceBox, IceBridge, IceDiscovery,
IceLocatorDiscovery) plus the DataStorm pub/sub library. The Python, PHP, Ruby and MATLAB runtimes wrap the C++
runtime; C#, Java, JavaScript and Swift are separate implementations of the same protocol.

`SECURITY.md` lists the supported versions. This scan targets `main`; fixes are cherry-picked to the 3.8 release
branch, so a finding that also applies to 3.8 matters more than one in code that is new on `main`.

## Where untrusted input enters

The attacker is a remote peer that can open a connection to, or send datagrams to, an Ice endpoint, or a network
attacker on a plain (non-TLS) connection. Unless stated otherwise, assume the peer has not authenticated: Ice has no
authentication of its own outside of TLS client certificates, Glacier2 sessions and IceGrid sessions.

In order of exposure:

- **Protocol and encoding decoding**, in every runtime: message headers and framing, compression (bzip2), request and
  reply bodies, encapsulations, the Ice encoding of every Slice type (sequences, dictionaries, optional members,
  classes and class graphs, exceptions, slicing of unknown types, proxies and endpoints sent by the peer), and the
  generated unmarshaling code that the Slice compilers emit. C++: `cpp/src/Ice/InputStream.cpp`, `Protocol.cpp`,
  `ConnectionI.cpp`; the equivalents live under `csharp/src/Ice`, `java/src/com.zeroc.ice`,
  `js/packages/ice/src/Ice` and `swift/src/Ice`. `Ice.MessageSizeMax` (1 MB by default) and
  `Ice.ClassGraphDepthMax` (10 by default) are the configured limits on what a peer may send.
- **Transports**: TCP, UDP (including multicast), WebSocket (`WSTransceiver`, `HttpParser`, `Base64`, `SHA1`), SSL/TLS
  (`cpp/src/Ice/SSL`: OpenSSL on Linux; certificate verification and the `IceSSL.TrustOnly` and distinguished-name
  parsing in `RFC2253.cpp`), and Bluetooth (`cpp/src/IceBT`).
- **Glacier2**, the router meant to be exposed to the Internet: session creation, the permissions verifiers, request
  forwarding and buffering, the filters that restrict what a client may reach, and anything a client can do before
  or without a session. A client that escapes its filters or reaches a server it should not is a serious finding.
- **IceGrid registry and nodes**: the client-facing locator and query interfaces, session and admin session creation,
  replica and node sessions between registries and nodes.
- **IceStorm and DataStorm**: publishers and subscribers send data and control messages to the service or to each
  other. DataStorm nodes also accept multicast discovery messages by default (`DataStorm.Node.Multicast.Enabled`).
- **IceDiscovery and IceLocatorDiscovery**: multicast requests and replies from anyone on the local network.

## What is trusted

- **Configuration**: properties, configuration files, command-line arguments and environment variables. They are
  chosen by the operator.
- **Slice definitions and the Slice compilers**: `.ice` files are written by the application developer and compiled at
  build time. Compiler crashes on malformed Slice are ordinary bugs, not vulnerabilities. Generated code is in scope
  because it decodes peer data at run time.
- **The application's own code**: servants, custom verifiers, value factories, dispatch interceptors.
- **Authenticated administrators**: an IceGrid administrator can deploy and start arbitrary servers, and Ice admin
  facets (`Ice.Admin`) can change properties and shut down the process. These are code-execution or full-control
  privileges by design; something only reachable with them is not a vulnerability unless a less privileged party can
  reach it.
- **Local files and the local machine**: IceGrid and IceStorm databases (LMDB), certificates and key stores on disk,
  log files.

## How to exercise it

The image has two builds, described at the top of `/src/.oss-scanner/Dockerfile`:

- `/src`: every buildable mapping, debug build with asserts enabled. Each language directory has an `allTests.py`;
  run one suite with, for example, `python3 /src/cpp/allTests.py --filter=Ice/stream`. Tests start a server and a
  client on localhost ports from 12010, so run one suite at a time.
- `/opt/ice-asan`: C++ only, with AddressSanitizer and UndefinedBehaviorSanitizer. Use it to confirm memory-safety
  findings: `python3 /opt/ice-asan/cpp/allTests.py --filter=Ice/stream`. Leak detection is off by default
  (`ASAN_OPTIONS`); re-enable it only when a leak is the finding.

The test suites under `cpp/test` (and `csharp/test`, `java/test`, ...) are the best examples of how to stand up a
server and client. A reproducer that sends raw bytes over a socket to a test server is the most convincing form for a
decoding bug.

## How we rate severity

- **Critical**: remote code execution, or memory corruption (out-of-bounds write, use-after-free, double free) that
  an unauthenticated peer can trigger with a crafted message.
- **High**: other memory-safety bugs an unauthenticated peer can trigger (out-of-bounds read, null dereference in C++
  that crashes the process); a Glacier2 filter or session bypass; TLS certificate verification that accepts a
  certificate it should reject; a crash of an Ice service (Glacier2, IceGrid registry, IceStorm) by an
  unauthenticated peer.
- **Medium**: the same bugs when the peer must first authenticate (a Glacier2 or IceGrid session, a TLS client
  certificate); denial of service through resource exhaustion that an unauthenticated peer can cause with small input
  and that `Ice.MessageSizeMax` and the other configured limits do not bound; an unhandled exception that terminates a
  managed (C#, Java, JavaScript, Python) process.
- **Low**: denial of service that needs a large amount of traffic or a misconfigured limit; problems that only affect
  the connection or request the attacker sent; information disclosure into the server's own logs.
- An exception sent back to the peer for malformed input (`MarshalException`, `ProtocolException`, closing the
  connection) is the expected behavior, not a finding.

## Already considered and not vulnerabilities

- Full exception detail in logs at opt-in trace levels (for example `Glacier2.Trace.Session`, `IceGrid.Trace.*`).
  Tracing is off by default, the detail stays in the server's log, and clients only get a generic error.
- The plain transports (`tcp`, `udp`, `ws`, `bt`) are unauthenticated and unencrypted by design. The secure variants
  (`ssl`, `wss`, `bts`) are the answer; adding authentication to the plain ones is not.
- Memory or CPU spent decoding a message that `Ice.MessageSizeMax` already bounds, such as a large request context or
  a compressed message (the uncompressed size is checked against the limit before anything is allocated). Report it
  only if the cost escapes that bound.
- Certificate revocation checking is off by default (`IceSSL.RevocationCheck=0`, and cache-only when enabled). Many
  deployments use private CAs without OCSP or CRL distribution points.
- WebSocket masks and `Sec-WebSocket-Key` values come from `std::random_device`. Neither is a secret: the key is a
  handshake nonce, and the mask defends against browser cache poisoning, which does not apply to native Ice clients.
- A UDP server replies to the source address of the last datagram it received, so a spoofed datagram can redirect
  replies. UDP is documented as an insecure transport.
- Glacier2 forwards the client's request context to the permissions verifiers on purpose. The reserved `_con.*`
  connection keys are removed first.
- DataStorm sample history grows without bound when a writer sets `clearHistory` to `Never` and leaves the sample
  count and lifetime unlimited. That is a configuration choice.
- The parsers of the `icegridadmin` and `icestormadmin` command-line tools read input from a trusted local operator.
- Cleanup that is skipped only when a fatal exception (for example `std::bad_alloc`) escapes.
- Races at the idle-timeout boundary of a connection. A connection only reaches that boundary after receiving nothing,
  not even a heartbeat, for a full idle timeout; the outcome is a closed connection that the client re-establishes.
- Assemblies loaded through the C# Slice loader cache cannot be unloaded (see `AGENTS.md`).
- A DataStorm partial update that has no base value (never set, or removed) is discarded with a warning, by design.
- Hardening that only matters after an API contract is already violated by the application itself.

## Reports and patches

- Name the language mapping and the component in the title, and say whether the bug is reachable before
  authentication.
- When the same decoding logic exists in several mappings (C++, C#, Java, JavaScript, Swift), check the others and
  report each affected mapping in the same report.
- Patches must keep the wire format and the public API unchanged unless the report explains why that is impossible.
  C++ must stay C++17 compatible.
