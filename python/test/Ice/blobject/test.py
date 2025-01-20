# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

# This test doesn't support running with IceSSL, the Router object in the client process uses
# the client certificate and fails with "unsupported certificate purpose"

from Util import TestSuite


TestSuite(__name__, options={"protocol": ["tcp"]})
