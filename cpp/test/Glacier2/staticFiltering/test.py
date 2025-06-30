# Copyright (c) ZeroC, Inc.

import os
import socket
import platform
import subprocess
import psutil
from Glacier2Util import Glacier2Router, Glacier2TestSuite

from Util import Client, ClientServerTestCase, Server



def print_network_debug_info():
    print("=== Host Information ===")
    hostname = socket.gethostname()
    fqdn = socket.getfqdn()
    print(f"Hostname: {hostname}")
    print(f"FQDN: {fqdn}")

    try:
        host_ip = socket.gethostbyname(hostname)
        fqdn_ip = socket.gethostbyname(fqdn)
        print(f"Resolved IP (hostname): {host_ip}")
        print(f"Resolved IP (FQDN): {fqdn_ip}")
    except socket.gaierror as e:
        print(f"DNS resolution error: {e}")

    print("\n=== Network Interfaces ===")
    for iface, addrs in psutil.net_if_addrs().items():
        print(f"\nInterface: {iface}")
        for addr in addrs:
            if addr.family == socket.AF_INET:
                print(f"  IPv4 Address: {addr.address}")
            elif addr.family == socket.AF_INET6:
                print(f"  IPv6 Address: {addr.address}")
            elif addr.family == psutil.AF_LINK:
                print(f"  MAC Address: {addr.address}")

    print("\n=== Default Route ===")
    if platform.system() == "Windows":
        subprocess.run(["route", "print"], shell=True)
    else:
        subprocess.run(["ip", "route"], check=False)

    print("\n=== Test Connections ===")
    for target in [hostname, fqdn, "127.0.0.1", "::1"]:
        try:
            print(f"Testing connection to {target}: ", end="")
            sock = socket.create_connection((target, 80), timeout=2)
            sock.close()
            print("OK")
        except Exception as e:
            print(f"Failed ({e})")

class Glacier2StaticFilteringTestCase(ClientServerTestCase):
    def __init__(self, testcase, hostname):
        self.hostname = hostname
        description, self.tcArgs, self.attacks, self.xtraConfig = testcase

        clientProps = {"Ice.Config": "{testdir}/client.cfg", "Ice.Warn.Connections": 0}
        clientProps["Ice.Trace.Network"] = "3"
        clientProps["Ice.Trace.Protocol"] = "1"
        serverProps = {"Ice.Config": "{testdir}/server.cfg", "Ice.Warn.Connections": 0}
        routerProps = {
            "Ice.Config": "{testdir}/router.cfg",
            "Glacier2.RoutingTable.MaxSize": 10,
        }

        # Override the server/router default host property, we don't want to use the loopback
        serverProps["Ice.Default.Host"] = ""
        serverProps["Ice.Trace.Network"] = "3"
        serverProps["Ice.Trace.Protocol"] = "1"

        routerProps["Ice.Default.Host"] = ""
        routerProps["Ice.Trace.Network"] = "3"
        routerProps["Ice.Trace.Protocol"] = "1"

        ClientServerTestCase.__init__(
            self,
            description,
            desc=description,
            servers=[Glacier2Router(props=routerProps), Server(props=serverProps)],
            client=Client(props=clientProps),
        )

    def setupClientSide(self, current):
        current.write("testing {0}... ".format(self))

    def setupServerSide(self, current):
        (
            acceptFilter,
            rejectFilter,
            maxEndpoints,
            categoryFilter,
            idFilter,
            adapterFilter,
        ) = self.tcArgs

        #
        # The test client performs multiple tests during one 'run'. We could
        # use command line arguments to pass the test cases in, but a
        # configuration file is easier.
        #
        with open(
            os.path.join(self.getTestSuite().getPath(), "client.cfg"), "w"
        ) as clientConfig:
            accepts = 0
            rejects = 0
            for expect, proxy in self.attacks:
                if expect:
                    clientConfig.write("Accept.Proxy." + str(accepts) + "=")
                    print("Accept.Proxy." + str(accepts) + "=" + proxy)
                    accepts += 1
                else:
                    clientConfig.write("Reject.Proxy." + str(rejects) + "=")
                    print("Reject.Proxy." + str(accepts) + "=" + proxy)
                    rejects += 1
                clientConfig.write(proxy + "\n")

        with open(
            os.path.join(self.getTestSuite().getPath(), "server.cfg"), "w"
        ) as serverConfig:
            if current.config.protocol != "ssl":
                serverConfig.write("BackendAdapter.Endpoints=tcp -p 12010\n")

        with open(
            os.path.join(self.getTestSuite().getPath(), "router.cfg"), "w"
        ) as routerConfig:
            routerConfig.write(
                "Ice.Default.Locator=locator:tcp -h %s -p 12010\n" % self.hostname
            )
            routerConfig.write("Glacier2.Client.Trace.Reject=0\n")
            routerConfig.write("#\n")

            for line in self.xtraConfig:
                routerConfig.write(line)
                routerConfig.write("\n")

            #
            # We configure the AddProxy constraints as a configuration file.
            # Regular expression syntax can easily confuse the command line.
            #
            if not len(acceptFilter) == 0:
                routerConfig.write("Glacier2.Filter.Address.Accept=%s\n" % acceptFilter)
            if not len(rejectFilter) == 0:
                routerConfig.write("Glacier2.Filter.Address.Reject=%s\n" % rejectFilter)
            if not len(maxEndpoints) == 0:
                routerConfig.write("Glacier2.Filter.ProxySizeMax=%s\n" % maxEndpoints)
            if not len(categoryFilter) == 0:
                routerConfig.write(
                    "Glacier2.Filter.Category.Accept=%s\n" % categoryFilter
                )
            if not len(idFilter) == 0:
                routerConfig.write("Glacier2.Filter.Identity.Accept=%s\n" % idFilter)
            if not len(adapterFilter) == 0:
                routerConfig.write(
                    "Glacier2.Filter.AdapterId.Accept=%s\n" % adapterFilter
                )

    def teardownServerSide(self, current, success):
        for c in ["client.cfg", "router.cfg", "server.cfg"]:
            path = os.path.join(self.getTestSuite().getPath(), c)
            print(f"Removing file: {path}")
            try:
                with open(path, "r", encoding="utf-8") as f:
                    print(f"Contents of {c}:\n{f.read()}")
            except Exception as e:
                print(f"Failed to read {path}: {e}")
            try:
                os.remove(path)
            except Exception as e:
                print(f"Failed to remove {path}: {e}")



class Glacier2StaticFilteringTestSuite(Glacier2TestSuite):
    def setup(self, current):
        Glacier2TestSuite.setup(self, current)

        import socket

        print_network_debug_info()

        hostname = socket.gethostname().lower()
        fqdn = socket.getfqdn().lower()

        limitedTests = False

        #
        # Try and figure out what tests are reasonable with this host's
        # configuration.
        #
        if (
            fqdn.endswith("localdomain")
            or fqdn.endswith("local")
            or fqdn.endswith("domain")
        ):
            #
            # No real configured domain name, this means that anything that
            # requires a domain name isn't likely going to work. Furthermore, it
            # might be the case that the hostname contains this suffix, so we
            # should just toss it and pretend that there is no 'hostname'
            # configured for this box.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        elif hostname.startswith("localhost"):
            #
            # No configured host name (and possibly no domain name), minimal
            # tests.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        elif fqdn.find(".") == -1:
            #
            # No real configured domain.
            #
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""
            limitedTests = True
        else:
            dot = fqdn.find(".")
            domainname = fqdn[dot + 1 :]
            #
            # Some Python installs are going to return a FQDN for gethostname().
            # This invalidates the tests that need to differentiate between the
            # hostname and the FQDN. If these tests fail because of a DNS error,
            # it may be that an alias is not present for the host partition of
            # the FQDN.
            #
            if fqdn == hostname:
                hostname = hostname[:dot]
            if domainname == "":
                limitedTests = True
        try:
            testaddr1 = socket.gethostbyname(fqdn)
            testaddr2 = socket.gethostbyname(hostname)

            # On SUSE distributions, 127.0.0.2 is sometimes used in /etc/hosts
            # for the hostname (apparently if no network interface was found
            # when the OS was installed). However, connecting to this IP address
            # doesn't work (even if it can be pinged).
            if testaddr1 == "127.0.0.2" or testaddr2 == "127.0.0.2":
                limitedTests = True
                hostname = "127.0.0.1"
                fqdn = ""
                domainname = ""
        except socket.gaierror:
            limitedTests = True
            hostname = "127.0.0.1"
            fqdn = ""
            domainname = ""

        testcases = [
            (
                "testing category filter",
                ("", "", "", 'foo "a cat with spaces"', "", ""),
                [
                    (True, "foo/helloA:tcp -h 127.0.0.1 -p 12010"),
                    (True, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                    (False, "nocat/helloC:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/helloD:tcp -h 127.0.0.1 -p 12010"),
                ],
                [],
            ),
            (
                "testing adapter id filter",
                ("", "*", "", "", "", 'foo "an adapter with spaces"'),
                [
                    (False, "foo/helloA:tcp -h 127.0.0.1 -p 12010"),
                    (False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                    (False, "nocat/helloC:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/helloD:tcp -h 127.0.0.1 -p 12010"),
                    (False, "helloE @ bar"),
                    (True, 'helloF1 @ "an adapter with spaces"'),
                    (True, "helloF @ foo"),
                ],
                [],
            ),
            (
                "test identity filters",
                (
                    "",
                    "",
                    "",
                    "",
                    'myident cata/fooa "a funny id/that might mess it up"',
                    "",
                ),
                [
                    (False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                    (False, "nocat/helloC:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/helloD:tcp -h 127.0.0.1 -p 12010"),
                    (False, "baz/myident:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cata/foo:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (True, "myident:tcp -h 127.0.0.1 -p 12010"),
                    (True, "cata/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (
                        True,
                        '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010',
                    ),
                ],
                [],
            ),
            (
                "test mixing filters",
                (
                    "",
                    "",
                    "",
                    'mycat "a sec cat"',
                    'myident cata/fooa "a funny id/that might mess it up" "a\\"nother"',
                    "myadapter",
                ),
                [
                    (False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                    (False, "nocat/helloC:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/helloD:tcp -h 127.0.0.1 -p 12010"),
                    (False, "baz/myident:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cata/foo:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (True, "mycat/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (True, '"a sec cat/fooa":tcp -h 127.0.0.1 -p 12010'),
                    (True, "mycat/foo @ jimbo"),
                    (False, "hiscatA @ jimbo"),
                    (True, "hiscat @ myadapter"),
                    (True, 'a"nother @ jimbo'),
                    (True, "myident:tcp -h 127.0.0.1 -p 12010"),
                    (True, "cata/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (
                        True,
                        '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010',
                    ),
                ],
                [],
            ),
            (
                "test mixing filters (indirect only)",
                (
                    "",
                    "*",
                    "",
                    'mycat "a sec cat"',
                    'myident cata/fooa "a funny id/that might mess it up"',
                    "myadapter",
                ),
                [
                    (False, '"a cat with spaces/helloB":tcp -h 127.0.0.1 -p 12010'),
                    (False, "nocat/helloC:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/helloD:tcp -h 127.0.0.1 -p 12010"),
                    (False, "baz/myident:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cata/foo:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cat/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (False, "mycat/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (False, '"a sec cat/fooa":tcp -h 127.0.0.1 -p 12010'),
                    (True, "mycat/foo @ jimbo"),
                    (False, "hiscatA @ jimbo"),
                    (True, "hiscat @ myadapter"),
                    (False, "myident:tcp -h 127.0.0.1 -p 12010"),
                    (False, "cata/fooa:tcp -h 127.0.0.1 -p 12010"),
                    (True, '"a funny id/that might mess it up" @ myadapter'),
                    (
                        False,
                        '"a funny id/that might mess it up":tcp -h 127.0.0.1 -p 12010',
                    ),
                ],
                [],
            ),
        ]

        if not limitedTests:
            testcases.extend(
                [
                    (
                        "testing reject all",
                        ("", "*", "", "", "", ""),
                        [
                            (False, "helloA:tcp -h %s -p 12010" % fqdn),
                            (False, "helloB:tcp -h %s -p 12010" % hostname),
                            (False, "helloC:tcp -h 127.0.0.1 -p 12010"),
                            (True, "bar @ foo"),
                        ],
                        [],
                    ),
                    (
                        "testing loopback only rule",
                        ("127.0.0.1 localhost", "", "", "", "", ""),
                        [
                            (False, "hello:tcp -h %s -p 12010" % fqdn),
                            (False, "hello:tcp -h %s -p 12010" % hostname),
                            (False, "127.0.0.1:tcp -h %s -p 12010" % hostname),
                            (False, "localhost:tcp -h %s -p 12010" % hostname),
                            (
                                False,
                                "localhost/127.0.0.1:tcp -h %s -p 12010" % hostname,
                            ),
                            (True, "localhost:tcp -h 127.0.0.1 -p 12010"),
                            (True, "localhost/127.0.0.1:tcp -h localhost -p 12010"),
                            (True, "hello:tcp -h 127.0.0.1 -p 12010"),
                            (True, "hello/somecat:tcp -h localhost -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing port filter rule",
                        ("127.0.0.1:12010 localhost:12010", "", "", "", "", ""),
                        [
                            (False, "hello1:tcp -h 127.0.0.1 -p 12011"),
                            (False, "hello2:tcp -h localhost -p 12011"),
                            (False, "hello5:tcp -h %s -p 12010" % hostname),
                            (True, "hello3:tcp -h 127.0.0.1 -p 12010"),
                            (True, "hello4:tcp -h localhost -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing reject port filter rule",
                        (
                            "",
                            "127.0.0.1:[0-12009,12011-65535] localhost:[0-12009,12011-65535]",
                            "",
                            "",
                            "",
                            "",
                        ),
                        [
                            (False, "hello1:tcp -h 127.0.0.1 -p 12011"),
                            (False, "hello2:tcp -h localhost -p 12011"),
                            (True, "hello5:tcp -h %s -p 12010" % hostname),
                            (True, "hello3:tcp -h 127.0.0.1 -p 12010"),
                            (True, "hello4:tcp -h localhost -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing port filter rule with wildcard address rule",
                        ("*:12010", "", "", "", "", ""),
                        [
                            (False, "hello1:tcp -h 127.0.0.1 -p 12011"),
                            (False, "hello2:tcp -h localhost -p 12011"),
                            (True, "hello5:tcp -h %s -p 12010" % hostname),
                            (True, "hello3:tcp -h 127.0.0.1 -p 12010"),
                            (True, "hello4:tcp -h localhost -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing domain filter rule (accept)",
                        ("*" + domainname, "", "", "", "", ""),
                        [
                            (True, "hello:tcp -h %s -p 12010" % fqdn),
                            (False, "hello:tcp -h %s -p 12010" % hostname),
                        ],
                        [],
                    ),
                    (
                        "testing domain filter rule (reject)",
                        ("", "*" + domainname, "", "", "", ""),
                        [
                            (False, "hello:tcp -h %s -p 12010" % fqdn),
                            (True, "hello:tcp -h %s -p 12010" % hostname),
                            (True, "bar:tcp -h 127.0.0.1 -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing domain filter rule (mixed)",
                        ("127.0.0.1", fqdn, "", "", "", ""),
                        [
                            (
                                False,
                                "hello:tcp -h %s -p 12010:tcp -h 127.0.0.1 -p 12010"
                                % fqdn,
                            ),
                            (True, "bar:tcp -h 127.0.0.1 -p 12010"),
                        ],
                        [],
                    ),
                    (
                        "testing maximum proxy length rule",
                        ("", "", "53", "", "", ""),
                        [
                            (True, "hello:tcp -h 127.0.0.1 -p 12010 -t infinite"),
                            (
                                False,
                                "012345678901234567890123456789012345678901234567890123456789:tcp -h 127.0.0.1 -p 12010",
                            ),
                        ],
                        [],
                    ),
                ]
            )

        if len(testcases) == 0:
            current.writeln(
                "WARNING: You are running this test with SSL disabled and the network "
            )
            current.writeln(
                "         configuration for this host does not permit the other tests "
            )
            current.writeln("         to run correctly.")
        elif len(testcases) < 6:
            current.writeln(
                "WARNING: The network configuration for this host does not permit all "
            )
            current.writeln(
                "         tests to run correctly, some tests have been disabled."
            )

        self.testcases = {}
        for testcase in testcases:
            self.addTestCase(Glacier2StaticFilteringTestCase(testcase, hostname))


Glacier2StaticFilteringTestSuite(
    __name__,
    testcases=[],
    runOnMainThread=True,
    options={"ipv6": [False]},
    multihost=False,
)
