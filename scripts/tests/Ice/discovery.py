# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc. All rights reserved.

import uuid

domainId = uuid.uuid4() # Ensures each test uses a unique domain ID

def props(process, current):
    port = current.driver.getTestPort(10)
    discoveryProps = {
        "Ice.ProgramName": "server{}".format(process.args[0]) if isinstance(process, Server) else "client", # This is used for the trace file
        "Ice.Default.Locator": "discovery",
        "Ice.Discovery.RetryCount": 20,
        "Ice.Discovery.DomainId": domainId,
        "Ice.Discovery.Timeout": "50ms",
        "Ice.Discovery.Multicast.Endpoints":
            f"udp -h 239.255.0.1 -p {port} --interface 127.0.0.1:udp -h \"ff15::1\" -p {port} --interface \"::1\"",
        "Ice.Discovery.Lookup":
            f"udp -h 239.255.0.1 -p {port} --interface 127.0.0.1:udp -h \"ff15::1\" -p {port} --interface \"::1\"",
        "Ice.Discovery.Reply.Endpoints": "udp -h 127.0.0.1 -p 0:udp -h \"::1\" -p 0",
    }
    return discoveryProps

traceProps = {
    "Ice.Trace.Locator" : 2,
    "Ice.Trace.Protocol" : 1
}

# Suppress the warning lines
suppressDiscoveryWarning=False
def suppressWarning(x):
    global suppressDiscoveryWarning
    if re.search("-! .* warning: .*failed to lookup adapter.*\n", x):
        suppressDiscoveryWarning = True
        return re.sub("-! .* warning: .*failed to lookup adapter.*\n", "", x)
    elif suppressDiscoveryWarning and re.search("^   .*\n", x):
        return ""
    else:
        suppressDiscoveryWarning = False
        return x

# Filter-out the warning about invalid lookup proxy
outfilters = [ lambda x: suppressWarning(x) ]

options = None

TestSuite(__name__, [
   ClientServerTestCase(client=Client(args=[3], props=props, outfilters=outfilters),
                        servers=[Server(args=[i], readyCount=4, props=props) for i in range(0, 3)],
                        traceProps=traceProps)
], multihost=False, options=options)
