# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os

from IceGridUtil import (
    IceGridRegistryMaster,
    IceGridRegistrySlave,
    IceGridTestCase,
    IceGridClient,
    IceGridServer,
)
from Util import ClientServerTestCase, Linux, TestSuite, Windows, platform


serverProps = {
    "TestAdapter.Endpoints": "default",
    "TestAdapter.AdapterId": "TestAdapter",
}

serverTraceProps = {"Ice.Trace.Network": 1}

registryProps = {"IceGrid.Registry.DynamicRegistration": 1}
registryTraceProps = {"IceGrid.Registry.Trace.Discovery": 2, "Ice.Trace.Network": 1}


def clientProps(process, current):
    return {
        "IceLocatorDiscovery.Timeout": 50,
        "IceLocatorDiscovery.RetryCount": 5,
        "IceLocatorDiscovery.Interface": ""
        if isinstance(platform, Linux)
        else "::1"
        if current.config.ipv6
        else "127.0.0.1",
        "IceLocatorDiscovery.Port": current.driver.getTestPort(99),
    }


clientTraceProps = {"IceLocatorDiscovery.Trace.Lookup": 3, "Ice.Trace.Network": 1}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __name__,
        [
            IceGridTestCase(
                "without deployment",
                application=None,
                icegridregistry=[
                    IceGridRegistryMaster(
                        props=registryProps, traceProps=registryTraceProps
                    ),
                    IceGridRegistrySlave(
                        1, props=registryProps, traceProps=registryTraceProps
                    ),
                    IceGridRegistrySlave(
                        2, props=registryProps, traceProps=registryTraceProps
                    ),
                ],
                client=ClientServerTestCase(
                    client=IceGridClient(
                        props=clientProps,
                        traceProps=clientTraceProps,
                    ),
                    server=IceGridServer(props=serverProps, traceProps=serverTraceProps),
                ),
            ),
            IceGridTestCase(
                "with deployment", client=IceGridClient(args=["--with-deploy"])
            ),
        ],
        multihost=False,
    )
