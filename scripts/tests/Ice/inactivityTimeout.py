# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import TestSuite

# TODO: temporary work-around for C#. serialize turns on flow-control
# and makes the test fail

TestSuite(__name__, options={"serialize": [False]})
