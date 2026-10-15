# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import os
import socket
from collections.abc import Sequence
from typing import NamedTuple

from Glacier2Util import Glacier2Router, Glacier2TestSuite
from Util import Client, ClientServerTestCase, Driver, Server


def tcp(host: str, port: int = 12010) -> str:
    """Returns a TCP endpoint string for host and port; the backend server listens on port 12010."""
    return f"tcp -h {host} -p {port}"


LOOPBACK = tcp("127.0.0.1")


# The test driver runs this script without registering it in sys.modules, which @dataclass requires, so the records
# below are NamedTuples.
class FilterTest(NamedTuple):
    """A Glacier2 router filter configuration and the proxies that the router must accept or reject under it."""

    name: str
    # The Glacier2.Filter properties of the router, keyed by the property name without the Glacier2.Filter. prefix.
    filters: dict[str, str]
    accepted: Sequence[str] = ()
    rejected: Sequence[str] = ()


class HostNames(NamedTuple):
    """The names of this host, for tests that need to address it by something other than the loopback address."""

    hostname: str
    fqdn: str
    domainname: str


class Glacier2StaticFilteringTestCase(ClientServerTestCase):
    def __init__(self, test: FilterTest, hostname: str):
        self.test = test
        self.hostname = hostname

        clientProps = {"Ice.Config": "{testdir}/client.cfg", "Ice.Warn.Connections": 0}
        serverProps = {"Ice.Config": "{testdir}/server.cfg", "Ice.Warn.Connections": 0}
        routerProps = {
            "Ice.Config": "{testdir}/router.cfg",
            "Glacier2.RoutingTable.MaxSize": 10,
            "Ice.Warn.Connections": 0,
        }

        # Override the server/router default host property, we don't want to use the loopback
        serverProps["Ice.Default.Host"] = ""
        routerProps["Ice.Default.Host"] = ""

        ClientServerTestCase.__init__(
            self,
            test.name,
            desc=test.name,
            servers=[Glacier2Router(props=routerProps), Server(props=serverProps)],
            client=Client(props=clientProps),
        )

    def setupClientSide(self, current: Driver.Current) -> None:
        current.write(f"testing {self}... ")

    def setupServerSide(self, current: Driver.Current) -> None:
        # The test client performs multiple tests during one 'run'. We could use command line arguments to pass the
        # test cases in, but a configuration file is easier.
        with open(os.path.join(self.getTestSuite().getPath(), "client.cfg"), "w") as clientConfig:
            for i, proxy in enumerate(self.test.accepted):
                clientConfig.write(f"Accept.Proxy.{i}={proxy}\n")
            for i, proxy in enumerate(self.test.rejected):
                clientConfig.write(f"Reject.Proxy.{i}={proxy}\n")

        with open(os.path.join(self.getTestSuite().getPath(), "server.cfg"), "w") as serverConfig:
            if current.config.protocol != "ssl":
                serverConfig.write("BackendAdapter.Endpoints=tcp -p 12010\n")

        # The filters go in a configuration file because their regular expression syntax can easily confuse the
        # command line.
        with open(os.path.join(self.getTestSuite().getPath(), "router.cfg"), "w") as routerConfig:
            routerConfig.write(f"Ice.Default.Locator=locator:{tcp(self.hostname)}\n")
            routerConfig.write("Glacier2.Client.Trace.Reject=0\n")
            for name, value in self.test.filters.items():
                routerConfig.write(f"Glacier2.Filter.{name}={value}\n")

    def teardownServerSide(self, current: Driver.Current, success: bool) -> None:
        for c in ["client.cfg", "router.cfg", "server.cfg"]:
            os.remove(os.path.join(self.getTestSuite().getPath(), c))


def detectHostNames() -> HostNames | None:
    """Returns the names of this host, or None if its network configuration only permits the loopback tests."""
    hostname = socket.gethostname().lower()
    fqdn = socket.getfqdn().lower()

    # A "localdomain", "local" or "domain" suffix means that there is no real configured domain name, so anything that
    # requires a domain name isn't likely going to work. The hostname might contain this suffix too, so we pretend
    # that this host has no name at all.
    if fqdn.endswith(("local", "domain")) or hostname.startswith("localhost") or "." not in fqdn:
        return None

    dot = fqdn.find(".")
    domainname = fqdn[dot + 1 :]
    if domainname == "":
        return None

    # Some Python installs return a FQDN for gethostname(). This invalidates the tests that need to differentiate
    # between the hostname and the FQDN. If these tests fail because of a DNS error, it may be that an alias is not
    # present for the host partition of the FQDN.
    if fqdn == hostname:
        hostname = hostname[:dot]

    try:
        addresses = {socket.gethostbyname(fqdn), socket.gethostbyname(hostname)}
    except socket.gaierror:
        return None

    # On SUSE distributions, 127.0.0.2 is sometimes used in /etc/hosts for the hostname (apparently if no network
    # interface was found when the OS was installed). However, connecting to this IP address doesn't work (even if it
    # can be pinged).
    if "127.0.0.2" in addresses:
        return None

    return HostNames(hostname, fqdn, domainname)


def loopbackTests() -> list[FilterTest]:
    """Returns the tests that only use the loopback address and therefore run on any host."""
    return [
        FilterTest(
            "category filter",
            filters={"Category.Accept": 'foo "a cat with spaces"'},
            accepted=[
                f"foo/helloA:{LOOPBACK}",
                f'"a cat with spaces/helloB":{LOOPBACK}',
            ],
            rejected=[
                f"nocat/helloC:{LOOPBACK}",
                f"cat/helloD:{LOOPBACK}",
            ],
        ),
        FilterTest(
            "adapter id filter",
            filters={"Address.Reject": "*", "AdapterId.Accept": 'foo "an adapter with spaces"'},
            accepted=[
                'helloF1 @ "an adapter with spaces"',
                "helloF @ foo",
            ],
            rejected=[
                f"foo/helloA:{LOOPBACK}",
                f'"a cat with spaces/helloB":{LOOPBACK}',
                f"nocat/helloC:{LOOPBACK}",
                f"cat/helloD:{LOOPBACK}",
                "helloE @ bar",
            ],
        ),
        FilterTest(
            "identity filters",
            filters={"Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up"'},
            accepted=[
                f"myident:{LOOPBACK}",
                f"cata/fooa:{LOOPBACK}",
                f'"a funny id/that might mess it up":{LOOPBACK}',
            ],
            rejected=[
                f'"a cat with spaces/helloB":{LOOPBACK}',
                f"nocat/helloC:{LOOPBACK}",
                f"cat/helloD:{LOOPBACK}",
                f"baz/myident:{LOOPBACK}",
                f"cata/foo:{LOOPBACK}",
                f"cat/fooa:{LOOPBACK}",
            ],
        ),
        FilterTest(
            "mixing filters",
            filters={
                "Category.Accept": 'mycat "a sec cat"',
                "Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up" "a\\"nother"',
                "AdapterId.Accept": "myadapter",
            },
            accepted=[
                f"mycat/fooa:{LOOPBACK}",
                f'"a sec cat/fooa":{LOOPBACK}',
                "mycat/foo @ jimbo",
                "hiscat @ myadapter",
                'a"nother @ jimbo',
                f"myident:{LOOPBACK}",
                f"cata/fooa:{LOOPBACK}",
                f'"a funny id/that might mess it up":{LOOPBACK}',
            ],
            rejected=[
                f'"a cat with spaces/helloB":{LOOPBACK}',
                f"nocat/helloC:{LOOPBACK}",
                f"cat/helloD:{LOOPBACK}",
                f"baz/myident:{LOOPBACK}",
                f"cata/foo:{LOOPBACK}",
                f"cat/fooa:{LOOPBACK}",
                "hiscatA @ jimbo",
            ],
        ),
        FilterTest(
            "mixing filters (indirect only)",
            filters={
                "Address.Reject": "*",
                "Category.Accept": 'mycat "a sec cat"',
                "Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up"',
                "AdapterId.Accept": "myadapter",
            },
            accepted=[
                "mycat/foo @ jimbo",
                "hiscat @ myadapter",
                '"a funny id/that might mess it up" @ myadapter',
            ],
            rejected=[
                f'"a cat with spaces/helloB":{LOOPBACK}',
                f"nocat/helloC:{LOOPBACK}",
                f"cat/helloD:{LOOPBACK}",
                f"baz/myident:{LOOPBACK}",
                f"cata/foo:{LOOPBACK}",
                f"cat/fooa:{LOOPBACK}",
                f"mycat/fooa:{LOOPBACK}",
                f'"a sec cat/fooa":{LOOPBACK}',
                "hiscatA @ jimbo",
                f"myident:{LOOPBACK}",
                f"cata/fooa:{LOOPBACK}",
                f'"a funny id/that might mess it up":{LOOPBACK}',
            ],
        ),
        FilterTest(
            "address filter with numeric range in hostname",
            filters={"Address.Accept": "12[7].0.0.*"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[f"hello:{tcp('128.0.0.1')}", f"hello:{tcp('126.0.0.1')}"],
        ),
        FilterTest(
            # A rule must match the host in full, so 12[7] does not match 127.0.0.1.
            "address filter without a trailing wildcard matches the whole host",
            filters={"Address.Accept": "12[7]"},
            rejected=[f"hello:{LOOPBACK}"],
        ),
        FilterTest(
            # A rule without any wildcard is not a suffix match, so the reject rule 7.0.0.1 does not reject
            # 127.0.0.1.
            "address filter without a wildcard is not a suffix match",
            filters={"Address.Reject": "7.0.0.1"},
            accepted=[f"hello:{LOOPBACK}"],
        ),
        FilterTest(
            # A wildcard followed by a trailing numeric range matches when any alignment of the wildcard reaches the
            # end of the host: for 127.0.0.1, the first two dots leave part of the host unmatched and only the last
            # dot is the correct alignment. The range must match right after a dot, so 127.0.0.x does not match.
            "address filter with a wildcard followed by a trailing numeric range",
            filters={"Address.Accept": "*.[0-255]"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[f"hello:{tcp('127.0.0.x')}"],
        ),
        FilterTest(
            # Same with the numeric range directly after the wildcard: only the final 1 of 127.0.0.1 ends at the end
            # of the host and lies in the 0-9 range.
            "address filter with a wildcard followed by a single-digit numeric range",
            filters={"Address.Accept": "*[0-9]"},
            accepted=[f"hello:{LOOPBACK}"],
        ),
        FilterTest(
            # A rule ending with a numeric range followed by a wildcard is a prefix match: 12[0-255]* matches
            # 127.0.0.1, with the range matching the number right after the 12 prefix.
            "address filter with a numeric range followed by a trailing wildcard",
            filters={"Address.Accept": "12[0-255]*"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[f"hello:{tcp('12x.0.0.1')}"],
        ),
        FilterTest(
            # A digit run too long to fit in an int does not match a numeric range: the scan skips the run, finds no
            # other number in the host, and the rule does not match.
            "address filter numeric range against a long digit run",
            filters={"Address.Accept": "*[0-9]"},
            rejected=[f"hello:{tcp('9' * 255)}"],
        ),
        FilterTest(
            # A rule with many wildcard segments stays fast even when every alignment of the wildcards has to be
            # tried before concluding that the host does not match.
            "address filter with many wildcard segments",
            filters={"Address.Accept": "*aa*aa*aa*aa*aa*aa*aab *7.*.1"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[f"hello:{tcp('a' * 255)}"],
        ),
        FilterTest(
            # Host names are case-insensitive: an accept rule matches the host regardless of the case of either the
            # rule or the host.
            "address filter accept rule is case-insensitive",
            filters={"Address.Accept": "LocalHost"},
            accepted=[f"hello:{tcp('localhost')}", f"hello:{tcp('LOCALHOST')}"],
            rejected=[f"hello:{LOOPBACK}"],
        ),
        FilterTest(
            # A reject rule matches a proxy as soon as one of its endpoints matches: extra endpoints that do not
            # match the rule do not get the proxy accepted.
            "address filter reject rule against a multi-endpoint proxy",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[f"hello:{tcp('localhost')}"],
            rejected=[
                f"hello:{LOOPBACK}:{tcp('127.0.0.2')}",
                f"hello:{tcp('127.0.0.2')}:{LOOPBACK}",
            ],
        ),
        FilterTest(
            # Same for a reject rule: a case variation of the host still matches the rule.
            "address filter reject rule is case-insensitive",
            filters={"Address.Reject": "badhost.example.com"},
            accepted=[f"hello:{tcp('localhost')}"],
            rejected=[f"hello:{tcp('BadHost.Example.COM')}"],
        ),
        FilterTest(
            # A non-canonical spelling of an IPv4 address (octal or hexadecimal parts, leading zeros, fewer than four
            # parts, a single number) is rejected outright when filters are configured: the resolver accepts these
            # spellings as aliases for an address that string-based rules spell differently. A DNS name keeps its
            # meaning with a trailing dot, so 'name.' matches the rules written for 'name'.
            "address filter against non-canonical hosts",
            filters={"Address.Accept": "localhost 127.0.0.1"},
            accepted=[
                f"hello:{tcp('localhost')}",
                f"hello:{LOOPBACK}",
                f"hello:{tcp('localhost.')}",
            ],
            rejected=[
                f"hello:{tcp('127.000.000.001')}",
                f"hello:{tcp('localhost.example.com')}",
            ],
        ),
        FilterTest(
            "address filter reject rule against a trailing-dot host",
            filters={"Address.Reject": "badhost.example.com *.internal.example.com"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[
                f"hello:{tcp('badhost.example.com.')}",
                f"hello:{tcp('db.internal.example.com.')}",
            ],
        ),
        FilterTest(
            # A rule written with a trailing dot is stripped like the host, so both spellings of the rule match both
            # spellings of the host.
            "address filter reject rule written with a trailing dot",
            filters={"Address.Reject": "badhost.example.com."},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[
                f"hello:{tcp('badhost.example.com')}",
                f"hello:{tcp('badhost.example.com.')}",
            ],
        ),
        FilterTest(
            # The non-canonical spellings are rejected even though none of them matches the reject rule as a string:
            # without the outright rejection, each would be accepted and the resolver would connect it to the very
            # address the rule rejects.
            "address filter reject rule against non-canonical IPv4 literals",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[f"hello:{tcp('localhost')}"],
            rejected=[
                f"hello:{tcp('127.000.000.001')}",
                f"hello:{tcp('0x7f000001')}",
                f"hello:{tcp('2130706433')}",
                f"hello:{tcp('127.1')}",
                f"hello:{tcp('0177.0.0.1')}",
                f"hello:{tcp('0x.0.0.1')}",
                f"hello:{tcp('127.0.0.1.')}",
            ],
        ),
        FilterTest(
            # The outright rejection of non-canonical spellings applies only when an address filter is configured:
            # with just a size limit, the proxy is matched against no address rule, so the spelling of the host does
            # not matter.
            "proxy size limit alone allows non-canonical hosts",
            filters={"ProxySizeMax": "100"},
            accepted=[f"hello:{tcp('127.1')}"],
        ),
        FilterTest(
            # A proxy with an endpoint host longer than 255 characters is rejected outright, even when no reject
            # rule matches it: no legal host name or IP address is that long.
            "address filter rejects an oversized host",
            filters={"Address.Reject": "*.invalid"},
            accepted=[f"hello:{LOOPBACK}"],
            rejected=[f"hello:{tcp('a' * 300)}"],
        ),
        FilterTest(
            # An endpoint with no host is rejected outright: Ice resolves it to the loopback address, yet it matches
            # no address rule, so without the rejection a reject rule for loopback would not catch a proxy that
            # connects to loopback. The same accepted IP endpoint flips to rejected once the host-less endpoint is
            # added alongside it, so the host-less endpoint cannot ride in behind a legitimate one.
            "address filter rejects an endpoint with no host",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[f"hello:{tcp('localhost')}"],
            rejected=[
                "hello:tcp -p 12010",
                f"hello:{tcp('localhost')}:tcp -p 12010",
            ],
        ),
        FilterTest(
            # A proxy with a non-IP endpoint (here an opaque endpoint of an unknown transport) is rejected outright:
            # the address rules describe IP hosts and ports, so an endpoint they cannot interpret is rejected rather
            # than admitted unchecked. The same accepted IP endpoint flips to rejected once the non-IP endpoint is
            # added alongside it, so the non-IP endpoint cannot ride in behind a legitimate one.
            "address filter rejects a non-IP endpoint",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[f"hello:{tcp('localhost')}"],
            rejected=[f"hello:{tcp('localhost')}:opaque -t 99 -v ABCD"],
        ),
    ]


def hostNameTests(host: HostNames) -> list[FilterTest]:
    """Returns the tests that address this host by its name, which require a configured host and domain name."""
    hostname, fqdn, domainname = host
    return [
        FilterTest(
            "reject all",
            filters={"Address.Reject": "*"},
            accepted=["bar @ foo"],
            rejected=[
                f"helloA:{tcp(fqdn)}",
                f"helloB:{tcp(hostname)}",
                f"helloC:{LOOPBACK}",
            ],
        ),
        FilterTest(
            "loopback only rule",
            filters={"Address.Accept": "127.0.0.1 localhost"},
            accepted=[
                f"localhost:{LOOPBACK}",
                f"localhost/127.0.0.1:{tcp('localhost')}",
                f"hello:{LOOPBACK}",
                f"hello/somecat:{tcp('localhost')}",
            ],
            rejected=[
                f"hello:{tcp(fqdn)}",
                f"hello:{tcp(hostname)}",
                f"127.0.0.1:{tcp(hostname)}",
                f"localhost:{tcp(hostname)}",
                f"localhost/127.0.0.1:{tcp(hostname)}",
            ],
        ),
        FilterTest(
            "port filter rule",
            filters={"Address.Accept": "127.0.0.1:12010 localhost:12010"},
            accepted=[f"hello3:{LOOPBACK}", f"hello4:{tcp('localhost')}"],
            rejected=[
                f"hello1:{tcp('127.0.0.1', 12011)}",
                f"hello2:{tcp('localhost', 12011)}",
                f"hello5:{tcp(hostname)}",
            ],
        ),
        FilterTest(
            "reject port filter rule",
            filters={"Address.Reject": "127.0.0.1:[0-12009,12011-65535] localhost:[0-12009,12011-65535]"},
            accepted=[f"hello5:{tcp(hostname)}", f"hello3:{LOOPBACK}", f"hello4:{tcp('localhost')}"],
            rejected=[f"hello1:{tcp('127.0.0.1', 12011)}", f"hello2:{tcp('localhost', 12011)}"],
        ),
        FilterTest(
            "port filter rule with wildcard address rule",
            filters={"Address.Accept": "*:12010"},
            accepted=[f"hello5:{tcp(hostname)}", f"hello3:{LOOPBACK}", f"hello4:{tcp('localhost')}"],
            rejected=[f"hello1:{tcp('127.0.0.1', 12011)}", f"hello2:{tcp('localhost', 12011)}"],
        ),
        FilterTest(
            "domain filter rule (accept)",
            filters={"Address.Accept": f"*{domainname}"},
            accepted=[f"hello:{tcp(fqdn)}"],
            rejected=[f"hello:{tcp(hostname)}"],
        ),
        FilterTest(
            "domain filter rule (reject)",
            filters={"Address.Reject": f"*{domainname}"},
            accepted=[f"hello:{tcp(hostname)}", f"bar:{LOOPBACK}"],
            rejected=[f"hello:{tcp(fqdn)}"],
        ),
        FilterTest(
            "domain filter rule (mixed)",
            filters={"Address.Accept": "127.0.0.1", "Address.Reject": fqdn},
            accepted=[f"bar:{LOOPBACK}"],
            rejected=[f"hello:{tcp(fqdn)}:{LOOPBACK}"],
        ),
        FilterTest(
            "maximum proxy length rule",
            filters={"ProxySizeMax": "53"},
            accepted=[f"hello:{LOOPBACK} -t infinite"],
            rejected=[f"012345678901234567890123456789012345678901234567890123456789:{LOOPBACK}"],
        ),
    ]


class Glacier2StaticFilteringTestSuite(Glacier2TestSuite):
    def setup(self, current: Driver.Current) -> None:
        Glacier2TestSuite.setup(self, current)

        host = detectHostNames()
        tests = loopbackTests()
        if host:
            tests += hostNameTests(host)
        else:
            current.writeln("WARNING: The network configuration for this host does not permit all ")
            current.writeln("         tests to run correctly, some tests have been disabled.")

        self.testcases.clear()
        for test in tests:
            self.addTestCase(Glacier2StaticFilteringTestCase(test, host.hostname if host else "127.0.0.1"))


Glacier2StaticFilteringTestSuite(
    __name__,
    testcases=[],
    runOnMainThread=True,
    options={"ipv6": [False]},
    multihost=False,
)
