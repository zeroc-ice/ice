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


BACKEND_ENDPOINT = tcp("127.0.0.1")


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
                # The category is listed.
                f"foo/helloA:{BACKEND_ENDPOINT}",
                # The category is listed; a quoted entry of the filter can contain spaces.
                f'"a cat with spaces/helloB":{BACKEND_ENDPOINT}',
            ],
            rejected=[
                # The category is not listed.
                f"nocat/helloC:{BACKEND_ENDPOINT}",
                # A category must match a listed one in full: being part of "a cat with spaces" is not enough.
                f"cat/helloD:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            "adapter id filter",
            filters={"Address.Reject": "*", "AdapterId.Accept": 'foo "an adapter with spaces"'},
            accepted=[
                # The adapter id is listed; a quoted entry of the filter can contain spaces.
                'helloF1 @ "an adapter with spaces"',
                # The adapter id is listed.
                "helloF @ foo",
            ],
            rejected=[
                # The adapter id filter only applies to indirect proxies, so the "*" reject rule is what rejects
                # these direct proxies.
                f"foo/helloA:{BACKEND_ENDPOINT}",
                f'"a cat with spaces/helloB":{BACKEND_ENDPOINT}',
                f"nocat/helloC:{BACKEND_ENDPOINT}",
                f"cat/helloD:{BACKEND_ENDPOINT}",
                # The adapter id is not listed.
                "helloE @ bar",
            ],
        ),
        FilterTest(
            "identity filters",
            filters={"Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up"'},
            accepted=[
                # The identity is listed, without a category.
                f"myident:{BACKEND_ENDPOINT}",
                # The identity is listed, with its category.
                f"cata/fooa:{BACKEND_ENDPOINT}",
                # The identity is listed; a quoted entry of the filter can contain spaces and a slash.
                f'"a funny id/that might mess it up":{BACKEND_ENDPOINT}',
            ],
            rejected=[
                # The identity is not listed.
                f'"a cat with spaces/helloB":{BACKEND_ENDPOINT}',
                f"nocat/helloC:{BACKEND_ENDPOINT}",
                f"cat/helloD:{BACKEND_ENDPOINT}",
                # An identity must match a listed one in both category and name: each of these matches only one.
                f"baz/myident:{BACKEND_ENDPOINT}",
                f"cata/foo:{BACKEND_ENDPOINT}",
                f"cat/fooa:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            # A request is accepted as soon as one of the category, identity, or adapter id filters matches it.
            "mixing filters",
            filters={
                "Category.Accept": 'mycat "a sec cat"',
                "Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up" "a\\"nother"',
                "AdapterId.Accept": "myadapter",
            },
            accepted=[
                # The category is listed.
                f"mycat/fooa:{BACKEND_ENDPOINT}",
                f'"a sec cat/fooa":{BACKEND_ENDPOINT}',
                # The category is listed, even though the adapter id is not.
                "mycat/foo @ jimbo",
                # The adapter id is listed.
                "hiscat @ myadapter",
                # The identity is listed, even though the adapter id is not; the filter entry escapes the quote.
                'a"nother @ jimbo',
                # The identity is listed.
                f"myident:{BACKEND_ENDPOINT}",
                f"cata/fooa:{BACKEND_ENDPOINT}",
                f'"a funny id/that might mess it up":{BACKEND_ENDPOINT}',
            ],
            rejected=[
                # Neither the category nor the identity is listed, and a direct proxy has no adapter id.
                f'"a cat with spaces/helloB":{BACKEND_ENDPOINT}',
                f"nocat/helloC:{BACKEND_ENDPOINT}",
                f"cat/helloD:{BACKEND_ENDPOINT}",
                f"baz/myident:{BACKEND_ENDPOINT}",
                f"cata/foo:{BACKEND_ENDPOINT}",
                f"cat/fooa:{BACKEND_ENDPOINT}",
                # None of the category, identity, and adapter id is listed.
                "hiscatA @ jimbo",
            ],
        ),
        FilterTest(
            # Same as above, with a "*" reject rule that rejects every direct proxy. Indirect proxies have no
            # endpoints for the address rules to match, so only the other filters apply to them.
            "mixing filters (indirect only)",
            filters={
                "Address.Reject": "*",
                "Category.Accept": 'mycat "a sec cat"',
                "Identity.Accept": 'myident cata/fooa "a funny id/that might mess it up"',
                "AdapterId.Accept": "myadapter",
            },
            accepted=[
                # The category is listed.
                "mycat/foo @ jimbo",
                # The adapter id is listed.
                "hiscat @ myadapter",
                # The identity and the adapter id are listed.
                '"a funny id/that might mess it up" @ myadapter',
            ],
            rejected=[
                # Direct proxies, rejected by the "*" reject rule.
                f'"a cat with spaces/helloB":{BACKEND_ENDPOINT}',
                f"nocat/helloC:{BACKEND_ENDPOINT}",
                f"cat/helloD:{BACKEND_ENDPOINT}",
                f"baz/myident:{BACKEND_ENDPOINT}",
                f"cata/foo:{BACKEND_ENDPOINT}",
                f"cat/fooa:{BACKEND_ENDPOINT}",
                # Direct proxies, rejected by the "*" reject rule even though their category is listed.
                f"mycat/fooa:{BACKEND_ENDPOINT}",
                f'"a sec cat/fooa":{BACKEND_ENDPOINT}',
                # Indirect, but none of the category, identity, and adapter id is listed.
                "hiscatA @ jimbo",
                # Direct proxies, rejected by the "*" reject rule even though their identity is listed.
                f"myident:{BACKEND_ENDPOINT}",
                f"cata/fooa:{BACKEND_ENDPOINT}",
                f'"a funny id/that might mess it up":{BACKEND_ENDPOINT}',
            ],
        ),
        FilterTest(
            "address filter with numeric range in hostname",
            filters={"Address.Accept": "12[7].0.0.*"},
            accepted=[
                # The 7 after the 12 prefix is in the range [7].
                f"hello:{BACKEND_ENDPOINT}",
            ],
            rejected=[
                # The 8 and the 6 after the 12 prefix are not.
                f"hello:{tcp('128.0.0.1')}",
                f"hello:{tcp('126.0.0.1')}",
            ],
        ),
        FilterTest(
            # A rule must match the host in full, so 12[7] does not match 127.0.0.1.
            "address filter without a trailing wildcard matches the whole host",
            filters={"Address.Accept": "12[7]"},
            rejected=[f"hello:{BACKEND_ENDPOINT}"],
        ),
        FilterTest(
            # A rule without any wildcard is not a suffix match, so the reject rule 7.0.0.1 does not reject
            # 127.0.0.1.
            "address filter without a wildcard is not a suffix match",
            filters={"Address.Reject": "7.0.0.1"},
            accepted=[f"hello:{BACKEND_ENDPOINT}"],
        ),
        FilterTest(
            # A wildcard followed by a trailing numeric range matches when any alignment of the wildcard reaches the
            # end of the host: for 127.0.0.1, the first two dots leave part of the host unmatched and only the last
            # dot is the correct alignment. The range must match right after a dot, so 127.0.0.x does not match.
            "address filter with a wildcard followed by a trailing numeric range",
            filters={"Address.Accept": "*.[0-255]"},
            accepted=[f"hello:{BACKEND_ENDPOINT}"],
            rejected=[f"hello:{tcp('127.0.0.x')}"],
        ),
        FilterTest(
            # Same with the numeric range directly after the wildcard: only the final 1 of 127.0.0.1 ends at the end
            # of the host and lies in the 0-9 range.
            "address filter with a wildcard followed by a single-digit numeric range",
            filters={"Address.Accept": "*[0-9]"},
            accepted=[f"hello:{BACKEND_ENDPOINT}"],
        ),
        FilterTest(
            # A rule ending with a numeric range followed by a wildcard is a prefix match: 12[0-255]* matches
            # 127.0.0.1, with the range matching the number right after the 12 prefix.
            "address filter with a numeric range followed by a trailing wildcard",
            filters={"Address.Accept": "12[0-255]*"},
            accepted=[f"hello:{BACKEND_ENDPOINT}"],
            rejected=[
                # There is no number after the 12 prefix for the range to match.
                f"hello:{tcp('12x.0.0.1')}",
            ],
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
            accepted=[
                # Matches the second rule, *7.*.1.
                f"hello:{BACKEND_ENDPOINT}",
            ],
            rejected=[
                # A host of only 'a' characters has no 'b' for the first rule to end on, and no digits for the second.
                f"hello:{tcp('a' * 255)}",
            ],
        ),
        FilterTest(
            # Host names are case-insensitive: an accept rule matches the host regardless of the case of either the
            # rule or the host.
            "address filter accept rule is case-insensitive",
            filters={"Address.Accept": "LocalHost"},
            accepted=[f"hello:{tcp('localhost')}", f"hello:{tcp('LOCALHOST')}"],
            rejected=[
                # The rules match the host as written, without resolving it, so 127.0.0.1 is not localhost.
                f"hello:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            # A reject rule matches a proxy as soon as one of its endpoints matches: extra endpoints that do not
            # match the rule do not get the proxy accepted.
            "address filter reject rule against a multi-endpoint proxy",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[
                # The rules match the host as written, without resolving it, so localhost is not 127.0.0.1.
                f"hello:{tcp('localhost')}",
            ],
            rejected=[
                # The 127.0.0.1 endpoint matches, whether it comes first or last.
                f"hello:{BACKEND_ENDPOINT}:{tcp('127.0.0.2')}",
                f"hello:{tcp('127.0.0.2')}:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            # Same for a reject rule: a case variation of the host still matches the rule.
            "address filter reject rule is case-insensitive",
            filters={"Address.Reject": "badhost.example.com"},
            accepted=[
                # The host does not match the reject rule.
                f"hello:{tcp('localhost')}",
            ],
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
                # The host is listed.
                f"hello:{tcp('localhost')}",
                f"hello:{BACKEND_ENDPOINT}",
                # The trailing dot is stripped, so the host matches localhost.
                f"hello:{tcp('localhost.')}",
            ],
            rejected=[
                # Leading zeros: a non-canonical spelling of 127.0.0.1.
                f"hello:{tcp('127.000.000.001')}",
                # A rule must match the host in full, so localhost does not match localhost.example.com.
                f"hello:{tcp('localhost.example.com')}",
            ],
        ),
        FilterTest(
            "address filter reject rule against a trailing-dot host",
            filters={"Address.Reject": "badhost.example.com *.internal.example.com"},
            accepted=[
                # The host matches no reject rule.
                f"hello:{BACKEND_ENDPOINT}",
            ],
            rejected=[
                # The trailing dot is stripped, so the host matches badhost.example.com.
                f"hello:{tcp('badhost.example.com.')}",
                # The trailing dot is stripped, so the host matches *.internal.example.com.
                f"hello:{tcp('db.internal.example.com.')}",
            ],
        ),
        FilterTest(
            # A rule written with a trailing dot is stripped like the host, so both spellings of the rule match both
            # spellings of the host.
            "address filter reject rule written with a trailing dot",
            filters={"Address.Reject": "badhost.example.com."},
            accepted=[
                # The host does not match the reject rule.
                f"hello:{BACKEND_ENDPOINT}",
            ],
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
            accepted=[
                # Not an IPv4 literal, and the rules do not resolve it, so it does not match 127.0.0.1.
                f"hello:{tcp('localhost')}",
            ],
            rejected=[
                # Leading zeros.
                f"hello:{tcp('127.000.000.001')}",
                # A single hexadecimal number.
                f"hello:{tcp('0x7f000001')}",
                # A single decimal number.
                f"hello:{tcp('2130706433')}",
                # Fewer than four parts.
                f"hello:{tcp('127.1')}",
                # An octal part.
                f"hello:{tcp('0177.0.0.1')}",
                # A hexadecimal part with no digits.
                f"hello:{tcp('0x.0.0.1')}",
                # A trailing dot, which the resolver reads as a DNS name rather than an address.
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
            accepted=[
                # The host does not match the reject rule.
                f"hello:{BACKEND_ENDPOINT}",
            ],
            rejected=[f"hello:{tcp('a' * 300)}"],
        ),
        FilterTest(
            # An endpoint with no host is rejected outright: Ice resolves it to the loopback address, yet it matches
            # no address rule, so without the rejection a reject rule for loopback would not catch a proxy that
            # connects to loopback. The same accepted IP endpoint flips to rejected once the host-less endpoint is
            # added alongside it, so the host-less endpoint cannot ride in behind a legitimate one.
            "address filter rejects an endpoint with no host",
            filters={"Address.Reject": "127.0.0.1"},
            accepted=[
                # The host does not match the reject rule.
                f"hello:{tcp('localhost')}",
            ],
            rejected=[
                # The only endpoint has no host.
                "hello:tcp -p 12010",
                # The second endpoint has no host.
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
            accepted=[
                # The host does not match the reject rule.
                f"hello:{tcp('localhost')}",
            ],
            rejected=[
                # The second endpoint is not an IP endpoint.
                f"hello:{tcp('localhost')}:opaque -t 99 -v ABCD",
            ],
        ),
    ]


def hostNameTests(host: HostNames) -> list[FilterTest]:
    """Returns the tests that address this host by its name, which require a configured host and domain name."""
    hostname, fqdn, domainname = host
    return [
        FilterTest(
            "reject all",
            filters={"Address.Reject": "*"},
            accepted=[
                # An indirect proxy has no endpoints for the address rules to match.
                "bar @ foo",
            ],
            rejected=[
                # "*" matches every host.
                f"helloA:{tcp(fqdn)}",
                f"helloB:{tcp(hostname)}",
                f"helloC:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            "loopback only rule",
            filters={"Address.Accept": "127.0.0.1 localhost"},
            accepted=[
                # The host is listed; the identity plays no part in address rules.
                f"localhost:{BACKEND_ENDPOINT}",
                f"localhost/127.0.0.1:{tcp('localhost')}",
                f"hello:{BACKEND_ENDPOINT}",
                f"hello/somecat:{tcp('localhost')}",
            ],
            rejected=[
                # The host is not listed.
                f"hello:{tcp(fqdn)}",
                f"hello:{tcp(hostname)}",
                # The host is not listed; an identity spelled like a listed host does not help.
                f"127.0.0.1:{tcp(hostname)}",
                f"localhost:{tcp(hostname)}",
                f"localhost/127.0.0.1:{tcp(hostname)}",
            ],
        ),
        FilterTest(
            "port filter rule",
            filters={"Address.Accept": "127.0.0.1:12010 localhost:12010"},
            accepted=[
                # The host and the port are listed.
                f"hello3:{BACKEND_ENDPOINT}",
                f"hello4:{tcp('localhost')}",
            ],
            rejected=[
                # The host is listed, but not the port.
                f"hello1:{tcp('127.0.0.1', 12011)}",
                f"hello2:{tcp('localhost', 12011)}",
                # The port is listed, but not the host.
                f"hello5:{tcp(hostname)}",
            ],
        ),
        FilterTest(
            "reject port filter rule",
            filters={"Address.Reject": "127.0.0.1:[0-12009,12011-65535] localhost:[0-12009,12011-65535]"},
            accepted=[
                # The host matches no reject rule.
                f"hello5:{tcp(hostname)}",
                # The port is outside the rejected ranges.
                f"hello3:{BACKEND_ENDPOINT}",
                f"hello4:{tcp('localhost')}",
            ],
            rejected=[
                # The host matches and the port is in a rejected range.
                f"hello1:{tcp('127.0.0.1', 12011)}",
                f"hello2:{tcp('localhost', 12011)}",
            ],
        ),
        FilterTest(
            "port filter rule with wildcard address rule",
            filters={"Address.Accept": "*:12010"},
            accepted=[
                # Any host, on the listed port.
                f"hello5:{tcp(hostname)}",
                f"hello3:{BACKEND_ENDPOINT}",
                f"hello4:{tcp('localhost')}",
            ],
            rejected=[
                # The port is not listed.
                f"hello1:{tcp('127.0.0.1', 12011)}",
                f"hello2:{tcp('localhost', 12011)}",
            ],
        ),
        FilterTest(
            "domain filter rule (accept)",
            filters={"Address.Accept": f"*{domainname}"},
            accepted=[
                # The fully qualified name ends with the domain.
                f"hello:{tcp(fqdn)}",
            ],
            rejected=[
                # The short host name does not.
                f"hello:{tcp(hostname)}",
            ],
        ),
        FilterTest(
            "domain filter rule (reject)",
            filters={"Address.Reject": f"*{domainname}"},
            accepted=[
                # The host does not end with the domain.
                f"hello:{tcp(hostname)}",
                f"bar:{BACKEND_ENDPOINT}",
            ],
            rejected=[
                # The fully qualified name ends with the domain.
                f"hello:{tcp(fqdn)}",
            ],
        ),
        FilterTest(
            "domain filter rule (mixed)",
            filters={"Address.Accept": "127.0.0.1", "Address.Reject": fqdn},
            accepted=[
                # The host matches the accept rule and not the reject rule.
                f"bar:{BACKEND_ENDPOINT}",
            ],
            rejected=[
                # The fully qualified name endpoint matches the reject rule. It also fails the accept rule, which
                # every endpoint must match.
                f"hello:{tcp(fqdn)}:{BACKEND_ENDPOINT}",
            ],
        ),
        FilterTest(
            "maximum proxy length rule",
            filters={"ProxySizeMax": "53"},
            accepted=[
                # The stringified proxy is within the limit.
                f"hello:{BACKEND_ENDPOINT} -t infinite",
            ],
            rejected=[
                # The 60-character identity alone exceeds the limit.
                f"012345678901234567890123456789012345678901234567890123456789:{BACKEND_ENDPOINT}",
            ],
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
