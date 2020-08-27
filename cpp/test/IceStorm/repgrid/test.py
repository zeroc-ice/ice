# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# Use the IceGridServer for the client because the client is an IceStorm subscriber and needs to be able
# to accept connections (this is important for picking the correct server certificate for TLS).
if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [ IceGridTestCase(icegridregistry=IceGridRegistryMaster(),
                                          client=IceGridServer()) ],
              runOnMainThread=True, multihost=False)
