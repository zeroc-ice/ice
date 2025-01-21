# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

from Util import TestSuite


TestSuite(__name__, options={"ipv6": [False], "compress": [False]}, multihost=False)
