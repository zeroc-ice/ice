# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

def serverProps(process, current):
    return {
        "TestAdapter.Endpoints" : f"{current.config.transport} -h 127.0.0.1",
        "TestAdapter.AdapterId" : "TestAdapter"
    }

registryProps = {
    "IceGrid.Registry.DynamicRegistration" : 1
}
registryTraceProps = {
    "IceGrid.Registry.Trace.Discovery": 2
}

def clientProps(process, current):
    port = current.driver.getTestPort(99)
    if isinstance(current.testcase.getMapping(), CSharpMapping):
        return {
            "Ice.LocatorDiscovery.Timeout": "50ms" if isinstance(current.testcase.getMapping(), CSharpMapping) else "50",
            "Ice.LocatorDiscovery.RetryCount": 5,
            "Ice.LocatorDiscovery.Lookup":
                f"udp -h 239.255.0.1 -p {port} --interface 127.0.0.1:udp -h \"ff15::1\" -p {port} --interface \"::1\"",
        }
    else:
        return {
            "IceLocatorDiscovery.Timeout": "50",
            "IceLocatorDiscovery.RetryCount": 5,
            "IceLocatorDiscovery.Interface": "" if isinstance(platform, Linux) else "::1" if current.config.ipv6 else "127.0.0.1",
            "IceLocatorDiscovery.Port": port,
        }

clientTraceProps = { "Ice.LocatorDiscovery.Trace.Lookup" : 3, "IceLocatorDiscovery.Trace.Lookup" : 3 }

# Filter-out the warning about invalid lookup proxy
outfilters = [ lambda x: re.sub("-! .* warning: .*failed to lookup locator.*\n", "", x),
               lambda x: re.sub("^   .*\n", "", x) ]

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__name__, [
        IceGridTestCase("without deployment", application=None,
                        icegridregistry=[IceGridRegistryMaster(props=registryProps, traceProps=registryTraceProps),
                                         IceGridRegistrySlave(1, props=registryProps, traceProps=registryTraceProps),
                                         IceGridRegistrySlave(2, props=registryProps, traceProps=registryTraceProps)],
                        client=ClientServerTestCase(client=IceGridClient(props=clientProps,
                                                                         outfilters=outfilters,
                                                                         traceProps=clientTraceProps),
                                                    server=IceGridServer(props=serverProps))),
        IceGridTestCase("with deployment", client=IceGridClient(args=["--with-deploy"]))
    ], multihost=False)
