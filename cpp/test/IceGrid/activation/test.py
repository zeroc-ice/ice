# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#
outfilters = [re.compile("warning: server activation failed"),
              re.compile("cannot execute"),
              re.compile("cannot change working directory")]

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [IceGridTestCase(icegridnode=IceGridNode(outfilters=outfilters))], multihost=False)
