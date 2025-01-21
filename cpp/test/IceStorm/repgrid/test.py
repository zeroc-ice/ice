# Copyright (c) ZeroC, Inc.

# Use the IceGridServer for the client because the client is an IceStorm subscriber and needs to be able
# to accept connections (this is important for picking the correct server certificate for TLS).
import os
from IceGridUtil import IceGridRegistryMaster, IceGridServer, IceGridTestCase
from Util import TestSuite, Windows, platform


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [
            IceGridTestCase(
                icegridregistry=IceGridRegistryMaster(), client=IceGridServer()
            )
        ],
        runOnMainThread=True,
        multihost=False,
    )
