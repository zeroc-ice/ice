# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import TestSuite

# This test relies on send blocking for large payloads so don't run it with payload compression
TestSuite(__name__, options={"compress": [False]})
