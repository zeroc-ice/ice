# Copyright (c) ZeroC, Inc.
import os
import re

from IceGridUtil import IceGridNode, IceGridTestCase
from Util import TestSuite, Windows, platform

outfilters = [
    re.compile("cannot find the file specified"),
    re.compile("warning: server activation failed"),
    re.compile("cannot execute"),
    re.compile("cannot change working directory"),
]

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [IceGridTestCase(icegridnode=IceGridNode(outfilters=outfilters))],
        multihost=False,
    )
